// Server.cpp : Defines the entry point for the console application.  
//  
#include <iostream> 
#include <thread> //thread 头文件,实现了有关线程的类
#include "winsock2.h"  
#include "MySQLInterface.h"
#include "assignment.h"
#include "ack.h"
#include "tele.h"
using namespace std;

CRITICAL_SECTION g_CS;//全局关键代码段对象
int STOP = 0;//是否停止socket程序运行
int main(int argc, char* argv[])
{
	
	InitializeCriticalSection(&g_CS);//初始化关键代码段对象
    //创建线程
	HANDLE hThread1;//ACK
	HANDLE hThread2;//任务分配
	HANDLE hThread3;//遥测
	hThread1 = CreateThread(NULL, 0, ack_service, NULL, 0, NULL);
	hThread2 = CreateThread(NULL, 0, assignment, NULL, 0, NULL);
	hThread3 = CreateThread(NULL, 0, message_rec, NULL, 0, NULL);
	CloseHandle(hThread1);
	CloseHandle(hThread2);
	CloseHandle(hThread3);
	while (1) {
		if (STOP == 1)break;
	}

	Sleep(4000);//主线程函数静默4秒
	DeleteCriticalSection(&g_CS);//删除关键代码段对象
	system("pause");
	return 0;
}