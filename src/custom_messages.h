#pragma once
#include <cstdint>

enum class CustomMsgTypes : uint32_t
{
  ServerAccept,
	ServerPing,
  StringMessage,
  NameRequest,
};
