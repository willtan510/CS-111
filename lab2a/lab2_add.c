#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

int fatal;
int numThreads=1;
int numIts=1;
static int opt_yield=0;
static int synced=0;
char syncType;
pthread_mutex_t lock;
int spinLock=0;

void add(long long *pointer, long long value)
{
  long long sum = *pointer + value;
  if (opt_yield)
    sched_yield();
  *pointer = sum;
}

void compare_add(long long * count, long long value)
{
  long long old= *count;
  long long newVal= old+value;
  while(__sync_val_compare_and_swap(count, old,newVal)!=old)
  {
    old=*count;
    newVal=old+value;
    if(opt_yield)
      sched_yield();
  }
}

void *thread_add(void* count)
{

  for (int i=0; i<numIts;i++)  //increment
  {
    if(syncType=='m')
    {
      pthread_mutex_lock(&lock);
      add(count,1);
      pthread_mutex_unlock(&lock);
    }
    else if(syncType=='s')
    {
       while(__sync_lock_test_and_set(&spinLock,1));
       add(count,1);
       __sync_lock_release(&spinLock);
    }
    else if(syncType=='c')
    {
      compare_add(count,1);
    }
    else
      add(count,1);
  }
  for(int i=0;i<numIts;i++)  //decrement
  {
    if(syncType=='m')
    {
      pthread_mutex_lock(&lock);
      add(count,-1);
      pthread_mutex_unlock(&lock);
    }
    else if(syncType=='s')
    {
       while(__sync_lock_test_and_set(&spinLock,1));
       add(count,-1);
       __sync_lock_release(&spinLock);
    }
    else if(syncType=='c')
    {
      compare_add(count,-1);
    }
    else
      add(count,-1);
  }
  return NULL;
}

char* checkTest()
{
  if(synced)
  {
    if(opt_yield)
      {
        switch(syncType)
        {
          case 'm':
            return "add-yield-m";
            break;
          case 's':
            return "add-yield-s";
            break;
          case 'c':
            return "add-yield-c";
            break;
        }
      }
    else
      switch(syncType)
      {
        case 'm':
            return "add-m";
            break;
        case 's':
            return "add-s";
            break;
        case 'c':
            return "add-c";
            break;
      }
  }
  else
  {
    if(opt_yield)
      return "add-yield-none";
    else
      return "add-none";
  }
  return NULL;
}

int main(int argc,char** argv)
{
  int option;
  long long counter=0;
  
  
  if(pthread_mutex_init(&lock,NULL)!=0)
  {
    fatal=errno;
    fprintf(stderr,"%s\n",strerror(fatal));
    exit(1);
  }

  static struct option long_options[]=
  {
    {"threads",required_argument,0,'t'},
    {"iterations",required_argument,0,'i'},
    {"yield",no_argument,&opt_yield,'y'},
    {"sync",required_argument,0,'s'},
    {0,0,0,0}
  };
  
  while(1)
  {
    option=getopt_long(argc,argv,"t:i:ys:",long_options,0);
    if(option==-1)
      break;
    switch(option)
    {
      case 't':
        numThreads=atoi(optarg);
        break;
      case 'i':
        numIts=atoi(optarg);
        break;
      case 'y':
        break;
      case 's':
        synced=1;
        if (strlen(optarg)==1)
        {
          if(optarg[0]=='m')
            syncType='m';
          else if(optarg[0]=='s')
            syncType='s';
          else if(optarg[0]=='c')
            syncType='c';
          else 
          {
            fprintf(stderr,"Invalid argument for option --sync\n");
            exit(1);
          }
        }
        else
          {  
            fprintf(stderr,"Invalid argument for option --sync\n");
            exit(1);
          }
        break;
      case '?':
        fatal=errno;
        fprintf(stderr,"%s\n",strerror(fatal));
        exit(1);
    }
  }
  pthread_t threads[numThreads];
  struct timespec start_time,end_time;
  clock_gettime(CLOCK_MONOTONIC,&start_time);
  
  for (int i=0;i<numThreads;i++)  //create threads
  {
    int status=pthread_create(&threads[i],NULL,&thread_add,&counter);
    if (status==-1)
    {
      fatal=errno;
      fprintf(stderr,"%s\n",strerror(fatal));
    }
  }
  
  for (int i=0;i<numThreads;i++)
  {
    int status=pthread_join(threads[i],NULL);
    if (status==-1)
    {
      fatal=errno;
      fprintf(stderr,"%s\n",strerror(fatal));
      exit(1);
    }
  }
  
  clock_gettime(CLOCK_MONOTONIC, &end_time);
  long long elapsed_ns=(end_time.tv_sec-start_time.tv_sec)*1000000000;
  elapsed_ns+=end_time.tv_nsec;
  elapsed_ns-=start_time.tv_nsec;
  
  int numOps=numThreads*numIts*2;
  int avgTime=elapsed_ns/numOps;
  
  char* testType=checkTest();
  pthread_mutex_destroy(&lock);
  fprintf(stdout,"%s,%d,%d,%d,%lld,%d,%lld\n",testType,numThreads,numIts,numOps,
          elapsed_ns,avgTime,counter);
  
  exit(0);
}