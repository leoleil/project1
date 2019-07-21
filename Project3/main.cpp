// Server.cpp : Defines the entry point for the console application.  
//  
#include <iostream> 
#include <thread> //thread 头文件,实现了有关线程的类
#include "winsock2.h"  
#include "MySQLInterface.h"
#include "assignment.h"
#include "ack.h"
#include "tele.h"
#include <Windows.h>
using namespace std;

CRITICAL_SECTION g_CS;//全局关键代码段对象
int STOP = 0;//是否停止socket程序运行
string MYSQL_SERVER = "";//连接的数据库ip
string MYSQL_USERNAME = "";
string MYSQL_PASSWORD = "";


#define IDR_PAUSE 12
#define IDR_ABOUT 13
#define IDR_HIDE 14
#define IDR_SHOW 15
LPCTSTR szAppClassName = TEXT("服务程序");
LPCTSTR szAppWindowName = TEXT("服务程序");
HMENU hmenu;//菜单句柄
HWND hwnd_head = GetForegroundWindow();//使hwnd代表最前端的窗口

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	NOTIFYICONDATA nid;
	UINT WM_TASKBARCREATED;
	POINT pt;//用于接收鼠标坐标
	int xx;//用于接收菜单选项返回值

		   // 不要修改TaskbarCreated，这是系统任务栏自定义的消息
	WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));
	switch (message)
	{
	case WM_CREATE://窗口创建时候的消息.
		nid.cbSize = sizeof(nid);
		nid.hWnd = hwnd;
		nid.uID = 0;
		nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nid.uCallbackMessage = WM_USER;
		nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		lstrcpy(nid.szTip, szAppClassName);
		Shell_NotifyIcon(NIM_ADD, &nid);
		hmenu = CreatePopupMenu();//生成菜单
		//AppendMenu(hmenu, MF_STRING, IDR_SHOW, "显示窗口");//为菜单添加两个选项
		AppendMenu(hmenu, MF_STRING, IDR_HIDE, "隐藏窗口");
		AppendMenu(hmenu, MF_STRING, IDR_PAUSE, "退出服务");//为菜单添加两个选项
		AppendMenu(hmenu, MF_STRING, IDR_ABOUT, "关于");
		break;
	case WM_USER://连续使用该程序时候的消息.
		if (lParam == WM_LBUTTONDOWN) {
			//MessageBox(hwnd, TEXT("Win32 API 实现系统托盘程序,双击托盘可以退出!"), szAppClassName, MB_OK);
			ShowWindow(hwnd_head, SW_SHOW);
		}
			
		if (lParam == WM_LBUTTONDBLCLK)//双击托盘的消息,退出.
			SendMessage(hwnd, WM_CLOSE, wParam, lParam);
		if (lParam == WM_RBUTTONDOWN)
		{
			GetCursorPos(&pt);//取鼠标坐标
			::SetForegroundWindow(hwnd);//解决在菜单外单击左键菜单不消失的问题
			//EnableMenuItem(hmenu, IDR_PAUSE, MF_GRAYED);//让菜单中的某一项变灰
			xx = TrackPopupMenu(hmenu, TPM_RETURNCMD, pt.x, pt.y, NULL, hwnd, NULL);//显示菜单并获取选项ID
			if(xx == IDR_HIDE) {
				ShowWindow(hwnd_head, SW_HIDE);
			}
			
			if (xx == IDR_PAUSE) {
				//MessageBox(hwnd, TEXT("111"), szAppClassName, MB_OK); 
				SendMessage(hwnd, WM_CLOSE, wParam, lParam);
			}
			if (xx == IDR_ABOUT) MessageBox(hwnd, TEXT("通信中心机程序"), szAppClassName, MB_OK);
			if (xx == 0) PostMessage(hwnd, WM_LBUTTONDOWN, NULL, NULL);
			//MessageBox(hwnd, TEXT("右键"), szAppName, MB_OK);
		}
		break;
	case WM_DESTROY://窗口销毁时候的消息.
		Shell_NotifyIcon(NIM_DELETE, &nid);
		PostQuitMessage(0);
		break;
	default:
		/*
		* 防止当Explorer.exe 崩溃以后，程序在系统系统托盘中的图标就消失
		*
		* 原理：Explorer.exe 重新载入后会重建系统任务栏。当系统任务栏建立的时候会向系统内所有
		* 注册接收TaskbarCreated 消息的顶级窗口发送一条消息，我们只需要捕捉这个消息，并重建系
		* 统托盘的图标即可。
		*/
		if (message == WM_TASKBARCREATED)
			SendMessage(hwnd, WM_CREATE, wParam, lParam);
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}




int main(int argc, char* argv[])
{
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass;

	ifstream is("config.txt", ios::in);
	if (!is.is_open()) {
		cout << "| 数据库连接       | ";
		cout << " 无法打开配置文件，请检查config.txt是否配置，并重启系统" << endl;
		//创建不成功释放资源
		system("pause");
		return 0;
	}
	getline(is, MYSQL_SERVER);
	getline(is, MYSQL_USERNAME);
	getline(is, MYSQL_PASSWORD);
	is.close();
	InitializeCriticalSection(&g_CS);//初始化关键代码段对象
									 //创建线程
	HANDLE hThread1;//ACK
	hThread1 = CreateThread(NULL, 0, ack_service, NULL, 0, NULL);
	CloseHandle(hThread1);
	HANDLE hThread2;//任务分配
	hThread2 = CreateThread(NULL, 0, assignment, NULL, 0, NULL);
	CloseHandle(hThread2);
	HANDLE hThread3;//遥测
	hThread3 = CreateThread(NULL, 0, message_rec, NULL, 0, NULL);
	CloseHandle(hThread3);
	

	HWND handle = FindWindow(NULL, szAppWindowName);
	if (handle != NULL)
	{
		MessageBox(NULL, TEXT("Application is already running"), szAppClassName, MB_ICONERROR);
		return 0;
	}

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppClassName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"), szAppClassName, MB_ICONERROR);
		return 0;
	}
	ShowWindow(hwnd_head, SW_HIDE);
	// 此处使用WS_EX_TOOLWINDOW 属性来隐藏显示在任务栏上的窗口程序按钮
	hwnd = CreateWindowEx(WS_EX_TOOLWINDOW,
		szAppClassName, szAppWindowName,
		WS_POPUP,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL, NULL, NULL, NULL);

	ShowWindow(hwnd, 0);
	UpdateWindow(hwnd);
	
	//消息循环
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	
	//while (1) {
	//	if (STOP == 1)break;
	//	
	//	Sleep(40000);//主线程函数静默4秒
	//}

	//Sleep(4000);//主线程函数静默4秒
	//DeleteCriticalSection(&g_CS);//删除关键代码段对象
	//system("pause");
	return 0;
}