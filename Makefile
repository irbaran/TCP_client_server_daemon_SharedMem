all: server.exe client.exe server_processtop10d.exe server_infomemd.exe

server.exe: server.cpp
	g++ -o $@ $< -lrt -lpthread

client.exe: client.cpp
	g++ -o $@ $< -lrt -lpthread

server_processtop10d.exe: daemon_server_processtop10d.cpp
	g++ -o $@ $< -lrt -lpthread

server_infomemd.exe: daemon_server_infomemd.cpp
	g++ -o $@ $< -lrt -lpthread

clean:
	rm server.exe client.exe server_processtop10d.exe server_infomemd.exe
	rm /dev/shm/posix-shared-mem-top10ps /dev/shm/sem.sem-mutex-top10ps
	rm /dev/shm/posix-shared-mem-infomem /dev/shm/sem.sem-mutex-infomem	
	pkill -f server_processtop10d
	pkill -f server_infomemd
