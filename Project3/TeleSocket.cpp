#include "TeleSocket.h"
#include <iostream> 
#include <sstream>
#include <string>
#include "MySQLInterface.h"
using namespace std;
extern string MYSQL_SERVER;
extern string MYSQL_USERNAME;
extern string MYSQL_PASSWORD;
extern CRITICAL_SECTION data_CS;//全局关键代码段对象
extern vector<message_buf> DATA_MESSAGES;//全局上传数据报文池

TeleSocket::TeleSocket()
{
}


TeleSocket::~TeleSocket()
{
}

int TeleSocket::createReceiveServer(const int port, std::vector<message_buf>& message)
{
	const char* SERVER = MYSQL_SERVER.data();//连接的数据库ip
	const char* USERNAME = MYSQL_USERNAME.data();
	const char* PASSWORD = MYSQL_PASSWORD.data();
	const char DATABASE[20] = "satellite_message";
	const int PORT = 3306;
	MySQLInterface mysql;
	if (mysql.connectMySQL(SERVER, USERNAME, PASSWORD, DATABASE, PORT)) {

	}
	else {
		cout << "| 卫星遥测         | 数据库连接失败" << endl;
		return 0;
	}
	cout << "| 卫星遥测         | 服务启动" << endl;
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
	cout << "| 卫星遥测         | listening" << endl;
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

	cout << "| 卫星遥测         | TCP连接创建" << endl;
	//写日志
	string logSql = "insert into 系统日志表 (时间,对象,事件类型,参数) values (now(),'遥测通信模块',11000,'建立TCP连接');";
	mysql.writeDataToDB(logSql);
	//接收数据
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
				cout << "| 卫星遥测         | 接收程序出错" << endl;
				closesocket(sServer);   //关闭套接字    
				closesocket(sClient);   //关闭套接字
				return -1;
			}
			if (retVal == 0) {
				cout << "| 卫星遥测         | 接收完毕断开本次连接" << endl;
				//写日志
				logSql = "insert into 系统日志表 (时间,对象,事件类型,参数) values (now(),'遥测通信模块',11001,'断开TCP连接');";
				mysql.writeDataToDB(logSql);
				closesocket(sServer);   //关闭套接字    
				closesocket(sClient);   //关闭套接字
				return -1;
			}
			Sleep(10);
			memcpy(data_ptr, buf, retVal);
			r_len = r_len + retVal;

			data_ptr = data_ptr + retVal;
			if ((data_ptr - data) >= data_len) {
				break;//如果接收到的数据大于最大窗口跳出循环（数据包最大64K）
			}

		}


		//将获取到的数据放入数据池中
		char* ptr = data;
		UINT32 length = 0;
		for (int i = 0; i < data_len; i = i + length) {

			if (data[i] == NULL && data[i + 1] == NULL)break;
			//获取报文长度
			memcpy(&length, ptr + i + 2, 4);

			//获取一个buffer
			message_buf messageBuf;
			//内存复制
			memcpy(&messageBuf.val, ptr + i, length);
			//加入报文池
			EnterCriticalSection(&data_CS);//进入关键代码段
			message.push_back(messageBuf);
			LeaveCriticalSection(&data_CS);//离开关键代码段

		}

	}
	//退出  
	closesocket(sServer);   //关闭套接字  
	closesocket(sClient);   //关闭套接字  
	return 0;
}
