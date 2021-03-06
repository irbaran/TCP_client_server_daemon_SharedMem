/* daemon_server_processtop10d.cpp

Its a data producer daemon.
The daemon consults and stores the 10 processes that most consumes CPU in 
the server in a shared memory for inter process communication

Copyright (c) 2019, irbaran. 
All rights reserved.

Permission to use, copy, modify, and distribute this software for any purpose
with or without fee is hereby granted, provided that the above copyright
notice and this permission notice appear in all copies.

The software is provided "as is", without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability,
fitness for a particular purpose and noninfringement of third party rights. 
In no event shall the authors or copyright holders be liable for any claim,
damages or other liability, whether in an action of contract, tort or
otherwise, arising from, out of or in connection with the software or the use
or other dealings in the software.

Except as contained in this notice, the name of a copyright holder shall not
be used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization of the copyright holder.
 */

#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h>  
#include <netinet/in.h> 
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
using namespace std;

// Buffer data structures
#define MAX_BUFFERS 1
#define SEM_MUTEX_TOP10PS "/sem-mutex-top10ps"
#define SHARED_MEM_TOP10PS "/posix-shared-mem-top10ps"

struct shared_memory {
    char buf [MAX_BUFFERS] [1024];
    int buffer_index;
};

//Define functions
void create_daemon(void);
void log_file_error (const char *err_msg);
void get_stdout_cmd(char *cmd, char *filename, char (&buffer)[1024] );
static void server_processtop10d(void);


/////////////////////////
// Function create_daemon()
// Creates a deamon from current process
/////////////////////////
void create_daemon(void)
{
	// process pid_processtop10 that will become daemon
	pid_t pid_processtop10 = 0;

	/* Fork off the parent process */
	pid_processtop10 = fork();

	/* check fork */
	if (pid_processtop10 < 0)
	{
		/* When an error occurred */
		cout << "Error server_processtop10d: fork from parent failed!\n";
		exit(EXIT_FAILURE);
	}

	/* Let the parent process terminate */
	if (pid_processtop10 > 0)
		exit(EXIT_SUCCESS);

	/* Child process session become process leader */
	if (setsid() < 0)
	{
		/* An error occurred */
		cout << "Error server_processtop10d: Became child process as leader failed!\n";
		exit(EXIT_FAILURE);
	}
	
	/* Fork off for the second time to ensure that the daemon will not re-acquire a tty*/
	pid_processtop10 = fork();

	/* check fork */
	if (pid_processtop10 < 0)
	{
		/* When an error occurred */
		cout << "Error server_processtop10d: 2nd fork from parent failed!\n";
		exit(EXIT_FAILURE);
	}

	/* Let the parent (first child) terminate */
	if (pid_processtop10 > 0)
		exit(EXIT_SUCCESS);

	//unmask the file mode
	umask(0);
	
	/* Change the working directory to the root/tmp directory */
	chdir("/tmp");

	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	
	cout << "daemon server_processtop10d initialized!\n";
}

/////////////////////////
// Function log_file_error()
// Log an error msg in a file
/////////////////////////
void log_file_error (const char *err_msg)
{
	FILE *fp_top10= NULL;				// log file 
	time_t my_time = time(NULL);		// current time
	
	// Open a log file in case of errros
	fp_top10 = fopen ("Log_server_processtop10d.txt", "a+");
	fprintf(fp_top10, "Error server_processtop10d: %s - %s\n", err_msg, ctime(&my_time));
	fclose(fp_top10);
}

//////////////////////////
// Function get_stdout_cmd()
// It gets stdout from a command cmd captured in filename and returned in buffer parameter
/////////////////////////
void get_stdout_cmd(char *cmd, char *filename, char (&buffer)[1024] )
{
	int ret;                         // funtion return
	int file;                        // file descriptor
	int file_read_size;              // Number of bytes of file read
		
	ret = system(cmd);
	if ( ret == 0 )		
		log_file_error("system command");

	file = open(filename,O_RDONLY);
	if(file < 0)
		log_file_error("open file generated by system command");
	else
	{
		file_read_size = read(file, buffer, sizeof(buffer));
		if(file_read_size < 0)
			log_file_error("read from file generated by system command");
		close(file);
		
		// assume buffer size and include termination
		buffer[1021] == '\0';	
	}
}

