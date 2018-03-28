 //Name: William Tan
//Email: Willtan510@gmail.com


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/poll.h>

#include <math.h>
#include <mraa.h>
#include <mraa/aio.h>


sig_atomic_t volatile run_flag=1;
char scale='F';
int fatal;
char* logFile=NULL;
int outputFD;
struct pollfd ufds[1];
int stopped=0;
int period=1;

void sig_handler(int sig)
{
  if(sig==SIGINT) 
    run_flag=0;
}

void shutdown()
{
  time_t currTime;
  struct tm* locTime;
  time(&currTime);
  
  locTime=localtime(&currTime);
  int hour=locTime->tm_hour;
  int min=locTime->tm_min;
  int sec=locTime->tm_sec;
  dprintf(1,"%02d:%02d:%02d SHUTDOWN\n",hour,min,sec);
  if(logFile)
    dprintf(outputFD,"%02d:%02d:%02d SHUTDOWN\n",hour,min,sec);
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
  
    dprintf(1,"%02d:%02d:%02d %.1f\n",hour,min,sec,temp);
    if(logFile)
      dprintf(outputFD,"%02d:%02d:%02d %.1f\n",hour,min,sec,temp);
    
}

void processBuffer(char* buff)
{
      if(strcmp(buff,"SCALE=F")==0)
      {
        scale='F';
        if(logFile)
          dprintf(outputFD,"%s\n",buff);
      } 
      else if(strcmp(buff,"SCALE=C")==0)
      {
        scale='C';
        if(logFile)
          dprintf(outputFD,"%s\n",buff);
      }
      else if(strcmp(buff,"STOP")==0)
      {
        stopped=1;
        if(logFile)
          dprintf(outputFD,"%s\n",buff);
      }
      else if(strcmp(buff,"START")==0)
      {
        stopped=0;
        if(logFile)
          dprintf(outputFD,"%s\n",buff);
      }
      else if(strcmp(buff,"OFF")==0)
        {
          if(logFile)
            dprintf(outputFD,"%s\n",buff);
          shutdown();
        }
      else if(strncmp(buff,"PERIOD=",strlen("PERIOD="))==0)
      {
        char subbuff[32];
        memcpy(subbuff,&buff[7],strlen("PERIOD=")-1);
        subbuff[strlen("PERIOD=")-1]='\0';
        period=atoi(subbuff);
        if(logFile)
          dprintf(outputFD,"%s\n",buff);
      }   
  
}

int main(int argc, char** argv)
{
  int option=0;
  int voltage;
  float finalTemp;

  mraa_aio_context tempSensor;
  mraa_gpio_context button;
  tempSensor=mraa_aio_init(1);
  button=mraa_gpio_init(62);

  ufds[0].fd=0;
  ufds[0].events=POLLIN;
  
  mraa_gpio_dir(button, MRAA_GPIO_IN);
  mraa_gpio_isr(button,MRAA_GPIO_EDGE_RISING,&shutdown,NULL);
  
  signal(SIGINT, sig_handler);

  static struct option long_options[]=
  {
    {"period",required_argument,0,'p'},
    {"scale",required_argument,0,'s'},
    {"log",required_argument,0,'l'},
    {0,0,0,0}
  };
  while(1)
  {
    option=getopt_long(argc,argv,"p:s:l:",long_options,0);
    if (option==-1)
      break;
    switch(option)
    {
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
      case 'l':
        logFile=optarg;
        outputFD=creat(logFile,S_IRWXU);
        if(outputFD<0)
        {
          fatal=errno;
          fprintf(stderr,"%s\n",strerror(fatal));
          exit(1);
        }
        break;
      case '?':
        fatal=errno;
        fprintf(stderr,"%s\n",strerror(fatal));
        exit(1);
    }
  }
  fcntl(0, F_SETFL, O_NONBLOCK);
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
        char buffer[30];
        int endChar=scanf("%s",buffer);
        if(endChar!=-1)
        {
          processBuffer(buffer);
        }
      }
      if(!stopped)
        time(&endTime);
    }
  }
  
  mraa_aio_close(tempSensor);
  mraa_gpio_close(button);
  return 0;
}

