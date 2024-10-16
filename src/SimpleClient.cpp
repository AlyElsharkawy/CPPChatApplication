#include <iostream>
#include <mutex>
#include <thread>
#include "custom_messages.h"
#include "net_All.h"

using namespace std;

class CustomClient : public client_interface<CustomMsgTypes>
{
public:
	void PingServer()	
	{
		message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerPing;

		// Caution with this...
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();		

		msg << timeNow;
		Send(msg);
	}

  void SendStringMessage(const char* inputString)
  {
    message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::StringMessage;
    while(*inputString != '\0')
    {
      msg << *inputString;
      inputString++;
    }
    Send(msg);
  }
};

std::mutex mtx;
int main()
{
	CustomClient CC;
	CC.Connect("127.0.0.1", 60000);
  while(true)
  {
		if (CC.IsConnected())
		{
			if (!CC.Incoming().empty())
			{

				auto msg = CC.Incoming().pop_front().msg;

				switch (msg.header.id)
				{
				  case CustomMsgTypes::ServerAccept:
				  {
					  // Server has responded to a ping request				
					  std::cout << "Server Accepted Connection\n";
            break;
				  }

				  case CustomMsgTypes::ServerPing:
				  {
					  // Server has responded to a ping request
					  std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
					  std::chrono::system_clock::time_point timeThen;
					  msg >> timeThen;
					  std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
            break;
				  }
          case CustomMsgTypes::StringMessage:
            {
              vector<uint8_t> tempVec = msg.body;
              string toPrint;
              cout << "[CLIENT]: Recieved vector size (bytes): " << tempVec.size() << "\n";
              for(int i = 0; i < tempVec.size(); i++)
                toPrint += tempVec[i];
              cout << "[CLIENT] Message: " << toPrint << "\n";
              break;
            }
				}
			}
		}
		else
		{
			std::cout << "Server Down. Please reopen client!\n";
      return -1;
		}
	}
}
