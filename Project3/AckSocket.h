#pragma once
#include "Socket.h"
class AckSocket :
	public Socket
{
public:
	AckSocket();
	~AckSocket();
public:
	int createReceiveServer(const int port);
};

