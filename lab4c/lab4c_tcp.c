#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <sys/stat.h>
#include <math.h>
#include <mraa.h>
#include <mraa/aio.h>


struct pollfd ufds[1];
int socketfd;
char* logged=NULL;
char buff[2500];
int count=0;
int output;
int fatal;
sig_atomic_t volatile run_flag=1;
char scale='F';
int stopped=0;
int period=1;


void sig_handler(int sig)
{
  if(sig==SIGINT) 
    run_flag=0;
}

void shut()
{
  time_t currTime;
  struct tm* locTime;
  time(&currTime);
  
  locTime=localtime(&currTime);
  int hour=locTime->tm_hour;
  int min=locTime->tm_min;
  int sec=locTime->tm_sec;
  dprintf(socketfd,"%02d:%02d:%02d SHUTDOWN\n",hour,min,sec);
  if(logged)
    dprintf(output,"%02d:%02d:%02d SHUTDOWN\n",hour,min,sec);
  exit(0);
}

float returnTemp(int volts)
{
  const int B=4275;
  const int R0=100000;
  
  float R=1023.0/volts-1.0;
  R=R0*R;
  
  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15;  //in celsius
  if(scale=='F')
    temperature=temperature*1.8 +32;
  return temperature;
}

void printReport(float temp)
{
  time_t currTime;
  struct tm* locTime;
  time(&currTime);
  locTime=localtime(&currTime);
  int hour=locTime->tm_hour;
  int min=locTime->tm_min;
  int sec=locTime->tm_sec;
  
    dprintf(socketfd,"%02d:%02d:%02d %.1f\n",hour,min,sec,temp);
    if(logged)
      dprintf(output,"%02d:%02d:%02d %.1f\n",hour,min,sec,temp);
    
}

void processBuffer(char* buff)
{
      if(strcmp(buff,"SCALE=F")==0)
      {
        scale='F';
        if(logged)
          dprintf(output,"%s\n",buff);
      } 
      else if(strcmp(buff,"SCALE=C")==0)
      {
        scale='C';
        if(logged)
          dprintf(output,"%s\n",buff);
      }
      else if(strcmp(buff,"STOP")==0)
      {
        stopped=1;
        if(logged)
          dprintf(output,"%s\n",buff);
      }
      else if(strcmp(buff,"START")==0)
      {
        stopped=0;
        if(logged)
          dprintf(output,"%s\n",buff);
      }
      else if(strcmp(buff,"OFF")==0)
        {
          if(logged)
            dprintf(output,"%s\n",buff);
          shut();
        }
      else if(strncmp(buff,"LOG",strlen("LOG"))==0)
      {
        if(logged)
          dprintf(output,"%s\n",buff);
      }
      else if(strncmp(buff,"PERIOD=",strlen("PERIOD="))==0)
      {
        char subbuff[32];
        memcpy(subbuff,&buff[7],strlen("PERIOD=")-1);
        subbuff[strlen("PERIOD=")-1]='\0';
        period=atoi(subbuff);
        if(logged)
          dprintf(output,"%s\n",buff);
      }   
  
}

int main(int argc, char *argv[])
{
  int option=0;  
  int port;
  int error_;
  char* host;
  int id;

  struct sockaddr_in server_address;
  struct hostent *server;
  

  int voltage;
  float finalTemp;
  mraa_aio_context tempSensor;
  tempSensor=mraa_aio_init(1);

    signal(SIGINT, sig_handler);
  static struct option long_options[]=
  {
    {"host",required_argument,0,'h'},
    {"log",required_argument,0,'l'},
    {"id",required_argument,0,'i'},
    {"period",required_argument,0,'p'},
    {"scale",required_argument,0,'s'},
    {0,0,0,0}
  };
  
  while(1)
  {
    option=getopt_long(argc,argv,"hlips",long_options,0);
    if (option==-1)
      break;
    switch(option)
    {
      case 'h':
        host=optarg;
        break;
      case 'l':
        logged=optarg;
        output=creat(logged,S_IRWXU);
        if(output<0)
        {
          error_=errno;
          fprintf(stderr,"%s\n",strerror(error_));
          exit(1);
         }
        break;
      case 'i':
        id=atoi(optarg);
        break;
      case 'p':
        period=atoi(optarg);
        break; 
      case 's':
        if(strlen(optarg)>1)
          {
            fprintf(stderr,"Only takes C or F as argument\n");
            exit(1);
          }
        if(optarg[0]=='C')
          scale='C';
        else if(optarg[0]=='F')
          scale='F';
        else 
        {
          fprintf(stderr,"Only takes C or F as argument\n");
          exit(1);
        }
        break;
      case '?':
        error_=errno;
        fprintf(stderr,"%s\n",strerror(error_));
        exit(1);
    }
  }
  port=atoi(argv[argc-1]);

  socketfd=socket(AF_INET,SOCK_STREAM,0);
  if (socketfd<0)
  {
    error_=errno;
    fprintf(stderr,"%s\n",strerror(error_));
    exit(1);
  }
  
  server=gethostbyname(host);
  
  if (server==NULL)
  {
    error_=errno;
    fprintf(stderr,"%s\n",strerror(error_));
    exit(1);
  }
  
  memset((char *) &server_address,0, sizeof(server_address));
  server_address.sin_family=AF_INET;
  
  memcpy( (char* )&server_address.sin_addr.s_addr,(char *)server->h_addr, server->h_length);
  server_address.sin_port=htons(port);
  
  int status=connect(socketfd,(struct sockaddr *) &server_address,sizeof(server_address));
  if (status<0)
  {
    error_=errno;
   
    fprintf(stderr,"%s\n",strerror(error_));
    exit(0);
  }
  
  ufds[0].fd=socketfd;
  ufds[0].events=POLLIN | POLLHUP | POLLERR; 

  fcntl(socketfd, F_SETFL, O_NONBLOCK);
  
  dprintf(socketfd,"ID=%d\n",id);
  dprintf(output,"ID=%d\n",id);
  
  while(run_flag)
  {
    if(!stopped)
    {
      voltage=mraa_aio_read(tempSensor);
      finalTemp=returnTemp(voltage);
      printReport(finalTemp);
    }
    time_t startTime, endTime;
    time(&startTime);
    time(&endTime);

    while(difftime(endTime,startTime)< period)
    {
      if(poll(ufds,1,0)<0)
      {
        fatal=errno;
        fprintf(stderr,"%s\n",strerror(fatal));
        exit(1);
      }      
      if(ufds[0].revents&POLLIN)
      {
        char buffer[100];
        int index=0;
        while(read(socketfd,&buffer[index],1)>0)
        {
            if(buffer[index]=='\n')
            {
                buffer[index]='\0';
                index=0;
                break;
            }
            index++;
        }
          processBuffer(buffer);
      }
      if(!stopped)
        time(&endTime);
    }
        
   }

  mraa_aio_close(tempSensor);
  
  return 0;
}