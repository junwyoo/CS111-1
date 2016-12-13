#include <stdio.h>
#include <stdlib.h>
/* for open */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
/* for getopt_long */
#include <getopt.h>
/* for signal */ 
#include <signal.h>
/* for FILENO */
#include <unistd.h>

#define MAX_SIZE 2048

void SIGSEGV_handler(int signum)
{
  fprintf(stderr, "SIGSEGV signal caught\n");
  exit(3);
}

int main (int argc, char **argv)
{
	int ifd, ofd;
	int c, option_index;
	char buf[MAX_SIZE];
	int ret_size;
	static int fault_flag = 0;

 	static struct option long_options[] = 
 	{ 
 		{"catch", no_argument,  NULL, 'c'},
 		{"segfault", no_argument, &fault_flag, 's'},
 		{"input", required_argument, NULL, 'i'},
 		{"output", required_argument, NULL, 'o'},
 		{0, 0, 0, 0},
 	};

 	while((c = getopt_long(argc, argv, "csi:o:", long_options, &option_index)) != -1)
 	{
		switch (c)
		{
			case 'c':
				signal(SIGSEGV, SIGSEGV_handler);
			   	break;
			case 'i':
				ifd = open(optarg, O_RDONLY);
				if (ifd >= STDIN_FILENO) {
					close(STDIN_FILENO);
					dup(ifd);
					close(ifd);
				} else {
					fprintf(stderr, "Unable to open the input file: %s\n", optarg);
					perror("Unable to open the input file");
					exit(1);
				}
				break;
			case 'o':
				ofd = creat(optarg, 0666);
				if (ofd >= STDIN_FILENO) {
					close(STDOUT_FILENO);
					dup(ofd);
					close(ofd);
				} else {
					fprintf(stderr, "Unable to create the output file: %s\n", optarg);
					perror("Unable to create the output file");
					exit(2);
				}
				break;
		}
	}

	if(fault_flag == 115)
	{
		char *segfault = NULL;
		*segfault = 'c';
	}

	while((ret_size = read(STDIN_FILENO, buf, MAX_SIZE)) > 0)
		write(STDOUT_FILENO, buf, ret_size);

	exit(0);
}
 


