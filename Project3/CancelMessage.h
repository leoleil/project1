#pragma once
#include "Message.h"
class CancelMessage :
	public Message
{
public:
	CancelMessage(UINT16 messageId, long long dateTime, bool encrypt, UINT32 taskNum);
	~CancelMessage();
private:
	UINT32 taskNum;
public:
	UINT32 getterTaskNum();
	//为数据传输创造一个message的接口
	void createMessage(char* buf, int & message_size, int buf_size);
	//通过数据包来解析数据接口
	void messageParse(char* buf);
};

