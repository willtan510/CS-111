#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "SortedList.h"

int numThreads=1;
int numIts=1;
char syncType;
int opt_yield=0;
static int synced=0;
int fatal;
int iVal,dVal,lVal;
int numElements=0;
char* listString="";
SortedListElement_t* elements;
SortedList_t* list;
pthread_mutex_t lock;
int typeM,typeS;
int spinLock=0;

void *threadList(void* index)
{
  int listLen=0;
  int indice=*((int *)index);
  for (int i=indice;i<numElements;i+=numThreads)
  {
    if(typeM)
    {
      pthread_mutex_lock(&lock);
      SortedList_insert(list,&elements[i]);
      pthread_mutex_unlock(&lock);
    }
    else if(typeS)
    {
      while(__sync_lock_test_and_set(&spinLock,1));
      SortedList_insert(list,&elements[i]);
      __sync_lock_release(&spinLock);
    }
    else
      SortedList_insert(list,&elements[i]);
  }
  
  if(typeM)
  {
    pthread_mutex_lock(&lock);
    listLen=SortedList_length(list);
    pthread_mutex_unlock(&lock);
  }
  else if(typeS)
  {
    while(__sync_lock_test_and_set(&spinLock,1));
    listLen=SortedList_length(list);
    __sync_lock_release(&spinLock);
  }
  else
    listLen=SortedList_length(list);
  if(listLen==-1)
  {
    fprintf(stderr,"Corrupted List: Error during attempt to get length\n");
    exit(2);
  }
  
  for(int i=indice;i<numElements;i+=numThreads)
  {
    if(typeM)
    {
      pthread_mutex_lock(&lock);
      SortedListElement_t* temp=SortedList_lookup(list,elements[i].key);
      if(temp==NULL)
      {
        fprintf(stderr,"Corrupted List: Error searching for inserted element\n");
        exit(2);
      }
        if(SortedList_delete(temp)==1)
        {
          fprintf(stderr,"Corrupted List: Error deleting inserted element \n");
          exit(2);
        }
      pthread_mutex_unlock(&lock);
    }
    else if(typeS)
    {
      while(__sync_lock_test_and_set(&spinLock,1));
      SortedListElement_t* temp=SortedList_lookup(list,elements[i].key);
      if(temp==NULL)
      {
        fprintf(stderr,"Corrupted List: Error searching for inserted element\n");
        exit(2);
      }
      if(SortedList_delete(temp)==1)
      {
        fprintf(stderr,"Corrupted List: Error deleting inserted element \n");
        exit(2);
      }
      __sync_lock_release(&spinLock);
    }
    else
    {
      SortedListElement_t* temp=SortedList_lookup(list,elements[i].key);
      if(temp==NULL)
      {
        fprintf(stderr,"Corrupted List: Error searching for inserted element\n");
        exit(2);
      }
      if(SortedList_delete(temp)==1)
      {
        fprintf(stderr,"Corrupted List: Error deleting inserted element \n");
        exit(2);
      }
    }
  }
  return NULL;
}

void changeString()
{
  strcat(listString,"list-");
  if(opt_yield)
  {
    if(iVal)
      strcat(listString,"i");
    if(dVal)
      strcat(listString,"d");
    if(lVal)
      strcat(listString,"l");
  }
  else
    strcat(listString,"none");
  strcat(listString,"-");
  if(syncType)
  {
    if(syncType=='s')
      strcat(listString,"s");
    else if (syncType=='m')
      strcat(listString,"m");
  }
  else
    strcat(listString,"none");
}

void signal_handler()
{
  fprintf(stderr,"Segmentation Fault!!!\n");
  exit(2);
}

int main(int argc,char** argv)
{
  int option;  
  listString=(char *)malloc(25);
  signal(SIGSEGV,signal_handler);
  
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
    {"yield",required_argument,0,'y'},
    {"sync",required_argument,0,'s'},
    {0,0,0,0}
  };
  while(1)
  {
    option=getopt_long(argc,argv,"t:i:y:s:",long_options,0);
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
        for(unsigned int i=0;i<strlen(optarg);i++)
        {
          if(optarg[i]=='i')
            {
              iVal=1;
              opt_yield|=INSERT_YIELD;
            }
          else if (optarg[i]=='d')
          {
            dVal=1;
            opt_yield|=DELETE_YIELD;
          }
          else if(optarg[i]=='l')
          {
            lVal=1;
            opt_yield|=LOOKUP_YIELD;
          }
          else
          {
            fprintf(stderr,"Invalid argument for option --yield\n");
            exit(1);
          }
        }
        break;
      case 's':
        synced=1;
        if (strlen(optarg)==1)
        {
          if(optarg[0]=='m')
            syncType='m';
          else if(optarg[0]=='s')
            syncType='s';
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
  if(syncType=='m')
    typeM=1;
  if(syncType=='s')
    typeS=1;
  list=(SortedList_t *)malloc(1*sizeof(SortedList_t));
  list->key=NULL;
  list->next=list;
  list->prev=list;
  
  numElements=numThreads*numIts;
  elements=(SortedListElement_t*)malloc(numElements*sizeof(SortedListElement_t));
  srand(time(NULL));
  for (int i=0;i<numElements;i++)
  {
    int len= rand() % 10 + 1;
    int randChar=rand()%26;
    char* randKey=(char *) malloc((len+1)*sizeof(char));
    for(int k=0;k<len;k++)
    {
      randKey[k]='a'+ randChar;
      randChar=rand()%26;
    }
    randKey[len]='\0';
    elements[i].key=randKey;
  }
  
  pthread_t threads[numThreads];
  int threadIndices[numThreads];
  for(int i=0;i<numThreads;i++)
  {
    threadIndices[i]=i;
  }
  
  struct timespec start_time,end_time;
  clock_gettime(CLOCK_MONOTONIC,&start_time);
  

  for (int i=0;i<numThreads;i++)  //create threads
  {
    int status=pthread_create(&threads[i],NULL,&threadList,&threadIndices[i]); 
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
  
  int numOps=numThreads*numIts*3;
  int avgTime=elapsed_ns/numOps;
  
  int lenTemp=SortedList_length(list);
  if(lenTemp!=0)
  {
    fprintf(stderr,"Corrupted List: Length not 0\n");
    exit(2);
  }
  
  changeString();
  
  fprintf(stdout,"%s,%d,%d,1,%d,%lld,%d\n",listString,numThreads,numIts,numOps,
          elapsed_ns,avgTime);
}