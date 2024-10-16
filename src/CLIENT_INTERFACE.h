#pragma once
#include "custom_messages.h"
#include "net_All.h"

using namespace std;

class CustomClient : public client_interface<CustomMsgTypes>
{
public:
	void PingServer();
  void SendStringMessage(const char* name, const char* content);
  void InitalizeServer();
  bool AddMessage(string nameStr, string contentStr);
  void MainThread();
  string StringListen();
  string UsernameListen();
  void RequestName();
};

