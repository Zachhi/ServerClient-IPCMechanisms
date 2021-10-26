#include "MQRequestChannel.h"
#include <mqueue.h>
using namespace std;

//this will basically follow the exact same structure as FIFOreqchannel, with slight differences
MQRequestChannel::MQRequestChannel(const string _name, const Side _side, int buffercapacity) : RequestChannel (_name, _side){
    //use a '/' otherwise mq_open will fail. mq's need a forward slash before their name
	length = buffercapacity;
	pipe1 = "/queue_" + myName + "_1";
	pipe2 = "/queue_" + myName + "_2";
	
    //msg queue is bidirectional
	if (_side == SERVER_SIDE){
		wfd = openIPC(pipe1, O_RDWR | O_CREAT);
		rfd = openIPC(pipe2, O_RDWR | O_CREAT);
	}
	else{
		rfd = openIPC(pipe1, O_RDWR | O_CREAT);
		wfd = openIPC(pipe2, O_RDWR | O_CREAT);
	}
}

MQRequestChannel::~MQRequestChannel(){ 
    //destructor will close wfd, rfd, and unlink the pipes
    //use the special mq_close() instead of just close() from man pages
	mq_close(wfd);
	mq_close(rfd);

    //use the special mq_unlink instead of remove() from man pages
	mq_unlink(pipe1.c_str());
	mq_unlink(pipe2.c_str());
}

int MQRequestChannel::openIPC(string _pipe_name, int m){
	//mkfifo (_pipe_name.c_str (), 0600);
	//int fd = open(_pipe_name.c_str(), mode);
	struct mq_attr queueAttributes;
		queueAttributes.mq_curmsgs = 0;
		queueAttributes.mq_maxmsg = 1;
		queueAttributes.mq_flags = 0;
		queueAttributes.mq_msgsize = length;

    int fd = mq_open(_pipe_name.c_str(), O_RDWR | O_CREAT, 0600, &queueAttributes);

	if (fd < 0)
		EXITONERROR(_pipe_name);

	return fd;
}

int MQRequestChannel::cread(void* msgbuf, int bufcapacity){
    //mq_recieve instead of read, from man pages
	return mq_receive(rfd, (char*)msgbuf, 8192, NULL);
}

int MQRequestChannel::cwrite(void* msgbuf, int len){
    //mq_send instead of write, from man pages
	return mq_send(wfd, (char*)msgbuf, len, 0);
}