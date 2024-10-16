#pragma once

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_connection.h"
#include <fstream>

using namespace std;

template<typename T>
class server_interface
{
public:
	// Create a server, ready to listen on specified port
	server_interface(uint16_t port)
		: serverAcceptor(serverIoContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
	{

	}

	virtual ~server_interface()
	{
		// May as well try and tidy up
		Stop();
	}

	// Starts the server!
	bool Start()
	{
		try
		{
			// Issue a task to the asio context - This is important
			// as it will prime the context with "work", and stop it
			// from exiting immediately. Since this is a server, we 
			// want it primed ready to handle clients trying to
			// connect.
			WaitForClientConnection();

			// Launch the asio context in its own thread
			serverIoThread = thread([this]() { serverIoContext.run(); });
		}
		catch (exception& e)
		{
			// Something prohibited the server from listening
			cerr << "[SERVER] Exception: " << e.what() << "\n";
			return false;
		}

		cout << "[SERVER] Started!\n";
		return true;
	}

	// Stops the server!
	void Stop()
	{
		// Request the context to close
		serverIoContext.stop();

		// Tidy up the context thread
		if (serverIoThread.joinable()) serverIoThread.join();

		// Inform someone, anybody, if they care...
		cout << "[SERVER] Stopped!\n";
	}

	// ASYNC - Instruct asio to wait for connection
	void WaitForClientConnection()
	{
		// Prime context with an instruction to wait until a socket connects. This
		// is the purpose of an "acceptor" object. It will provide a unique socket
		// for each incoming connection attempt
		serverAcceptor.async_accept(
			[this](error_code ec, asio::ip::tcp::socket socket)
			{
				// Triggered by incoming connection request
				if (!ec)
				{
					// Display some useful(?) information
					cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";

					// Create a new connection to handle this client 
					shared_ptr<connection<T>> newconn = 
						make_shared<connection<T>>(connection<T>::owner::server, 
							serverIoContext, std::move(socket), serverIncomingMessages);
					
					

					// Give the user server a chance to deny connection
					if (OnClientConnect(newconn))
					{								
						// Connection allowed, so add to container of new connections
						serverCurrentConnections.push_back(move(newconn));

						// And very important! Issue a task to the connection's
						// asio context to sit and wait for bytes to arrive!
						serverCurrentConnections.back()->ConnectToClient(nIDCounter++);

						cout << "[" << serverCurrentConnections.back()->GetID() << "] Connection Approved\n";
					}
					else
					{
						cout << "[-----] Connection Denied\n";

						// Connection will go out of scope with no pending tasks, so will
						// get destroyed automagically due to the wonder of smart pointers
					}
				}
				else
				{
					// Error has occurred during acceptance
					cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
				}

				// Prime the asio context with more work - again simply wait for
				// another connection...
				WaitForClientConnection();
			});
	}

	// Send a message to a specific client
	void MessageClient(shared_ptr<connection<T>> client, const message<T>& msg)
	{
		// Check client is legitimate...
		if (client && client->IsConnected())
		{
			// ...and post the message via the connection
			client->Send(msg);
		}
		else
		{
			// If we cant communicate with client then we may as 
			// well remove the client - let the server know, it may
			// be tracking it somehow
			OnClientDisconnect(client);

			// Off you go now, bye bye!
			client.reset();

			// Then physically remove it from the container
			serverCurrentConnections.erase(
				remove(serverCurrentConnections.begin(), serverCurrentConnections.end(), client), serverCurrentConnections.end());
		}
	}
	
	// Send message to all clients
	void MessageAllClients(const message<T>& msg, shared_ptr<connection<T>> pIgnoreClient = nullptr)
	{
		bool bInvalidClientExists = false;

		// Iterate through all clients in container
		for (auto& client : serverCurrentConnections)
		{
			// Check client is connected...
			if (client && client->IsConnected())
			{
				// ..it is!
        string messageContent;
				if(client != pIgnoreClient)
        {
          vector<uint8_t> messageBody = msg.body;
          for(int i = 0; i < messageBody.size(); i++)
            messageContent += messageBody[i];
          cout << "[SERVER]: Sending client " << client->GetID() << " a message containing: " << messageContent << "\n";
					client->Send(msg);
        }
			}
			else
			{
				// The client couldnt be contacted, so assume it has
				// disconnected.
				OnClientDisconnect(client);
				client.reset();

				// Set this flag to then remove dead clients from container
				bInvalidClientExists = true;
			}
		}

		// Remove dead clients, all in one go - this way, we dont invalidate the
		// container as we iterated through it.
		if (bInvalidClientExists)
			serverCurrentConnections.erase(
				remove(serverCurrentConnections.begin(), serverCurrentConnections.end(), nullptr), serverCurrentConnections.end());
	}

	// Force server to respond to incoming messages
	void Update(size_t nMaxMessages = -1, bool bWait = false)
	{
		if (bWait) serverIncomingMessages.wait();
    ofstream outputFile("update_loop.txt",ios::app);
		// Process as many messages as you can up to the value
		// specified
		size_t nMessageCount = 0;
		while (nMessageCount < nMaxMessages && !serverIncomingMessages.empty())
		{
			// Grab the front message
			auto msg = serverIncomingMessages.pop_front();
      vector<uint8_t> msgBody = msg.msg.body;
      string msgString;
      for(int i = 0; i < msgBody.size(); i++)
        msgString += msgBody[i];
      outputFile << "[SERVER HANDLER]: CONTENTS OF MESSAGE: " << msgString << endl;
			// Pass to message handler
			OnMessage(msg.remote, msg.msg);

			nMessageCount++;
		}
	}

protected:
	// This server class should override thse functions to implement
	// customised functionality

	// Called when a client connects, you can veto the connection by returning false
	virtual bool OnClientConnect(shared_ptr<connection<T>> client)
	{
		return false;
	}

	// Called when a client appears to have disconnected
	// Should be overwritten by developer in their own custom class
	virtual void OnClientDisconnect(shared_ptr<connection<T>> client)
	{

	}

	// Called when a message arrives
	// Should be overwritten by developer in their own custom class
	virtual void OnMessage(shared_ptr<connection<T>> client, message<T>& msg)
	{

	}


protected:
	// Thread Safe Queue for incoming message packets
	tsqueue<owned_message<T>> serverIncomingMessages;

	// Container of active validated connections
	deque<shared_ptr<connection<T>>> serverCurrentConnections;

	// Order of declaration is important - it is also the order of initialisation
	asio::io_context serverIoContext;
	thread serverIoThread;

	// These things need an asio context
	asio::ip::tcp::acceptor serverAcceptor; // Handles new incoming connection attempts...

	// Clients will be identified in the "wider system" via an ID
	// ALl clients IDs will start from this below number and continue till 2^32 - 1 (due to the uint32_t) 
	uint32_t nIDCounter = 0;
};
