#pragma once
#include "net_All.h"
#include <cstdint>

template<typename T>
class connection : public enable_shared_from_this<connection<T>>
{
public:
	// A connection is "owned" by either a server or a client, and its
	// behaviour is slightly different bewteen the two.
	// This enum allows us to differentiate between the two
	enum class owner
	{
		server,
		client
	};

public:
	// Constructor: Specify Owner, connect to context, transfer the socket
	//				Provide reference to incoming message queue
	connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message<T>>& qIn)
		: connectionIoContext(asioContext), connectionSocket(move(socket)), incomingMessagesQueue(qIn)
	{
		nOwnerType = parent;
	}

	virtual ~connection()
	{}

	// This ID is used system wide - its how clients will understand other clients
	// exist across the whole system.
	uint32_t GetID() const
	{
		return id;
	}

public:
	void ConnectToClient(uint32_t uid = 0)
	{
		if (nOwnerType == owner::server)
		{
			if (connectionSocket.is_open())
			{
				id = uid;
				ReadHeader();
			}
		}
	}

	void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
	{
		// Only clients can connect to servers
		if (nOwnerType == owner::client)
		{
			// Request asio attempts to connect to an endpoint
			asio::async_connect(connectionSocket, endpoints,
				[this](error_code ec, asio::ip::tcp::endpoint endpoint)
				{
					if (!ec)
					{
						ReadHeader();
					}
					else
					{
						cerr << "[CLIENT] failed to connect to server at " << endpoint << "\n";
					}
				});
		}
	}


	void Disconnect()
	{
		if (IsConnected())
			asio::post(connectionIoContext, [this]() { connectionSocket.close(); });
	}

	bool IsConnected() const
	{
		return connectionSocket.is_open();
	}

	// This function is unimplemented as only a subset of the tutorial series was implemented
	void StartListening()
	{
		
	}

public:
	// ASYNC - Send a message, connections are one-to-one so no need to specifiy
	// the target, for a client, the target is the server and vice versa
	void Send(const message<T>& msg)
	{
    string messageContent;
    vector<uint8_t> messageBody = msg.body;
    for(int i = 0; i < messageBody.size(); i++)
      messageContent += messageBody[i];
    cout << "[CONNECTION OBJECT]: I recieved a message containing: " << messageContent << " \n";
		asio::post(connectionIoContext,
			[this, msg]()
			{
				// If the queue has a message in it, then we must 
				// assume that it is in the process of asynchronously being written.
				// Either way add the message to the queue to be output. If no messages
				// were available to be written, then start the process of writing the
				// message at the front of the queue.
				bool bWritingMessage = !outgoingMessagesQueue.empty();
				outgoingMessagesQueue.push_back(msg);
				if (!bWritingMessage)
				{
					WriteHeader();
				}
			});
	}



private:
	void WriteHeader()
	{
		asio::async_write(connectionSocket, asio::buffer(&outgoingMessagesQueue.front().header, sizeof(message_header<T>)),
			[this](error_code ec, size_t length)
			{
				if (!ec)
				{
					// check if message has a message body
					if (outgoingMessagesQueue.front().body.size() > 0)
					{
						// ...it does, so issue the task to write the body bytes
						WriteBody();
					}
					else
					{
						//If no body, then pop the message
						outgoingMessagesQueue.pop_front();

						// If the queue is not empty, there are more messages to send, so
						// make this happen by issuing the task to send the next header.
						if (!outgoingMessagesQueue.empty())
						{
							WriteHeader();
						}
					}
				}
				else
				{
					cerr << "[CLIENT: " << id << "] Write Header Fail.\n";
					connectionSocket.close();
				}
			});
	}

	void WriteBody()
	{
		asio::async_write(connectionSocket, asio::buffer(outgoingMessagesQueue.front().body.data(), outgoingMessagesQueue.front().body.size()),
			[this](error_code ec, size_t length)
			{
				if (!ec)
				{
					// Sending was successful, so we are done with the message
					// and remove it from the queue
					outgoingMessagesQueue.pop_front();

					// If the queue still has messages in it, then issue the task to 
					// send the next messages' header.
					if (!outgoingMessagesQueue.empty())
					{
						WriteHeader();
					}
				}
				else
				{
					cerr << "[" << id << "] Write Body Fail.\n";
					connectionSocket.close();
				}
			});
	}

	void ReadHeader()
	{
		asio::async_read(connectionSocket, asio::buffer(&temporaryMessage.header, sizeof(message_header<T>)),
			[this](error_code ec, size_t length)
			{						
				if (!ec)
				{
					// A complete message header has been read, check if this message
					// has a body to follow...
					if (temporaryMessage.header.size > 0)
					{
						// ...it does, so allocate enough space in the messages' body
						// vector, and issue asio with the task to read the body.
						temporaryMessage.body.resize(temporaryMessage.header.size);
						ReadBody();
					}
					else
					{
						//Then the message had no body
						AddToIncomingMessageQueue();
					}
				}
				else
				{
					// Most likely client disconnected
					cout << "[" << id << "] Read Header Fail.\n";
					connectionSocket.close();
				}
			});
	}

	void ReadBody()
	{
		asio::async_read(connectionSocket, asio::buffer(temporaryMessage.body.data(), temporaryMessage.body.size()),
			[this](error_code ec, size_t length)
			{						
				if (!ec)
				{
					AddToIncomingMessageQueue();
				}
				else
				{
					cout << "[" << id << "] Read Body Fail.\n";
					connectionSocket.close();
				}
			});
	}

	// Once a full message is received, add it to the incoming queue
	void AddToIncomingMessageQueue()
	{				
		// Shove it in queue, converting it to an "owned message", by initialising
		// with the a shared pointer from this connection object
		if(nOwnerType == owner::server)
			incomingMessagesQueue.push_back({ this->shared_from_this(), temporaryMessage });
		else
			incomingMessagesQueue.push_back({ nullptr, temporaryMessage });

		//We must prime the ASIO context to Read more headers
		//If we dont do this, then the thread would terminate...which is bad news if you want to send more data
		ReadHeader();
	}

protected:
	// Each connection has a unique socket to a remote (another connection object to a server or client)
	asio::ip::tcp::socket connectionSocket;

	// This context is shared with the whole asio instance
	asio::io_context& connectionIoContext;

	// This queue holds all messages to be sent to the remote side
	// of this connection
	//Everything must be in a queue due to asynchronous nature of the networking operations
	//We can not guarantee how long everything is going to take
	tsqueue<message<T>> outgoingMessagesQueue;

	// This references the incoming queue of the parent object
	tsqueue<owned_message<T>>& incomingMessagesQueue;

	// Incoming messages are constructed asynchronously, so we will
	// store the part assembled message here, until it is ready
	message<T> temporaryMessage;

	// The "owner" decides how some of the connection behaves
	owner nOwnerType = owner::server;

	uint32_t id = 0;

};

