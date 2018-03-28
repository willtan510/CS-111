//NAME= William Tan
//EMAIL= willtan510@gmail.com
//ID= 104770108

//after ^C kill child and keep running until hits POLLUP or something

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
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

/* Use this variable to remember original terminal attributes. */

struct termios saved_attributes;
int pipe1fd[2];
int pipe2fd[2];
int count=0;
static int shell_flag;

pid_t child;

struct pollfd pfds[2];
int errorNum;

void reset_input_mode (void)
{

  tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
  if(shell_flag)
  {
    close(pipe1fd[0]);
    close(pipe1fd[1]);
    close(pipe2fd[0]);
    close(pipe2fd[1]);
    
    int status;
    if(waitpid(child,&status,0)==-1)
    {
      errorNum=errno;
      fprintf(stderr,"%s\n",strerror(errorNum));
      exit(1);
    }

  
  if(WIFEXITED(status))
  {
    fprintf(stderr,"SHELL EXIT=%d STATUS=%d\n",status&0x007F,(status&0xff00)>>8);
    exit(0);
  }
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

void signal_handler(int sig_num)
{
  if (shell_flag && sig_num==SIGINT)
    {
      kill(child,SIGINT);
      reset_input_mode();
    }
  if(sig_num==SIGPIPE)
  {
    reset_input_mode();
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
          errorNum=errno;
          fprintf(stderr,"%s\n",strerror(errorNum));
          exit(1);
        }
      
      if( pfds[0].revents&POLLIN)
      {
        read(0,&buffer,1);
        if (buffer==0x04)
        {
          close(pipe1fd[1]);
        }
        else if (buffer==0x0D || buffer == 0x0A)
        {
          char temp1=0x0D;
	      char temp2=0x0A;
          char temp[2]={temp1,temp2};
          write(1,&temp,2);
	        write(pipe1fd[1],&temp2,1);
        }
        else if (buffer==0x03)
          {
            kill(child,SIGINT);
            exit(0);
          }
        else
        {
          write(1,&buffer,1);
          write(pipe1fd[1],&buffer,1);
        }
      }
 
      if ( pfds[1].revents&POLLIN)  //check if can read from pipe2
      {
        read(pipe2fd[0],&buffer,1);
        if (buffer==0x0A)
        {
          char temp[2]={0x0D,0x0A};
          write(1,&temp,2);
        }
        else
          write(1,&buffer,1);
      } 
      
      if(pfds[1].revents & (POLLHUP|POLLERR))
        exit(0);
    }
}


int main (int argc, char** argv)
{
  set_input_mode();
  int option;

  static struct option long_options[]=
  {
    {"shell",no_argument,&shell_flag,'s'},
    {0,0,0,0}
  };
  
  while(1)
  {
    option=getopt_long(argc,argv,"s",long_options,0);
    if(option==-1)
      break;
    switch(option)
    {
      case 's':
        signal(SIGINT,signal_handler);
        signal(SIGPIPE,signal_handler);
        break;
      case '?':
        fprintf(stderr,"Try options: [--shell]\n");
        //fprintf(stderr,"%s\n",strerror(error_));
        exit(1);
    }
  }
  
    
  if(pipe(pipe1fd)==-1)
  {
    errorNum=errno;
    fprintf(stderr,"%s\n",strerror(errorNum));
  }
  if(pipe(pipe2fd)==-1)
  {
    errorNum=errno;
    fprintf(stderr,"%s\n",strerror(errorNum));
  }

  if (shell_flag)
    {
      pfds[0].fd=0;
      pfds[1].fd=pipe2fd[0];
      pfds[0].events=POLLIN | POLLHUP | POLLERR;
      pfds[1].events=POLLIN | POLLHUP | POLLERR;
      
      child=fork();
      if (child==-1)
      {
        errorNum=errno;
        fprintf(stderr,"%s\n",strerror(errorNum));
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
             errorNum=errno;
             fprintf(stderr,"%s\n",strerror(errorNum));
              exit(1);
          }
        }
        
    }
    
    if (!shell_flag)
    {
      char buffer2;
      while(1)
      {
      count=read(0,&buffer2,256);
      if (buffer2==0x04)
      {
          reset_input_mode();
          exit(0);
      }
      else if (buffer2==0x0D || buffer2 == 0x0A)
      {
        char temp[2]={0x0D,0x0A};
        write(1,&temp,2);
      }
      else
          write(1,&buffer2,count);
      }
    }
    
    
  return EXIT_SUCCESS;
}
