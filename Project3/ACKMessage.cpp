#include "ACKMessage.h"



ACKMessage::ACKMessage(UINT16 messageId, long long dateTime, bool encrypt, UINT32 taskNum, long long taskStartTime, long long taskEndTime, UINT ACK):
Message(messageId, dateTime, encrypt), taskNum(taskNum) , taskStartTime(taskStartTime), taskEndTime(taskEndTime), ACK(ACK)
{
	this->messageLength = this->messageLength + sizeof(UINT32) + sizeof(long long) + sizeof(long long) + sizeof(UINT);
}

ACKMessage::ACKMessage()
{
}


ACKMessage::~ACKMessage()
{
}

UINT32 ACKMessage::getTaskNum()
{
	return this->taskNum;
}

void ACKMessage::setTaskNum(UINT32 taskNum)
{
	this->taskNum = taskNum;
}

long long ACKMessage::getTaskStartTime()
{
	return this->taskStartTime;
}

void ACKMessage::setTaskStartTime(long long taskStartTime)
{
	this->taskStartTime = taskStartTime;
}

long long ACKMessage::getTaskEndTime()
{
	return this->taskEndTime;
}

void ACKMessage::setTaskEndTime(long long taskEndTime)
{
	this->taskEndTime = taskEndTime;
}

UINT ACKMessage::getACK()
{
	return this->ACK;
}

void ACKMessage::setACK(UINT ACK)
{
	this->ACK = ACK;
}

void ACKMessage::createMessage(char * buf, int & message_size, int buf_size)
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
		bufPtr = bufPtr + sizeof(UINT32);//移动指针
		memcpy(bufPtr, &taskStartTime, sizeof(long long));//计划开始时间戳
		bufPtr = bufPtr + sizeof(long long);//移动指针
		memcpy(bufPtr, &taskEndTime, sizeof(long long));//计划结束时间戳
		bufPtr = bufPtr + sizeof(long long);//移动指针
		memcpy(bufPtr, &ACK, sizeof(UINT));//卫星编号
		message_size = messageLength;

	}

}

void ACKMessage::messageParse(char * buf)
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
	bufPtr = bufPtr + sizeof(UINT32);//移动指针
	memcpy(&taskStartTime, bufPtr, sizeof(long long));//计划开始时间戳
	bufPtr = bufPtr + sizeof(long long);//移动指针
	memcpy(&taskEndTime, bufPtr, sizeof(long long));//计划结束时间戳
	bufPtr = bufPtr + sizeof(long long);//移动指针
	memcpy(&ACK, bufPtr, sizeof(UINT));
}

