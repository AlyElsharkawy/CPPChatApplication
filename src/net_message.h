#pragma once
#include "net_common.h"


// Message Header is sent at start of all messages. The template allows us
// to use "enum class" to ensure that the messages are valid at compile time
// This also allows us to give a way to tell the server what kind of a message to expect
// It could be a ping message, request connection message, etc.
// Although this framework is only partially implemented, it is still very powerful!

//For the sake of robustness, we cant use regular data types
//We need to use ones that have fixed platform independent sizes

template <typename T>
struct message_header
{
	T id{};
	uint32_t size = 0;
};

// Message Body contains a header and a vector, containing raw bytes
// of infomation. This way the message can be variable length, but the size
// in the header must be updated.

template <typename T>
struct message
{
	// Header & Body vector
	message_header<T> header{};
	vector<uint8_t> body;

	// returns size of entire message packet in bytes
	size_t size() const
	{
		return body.size();
	}

	// Override for cout compatibility - produces friendly description of message
	// The output will be the message's ID and its size in bytes
	// The message id is determined the enum class
	friend ostream& operator << (ostream& os, const message<T>& msg)
	{
		os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
		return os;
	}

	// Convenience Operator overloads - These allow us to add and remove stuff from
	// the body vector as if it were a stack, so First in, Last Out. These are a 
	// template in itself, because we dont know what data type the user is pushing or 
	// popping, so lets allow them all. NOTE: It assumes the data type is fundamentally
	// Plain Old Data (POD). TLDR: Serialise & Deserialise into/from a vector
	
	//Basically if its a struct, its ok
	//If its a class with only variables...its ok
	//If its a class with methods then it won't work
	//You will need to manually extract the variables from the object and send them and then construct the object at the destination

	// Pushes any POD-like data into the message buffer
	template<typename DataType>
	friend message<T>& operator << (message<T>& msg, const DataType& data)
	{
		// Check that the type of the data being pushed is trivially copyable
		static_assert(is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");

		// Cache current size of vector, as this will be the point we insert the data
		size_t i = msg.body.size();

		// Resize the vector by the size of the data being pushed
		msg.body.resize(msg.body.size() + sizeof(DataType));

		// Physically copy the data into the newly allocated vector space
		memcpy(msg.body.data() + i, &data, sizeof(DataType));

		// Recalculate the message size
		msg.header.size = msg.size();

		// Return the target message so it can be "chained"
		// for example msg << a << b << c << d;
		return msg;
	}

	// Pulls any POD-like data form the message buffer
	template<typename DataType>
	friend message<T>& operator >> (message<T>& msg, DataType& data)
	{
		// Check that the type of the data being pushed is trivially copyable
		static_assert(is_standard_layout<DataType>::value, "Data is too complex to be pulled from vector");

		//We will need this for memcpy as it will be where we add the new data
		size_t i = msg.body.size() - sizeof(DataType);

		// Physically copy the data from the vector into the user variable
		memcpy(&data, msg.body.data() + i, sizeof(DataType));

		// Shrink the vector to remove read bytes, and reset end position
		msg.body.resize(i);

		// Recalculate the message size
		msg.header.size = msg.size();

		// Return the target message so it can be "chained"
		return msg;
	}			
};


// An "owned" message is identical to a regular message, but it is associated with
// a connection. On a server, the owner would be the client that sent the message, 
// on a client the owner would be the server.

// Forward declare the connection
template <typename T>
class connection;

template <typename T>
struct owned_message
{
	shared_ptr<connection<T>> remote = nullptr;
	message<T> msg;

	// Again, a friendly string maker
	friend ostream& operator<<(ostream& os, const owned_message<T>& msg)
	{
		os << msg.msg;
		return os;
	}
};		

///[OLC_HEADERIFYIER] END "MESSAGE"

