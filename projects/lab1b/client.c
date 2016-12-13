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
#include <mcrypt.h>
#include <sys/stat.h>

#define BUF_SIZE 1

int port_num, sockfd, logfd;
static int encrypt_flag;
pthread_t input_thread;
struct termios saved_attr, new_attr;
char cr_lf[2] = {0x0D, 0x0A};
MCRYPT cryptfd, decryptfd;

void exit0() {
  pthread_cancel(input_thread);
  close(sockfd);
  exit(0);
}

void exit1() {
  close(sockfd);
  exit(1);
}

char* read_key(char *filename)
{
  struct stat key_stat;

  int key_fd = open(filename, O_RDONLY);
  if(fstat(key_fd, &key_stat) < 0) { 
    perror("fstat"); 
    exit1(); 
  }
  char* key = (char*) malloc(key_stat.st_size * sizeof(char));

  if(read(key_fd, key, key_stat.st_size) < 0) { 
    perror("read"); 
    exit1(); 
  }
  return key;
}

void encrypt(char *buf, int crypt_len)
{
  if(mcrypt_generic(cryptfd, buf, crypt_len) != 0) 
  { 
    perror("Error in encryption"); 
    exit1(); 
  }
}

void decrypt(char *buf, int decrypt_len)
{
  if(mdecrypt_generic(decryptfd, buf, decrypt_len) != 0) 
  { 
    perror("Error in decryption"); 
    exit1(); 
  }
}

void reset_input_mode (void)
{
  tcsetattr (STDIN_FILENO, TCSANOW, &saved_attr);
  if(encrypt_flag) {
    mcrypt_generic_deinit(cryptfd);
    mcrypt_module_close(cryptfd);
    mcrypt_generic_deinit(decryptfd);
    mcrypt_module_close(decryptfd);
  }
}

void set_input_mode (void)
{
  tcgetattr (STDIN_FILENO, &saved_attr);
  atexit(reset_input_mode);
  new_attr = saved_attr;
  new_attr.c_lflag &= ~(ICANON | ECHO); 
  tcsetattr (STDIN_FILENO, TCSANOW, &new_attr);
}

void write_to_log(int logfd, int rec_or_sent, char* buf) 
{
  char rec[18] = "RECEIVED 1 BYTE: ";
  char sent[14] = "SENT 1 BYTE: ";
  char new_line[1] = "\n";
  if(rec_or_sent == 0)
  {
    write(logfd, rec, strlen(rec));
    write(logfd, buf, 1);
    write(logfd, new_line, 1);
  } else {
    write(logfd, sent, strlen(sent));
    write(logfd, buf, 1);
    write(logfd, new_line, 1);
  }
}

void *read_input(void *param)
{
  int *sockfd = (int *)param;
  int bytes_read;
  char buf[BUF_SIZE];

  while (1)
  {
    bytes_read = read(*sockfd, buf, 1);
    if(bytes_read > 0) {
      if(logfd)
        write_to_log(logfd, 0, buf);
      if(encrypt_flag) 
        decrypt(buf,1);
      write(STDOUT_FILENO,buf,1);
    } 
  }
  return NULL;
}

int main(int argc, char** argv) {
  int c;
  char buf[BUF_SIZE];
  struct sockaddr_in serv_addr;
  struct hostent *server;

  static struct option longopts[] = {
    {"encrypt", no_argument, NULL , 'e'},
    {"log", required_argument, NULL, 'l'},
    {"port", required_argument, NULL, 'p'},
    {0, 0, 0, 0}
  };

  while(1) {
    int option_index = 0;
    c = getopt_long(argc, argv, "p:l:e", longopts, &option_index);
    if(c == -1)
      break;

    switch(c)
    {
      case 'p':
        port_num = atoi(optarg);
        break;
      case 'l':
        logfd = creat(optarg, 0666);
        break;
      case 'e':
        encrypt_flag = 1;
        char *key = read_key("my.key");
        cryptfd = mcrypt_module_open("blowfish", NULL, "ofb", NULL);
        if(cryptfd == MCRYPT_FAILED) { 
          perror("Error opening module"); 
          exit1(); 
        }
        if(mcrypt_generic_init(cryptfd, key, strlen(key), NULL) < 0) { 
          perror("Error while initializing encrypt");
          exit1(); 
        }
        decryptfd = mcrypt_module_open("blowfish", NULL, "ofb", NULL);
        if(decryptfd == MCRYPT_FAILED) { 
          perror("Error opening module"); 
          exit1(); 
        }
        if(mcrypt_generic_init(decryptfd, key, strlen(key), NULL) < 0) { 
          perror("Error while initializing decrypt");
          exit1(); 
        }
        break;
      default:
        exit1();
    }
  }

  set_input_mode();

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    perror("ERROR opening socket");
  server = gethostbyname("localhost");
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit1();
  }

  memset((char *) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  memcpy((char *) &serv_addr.sin_addr.s_addr, (char*) server->h_addr, server->h_length);
  serv_addr.sin_port = htons(port_num);
  if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
    perror("ERROR connecting");
    exit1();
  }

  int bytes_read;
  pthread_create(&input_thread, NULL, read_input, &sockfd);

  while (1)
  {
    bytes_read = read (STDIN_FILENO, buf, 1);
    if (*buf == 0x04)          /* C-d */
      exit0();
    if(bytes_read > 0) 
    {
      if (*buf == 0x0D || *buf == 0x0A) {
        if(logfd)
          write_to_log(logfd, 1, buf);
        write(STDOUT_FILENO, cr_lf, 2);
        if(encrypt_flag)
          encrypt(buf, 1);
        write(sockfd, buf, 1);
      }
      else {
        write(STDOUT_FILENO, buf, 1);
        if(encrypt_flag)
          encrypt(buf, 1);
        write(sockfd, buf, 1);
        if(logfd)
          write_to_log(logfd, 1, buf);
      }
    } else if (bytes_read == 0) {
      exit1();
    }
  }

}



