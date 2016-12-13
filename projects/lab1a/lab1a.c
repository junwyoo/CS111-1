#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
// #include <sys/types.h>
// #include <sys/uio.h>

#define BUF_SIZE 1

static int shell_flag = 0;
int shell_pid;
struct termios saved_attr, new_attr;
char cr_lf[2] = {0x0D, 0x0A};


void reset_input_mode (void)
{
  tcsetattr (STDIN_FILENO, TCSANOW, &saved_attr);
}

void set_input_mode (void)
{
  tcgetattr (STDIN_FILENO, &saved_attr);
  atexit(reset_input_mode);
  new_attr = saved_attr;
  new_attr.c_lflag &= ~(ICANON | ECHO); 
  tcsetattr (STDIN_FILENO, TCSANOW, &new_attr);
}

void *read_input(void *param)
{
  int *pipe = (int *)param;
  int bytes_read;
  char buf[BUF_SIZE];

  while (1)
  {
    bytes_read = read(*pipe, buf, 1);
    if (*buf == 0x04)         /* C-d */
      break;
    if(bytes_read > 0) 
      write(STDOUT_FILENO,buf,1);
  }
  return NULL;
}

static void SIGPIPE_handler(int signo)
{
  printf("SIGPIPE; exiting with code 1\n");
  exit(1);
}

int main(int argc, char** argv) {
  int c;
  static struct option longopts[] = {
    {"shell", no_argument, &shell_flag, 's'},
    {0, 0, 0, 0}
  };

  while(1) {
    int option_index = 0;
    c = getopt_long(argc, argv, "s", longopts, &option_index);
    if(c == -1)
      break;
  }

  int bytes_read;
  char buf[BUF_SIZE];

  set_input_mode();

  int pipefd1[2];
  int pipefd2[2];
  pid_t pid;

  if(!shell_flag)
  {
    while (1)
    {
      bytes_read = read (STDIN_FILENO, buf, 1);
      if (*buf == 0x04)          /* C-d */
        break;
      if(bytes_read > 0) 
      {
        if (*buf == 0x0D || *buf == 0x0A) 
          write(STDOUT_FILENO, cr_lf, 2);
        else
          write(STDOUT_FILENO, buf, 1);
      }
    }
  } else {
    if (pipe(pipefd1) == -1) {
      perror("pipe1 failure");
      exit(EXIT_FAILURE);
    }
    if (pipe(pipefd2) == -1) {
      perror("pipe2 failure");
      exit(EXIT_FAILURE);
    }
    pid = fork();
    if (pid == -1) {
      perror("fork failure");
      exit(EXIT_FAILURE);
    }
    if (pid == 0) { //If child process
      // printf("Child process!\n");

      signal(SIGPIPE, SIGPIPE_handler);
      close(pipefd1[1]); // close write of pipe1

      close(STDIN_FILENO);
      dup(pipefd1[0]);   // set read of fd1 as stdin
      close(pipefd1[0]); 

      close(pipefd2[0]);  // close read of pipe2

      close(STDOUT_FILENO);
      dup(pipefd2[1]);     // dup write of pipe2
      close(STDERR_FILENO);
      dup(pipefd2[1]);    
      close(pipefd2[1]);  

      char *name[] = {
        "/bin/bash",
        NULL
      };
      execvp(name[0], name);
    } else { //If parent process
      // printf("Parent process\n");
      shell_pid = pid;

      pthread_t input_thread;
      close(pipefd1[0]);
      close(pipefd2[1]);
      pthread_create(&input_thread, NULL, read_input, &pipefd2[0]);

      while (1)
      {
        bytes_read = read (STDIN_FILENO, buf, 1);
        if (*buf == 0x04)          /* C-d */ {
          printf("^D encountered; Exited with exit code 0\n");
          exit(0);
        }
        if (*buf == 0x03)          /* C-c */ {
          kill(shell_pid, SIGINT);
        }

        if(bytes_read > 0) {
          write(STDOUT_FILENO,buf,1);
          write(pipefd1[1], buf, 1);
        }
      }
    }
  }
}



