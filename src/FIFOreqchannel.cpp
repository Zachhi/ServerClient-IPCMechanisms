#include "FIFOreqchannel.h"
using namespace std;

FIFORequestChannel::FIFORequestChannel(const string _name, const Side _side) : RequestChannel (_name, _side){
	pipe1 = "fifo_" + myName + "_1";
	pipe2 = "fifo_" + myName + "_2";
		
	if (_side == SERVER_SIDE){
		wfd = openIPC(pipe1, O_WRONLY);
		rfd = openIPC(pipe2, O_RDONLY);
	}
	else{
		rfd = openIPC(pipe1, O_RDONLY);
		wfd = openIPC(pipe2, O_WRONLY);
		
	}
}

FIFORequestChannel::~FIFORequestChannel(){ 
	close(wfd);
	close(rfd);

	remove(pipe1.c_str());
	remove(pipe2.c_str());
}

int FIFORequestChannel::openIPC(string _pipe_name, int mode){
	mkfifo (_pipe_name.c_str (), 0600);
	int fd = open(_pipe_name.c_str(), mode);
	if (fd < 0){
		EXITONERROR(_pipe_name);
	}
	return fd;
}

int FIFORequestChannel::cread(void* msgbuf, int bufcapacity){
	return read (rfd, msgbuf, bufcapacity); 
}

int FIFORequestChannel::cwrite(void* msgbuf, int len){
	return write (wfd, msgbuf, len);
}

