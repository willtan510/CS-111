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

struct termios saved_attributes;
struct pollfd pfds[2];
int socketfd;
char* encrypted=NULL;
char* logged=NULL;
char buff[2500];
int count=0;
  int output;
  MCRYPT td;
  MCRYPT td2;
  
void reset_input_mode (void)
{
  if(count&&logged)
  {
    write(output,"RECEIVED ",9);
    dprintf(output,"%d",count);
    write(output," bytes: ",8);
    write(output,&buff,count);
    write(output,"\n",1);
    count=0;
  }
  tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
  
  if(encrypted)
    {
      mcrypt_generic_deinit(td);
      mcrypt_module_close(td);
      mcrypt_generic_deinit(td2);
      mcrypt_module_close(td2);
    }
  
}

void set_input_mode (void)
{
  struct termios tattr;

  /* Make sure stdin is a terminal. */
  if (!isatty (STDIN_FILENO))
    {
      fprintf (stderr, "Not a terminal.\n");
      exit (EXIT_FAILURE);
    }

  /* Save the terminal attributes so we can restore them later. */
  tcgetattr (STDIN_FILENO, &saved_attributes);
  atexit (reset_input_mode);

  /* Set the funny terminal modes. */
  tcgetattr (STDIN_FILENO, &tattr);
  tattr.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */
  tattr.c_cc[VMIN] = 1;
  tattr.c_cc[VTIME] = 0;
  tattr.c_iflag |= ISTRIP;
  tattr.c_oflag=0;
  tattr.c_lflag=0;
  tcsetattr (STDIN_FILENO, TCSANOW, &tattr);
}

int main(int argc, char *argv[])
{
  int option=0;  
  int port;
  int error_;
  char* key;

  int keysize=0;
  struct sockaddr_in server_address;
  struct hostent *server;
  
  char buffer;
  
  static struct option long_options[]=
  {
    {"port",required_argument,0,'p'},
    {"log",required_argument,0,'l'},
    {"encrypt",required_argument,0,'e'},
    {0,0,0,0}
  };
  
  while(1)
  {
    option=getopt_long(argc,argv,"p:le",long_options,0);
    if (option==-1)
      break;
    switch(option)
    {
      case 'p':
        port=atoi(optarg);
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
      case 'e':
        encrypted=optarg;
        break;
      case '?':
        error_=errno;
        fprintf(stderr,"%s\n",strerror(error_));
        exit(1);
    }
  }
  set_input_mode();
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
    if(td==MCRYPT_FAILED||td2==MCRYPT_FAILED)
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
    if(init<0)
    {
      error_=errno;
      fprintf(stderr,"%s\n",strerror(error_));
    }
  }

  socketfd=socket(AF_INET,SOCK_STREAM,0);
  if (socketfd<0)
  {
    error_=errno;
    fprintf(stderr,"%s\n",strerror(error_));
    exit(1);
  }
  
  server=gethostbyname("127.0.0.1");
  
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
  
  pfds[0].fd=0;
  pfds[1].fd=socketfd;
  pfds[0].events=POLLIN | POLLHUP | POLLERR;
  pfds[1].events=POLLIN | POLLHUP | POLLERR;  


  while(1)
  {
    if(poll(pfds,2,0)<0)
    {
      error_=errno;
      fprintf(stderr,"%s\n",strerror(error_));
      exit(1);
    }
    
    if( pfds[0].revents&POLLIN)
      {
        if(count&&logged)
        {
                write(output,"RECEIVED ",9);
                dprintf(output,"%d",count);
                write(output," bytes: ",8);
                write(output,&buff,count);
                write(output,"\n",1);
                count=0;
        }
        read(0,&buffer,1);
        char temp2=0x0A;
        if (buffer==0x0D || buffer == 0x0A)
        {
	        char temp1=0x0D;      
          char temp[2]={temp1,temp2};
          if (encrypted)
          {
            mcrypt_generic(td,&temp2,1);
          }
	        write(socketfd,&temp2,1);
          write(1,&temp,2);
         if (logged)
         {
           write(output,"SENT 1 bytes: ", 14);
           write(output,&temp2,1);  //was buffer before
           write(output,"\n",1);
         }
        }
        else
        {
          write(1,&buffer,1);
          if(encrypted)
            mcrypt_generic(td,&buffer,1);
          write(socketfd,&buffer,1);
          if (logged)
          {
            write(output,"SENT 1 bytes: ",14);
            write(output,&buffer,1);
            write(output,"\n",1);
          }
        }
      }
 
   /*    if((!(pfds[1].revents&POLLIN))&&count&&logged)  //new block
       {
         write(output,"RECEIVED ",9);
         dprintf(output,"%d",count);
         write(output," bytes: ",8);
         write(output,&buff,count);
         write(output,"\n",1);
         count=0;
       }*/
 
      if ( pfds[1].revents&POLLIN)  //check if can read from socket
      {
          read(socketfd,&buffer,1);
          if (encrypted &&logged)
          {
            buff[count]=buffer;
            count++;   
          }
          if (encrypted)
          {
            mdecrypt_generic(td2,&buffer,1);
          }
          if (buffer==0x0A||buffer==0x0D)
          {
            char temp[2]={0x0D,0x0A};
            write(1,&temp,2);
            if(logged)
              {
                buff[count]=buffer;
                count++;
              }
          }
          else
          {
            if(logged&&!encrypted)
            {
              buff[count]=buffer;
              count++;   
            }
            write(1,&buffer,1);
          }
          if(buffer==0x04)
          {
            close(socketfd);
            exit(0);
          }     
      }
      
      if(pfds[1].revents & (POLLHUP|POLLERR))
        exit(0);
        
   }

  
  
  return EXIT_SUCCESS;
}