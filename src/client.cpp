#include "common.h"
#include "FIFOreqchannel.h"
#include "MQRequestChannel.h"
#include "SHMRequestChannel.h"
#include "RequestChannel.h"
#include <sys/wait.h> //so we can use wait()

using namespace std;

int main(int argc, char *argv[]){

	int opt; 		
	int p = -1; 	//start p out as negative, that way if it is negative later on we know we are not requesting data points
	double t = -1.0; //time/starting time for the data point(s) requested
	int e = 1;		//ecg no
	int numChan = 1;

	int bufferSize = MAX_MESSAGE; //if -m is an argument, we can change this variable. Otherwise the default is MAX_MESSAGE
	char *serverBufferSize = "256"; //this is to give the correct buffer size to the server. Must be a char since we pass it in to server through a char *args[]

	string filename = "";    //name of the file the client wants
	string chanType = "f";

	bool newChannel = false; //track if client wants a new channel or not

	//----------GETTING INFO FROM COMMAND LINE-------------------
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c:i:")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
			case 'm':
				bufferSize = atoi(optarg);
				serverBufferSize = optarg; //keep at char*
				break;
			case 'c':
				newChannel = true;
				numChan = atoi(optarg);
				break;
			case 'i':
				chanType = optarg;
				break;
		}
	}

	int id = fork(); //creating child process
	//if id = 0, this is the server process. client runs server first, then connects to server
	if (id == 0)
	{
		char *args[]{"", "-m", serverBufferSize, "-i", (char*)chanType.c_str(), nullptr};
		execvp("./server", args); //executing ./server -m bufsize in a new terminal
	}
	//else, we run the client process
	else
	{
		//create vector of requestchannels so we can store and track all the channels we create
		vector <RequestChannel *> chanVector;

		//create a requestChannel, then depending on the value of i (m, q, or f), use the correct channel type
		//default to FIFO channel. If i doesnt match either m, q, or f, just assume it is f
		RequestChannel* mainChannel = nullptr;
		if(chanType == "s")
		{
			mainChannel = new SHMRequestChannel ("control", SHMRequestChannel::CLIENT_SIDE, bufferSize); //open shared memory channel
			cout << "Using shared memory IPC mechanism..." << endl;
		}
		else if(chanType == "q")
		{
			mainChannel = new MQRequestChannel ("control", MQRequestChannel::CLIENT_SIDE, bufferSize); //open message queue channel
			cout << "Using message queue IPC mechanism..." << endl;
		}
		else 
		{
			mainChannel = new FIFORequestChannel ("control", FIFORequestChannel::CLIENT_SIDE); //open FIFO channel by default
			cout << "Using FIFO IPC mechanism..." << endl;
		}
		RequestChannel* currChan = mainChannel;
		chanVector.push_back(currChan);

		//start the time after making the server a child and after oppening the client side server
		struct timeval startTime, endTime; //help track starting and ending time
		gettimeofday(&startTime, nullptr); //save the current time into startTime

		//loop through for the amount of channels the client want's
		//create the amount of desired channels and add them to chanVector
		for(int i = 0; i < numChan; i++)
		{
			//--------------REQUESTING NEW CHANNEL----------
			//if new channel is true, we need to set 'chan' from the control channel to a new channel
			//if not, keep 'chan' as the control channel since we will be using that to complete requests instead
			//of new channels
			//if -c was included, create a new channel
			if(newChannel)
			{
				MESSAGE_TYPE requestedChannelMsg = NEWCHANNEL_MSG;   //sending a new channel request
				mainChannel->cwrite (&requestedChannelMsg, sizeof (MESSAGE_TYPE));

				char chanName [bufferSize]; //create char array of buffer size
				mainChannel->cread(chanName, bufferSize); //getting the channel name into the char array

				if(chanType == "s")
					currChan = new SHMRequestChannel(chanName, RequestChannel::CLIENT_SIDE, bufferSize); //open shared memory channel
				else if(chanType == "q")
					currChan = new MQRequestChannel (chanName, RequestChannel::CLIENT_SIDE, bufferSize); //open message queue channel
				else 
					currChan = new FIFORequestChannel (chanName, RequestChannel::CLIENT_SIDE); //open FIFO channel by default

				cout << "New channel '" << chanName << "' created" << endl;
				chanVector.push_back(currChan);
			}
		}

			//-------------------REQUESTING DATA----------------------
			//if p >= 0, we are requesting data points
			if(p >= 0)
			{
				//if t is negative, we are requesting 1000 points of data
				if(t < 0)
				{
					cout << "1000 points of data requested for person " << p << endl;
					double currTime = 0; //if requesting 1000 points of data, start the time at 0 by default
					std::ofstream x1csv;
					x1csv.open ("x1.csv");
					if(x1csv.is_open()) //make sure the file opens before doing anything
					{
						if(newChannel) //if new channel we need to evenley split the transfer for all channels except main/control channel
						{
							int s = 1000;
							int c = chanVector.size()-1;
							int numBytes = ceil(s/c);
							int currByte = 0;
							for(int j = 1; j < chanVector.size(); j++)
							{
								currChan = chanVector[j];
								for(int i = 0; i < numBytes; i++) //loop 1000 times
								{	
									if(currByte < 1000)
									{	
										datamsg ecgMsg1 (p, currTime, 1);  //make a datamsg for both ecg1 and ecg2
										datamsg ecgMsg2 (p, currTime, 2);
										double ecgVal1, ecgVal2; //the actual ecg values

										currChan->cwrite (&ecgMsg1, sizeof (datamsg)); //write the msg to server
										int nbytes = currChan->cread (&ecgVal1, sizeof(double));  //read in the ecgVal
										//cout << "For person " << p <<", at time " << currTime << ", the value of ecg 1 is " << ecgVal1; //print ecg1

										currChan->cwrite (&ecgMsg2, sizeof (datamsg)); 
										nbytes = currChan->cread (&ecgVal2, sizeof(double)); 
										//cout << " and the value of ecg 2 is " << ecgVal2 << endl; //print ecg2

										x1csv << currTime << "," << ecgVal1 << "," << ecgVal2 << "\n"; //write the time, ecg1, ecg2 into x1.csv

										currTime += 0.004; //increase the time to go to the next data point
										currByte += 1;
									}
								}
							}
						}
						else //if no new channel just use the main channel
						{
							currChan = mainChannel;
							for(int i = 0; i < 1000; i++) //loop 1000 times
							{		
								datamsg ecgMsg1 (p, currTime, 1);  //make a datamsg for both ecg1 and ecg2
								datamsg ecgMsg2 (p, currTime, 2);
								double ecgVal1, ecgVal2; //the actual ecg values

								currChan->cwrite (&ecgMsg1, sizeof (datamsg)); //write the msg to server
								int nbytes = currChan->cread (&ecgVal1, sizeof(double));  //read in the ecgVal
								//cout << "For person " << p <<", at time " << currTime << ", the value of ecg 1 is " << ecgVal1; //print ecg1

								currChan->cwrite (&ecgMsg2, sizeof (datamsg)); 
								nbytes = currChan->cread (&ecgVal2, sizeof(double)); 
								//cout << " and the value of ecg 2 is " << ecgVal2 << endl; //print ecg2

								x1csv << currTime << "," << ecgVal1 << "," << ecgVal2 << "\n"; //write the time, ecg1, ecg2 into x1.csv

								currTime += 0.004; //increase the time to go to the next data point
							}
						}
						x1csv.close(); //close file
					}
				}
				else //else we are only requesting one data point
				{
					datamsg x (p, t, e); //data msg for the one data point
					currChan->cwrite (&x, sizeof (datamsg)); // write msg to server
					double reply;
					int nbytes = currChan->cread (&reply, sizeof(double)); // read in the reply
					cout << "For person " << p <<", at time " << t << ", the value of ecg "<< e <<" is " << reply << endl; //print the data
				}
			}

			//--------------REQUESING FILES--------------
			//if filename isn't empty, we are requesting a file
			if(filename != "")
			{
				cout << "Transferring BIMDC/" << filename << " to recieved/" << filename << endl; 
				filemsg fm (0,0); //declaring a filemsg with (__int64_ offset, int length) 
				string fname = filename; //get the file name that we recieved from the command line
			
				//getting length of file
				int len = sizeof (filemsg) + fname.size()+1; 
				char buf2 [len];
				__int64_t fileLength;
				memcpy (buf2, &fm, sizeof (filemsg));
				strcpy (buf2 + sizeof (filemsg), fname.c_str());
				currChan->cwrite (buf2, len);  // I want the file length;
				currChan->cread (&fileLength, sizeof (__int64_t));

				char buf [bufferSize]; // 256

				//creating the new file in /recieved and making it binary
				std::fstream copiedFile ("./recieved/"+fname,ios::binary|ios::out);

				if(newChannel)
				{
					int c = chanVector.size()-1;
					__int64_t numBytes = ceil(fileLength / c);
					int msgLength = bufferSize;
					int msgOffset = 0;

					for(int i = 1; i < chanVector.size(); i++)
					{
						currChan = chanVector[i];
						while(msgOffset < numBytes && msgOffset < fileLength)
						{
								//ensuring that we don't go out of bounds. If offset+length is greater then the file length, get the remainder of length
								if(msgOffset + msgLength > fileLength)
								{
									msgLength = fileLength % msgLength;
								}

								//making a temp filemsg so we can transfer later, with the correct offset and length for this iteration
								filemsg holder(msgOffset, msgLength);
								int holderLen = sizeof (filemsg) + fname.size() + 1;
								char holderBuf [holderLen];
								memcpy (holderBuf, &holder, sizeof(filemsg));
								strcpy (holderBuf + sizeof (filemsg), fname.c_str());
								currChan->cwrite(holderBuf, holderLen);
								char holderChar[msgLength];
								currChan->cread(holderChar, sizeof(holderChar));

								copiedFile.write(holderChar, msgLength);

								msgOffset += msgLength; //increase offset by the length of each chunk

						}
						numBytes += numBytes;
					}
				}
				else
				{
					//now we need to write to the new file
					int msgLength = bufferSize;
					int msgOffset = 0;

					while(msgOffset < fileLength)
					{
						//ensuring that we don't go out of bounds. If offset+length is greater then the file length, get the remainder of length
						if(msgOffset + msgLength > fileLength)
						{
							msgLength = fileLength % msgLength;
						}

						//making a temp filemsg so we can transfer later, with the correct offset and length for this iteration
						filemsg holder(msgOffset, msgLength);
						int holderLen = sizeof (filemsg) + fname.size() + 1;
						char holderBuf [holderLen];
						memcpy (holderBuf, &holder, sizeof(filemsg));
						strcpy (holderBuf + sizeof (filemsg), fname.c_str());
						currChan->cwrite(holderBuf, holderLen);
						char holderChar[msgLength];
						currChan->cread(holderChar, sizeof(holderChar));

						copiedFile.write(holderChar, msgLength);

						msgOffset += msgLength; //increase offset by the length of each chunk
					}
				}
				copiedFile.close();
				cout << filename << " has been transferred" << endl;
			}


		

		//before closing the channel, lets get the total time taken
		gettimeofday(&endTime, nullptr);
		double totalTime = (endTime.tv_sec - startTime.tv_sec) * 1e6; //get seconds
		totalTime = (totalTime + (endTime.tv_usec - startTime.tv_usec)) * 1e-6; //add previous with micro seconds
		cout << "This took " << totalTime << " seconds" << endl; //now we have accuracy of 6 decimals

		//---------------CLOSING CHANNEL----------------   
		MESSAGE_TYPE m = QUIT_MSG;
		for(int i = chanVector.size()-1; i > -1; i--)
		{
			currChan = chanVector[i];
			currChan->cwrite(&m, sizeof(MESSAGE_TYPE));
			chanVector.pop_back();
			delete currChan;
		}

		wait(nullptr); //allows program to wait for all processes to finish
	}
}