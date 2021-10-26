#ifndef MQREQUESTCHANNEL_H
#define MQREQUESTCHANNEL_H

#include "common.h"
#include "RequestChannel.h"

class MQRequestChannel : public RequestChannel
{
private:
	int length;

public:

	MQRequestChannel(const string _name, const Side _side, int buffercapacity);
	~MQRequestChannel();

	int cread (void* msgbuf, int bufcapacity);
	int cwrite (void *msgbuf , int msglen);
 
	int openIPC(string pipeName, int m);
};

#endif