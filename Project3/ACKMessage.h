#pragma once
#include "Message.h"
class ACKMessage :
	public Message
{
public:
	ACKMessage(UINT16 messageId, long long dateTime, bool encrypt, UINT32 taskNum, long long taskStartTime, long long taskEndTime, UINT ACK);
	ACKMessage();
	~ACKMessage();
private:
	UINT32 taskNum;//任务编号
	long long taskStartTime;//任务开始时间
	long long taskEndTime;//任务截止时间
	UINT ACK;
public:
	UINT32 getTaskNum();
	void setTaskNum(UINT32 taskNum);
	long long getTaskStartTime();//任务开始时间
	void setTaskStartTime(long long taskStartTime);
	long long getTaskEndTime();//任务截止时间
	void setTaskEndTime(long long taskEndTime);
	UINT getACK();
	void setACK(UINT ACK);
public:
	void createMessage(char* buf, int & message_size, int buf_size);
	void messageParse(char* buf);
};

