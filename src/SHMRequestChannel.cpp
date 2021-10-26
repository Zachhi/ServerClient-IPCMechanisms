#include "common.h"
#include "SHMRequestChannel.h"

using namespace std;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR FOR CLASS   R e q u e s t C h a n n e l  */
/*--------------------------------------------------------------------------*/

SHMRequestChannel::SHMRequestChannel(const string _name, const Side _side, int _len) : RequestChannel (_name, _side)
{
	pipe1 = "/sharedMem_" + myName + "_1";
	pipe2 = "/sharedMem_" + myName + "_2";
	length = _len;
	shared1 = new SHMQueue (pipe1, length);
	shared2 = new SHMQueue (pipe2, length);
	
	if (mySide == RequestChannel::CLIENT_SIDE)
		swap (shared1, shared2);
    
}

SHMRequestChannel::~SHMRequestChannel()
{ 
	delete shared1;
	delete shared2;
}

int SHMRequestChannel::cread(void* msgbuf, int bufcapacity)
{
	return shared1->cread(msgbuf, bufcapacity); 
}

int SHMRequestChannel::cwrite(void* msgbuf, int len)
{
	return shared2->cwrite(msgbuf, len);
}