#pragma once
#include <iostream>  
#include <sstream>
#include <string>
#include <fstream>  
#include <vector>
#include <unordered_map>
#include "winsock2.h" 
#include <time.h>
#include <Windows.h>
#include "MySQLInterface.h"
#include "StringNumUtils.h"
#include "TeleSocket.h"
#include <thread> //thread 头文件,实现了有关线程的类
#include "DownMessage.h"
#include <io.h>
#include <direct.h>

using namespace std;
const int M = 1048576;//1M
const int K = 1024;   //1K
const double NEW_NULL = 1000000000;//这个数代表NULL
string getTime();//获取时间字符化输出
string getType(UINT16 num);
//定义报文字段
struct field
{
	char name[20];
	UINT16 type;
	double min;
	double max;
	char unit[8];
	bool display;
};

//报文解析线程入口
DWORD WINAPI message_rec(LPVOID lpParameter);
DWORD WINAPI message_pasing(LPVOID lpParameter);