//////////////////////////
// daemon server_processtop10d
// It consults and stores the 10 processes that most consumes 
// server CPU in a shared memory
/////////////////////////
static void server_processtop10d(void)
{
	string top_cmd = "top -b -n1 -Em -o+%CPU -s -w85 -c -1";   // command top options:
                                                               // -b  --Batch-mode operation
                                                               // -n1 --number-of-iterations equals 1
                                                               // -Em --show memory in megabytes
                                                               // -o+%CPU ---sort processes by CPU percentage use
                                                               // -s --enable secure-mode operation
                                                               // -w85 --define width of colums to show
                                                               // -c --show only program-name
                                                               // -1 --show cpu info in a single line
	string sed_qualifier = "sed -n '7,17 p'";                  // Use sed to cut desired lines from file
	string filename = "server_proctop10_filed.txt";            // File to store server processes information
	char *filename_c = const_cast<char*>(filename.c_str());    // cast string filename to char*	
	string cli_cmd = top_cmd + " | " + sed_qualifier + " > " + filename;  // Format string system command
	char *cli_cmd_c = const_cast<char*>(cli_cmd.c_str());      // cast string cli_cmd to char*
	char shm_buf[1024];                                        // Shared memory buffer
	struct shared_memory *shm_top10ps_ptr;                     // Shared memory struct
	sem_t *mutex_sem_top10ps;                                  // Semaphores descriptors
	int fd_shm_top10ps;                                        // Shared memory descriptor

	//Create a deamon from this process
	create_daemon();

	//  mutual exclusion semaphore, mutex_sem_top10ps, for shared memory initialization 
	if ((mutex_sem_top10ps = sem_open (SEM_MUTEX_TOP10PS, O_RDONLY, 0, 0)) == SEM_FAILED)
		log_file_error("open mutex_sem_top10ps");

	// Get shared memory 
	if ((fd_shm_top10ps = shm_open (SHARED_MEM_TOP10PS, O_RDWR, 0)) == -1)
		log_file_error("open shm_open");

	if ((shm_top10ps_ptr = (shared_memory*)mmap (NULL, sizeof (struct shared_memory), 
		PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm_top10ps, 0)) == MAP_FAILED)
		log_file_error("mapping shared memory");

	while (1) {
		/* Ask server SO about processes most consume CPU*/
		memset(shm_buf, 0, sizeof(shm_buf));
		get_stdout_cmd(cli_cmd_c, filename_c, shm_buf);
		shm_top10ps_ptr -> buffer_index = 0;
	
		/* Wait in case of server is reading shared memory.  */
		if (sem_wait (mutex_sem_top10ps) == -1)
			log_file_error("sem_wait: mutex_sem_top10ps");

		// start critical section
		sprintf (shm_top10ps_ptr -> buf [shm_top10ps_ptr -> buffer_index], "%s\n", shm_buf);
		// end critical section

		// Release mutex sem: V (mutex_sem_top10ps)
		if (sem_post (mutex_sem_top10ps) == -1)
			log_file_error("sem_post: mutex_sem_top10ps");

		sleep(3); /* time to constantly updated shared memory shm_top10ps_ptr*/
	}
	
	if (munmap (shm_top10ps_ptr, sizeof (struct shared_memory)) == -1)
		log_file_error("unmapping shared memory");
	
	exit(EXIT_SUCCESS);	
}


//////////////////////////
//Main function
/////////////////////////
int main(int argc, char* argv[])
{
	//Init daemon
	server_processtop10d();
	
	exit(EXIT_SUCCESS);
}