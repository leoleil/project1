#pragma comment(lib, "ws2_32.lib")
#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h> 
#include <vector>
//Socket为使用者提供一个70k数据包
struct message_buf
{
	//70K
	char val[70 * 1024];
};
class Socket
{
public:
	Socket();
	~Socket();
public:
	bool createSendServer(const char* ip, const int hton, const double velocity);

	int sendMessage(char* message, int lengt);

	int createReceiveServer(const int port);

	int offSendServer();

	int offRecServer();

protected:
	const int BUF_SIZE = 1024;//buffer大小

protected:
	WSADATA wsd;//WSADATA变量  

	SOCKET sServer;//作为接收端的服务器套接字  

	SOCKET sClient;//作为接收端的客户端套接字

	SOCKADDR_IN addrServ;//服务器地址 

	char buf[1024];  //接收数据缓冲区  

	int retVal;//返回值 

	WSADATA S_wsd; //服务端WSADATA变量  

	WSADATA H_wsd; //客户端WSADATA变量  

	SOCKET sHost; //作为发送端的服务器套接字 

	SOCKADDR_IN servAddr; //服务器地址  
};

