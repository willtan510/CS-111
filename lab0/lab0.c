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

void segFault()
{
  char* point= NULL;
  *point=0xFF;
}

void sig_handler(int signo)
{
  if (signo == SIGSEGV)
  {
    fprintf(stderr, "Segmentation fault caught. \n");
    exit(4);
  }
}

int main(int argc, char** argv)
{
  int option;
  int error_;
  int stdin_fd=0;
  int stdout_fd=1;
  static int seg_flag=0;
  char* input=NULL;
  char* output=NULL;
  char* buffer;
  static struct option long_options[]=
    {
	    {"input",required_argument, 0 , 'i'},
      {"output",required_argument,0,'o'},
      {"segfault",no_argument,&seg_flag,'s'},
      {"catch",no_argument,0,'c'},
      {0,0,0,0}
    };
  while(1)
  {
    option=getopt_long(argc,argv,"iosc",long_options,0);
    if(option==-1)
      break;
    switch (option)
    {
      case 'i':
        input=optarg;
        break;
      case 'o':
        output=optarg;
        break;
      case 's':
        break;
      case 'c':
        signal(SIGSEGV, sig_handler);
        break;
      case '?':
        error_=errno;
        fprintf(stderr,"Try options: [--input='filename'] [--output='filename'] [--segfault] [--catch]\n");
        //fprintf(stderr,"%s\n",strerror(error_));
        exit(1);
    }
  }
  if (seg_flag)
  {
    segFault();
  }
  
  if (input)
  {
    stdin_fd=open(input,O_RDONLY);
    if(stdin_fd>=0)
    {
      close(0);
      dup(stdin_fd);
      close(stdin_fd);
    }
    else
    {
     error_=errno;
     fprintf(stderr,"%s",strerror(error_));
     printf(" as error for file known as \'%s\' for --input argument\n",input);
      exit(2);
    }
  }
  if (output)
  {
    stdout_fd=creat(output,S_IRWXU);
    if(stdout_fd>=0)
    {
      close(1);
      dup(stdout_fd);
      close(stdout_fd);
    }
    
    else
    {
    error_=errno;
    fprintf(stderr,"%s",strerror(error_));
    printf(" for file \'%s\' for option --output\n",output);
    exit(3);
    }
  }
  buffer = (char *) malloc(sizeof(char));
  ssize_t charLeft=read(0,buffer,1);

  while (charLeft>0)
  {
    write(1,buffer,1);
    charLeft=read(0,buffer,1);
  }
  free(buffer);
  exit(0);
}