#pragma once
#include "Socket.h"
class TeleSocket :
	public Socket
{
public:
	TeleSocket();
	~TeleSocket();
public:
	int createReceiveServer(const int port, std::vector<message_buf>& message);
};

