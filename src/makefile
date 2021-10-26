# makefile

all: server client

common.o: common.h common.cpp
	g++ -g -w -std=c++11 -c common.cpp

FIFOreqchannel.o: FIFOreqchannel.h FIFOreqchannel.cpp
	g++ -g -w -std=c++11 -c FIFOreqchannel.cpp

MQRequestChannel.o: MQRequestChannel.h MQRequestChannel.cpp
	g++ -g -w -std=c++11 -c MQRequestChannel.cpp

SHMRequestChannel.o: SHMRequestChannel.h SHMRequestChannel.cpp
	g++ -g -w -std=c++11 -c SHMRequestChannel.cpp

client: client.cpp FIFOreqchannel.o MQRequestChannel.o SHMRequestChannel.o common.o
	g++ -g -w -std=c++11 -o client client.cpp FIFOreqchannel.o MQRequestChannel.o SHMRequestChannel.o common.o -lpthread -lrt

server: server.cpp FIFOreqchannel.o MQRequestChannel.o SHMRequestChannel.o common.o
	g++ -g -w -std=c++11 -o server server.cpp FIFOreqchannel.o MQRequestChannel.o SHMRequestChannel.o common.o -lpthread -lrt

clean:
	rm -rf *.o *.csv fifo* server client data*_*