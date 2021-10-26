# ServerClient-IPCMechanisms

Implementation of a server client program that speaks to each other using one of three possible IPC mechanisms. The client connects to the server, who defines a 
communication protocol, and then the client sends properly formulated messages over a communication pipe. 

The server hosts electocardiogram points of 15 patients suffering from various cardiac diseases. The client is able to request and recieve data points in different formats.
The client is also able to request a file transfer, where the server will send several segments depedning on the buffer size to a new file. The file will be completley copied
and transferred to a new directory after this process is done.

I just created this repository so I could have this project on my main github account

## Demo

https://www.youtube.com/watch?v=sT9SZdZqSJE

### Commands

The format is `./client -p -t -e -m -f -c -i
where
* p = person
* t = time
* e = ecg number
* m = buffer size
* f = filename
* c = channels
* i = IPC method

#### Examples
* So if you want to request the data point for person 1, at time 4, for ecg 1 you would run `./client -p 1 -t 4 -e 1`
* Request 1000 data points for person 1, run `./client -p 1`, just dont include the time
* Request a file transfer run `./client -f <filename>`
* Request a file transfer with a buffer size of 2048, run `./client -f <filename> -m 2048`
* Request anything using 10 channels run `./client <extra commands> -c 10`
* Request anything using a specified IPC method run
  * `./client <extra commands> -i f` for FIFO named pipes IPC method
  * `./client <extra commands> -i q` for message queue IPC method
  * `./client <extra commands> -i q` for shared memory IPC method
  


### Dependencies

* Some way to compile and execute c++ code
* GNU Make (Not required, but makes executing easier)

### Installing and Executing

* Download the source code from github, or clone the repository into Visual Studio
* Type `make` and then execute `./client` with the desired commands after "./client"
* You will want to do this in a virtual enviroment if you edit the files, otherwise you may compromise your computer's health

## Authors

Zachary Chi
zachchi@tamu.edu

## License

This project is licensed under the GNU General Public License v2.0 - see the LICENSE.md file for details
