#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <mraa/aio.h>
#include <signal.h>

int logfd;
char time_buf[10];
char l_buf[40];
time_t t;
float temp;
int timer = 60;
struct tm *timestamp;
uint16_t value;
float R;
mraa_aio_context temp_sensor;

void SIGINT_handler() { timer = 0; }

void get_timestamp() {
	t = time(NULL);
	timestamp = localtime(&t);
	if (strftime(time_buf, sizeof(time_buf), "%H:%M:%S", timestamp) == 0) {
		fprintf(stderr, "strftime returned 0");
		exit(EXIT_FAILURE);
	}
}

void write_to_log() {
	get_timestamp();
	snprintf(l_buf, sizeof(l_buf), "%s TEMP=%2.1f\n", time_buf, temp);
	write(logfd, l_buf, strlen(l_buf));
	write(STDOUT_FILENO, l_buf, strlen(l_buf));
}

void get_temp() {
	value = mraa_aio_read(temp_sensor);
	R = 1023.0/((float)value)-1.0;
    R = 100000.0*R;
    temp = 1.0/(log(R/100000.0)/4275+1/298.15)-273.15;
    temp = temp * (9.0/5.0) + 32.0;
}

int main(int argc, char** argv) {
	signal(SIGINT, SIGINT_handler);
	if((logfd = creat("log4a", 0666)) == -1) {
		fprintf(stderr, "Error: create log file (part 1)\n");
		exit(EXIT_FAILURE);
	}
	temp_sensor = mraa_aio_init(0);
	while(timer--) {
		get_temp();
		write_to_log();
		sleep(1);
	}
	mraa_aio_close(temp_sensor);
	close(logfd);
	return 0;
}       
