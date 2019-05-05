TCP client server with daemon producers based on shared memory.

The project was developed using Cygwin 64bits (GNU and Open Source tools).

The project use TCP sockets to connect several client sections
to server.
First, server initializes two daemon process that constantly produce 
information about server CPU processes and server memory available.
Independent daemon services consults server resources each 3 seconds 
and store information in its shared memory. The writing (by daemon)
and reading (by server) synchronism in shared memory is controled by 
semaphore mechanism.
Then server configure and initialize main TCP socket and stay listen
to client connections.
In another terminals many clients (using client.exe or telnet) are
initialized and start requesting information to server.
The tests was done using 10 sections (been 5 clients.exe and 5 telnet 
sections) simultaneously communicating with server.

Included files in GitHub repository:
- server.cpp
- server.exe
- client.cpp
- client.exe
- daemon_server_infomemd.cpp
- server_infomemd.exe
- daemon_server_processtop10d.cpp
- server_processtop10d.exe
- Makefile
- README.txt


Project compilation. First clean process and older files and the make.
Expected result:
$ make clean
rm server.exe client.exe server_processtop10d.exe server_infomemd.exe
rm /dev/shm/posix-shared-mem-top10ps /dev/shm/sem.sem-mutex-top10ps
rm /dev/shm/posix-shared-mem-infomem /dev/shm/sem.sem-mutex-infomem
pkill -f server_processtop10d
pkill -f server_infomemd

$ make
g++ -o server.exe server.cpp -lrt -lpthread
g++ -o client.exe client.cpp -lrt -lpthread
g++ -o server_processtop10d.exe daemon_server_processtop10d.cpp -lrt -lpthread
g++ -o server_infomemd.exe daemon_server_infomemd.cpp -lrt -lpthread

--------
--Program server.exe
--------
In one terminal, run project server (from Cygwin prompt):
$ ./server.exe
daemon server_infomemd initialized!
daemon server_processtop10d_cmd initialized!
Listener on port 7000
Waiting for connections ...

When client connections are asked, server accepted, bind and log then. 
An example of server prompt with 3 granted connections:
$ ./server.exe
daemon server_infomemd initialized!
daemon server_processtop10d_cmd initialized!
Listener on port 7000
Waiting for connections ...
New client connection, socket fd is 4, ip is : 127.0.0.1, port : 49784
Welcome message sent successfully for new client
Adding to list of sockets as client 0
New client connection, socket fd is 5, ip is : 127.0.0.1, port : 49786
Welcome message sent successfully for new client
Adding to list of sockets as client 1
New client connection, socket fd is 6, ip is : 127.0.0.1, port : 49788
Welcome message sent successfully for new client
Adding to list of sockets as client 2

--------
--Client section using program client.exe 
--------
In next terminals, run several sections of program client.exe.
The first line parameter passed to program client.exe indicates the 
number of iterations. 
Each client iteration makes 3 requests to server in order to test communication:
1) First, an incorrect or invalid request, called "teste".
2) Second, a request for 10 top processes consuming server CPU, called "-server_processtop10".
3) Third, a request for server memory status, called "-server_infomem".

From Cygwin prompt, an example of first iteration:
$ ./client.exe 200
Server OK and listening

teste message sent
Invalid client request, use commands -server_processtop10 or -server_infomem
server buffer size received = 77
-server_processtop10 message sent
  PID USUARIO   PR  NI    VIRT    RES  %CPU  %MEM    TEMPO+ S COMANDO
 3110 ir         8   0    5,6m   4,8m  22,4   0,1   0:00.07 R top
 2921 ir         8   0    4,8m   4,2m   0,0   0,1   0:00.09 S server_infomemd
 3103 ir         8   0    4,6m   3,9m   0,0   0,0   0:00.01 S client
 1057 ir         8   0    6,4m   8,4m   0,0   0,1   0:00.04 S bash
 2925 ir         8   0    4,8m   4,2m   0,0   0,1   0:00.01 S server_processtop10d
  994 ir         8   0    7,3m  12,9m   0,0   0,2   0:02.32 S mintty
 3109 ir         8   0    5,0m   5,7m   0,0   0,1   0:00.04 S sh
 1111 ir         8   0    6,4m   8,1m   0,0   0,1   0:00.04 S bash
 1056 ir         8   0    8,0m  13,3m   0,0   0,2   0:10.48 S mintty
  995 ir         8   0    6,6m   8,8m   0,0   0,1   0:00.18 S bash
server buffer size received = 770
-server_infomem message sent
              total       usada       livre    compart.  buff/cache  disponível
Mem.:          8095        3233        4862           0           0        4862
Swap:          7424         158        7265
Total:        15519        3392       12127
server buffer size received = 249


--------
--Client section using program telnet 
--------
In next terminals, run several sections of program telnet.
The first line parameter passed is localhost and the second is the port.

From Cygwin prompt, an example using telnet:
$ telnet localhost 7000
Trying ::1...
Connection failed: Connection refused
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
Server OK and listening
-server_processtop10
  PID USUARIO   PR  NI    VIRT    RES  %CPU  %MEM    TEMPO+ S COMANDO
 9133 ir         8   0    5,6m   4,8m  22,4   0,1   0:00.10 R top
 9119 ir         8   0    4,8m   4,2m   0,0   0,1   0:00.00 S server_processtop10d
 9115 ir         8   0    4,8m   4,2m   0,0   0,1   0:00.01 S server_infomemd
 1057 ir         8   0    6,4m   8,4m   0,0   0,1   0:00.06 S bash
 9124 ir         8   0    4,6m   4,7m   0,0   0,1   0:00.01 S telnet
  994 ir         8   0    7,3m  13,0m   0,0   0,2   0:02.90 S mintty
 9111 ir         8   0    4,8m   4,4m   0,0   0,1   0:00.01 S server
 1111 ir         8   0    6,4m   8,1m   0,0   0,1   0:00.04 S bash
 1056 ir         8   0    8,1m  13,4m   0,0   0,2   0:11.39 S mintty
  995 ir         8   0    6,6m   8,8m   0,0   0,1   0:00.18 S bash
-server_infomem
              total       usada       livre    compart.  buff/cache  disponível
Mem.:          8095        3246        4849           0           0        4849
Swap:          7424         191        7232
Total:        15519        3438       12081


