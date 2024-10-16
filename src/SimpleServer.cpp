#include <cstdint>
#include <list>
#include "custom_messages.h"
#include "net_All.h"

using namespace std;

//This contains the maximum 24 usernames that this chatapp can support


class CustomServer : public server_interface<CustomMsgTypes>
{
public:
	CustomServer(uint16_t nPort) : server_interface<CustomMsgTypes>(nPort)
	{

	}

protected:
  list<string> POSSIBLE_USERNAMES = {"MR.A", "MR.B", "MR.C", "MR.D", "MR.E", "MR.F", "MR.G", "MR.H", "MR.I", "MR.J", "MR.K", "MR.L", "MR.M", "MR.N", "MR.O", "MR.P", "MR.Q", "MR.R","MR.S","MR.T","MR.U","MR.V","MR.W","MR.X","MR.Y","MR.Z"};

	virtual bool OnClientConnect(std::shared_ptr<connection<CustomMsgTypes>> client)
	{
		message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerAccept;
		client->Send(msg);
		return true;
	}

	// Called when a client appears to have disconnected
	virtual void OnClientDisconnect(std::shared_ptr<connection<CustomMsgTypes>> client)
	{
		std::cout << "Removing client [" << client->GetID() << "]\n";
	}

	// Called when a message arrives
	virtual void OnMessage(std::shared_ptr<connection<CustomMsgTypes>> client, message<CustomMsgTypes>& msg)
	{
		switch (msg.header.id)
		{
		  case CustomMsgTypes::ServerPing:
		  {
			  cout << "[" << client->GetID() << "]: Server Ping\n";
			  client->Send(msg);
        break;
		  }

      case CustomMsgTypes::StringMessage:
        {
          string messageContent;
          vector<uint8_t> msgBody = msg.body;
          for(int i = 0; i < msgBody.size(); i++)
            messageContent += msgBody[i];
          cout << "[SERVER]: Client " << client->GetID() << " is sending a message to all other users: " << messageContent << "\n";
          MessageAllClients(msg,client);
          break;
        }
      case CustomMsgTypes::ServerAccept:
        {
          //This exists just so that clangd CAN SHUT UP. I KNOW THAT IT IS NOT HANDLDED HERE
          //IT IS HANDLED SOMEWHERE ELSE 
          break;
        }

      case CustomMsgTypes::NameRequest:
        {
          cout << "[SERVER]: Client " << client->GetID() << " request a username\n";
          const char* nameToSend;
          if(!POSSIBLE_USERNAMES.empty())
          {
            nameToSend = POSSIBLE_USERNAMES.front().c_str();
            POSSIBLE_USERNAMES.pop_front();
          } 
          else
          {
            cerr << "[SERVER] Kicked client " << client->GetID() << " due to max users limit\n";
            client->Disconnect();
            return;
          }
          message<CustomMsgTypes> msgToSend;
          msgToSend.header.id = CustomMsgTypes::NameRequest;
          while(*nameToSend != '\0')
          {
            msgToSend << *nameToSend;
            nameToSend++;
          }
          client->Send(msgToSend);
          cout << "[SERVER] sent client " << client->GetID() << " a username!\n";
          break;
        }
		}
	}
};

int main()
{
	CustomServer server(60000); 
	server.Start();

	while (1)
	{
		server.Update(-1, true);
	}
	return 0;
}
