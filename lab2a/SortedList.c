#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "SortedList.h" 

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
  if(list==NULL || element==NULL)
    return;
  SortedList_t* curr = list->next;

  while(curr!=list)
  {
    if(strcmp(element->key,curr->key)<1)
      break;
    curr=curr->next;
  }
  
  if(opt_yield & INSERT_YIELD)
    sched_yield();
  
  element->next=curr;
  element->prev=curr->prev;
  curr->prev->next=element;
  curr->prev=element;
  return;
}

int SortedList_delete( SortedListElement_t *element)
{
  if(element==NULL)
    return 1;

  if(element->next->prev!=element->prev->next)
    return 1;
  else
  {    
    if (opt_yield & DELETE_YIELD)
      sched_yield();
    
    element->prev->next=element->next;
    element->next->prev=element->prev;
         
    return 0;
  }

}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
  if(list==NULL)
    return NULL;
  SortedList_t* curr = list->next;
  
  while(curr!=list)
  {
    if(strcmp(curr->key,key)>0)
      return NULL;
    if(strcmp(curr->key,key)==0)
      return curr;
    if(opt_yield&LOOKUP_YIELD)
      sched_yield();
    curr=curr->next;
  }
  return NULL;

}

int SortedList_length(SortedList_t *list)
{
  if(list==NULL)
    return -1;
  SortedList_t* curr = list->next;
 
  int elements=0;
  while(curr!=list)
  {
    if(curr==NULL)
      return -1;
    elements++;
    if(opt_yield&LOOKUP_YIELD)
      sched_yield();
    curr=curr->next;
  }
  return elements;
}