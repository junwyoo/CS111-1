#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/stat.h>
#include <time.h>
#include <mraa/aio.h>
#include <math.h>

#define BUF_SIZE 16
#define DEFAULT_PORT_NUM 16000
#define ID "304184918"

float temp;
char buf[BUF_SIZE];
char s_buf[20];
char l_buf[40];
char time_buf[10];
time_t t;
struct tm *timestamp;
int port_num, sockfd, logfd;
struct sockaddr_in serv_addr;
struct hostent *server;	
char message[] = "Port request 304184918\n";
struct termios saved_attr, new_attr;
int freq = 3;
char scale_mode = 'F';
int start_stop = 1;
uint16_t value;
float R;
mraa_aio_context temp_sensor;

void exit_function() {
	freq = 3;
	scale_mode = 'F';
	start_stop = 1;
	close(sockfd);
	close(logfd);
}

void SIGINT_handler() { exit(1); }

void get_temp() {
	value = mraa_aio_read(temp_sensor);
	R = 1023.0/((float)value)-1.0;
    R = 100000.0*R;
    temp = 1.0/(log(R/100000.0)/4275+1/298.15)-273.15;
    if(scale_mode == 'F')
    	temp = temp * (9.0/5.0) + 32.0;
}

void get_timestamp() {
	t = time(NULL);
	timestamp = localtime(&t);
	if (strftime(time_buf, sizeof(time_buf), "%H:%M:%S", timestamp) == 0) {
		fprintf(stderr, "strftime returned 0");
		exit(EXIT_FAILURE);
	}
}

void write_temp() {
	get_temp();
	snprintf(s_buf, sizeof(s_buf) + 1, "%s TEMP=%2.1f\n", ID, temp);
	write(sockfd, s_buf, strlen(s_buf));
	write(STDOUT_FILENO, "SENT TO SERVER: ", 16);
	write(STDOUT_FILENO, s_buf, strlen(s_buf));
	get_timestamp();
	snprintf(l_buf, sizeof(l_buf), "%s %2.1f %c\n", time_buf, temp, scale_mode);
	write(logfd, l_buf, strlen(l_buf));
}

void log_command(int valid) {
	write(STDOUT_FILENO, buf, 16);
	if(valid == 1)
		write(STDOUT_FILENO, " I", 2);
	write(STDOUT_FILENO, "\n", 1);
	write(logfd, buf, 16);
	if(valid == 1)
		write(logfd, " I", 2);
	write(logfd, "\n", 1);
	memset(&buf[0], 0, sizeof(buf));
}

void process_command() {
	char substring[5];
	int str_len = strlen(buf);
	if(strncmp(buf,"OFF", str_len) == 0) {
		log_command(0);
		exit(0);
	} else if(strncmp(buf,"STOP", str_len) == 0) {
		start_stop = 0;
		log_command(0);
	} else if(strncmp(buf,"START", str_len) == 0) {
		start_stop = 1;
		log_command(0);
	} else if(strncmp(buf,"SCALE=", 6) == 0) {
		if(str_len != 7 || (buf[6] != 'C' && buf[6] != 'F'))
			log_command(1);
		else {
			scale_mode = buf[6];
			log_command(0);
		}
	} else if(strncmp(buf,"FREQ=", 5) == 0) {	
		if(str_len > 9)
			log_command(1);
		else {
			strncpy(substring, buf+5, str_len-5);
			if(atoi(substring)  == 0)
				log_command(1);
			else {
				freq = atoi(substring);
				log_command(0);
			}
		}
	} else if(strncmp(buf,"DISP ", 5) == 0) {
		if(str_len != 6 || (buf[5] != 'Y' && buf[5] != 'N'))
			log_command(1);
		else 
			log_command(0);
	} else {
		log_command(1);
	}
}

void* temp_function(void* unused) {
	while(1) {
		while(start_stop) {
			sleep(freq);
			if(start_stop) 
				write_temp();
		}
	}
	return NULL;
}

void open_socket(int p_num) {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		perror("ERROR opening socket");
	server = gethostbyname("lever.cs.ucla.edu");
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(EXIT_FAILURE);
	}
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *) &serv_addr.sin_addr.s_addr, (char*) server->h_addr, server->h_length);
	serv_addr.sin_port = htons(p_num);
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
		perror("ERROR connecting");
}

int main(int argc, char** argv) {
	int bytes_read;
	atexit(exit_function);
	signal(SIGINT, SIGINT_handler);

 	if((logfd = creat("log4b", 0666)) == -1) {
		fprintf(stderr, "Error: cannot create log file (part 2)\n");
		exit(EXIT_FAILURE);
	}

	open_socket(DEFAULT_PORT_NUM);
	write(sockfd, message, strlen(message));
	read(sockfd, &port_num, sizeof(port_num));
	close(sockfd);

	open_socket(port_num);
	printf("Connected to port number %d\n", port_num);
	temp_sensor = mraa_aio_init(0);

	pthread_t c_thread;
	pthread_create(&c_thread, NULL, temp_function, NULL);

	while(1) {
		bytes_read = read(sockfd, buf, 16);
		if(bytes_read > 0) 
			process_command();
	}
	mraa_aio_close(temp_sensor);
}




