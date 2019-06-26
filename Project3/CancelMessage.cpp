#include "CancelMessage.h"



CancelMessage::CancelMessage(UINT16 messageId, long long dateTime, bool encrypt, UINT32 taskNum):
Message(messageId, dateTime, encrypt), taskNum(taskNum)
{
	this->messageLength = this->messageLength + sizeof(UINT32);
}


CancelMessage::~CancelMessage()
{
}

UINT32 CancelMessage::getterTaskNum()
{
	return this->taskNum;
}

void CancelMessage::createMessage(char * buf, int & message_size, int buf_size)
{
	if (buf_size >= messageLength) {
		char* bufPtr = buf;//buf指针
		memcpy(bufPtr, &(messageId), sizeof(UINT16));//任务标志
		bufPtr = bufPtr + sizeof(UINT16);//移动指针
		memcpy(bufPtr, &messageLength, sizeof(UINT32));//报文长度
		bufPtr = bufPtr + sizeof(UINT32);//移动指针
		memcpy(bufPtr, &dateTime, sizeof(long long));//时间戳
		bufPtr = bufPtr + sizeof(long long);//移动指针
		memcpy(bufPtr, &encrypt, sizeof(bool));//加密标识
		bufPtr = bufPtr + sizeof(bool);//移动指针
		memcpy(bufPtr, &taskNum, sizeof(UINT32));//任务编号
		message_size = messageLength;
	}
}

void CancelMessage::messageParse(char * buf)
{
	char* bufPtr = buf;//buf指针
	memcpy(&(messageId), bufPtr, sizeof(UINT16));//任务标志
	bufPtr = bufPtr + sizeof(UINT16);//移动指针
	memcpy(&messageLength, bufPtr, sizeof(UINT32));//报文长度
	bufPtr = bufPtr + sizeof(UINT32);//移动指针
	memcpy(&dateTime, bufPtr, sizeof(long long));//时间戳
	bufPtr = bufPtr + sizeof(long long);//移动指针
	memcpy(&encrypt, bufPtr, sizeof(bool));//加密标识
	bufPtr = bufPtr + sizeof(bool);//移动指针
	memcpy(&taskNum, bufPtr, sizeof(UINT32));//任务编号
}
