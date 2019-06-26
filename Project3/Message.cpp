#include "Message.h"



Message::Message()
{
	this->messageLength = 0;
}

Message::Message(UINT16 messageId, long long dateTime, bool encrypt):messageId(messageId), dateTime(dateTime), encrypt(encrypt)
{
	this->messageLength = sizeof(UINT16) + sizeof(long long) + sizeof(bool) + sizeof(UINT32);
}


Message::~Message()
{
}

UINT16 Message::getterMessageId()
{
	return this->messageId;
}

void Message::setterMessageId(UINT16 messageId)
{
	this->messageId = messageId;
}

UINT32 Message::getterMessageLength()
{
	return this->messageLength;
}

void Message::setterMessageLength(UINT32 messageLengh)
{
	this->messageLength = messageLengh;
}

void Message::messageLengthAdd(int len)
{
	this->messageLength = this->messageLength + len;
}

long long Message::getterDateTime()
{
	return dateTime;
}

void Message::setDateTime(long long dateTime)
{
	this->dateTime = dateTime;
}

bool Message::getterEncrypt()
{
	return encrypt;
}

void Message::setterEncrypt(bool encrypt)
{ 
	this->encrypt = encrypt;
}


