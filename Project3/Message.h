#pragma once
#include <winsock2.h> 
#include <sys/timeb.h>
class Message
{
public:
	Message();
	Message(UINT16 messageId, long long dateTime, bool encrypt);
	~Message();
protected:
	UINT16 messageId;//报文标识
	long long dateTime;//时间戳
	bool encrypt;//是否加密
protected:
	UINT32 messageLength;//报文长度
public:
	UINT16 getterMessageId();

	void setterMessageId(UINT16 messageId);
	
	UINT32 getterMessageLength();

	void setterMessageLength(UINT32 messageLengh);

	void messageLengthAdd(int len);

	long long getterDateTime();

	void setDateTime(long long dateTime);

	bool getterEncrypt();

	void setterEncrypt(bool encrypt);
public:
	//获取当前时间戳
	static long long getSystemTime()
	{
		struct timeb t;

		ftime(&t);

		return 1000 * t.time + t.millitm;
	}
public:
	//为数据传输创造一个message的接口
	virtual void createMessage(char* buf, int & message_size, int buf_size)=0;
	//通过数据包来解析数据接口
	virtual void messageParse(char* buf) = 0;
};

