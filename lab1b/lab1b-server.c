#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <mcrypt.h>
#include <sys/stat.h>

int pipe1fd[2];
int pipe2fd[2];
struct pollfd pfds[2];
pid_t child;
int error_;
int socketfd;
int newsocketfd;
char * encrypted=NULL;
MCRYPT td;
MCRYPT td2;
int keysize=0;

void cleanup()
{
  close(pipe1fd[0]);
  close(pipe1fd[1]);
  close(pipe2fd[0]);
  close(pipe2fd[1]);  
  
 char buffer=0x04;
 if(encrypted)
   mcrypt_generic(td,&buffer,1);
 write(newsocketfd,&buffer,1);
  
  close(newsocketfd);
  
  if(encrypted)
  {
    mcrypt_generic_deinit(td);
    mcrypt_module_close(td);
  }
  int status;
  if(waitpid(child,&status,0)==-1)
  {
    error_=errno;
    fprintf(stderr,"%s\n",strerror(error_));
    exit(1);
  }

  
  if(WIFEXITED(status))
  {
    fprintf(stderr,"SHELL EXIT=%d STATUS=%d\n",status&0x007F,(status&0xff00)>>8);
    exit(0);
  }
}    
  
void signal_handler(int sig_num)
{
  if (sig_num==SIGINT)
    {
      kill(child,SIGINT);
      cleanup();
    }
  if(sig_num==SIGPIPE)
  {
    cleanup();
    exit(1);
  }
}  
  
void readout()
{
  while(1)
  { 
      char buffer;
      if(poll(pfds,2,0)<0)
        {
          error_=errno;
          fprintf(stderr,"%s\n",strerror(error_));
          exit(1);
        }   
      if(pfds[0].revents&POLLIN)
      {
        read(newsocketfd,&buffer,1);
        if(encrypted)
          mdecrypt_generic(td2,&buffer,1);
        if (buffer==0x04)
        {
          close(pipe1fd[1]);
        }
        else if (buffer==0x03)
          {
            kill(child,SIGINT);
            exit(0);
          }
         else if(buffer==0x0D||buffer==0x0A)
        {
          char temp2=0x0A;
          write(pipe1fd[1],&temp2,1);
        }  
        else
        {
          write(pipe1fd[1],&buffer,1);
        }
      }
      if (pfds[1].revents&POLLIN)  //check if can read from pipe2
      {
        read(pipe2fd[0],&buffer,1);
        if(encrypted)
          mcrypt_generic(td,&buffer,1);
        write(newsocketfd,&buffer,1);
      } 
      if(pfds[1].revents & (POLLHUP|POLLERR))
          exit(0);
    }
}

int main(int argc, char *argv[])
{
  int error_;
  int option;
  int port;
  socklen_t clilen;
  char* key;
  struct sockaddr_in server_address, client_address;
  
  atexit(cleanup);
  
  static struct option long_options[]=
  {
    {"port",required_argument,0,'p'},
    {"encrypt",required_argument,0,'e'},
    {0,0,0,0}
  };
  
  while (1)
  {
    option=getopt_long(argc,argv,"p:e",long_options,0);
    if (option==-1)
      break;
    switch(option)
    {
      case 'p':
        port=atoi(optarg);
        break;
      case 'e':
        encrypted=optarg;
        break;
      case '?':
        error_=errno;
        fprintf(stderr,"%s\n",strerror(error_));
        exit(1);
    }
  }
  
  if(encrypted)
  {
    char* IV;
    int init;
    key=(char *) malloc(1000*sizeof(char));
    int fileFD=open(encrypted,O_RDONLY);
    if (fileFD<0)
    {
      error_=errno;
      fprintf(stderr,"%s\n",strerror(error_));
    }
    ssize_t charLeft=read(fileFD,key,1);
    if (charLeft<0)
    {
      error_=errno;
      fprintf(stderr,"%s\n",strerror(error_));
    }    
    keysize++;
    while(charLeft>0)
    {
      charLeft=read(fileFD,key+keysize,1);
      if (charLeft<0)
      {
        error_=errno;
        fprintf(stderr,"%s\n",strerror(error_));
      }
      keysize++;
    }

    td=mcrypt_module_open("twofish",NULL,"cfb",NULL);
    td2=mcrypt_module_open("twofish",NULL,"cfb",NULL);
    if(td==MCRYPT_FAILED || td2==MCRYPT_FAILED)
    {
      error_=errno;
      fprintf(stderr,"%s\n",strerror(error_));
    }
    IV=malloc(mcrypt_enc_get_iv_size(td));
    int i=0;
    for (i=0;i<mcrypt_enc_get_iv_size(td);i++)
      IV[i]=i;
    init=mcrypt_generic_init(td,key,keysize,IV);
    if(init<0)
    {
      error_=errno;
      fprintf(stderr,"%s\n",strerror(error_));
    }
    init=mcrypt_generic_init(td2,key,keysize,IV);
  }
  
  signal(SIGINT,signal_handler);
  signal(SIGPIPE,signal_handler);  
  
  socketfd=socket(AF_INET,SOCK_STREAM,0);
    
  if(socketfd<0)
  {
    error_=errno;
    fprintf(stderr,"%s\n",strerror(error_));
    exit(1);
  }
  
  memset((char *) &server_address,0, sizeof(server_address));
  
  server_address.sin_family=AF_INET;
  server_address.sin_port=htons(port);
  server_address.sin_addr.s_addr=INADDR_ANY;

  
  if (bind(socketfd,(struct sockaddr *) &server_address,sizeof(server_address))<0)
  {
    error_=errno;
    fprintf(stderr,"%s\n",strerror(error_));
    exit(1);
  }
  
  listen(socketfd,5);
  clilen=sizeof(client_address);
  
  newsocketfd=accept(socketfd,(struct sockaddr *)&client_address,&clilen);
  
  if (newsocketfd<0)
  {
    error_=errno;
    fprintf(stderr,"%s\n",strerror(error_));
    exit(1);
  }
  
  if(pipe(pipe1fd)==-1)
  {
    error_=errno;
    fprintf(stderr,"%s\n",strerror(error_));
  }
  if(pipe(pipe2fd)==-1)
  {
    error_=errno;
    fprintf(stderr,"%s\n",strerror(error_));
  }
  
  pfds[0].fd=newsocketfd;
  pfds[1].fd=pipe2fd[0];
  pfds[0].events=POLLIN | POLLHUP | POLLERR;
  pfds[1].events=POLLIN | POLLHUP | POLLERR;
  
  child=fork();
  if (child==-1)
  {
    error_=errno;
    fprintf(stderr,"%s\n",strerror(error_));
    exit(1);
  }
  else if (child>0)
  {
    close(pipe1fd[0]);
    close(pipe2fd[1]);
    
    readout();
  }
  else if (child==0)
  {
    close(pipe1fd[1]);
    close(pipe2fd[0]);
    dup2(pipe1fd[0],0);
    dup2(pipe2fd[1],1);
    dup2(pipe2fd[1],2);
    close(pipe1fd[0]);
    close(pipe2fd[1]);

    char *execvp_argv[2];
    char execvp_filename[] = "/bin/bash";
    execvp_argv[0] = execvp_filename;
    execvp_argv[1] = NULL;
    
    if(execvp("/bin/bash", execvp_argv)==-1)
    {
      error_=errno;
      fprintf(stderr,"%s\n",strerror(error_));
      exit(1);
    }  
  }
}