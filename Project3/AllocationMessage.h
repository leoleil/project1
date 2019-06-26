#pragma once
#include <winsock2.h> 
#include "Message.h"
class AllocationMessage :
	public Message
{
public:
	AllocationMessage(UINT16 messageId, long long dateTime, bool encrypt, UINT32 taskNum, UINT16 taskType, long long taskStartTime, long long taskEndTime, char* satelliteId, char* groundStationId);
	AllocationMessage();
	~AllocationMessage();
private:
	UINT32 taskNum;//任务编号
	UINT16 taskType;//任务类型
	long long taskStartTime;//计划开始时间
	long long taskEndTime;//计划截至时间
	char satelliteId[20];//卫星编号
	char groundStationId[20];//地面站编号
	int messageSize;//报文长度
	char* message;//报文内容
public:
	UINT32 getterTaskNum();
	void setterTaskNum(UINT32 taskNum);
	UINT16 getterTaskType();
	void setterTaskType(UINT16 taskType);
	long long getterTaskStartTime();
	void setterTaskStartTime(long long taskStartTime);
	long long getterTaskEndTime();
	void setterTaskEndTime(long long taskEndTime);
	void getterSatelliteId(char* satelliteId, int & size);
	void setterSatelliteId(char* satelliteId, int size);
	void getterGroundStationId(char* groundStationId, int & size);
	void setterGroundStationId(char* groundStationId, int size);
	int getterMessageSize();
	void setterMessageSize(int messageSize);
	void getterMessage(char* message,int & size);
	void setterMessage(char* message,int size);
public:
	void createMessage(char* buf, int & message_size, int buf_size);
	void messageParse(char* buf);

};

