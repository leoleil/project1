#include "AckSocket.h"
#include <iostream> 
#include <sstream>
#include <string>
#include "MySQLInterface.h"
#include "ACKMessage.h"
using namespace std;
extern string MYSQL_SERVER;//���ӵ����ݿ�ip
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
	cout << "| ACK ����         | ��������" << endl;
	//��ʼ���׽��ֶ�̬��  
	if (WSAStartup(MAKEWORD(2, 2), &S_wsd) != 0)
	{
		cout << "WSAStartup failed!" << endl;
		return 1;
	}

	//�����׽���  
	sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sServer)
	{
		cout << "socket failed!" << endl;
		WSACleanup();//�ͷ��׽�����Դ;  
		return  -1;
	}

	//�������׽��ֵ�ַ   
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(port);
	addrServ.sin_addr.s_addr = INADDR_ANY;
	//���׽���  
	retVal = bind(sServer, (LPSOCKADDR)&addrServ, sizeof(SOCKADDR_IN));
	if (SOCKET_ERROR == retVal)
	{
		cout << "bind failed!" << endl;
		closesocket(sServer);   //�ر��׽���  
		WSACleanup();           //�ͷ��׽�����Դ;  
		return -1;
	}

	//��ʼ����   
	cout << "| ACK ����         | listening" << endl;
	retVal = listen(sServer, 1);

	if (SOCKET_ERROR == retVal)
	{
		cout << "listen failed!" << endl;
		closesocket(sServer);   //�ر��׽���  
		WSACleanup();           //�ͷ��׽�����Դ;  
		return -1;
	}

	//���ܿͻ�������  
	sockaddr_in addrClient;
	int addrClientlen = sizeof(addrClient);
	sClient = accept(sServer, (sockaddr FAR*)&addrClient, &addrClientlen);
	if (INVALID_SOCKET == sClient)
	{
		cout << "accept failed!" << endl;
		closesocket(sServer);   //�ر��׽���  
		WSACleanup();           //�ͷ��׽�����Դ;  
		return -1;
	}

	cout << "| ACK ����         | TCP���Ӵ���" << endl;
	while (true) {
		//���ݴ���
		const int data_len = 66560;//ÿ�ν���65K���ݰ�
		char data[66560]; //���ݰ�
		ZeroMemory(data, data_len);//�����ݰ��ռ���0
		char* data_ptr = data;//����ָ��
		int r_len = 0;
		while (1) {
			//���տͻ�������
			//���buffer
			ZeroMemory(buf, BUF_SIZE);

			//��ȡ����
			retVal = recv(sClient, buf, BUF_SIZE, 0);

			if (SOCKET_ERROR == retVal)
			{
				cout << "| ACK ����         | ���ճ������" << endl;
				closesocket(sServer);   //�ر��׽���    
				closesocket(sClient);   //�ر��׽���
				return -1;
			}
			if (retVal == 0) {
				cout << "| ACK ����         | ������϶Ͽ���������" << endl;
				closesocket(sServer);   //�ر��׽���    
				closesocket(sClient);   //�ر��׽���
				return -1;
			}
			memcpy(data_ptr, buf, retVal);
			r_len = r_len + retVal;

			data_ptr = data_ptr + retVal;
			if ((data_ptr - data) >= data_len) {
				break;//������յ������ݴ�����󴰿�����ѭ�������ݰ����64K��
			}

		}
		
		//����ȡ�������ݷ�����µ����ݿ�
		const char* SERVER = MYSQL_SERVER.data();//���ӵ����ݿ�ip
		const char* USERNAME = MYSQL_USERNAME.data();
		const char* PASSWORD = MYSQL_PASSWORD.data();
		const char DATABASE[20] = "satellite_message";
		const int PORT = 3306;
		MySQLInterface mysql;
		char* ptr = data;
		UINT32 length = 0;
		for (int i = 0; i < data_len; i = i + length) {


			if (data[i] == NULL && data[i + 1] == NULL)break;
			//��ȡ���ĳ���
			memcpy(&length, ptr + i + 2, 4);

			//��ȡһ��buffer
			char val[65 * 1024];
			//�ڴ渴��
			memcpy(val, ptr + i, length);
			//���뱨�ĳ�
			ACKMessage message;
			message.messageParse(val);
			if (mysql.connectMySQL(SERVER, USERNAME, PASSWORD, DATABASE, PORT)) {
				string sql = "UPDATE `satellite_message`.`��������` SET `����ʼʱ��` = FROM_UNIXTIME(" + to_string(message.getTaskStartTime()) + "), `�������ʱ��` = FROM_UNIXTIME(" + to_string(message.getTaskEndTime()) + "), `ACK` = " + to_string(message.getACK()) + " WHERE `������` = " + to_string(message.getTaskNum()) + ";";
				mysql.writeDataToDB(sql);
				if (message.getACK() == 1200) {
					sql = "UPDATE `satellite_message`.`��������` SET `����״̬` = 5 WHERE `������` = " + to_string(message.getTaskNum()) + ";";
				}
				else if (message.getACK() == 1100) {
					sql = "UPDATE `satellite_message`.`��������` SET `����״̬` = 7 WHERE `������` = " + to_string(message.getTaskNum()) + ";";
				}
				else{
					sql = "UPDATE `satellite_message`.`��������` SET `����״̬` = 6 WHERE `������` = " + to_string(message.getTaskNum()) + ";";
				}
				mysql.writeDataToDB(sql);
				cout << "| ACK ����         | �ɹ�" << endl;
				mysql.closeMySQL();
			}
			else {
				cout << "| ACK ����         | ���ݿ�����ʧ��" << endl;
			}
			
			
			
			

		}

	}
	//�˳�  
	closesocket(sServer);   //�ر��׽���  
	closesocket(sClient);   //�ر��׽���  
	return 0;
}
