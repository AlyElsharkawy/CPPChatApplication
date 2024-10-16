#include "MainFrame.h"
#include "custom_messages.h"
#include "CLIENT_INTERFACE.h"
#include "net_All.h"
#include <fstream>

using namespace std;

char* NAME_STR = new char[16];
char* CONTENT_STRING = new char[2048];

thread mainTH;
void CustomClient::MainThread()
{ 
  while(true)
  {
		if (this->IsConnected())
		{
			if (!this->Incoming().empty())
			{

				auto msg = this->Incoming().pop_front().msg;

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
				}
			}
		}
		else
		{
		  std::cout << "Server Down\n";
	    this_thread::sleep_for(chrono::seconds(2));
		}
	}
}

bool CustomClient::AddMessage(string nameStr, string contentStr)
{
  //mf->AddMessageToWindow(nameStr, contentStr);
  for(int i = 0; i < nameStr.length(); i++)
    NAME_STR[i] = nameStr[i];
  NAME_STR[nameStr.length()] = '\0';
  for(int i = 0; i < contentStr.length(); i++)
    CONTENT_STRING[i] = contentStr[i];
  CONTENT_STRING[contentStr.length()] = '\0';
  SendStringMessage(NAME_STR, CONTENT_STRING);
  return true;
}

void CustomClient::PingServer()	
{
	message<CustomMsgTypes> msg;
	msg.header.id = CustomMsgTypes::ServerPing;

	// Caution with this...
	std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();		

	msg << timeNow;
	Send(msg);
}

void CustomClient::SendStringMessage(const char* name, const char* content)
{
  message<CustomMsgTypes> msg;
  msg.header.id = CustomMsgTypes::StringMessage;
  string CONTENT_STRING(content);
  while(*name != '\0')
  {
    msg << *name;
    name++;
  }
  msg << ' ';
  while(*content != '\0')
  {
    msg << *content;
    content++;
  }
  ofstream outputFile("update_loop.txt",ios::app);
  outputFile << "[CLIENT MECHANISM]: Sending content this content to server: " << CONTENT_STRING << endl;
  outputFile.close();

  //Add bool?
  Send(msg);
}

void CustomClient::RequestName()
{
  message<CustomMsgTypes> tempMsg;
  tempMsg.header.id = CustomMsgTypes::NameRequest;
  tempMsg << 33;
  Send(tempMsg);
}

void CustomClient::InitalizeServer()
{
  this->Connect("127.0.0.1", 60000);
  //mainTH = thread([&](){ MainThread();});
  //mainTH.detach();
}

string CustomClient::StringListen()
{
  while(true)
  {
		if (this->IsConnected())
		{
			if (!this->Incoming().empty())
			{

				auto msg = this->Incoming().pop_front().msg;

				switch (msg.header.id)
				{
        case CustomMsgTypes::StringMessage:
        {
          vector<uint8_t> tempVec = msg.body;
          ofstream outputFile("update_loop.txt", ios::app);
          for(int i = 0; i < tempVec.size(); i++)
              {
                if(tempVec[i] != ' ')
                  NAME_STR[i] = tempVec[i];
                else
                {
                  tempVec.erase(tempVec.begin(), tempVec.begin() + i);
                  NAME_STR[i] = '\0';
                  break;
                }
              }
          for(int i = 0; i < tempVec.size(); i++)
              {
                CONTENT_STRING[i] = tempVec[i];
              }
              CONTENT_STRING[tempVec.size()] = '\0';
          string output1(NAME_STR);
          string output2(CONTENT_STRING);
          outputFile << "[SERVER]: Name Recieved is: " << output1 << endl;
          outputFile << "[SERVER]: Content Recieved is: " << output2 << endl;
          outputFile.close();
          string finalOutput = output1 + ' ' + output2;
          return finalOutput;
          break;
          }
				}
			}
		}
		else
		{
		  std::cout << "Server Down\n";
	    this_thread::sleep_for(chrono::seconds(2));
		}
	}
}

string CustomClient::UsernameListen()
{
  string tempStr = "";
  while(tempStr == "")
  {
		if (this->IsConnected())
		{
			if (!this->Incoming().empty())
			{
				auto msg = this->Incoming().pop_front().msg;
				switch (msg.header.id)
				{ 
          case CustomMsgTypes::NameRequest:
            {
              vector<uint8_t> tempVec = msg.body;
              int stringSize = msg.body.size();
              char* tempCharString = new char[stringSize + 1];
              for(int i = 0; i < tempVec.size(); i++)
              {
                tempStr += (char)tempVec[i];
                tempCharString[i] = (char)tempVec[i];
              }
              tempCharString[stringSize] = '\0';
              cout << "Value of tempStr is: " << tempStr << endl;
              cout << "[CLIENT] Recieved username: " << tempStr << endl;
              break;
          }
        }
			}
		}
		else
		{
		  std::cout << "Server Down\n";
	    this_thread::sleep_for(chrono::seconds(2));
		}
	}
  cout << "Exiting function!" << endl;
  return tempStr;
}

