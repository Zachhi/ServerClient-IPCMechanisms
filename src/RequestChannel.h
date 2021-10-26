#ifndef REQUESTCHANNEL_H
#define REQUESTCHANNEL_H
#include "common.h"

class RequestChannel
{
public:
    typedef enum {SERVER_SIDE, CLIENT_SIDE} Side;
    typedef enum {READ_MODE, WRITE_MODE} Mode;

//make these variables common across all IPC methods
protected:
    string myName;
    Side mySide;
    int wfd;
	int rfd;
	string pipe1, pipe2;
	int openIPC(string pipeName, int m){}

//each ipc method will have the functions below
public:
    /* CONSTRUCTOR/DESTRUCTOR */
    RequestChannel (const string _name, const Side _side): myName(_name), mySide(_side) {}
    virtual ~RequestChannel(){} /* destruct operation should be derived class-specific */

    virtual int cread (void* msgbuf, int bufcapacity) = 0; /* Blocking read; returns the number of bytes read.If the read fails, it returns -1. */
    virtual int cwrite (void* msgbuf, int bufcapacity) = 0; /* Write the data to the channel. The function returnsthe number of characters written, or -1 when it fails */
};

#endif 