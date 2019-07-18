#include "AckSocket.h"
#include <iostream> 
#include <sstream>
#include <string>
#include "MySQLInterface.h"
#include "ACKMessage.h"
using namespace std;
extern string MYSQL_SERVER;//连接的数据库ip
extern string MYSQL_USERNAME;
extern string MYSQL_PASSWORD;

AckSocket::AckSocket()
{
}


AckSocket::~AckSocket()
{
}

int AckSocket::createReceiveServer(const int port)
{
	cout << "| ACK 接收         | 服务启动" << endl;
	//初始化套结字动态库  
	if (WSAStartup(MAKEWORD(2, 2), &S_wsd) != 0)
	{
		cout << "WSAStartup failed!" << endl;
		return 1;
	}

	//创建套接字  
	sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sServer)
	{
		cout << "socket failed!" << endl;
		WSACleanup();//释放套接字资源;  
		return  -1;
	}

	//服务器套接字地址   
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(port);
	addrServ.sin_addr.s_addr = INADDR_ANY;
	//绑定套接字  
	retVal = bind(sServer, (LPSOCKADDR)&addrServ, sizeof(SOCKADDR_IN));
	if (SOCKET_ERROR == retVal)
	{
		cout << "bind failed!" << endl;
		closesocket(sServer);   //关闭套接字  
		WSACleanup();           //释放套接字资源;  
		return -1;
	}

	//开始监听   
	cout << "| ACK 接收         | listening" << endl;
	retVal = listen(sServer, 1);

	if (SOCKET_ERROR == retVal)
	{
		cout << "listen failed!" << endl;
		closesocket(sServer);   //关闭套接字  
		WSACleanup();           //释放套接字资源;  
		return -1;
	}

	//接受客户端请求  
	sockaddr_in addrClient;
	int addrClientlen = sizeof(addrClient);
	sClient = accept(sServer, (sockaddr FAR*)&addrClient, &addrClientlen);
	if (INVALID_SOCKET == sClient)
	{
		cout << "accept failed!" << endl;
		closesocket(sServer);   //关闭套接字  
		WSACleanup();           //释放套接字资源;  
		return -1;
	}

	cout << "| ACK 接收         | TCP连接创建" << endl;
	while (true) {
		//数据窗口
		const int data_len = 66560;//每次接收65K数据包
		char data[66560]; //数据包
		ZeroMemory(data, data_len);//将数据包空间置0
		char* data_ptr = data;//数据指针
		int r_len = 0;
		while (1) {
			//接收客户端数据
			//清空buffer
			ZeroMemory(buf, BUF_SIZE);

			//获取数据
			retVal = recv(sClient, buf, BUF_SIZE, 0);

			if (SOCKET_ERROR == retVal)
			{
				cout << "| ACK 接收         | 接收程序出错" << endl;
				closesocket(sServer);   //关闭套接字    
				closesocket(sClient);   //关闭套接字
				return -1;
			}
			if (retVal == 0) {
				cout << "| ACK 接收         | 接收完毕断开本次连接" << endl;
				closesocket(sServer);   //关闭套接字    
				closesocket(sClient);   //关闭套接字
				return -1;
			}
			memcpy(data_ptr, buf, retVal);
			r_len = r_len + retVal;

			data_ptr = data_ptr + retVal;
			if ((data_ptr - data) >= data_len) {
				break;//如果接收到的数据大于最大窗口跳出循环（数据包最大64K）
			}

		}
		
		//将获取到的数据放入更新到数据库
		const char* SERVER = MYSQL_SERVER.data();//连接的数据库ip
		const char* USERNAME = MYSQL_USERNAME.data();
		const char* PASSWORD = MYSQL_PASSWORD.data();
		const char DATABASE[20] = "satellite_message";
		const int PORT = 3306;
		MySQLInterface mysql;
		char* ptr = data;
		UINT32 length = 0;
		for (int i = 0; i < data_len; i = i + length) {


			if (data[i] == NULL && data[i + 1] == NULL)break;
			//获取报文长度
			memcpy(&length, ptr + i + 2, 4);

			//获取一个buffer
			char val[65 * 1024];
			//内存复制
			memcpy(val, ptr + i, length);
			//加入报文池
			ACKMessage message;
			message.messageParse(val);
			if (mysql.connectMySQL(SERVER, USERNAME, PASSWORD, DATABASE, PORT)) {
				string sql = "UPDATE `satellite_message`.`任务分配表` SET `任务开始时间` = FROM_UNIXTIME(" + to_string(message.getTaskStartTime()) + "), `任务结束时间` = FROM_UNIXTIME(" + to_string(message.getTaskEndTime()) + "), `ACK` = " + to_string(message.getACK()) + " WHERE `任务编号` = " + to_string(message.getTaskNum()) + ";";
				mysql.writeDataToDB(sql);
				if (message.getACK() == 1200) {
					sql = "UPDATE `satellite_message`.`任务分配表` SET `任务状态` = 5 WHERE `任务编号` = " + to_string(message.getTaskNum()) + ";";
				}
				else{
					sql = "UPDATE `satellite_message`.`任务分配表` SET `任务状态` = 6 WHERE `任务编号` = " + to_string(message.getTaskNum()) + ";";
				}
				mysql.writeDataToDB(sql);
				cout << "| ACK 接收         | 成功" << endl;
				mysql.closeMySQL();
			}
			else {
				cout << "| ACK 接收         | 数据库连接失败" << endl;
			}
			
			
			
			

		}

	}
	//退出  
	closesocket(sServer);   //关闭套接字  
	closesocket(sClient);   //关闭套接字  
	return 0;
}
