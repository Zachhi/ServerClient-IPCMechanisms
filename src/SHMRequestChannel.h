#ifndef SMHREQUESTCHANNEL_H
#define SMHREQUESTCHANNEL_H

#include "RequestChannel.h"
#include <semaphore.h>
#include <sys/mman.h>
using namespace std;

class SHMQueue
{
	private:
		char* shmbuffer;
		sem_t* writerdone;
		sem_t* readerdone;
		string name;
		int length;

	public:
		SHMQueue (string _name, int _len ): name(_name), length (_len)
		{
            //create SHM segment and connect to it
			int fd = shm_open(name.c_str(), O_RDWR|O_CREAT, 0600);

			if (fd < 0) 
				EXITONERROR ("could not create shared memory seg");
				
            //set length, default is 0
			ftruncate(fd, length);  

			shmbuffer = (char *) mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

			if (!shmbuffer)
				EXITONERROR("could not map the buffer");

            //create the 2 semaphores
			readerdone = sem_open((name + "_rd").c_str(), O_CREAT, 0600, 1);
			writerdone = sem_open((name + "_sd").c_str(), O_CREAT, 0600, 0);
		}

        ~SHMQueue ()
		{
			sem_close (writerdone);
			sem_close (readerdone);
			sem_unlink ((name + "_rd").c_str());
			sem_unlink ((name + "_sd").c_str());

			munmap (shmbuffer, length);
			shm_unlink (name.c_str());
		}

		int cwrite(void* msg, int length)
		{
			sem_wait(readerdone);
			memcpy(shmbuffer, msg, length);
			sem_post (writerdone);
			return length;
		}

		int cread(void* msg, int length)
		{
			sem_wait(writerdone);
			memcpy(msg, shmbuffer, length);
			sem_post (readerdone);
			return length;
		}
};


class SHMRequestChannel: public RequestChannel
{
private:

	SHMQueue* shared1;
	SHMQueue* shared2;
	int length;
	
public:

	SHMRequestChannel(const string _name, const Side _side, int _len);
	~SHMRequestChannel();

	int cread (void* msgbuf, int bufcapacity);
	int cwrite(void *msgbuf , int msglen);
};

#endif