#pragma once
#include "net_common.h"

template <typename T>
class client_interface
{
public:
	client_interface() 
	{}

	virtual ~client_interface()
	{
		// If the client is destroyed, disconnect from server
		// There is nothing nice about memory leaks
		Disconnect();
	}

public:
	// Connect to server with hostname/ip-address and port
	// A resolver allows us to use both hostnames and standard IPs/Ports
	bool Connect(const string& host, const uint16_t port)
	{
		try
		{
			// The resolves allows us to resolve hostname/ip-address into tangiable physical address
			asio::ip::tcp::resolver resolver(clientIoContext);
			asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, to_string(port));

			// Create the actual connection object
			ncl_connection = make_unique<connection<T>>(connection<T>::owner::client, clientIoContext, asio::ip::tcp::socket(clientIoContext), incomingMessagesQueue);
			
			// Tell the connection object to connect to server
			ncl_connection->ConnectToServer(endpoints);

			// Start Context Thread to allow asynchronous operations
			thrContext = thread([this]() { clientIoContext.run(); });
		}
		catch (exception& e)
		{
			cerr << "[CLIENT] Exception: " << e.what() << "\n";
			return false;
		}
		return true;
	}

	// Disconnect from server
	void Disconnect()
	{
		// If connection exists, and it's connected then...
		if(IsConnected())
		{
			// ...disconnect from server gracefully
			ncl_connection->Disconnect();
		}

		// We're done with the asio context since there will be no more ASYNC operations			
		clientIoContext.stop();
		// Join the thread so that it can exit gracefully without seg faults
		if (thrContext.joinable())
			thrContext.join();

		// Destroy the connection object
		ncl_connection.release();
	}

	bool IsConnected()
	{
		if (ncl_connection)
			return ncl_connection->IsConnected();
		else
			return false;
	}

public:
	// Send message to server by delegating the responsiblity to the 'connection' object
	void Send(const message<T>& msg)
	{
		if (IsConnected())
			 ncl_connection->Send(msg);
	}

	// This is a getter for the queue of incoming messages
	tsqueue<owned_message<T>>& Incoming()
	{ 
		return incomingMessagesQueue;
	}

protected:
	//The IO context deals with all of the platform specific code for us and abstracts it
	asio::io_context clientIoContext;
	// ASIO needs a thread of its own to execute its work commands to prevent blocking and allow ASYNC operations
	thread thrContext;
	// The client has a single instance of a "connection" object, which handles data transfer
	unique_ptr<connection<T>> ncl_connection;
	
private:
	// This is the thread safe queue of incoming messages from server
	tsqueue<owned_message<T>> incomingMessagesQueue;
};
