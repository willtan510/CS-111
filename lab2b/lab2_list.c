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
int numLists=1;
char syncType;
int opt_yield=0;
static int synced=0;
int fatal;
int iVal,dVal,lVal;
int numElements=0;
char* listString="";
SortedListElement_t* elements;
SortedList_t* lists;
int * eBelongsIn;
//SortedListElement_t** elementsArr;
int typeM,typeS;
int * spinLocks;
pthread_mutex_t* mutexLocks;
int lenSpinLock;
pthread_mutex_t lenMutexLock;

void *threadList(void* index)
{
  struct timespec start_time1,end_time1;
  int listLen=0;
  int indice=*((int *)index);
  long long threadTime=0;
  for (int i=indice;i<numElements;i+=numThreads)
  {
    if(typeM)
    {
      clock_gettime(CLOCK_MONOTONIC,&start_time1);
      
      pthread_mutex_lock(&mutexLocks[eBelongsIn[i]]);
      clock_gettime(CLOCK_MONOTONIC, &end_time1);
      SortedList_insert(&lists[eBelongsIn[i]],&elements[i]);
      long long elapsed_ns1=(end_time1.tv_sec-start_time1.tv_sec)*1000000000;
      elapsed_ns1+=end_time1.tv_nsec;
      elapsed_ns1-=start_time1.tv_nsec;
      threadTime+=elapsed_ns1;
      pthread_mutex_unlock(&mutexLocks[eBelongsIn[i]]);
      
    }
    else if(typeS)
    {
      clock_gettime(CLOCK_MONOTONIC,&start_time1);
      while(__sync_lock_test_and_set(&spinLocks[eBelongsIn[i]],1));
      clock_gettime(CLOCK_MONOTONIC, &end_time1);
      long long elapsed_ns1=(end_time1.tv_sec-start_time1.tv_sec)*1000000000;
      elapsed_ns1+=end_time1.tv_nsec;
      elapsed_ns1-=start_time1.tv_nsec;
      threadTime+=elapsed_ns1;
      SortedList_insert(&lists[eBelongsIn[i]],&elements[i]);
      __sync_lock_release(&spinLocks[eBelongsIn[i]]);
    }
    else
    {
      SortedList_insert(&lists[eBelongsIn[i]],&elements[i]);
    }
  }
  
  if(typeM)
  {
    for(int i=0;i<numLists;i++)
    {
        int temp=0;
        clock_gettime(CLOCK_MONOTONIC,&start_time1);
        
        pthread_mutex_lock(&mutexLocks[i]);
        
        clock_gettime(CLOCK_MONOTONIC, &end_time1);
        long long elapsed_ns1=(end_time1.tv_sec-start_time1.tv_sec)*1000000000;
        elapsed_ns1+=end_time1.tv_nsec;
        elapsed_ns1-=start_time1.tv_nsec;
        threadTime+=elapsed_ns1;
        
        temp=SortedList_length(&lists[i]);
        pthread_mutex_unlock(&mutexLocks[i]);
  
        if(temp==-1)
        {
          fprintf(stderr,"Corrupted List: Error during attempt to get length\n");
          exit(2);
        }
        clock_gettime(CLOCK_MONOTONIC,&start_time1);
        
        pthread_mutex_lock(&lenMutexLock);
        
        clock_gettime(CLOCK_MONOTONIC, &end_time1);
        elapsed_ns1=(end_time1.tv_sec-start_time1.tv_sec)*1000000000;
        elapsed_ns1+=end_time1.tv_nsec;
        elapsed_ns1-=start_time1.tv_nsec;
        threadTime+=elapsed_ns1;
        
        listLen+=temp;
        pthread_mutex_unlock(&lenMutexLock);
    }
    

  }
  else if(typeS)
  {
    listLen=SortedList_length(lists);

    for(int i=0;i<numLists;i++)
    {
      int temp=0;
      clock_gettime(CLOCK_MONOTONIC,&start_time1);
      while(__sync_lock_test_and_set(&spinLocks[i],1));
      clock_gettime(CLOCK_MONOTONIC, &end_time1);
      long long elapsed_ns1=(end_time1.tv_sec-start_time1.tv_sec)*1000000000;
      elapsed_ns1+=end_time1.tv_nsec;
      elapsed_ns1-=start_time1.tv_nsec;
      threadTime+=elapsed_ns1;
      
      temp=SortedList_length(&lists[i]);
      __sync_lock_release(&spinLocks[i]);
      if(temp==-1)
      {
        fprintf(stderr,"Corrupted List: Error during attempt to get length\n");
        exit(2);
      }
      
      clock_gettime(CLOCK_MONOTONIC,&start_time1);
        
      while(__sync_lock_test_and_set(&lenSpinLock,1));
        
      clock_gettime(CLOCK_MONOTONIC, &end_time1);
      elapsed_ns1=(end_time1.tv_sec-start_time1.tv_sec)*1000000000;
      elapsed_ns1+=end_time1.tv_nsec;
      elapsed_ns1-=start_time1.tv_nsec;
      threadTime+=elapsed_ns1;
        
      listLen+=temp;
      __sync_lock_release(&lenSpinLock);   
      
    }
  }
  else
  {
      for(int i=0;i<numLists;i++)
      {
        int temp=0;
        temp=SortedList_length(&lists[i]);
        if(temp==-1)
        {
          fprintf(stderr,"Corrupted List: Error during attempt to get length\n");
          exit(2);
        }
        listLen+=temp;
      }
  }
  
  for(int i=indice;i<numElements;i+=numThreads)
  {
    if(typeM)
    {
      clock_gettime(CLOCK_MONOTONIC,&start_time1);
      pthread_mutex_lock(&mutexLocks[eBelongsIn[i]]);
      clock_gettime(CLOCK_MONOTONIC, &end_time1);
      long long elapsed_ns1=(end_time1.tv_sec-start_time1.tv_sec)*1000000000;
      elapsed_ns1+=end_time1.tv_nsec;
      elapsed_ns1-=start_time1.tv_nsec;
      threadTime+=elapsed_ns1;
      SortedListElement_t* temp=SortedList_lookup(&lists[eBelongsIn[i]],elements[i].key);
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
      pthread_mutex_unlock(&mutexLocks[eBelongsIn[i]]);
    }
    else if(typeS)
    {
      clock_gettime(CLOCK_MONOTONIC,&start_time1);
      while(__sync_lock_test_and_set(&spinLocks[eBelongsIn[i]],1));
      clock_gettime(CLOCK_MONOTONIC, &end_time1);
      long long elapsed_ns1=(end_time1.tv_sec-start_time1.tv_sec)*1000000000;
      elapsed_ns1+=end_time1.tv_nsec;
      elapsed_ns1-=start_time1.tv_nsec;
      threadTime+=elapsed_ns1;
      SortedListElement_t* temp=SortedList_lookup(&lists[eBelongsIn[i]],elements[i].key);
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
      __sync_lock_release(&spinLocks[eBelongsIn[i]]);
    }
    else
    {
      SortedListElement_t* temp=SortedList_lookup(&lists[eBelongsIn[i]],elements[i].key);
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
  return (void *)threadTime;
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
  long long totalLockTime=0;
  int option;  
  listString=(char *)malloc(25);
  signal(SIGSEGV,signal_handler);
  
  static struct option long_options[]=
  {
    {"threads",required_argument,0,'t'},
    {"iterations",required_argument,0,'i'},
    {"yield",required_argument,0,'y'},
    {"sync",required_argument,0,'s'},
    {"lists",required_argument,0,'l'},
    {0,0,0,0}
  };
  while(1)
  {
    option=getopt_long(argc,argv,"t:i:y:s:l:",long_options,0);
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
      case 'l':
        numLists=atoi(optarg);
        break;
      case '?':
        fatal=errno;
        fprintf(stderr,"%s\n",strerror(fatal));
        exit(1);
    }
  }  
  if(syncType=='m')
    {
      typeM=1;
      mutexLocks=malloc(numLists*sizeof(pthread_mutex_t));
    }
  if(syncType=='s')
    {
      typeS=1;
      spinLocks=malloc(numLists*sizeof(int));
      for(int i=0;i<numLists;i++)
        spinLocks[i]=0;
    }
  lists=(SortedList_t *)malloc(numLists*sizeof(SortedList_t));
  if(pthread_mutex_init(&lenMutexLock,NULL)!=0)
  {
    fatal=errno;
    fprintf(stderr,"%s\n",strerror(fatal));
    exit(1);
  }
  for (int i=0;i<numLists;i++)
  {
    lists[i].key=NULL;
    lists[i].next=&lists[i];
    lists[i].prev=&lists[i];
    if(typeM)
    {
      if(pthread_mutex_init(&mutexLocks[i],NULL)!=0)
      {
          fatal=errno;
          fprintf(stderr,"%s\n",strerror(fatal));
          exit(1);
      }
    }
  }
  
  numElements=numThreads*numIts;
/*  elementsArr=(SortedListElement_t**)malloc(numLists*sizeof(SortedListElement_t *));
  for (int i=0;i<numLists;i++)
    elementsArr[i]=(SortedListElement_t *)malloc(numElements*sizeof(SortedListElement_t));*/
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
  

 /* int hashCounter[numLists];
  for(int i=0;i<numElements;i++)
  {
    int hashNum=(elements[i].key[0])%numLists;
    elementsArr[hashNum][hashCounter[hashNum]]=elements[i];
    hashCounter[hashNum]++;
  }*/
  
  eBelongsIn=malloc(numElements*sizeof(int));
  
  for(int i=0;i<numElements;i++)
  {
    int hashNum=(elements[i].key[0])%numLists;
    eBelongsIn[i]=hashNum;
  }
  
  long long threadTimes[numThreads];
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
    int status=pthread_join(threads[i],(void **)&threadTimes[i]);
    if (status==-1)
    {
      fatal=errno;
      fprintf(stderr,"%s\n",strerror(fatal));
      exit(1);
    }
  }
  clock_gettime(CLOCK_MONOTONIC, &end_time);
  for(int i=0;i<numThreads;i++)
    totalLockTime+=threadTimes[i];
  long long elapsed_ns=(end_time.tv_sec-start_time.tv_sec)*1000000000;
  elapsed_ns+=end_time.tv_nsec;
  elapsed_ns-=start_time.tv_nsec;

  int numOps=numThreads*numIts*3;
  int avgTime=elapsed_ns/numOps;
  long long avgLockTime=totalLockTime/numOps;
  int lenTemp=SortedList_length(lists);
  if(lenTemp!=0)
  {
    fprintf(stderr,"Corrupted List: Length not 0\n");
    exit(2);
  }
  
  changeString();
  
  fprintf(stdout,"%s,%d,%d,%d,%d,%lld,%d,%lld\n",listString,numThreads,numIts,numLists,numOps,
          elapsed_ns,avgTime,avgLockTime);
free(listString);
free(elements);
free(lists);
free(eBelongsIn);
free(spinLocks);
free(mutexLocks);
}