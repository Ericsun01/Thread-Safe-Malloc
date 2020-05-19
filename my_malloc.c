#include "my_malloc.h"

block_ptr head=NULL; // head of freelist
size_t heapsize=0;
__thread block_ptr head_nolock=NULL;


block_ptr first_find(block_ptr curr, size_t size)
{
  block_ptr search=curr;
  if(search==NULL)
    {return NULL;}
  while(search!=NULL)
    {if(search->size<size)
	{search=search->next;}
     else
	{break;}
    }
  return search;
}

block_ptr best_find(block_ptr curr, size_t size)
{
 block_ptr search=first_find(curr,size);
  block_ptr best=search;
  if(search==NULL)
    {return NULL;}
  while(search!=NULL)
      { if(search->size >= size)
	  {
            if(search->size==size)
	      { best=search;
		break; }
	    else if(search->size<best->size)
	      { best=search;}
	  }
        search=search->next;
    }
  return best;
}

block_ptr alloc_new(size_t size)
{
  block_ptr space=sbrk(sizeof(struct block)+size);
  if(space==(void*)-1)
    {return NULL;}
  else
    { space->size=size;
      space->prev=NULL;
      space->next=NULL;
      space->isfree=0;}
  heapsize+=sizeof(struct block)+size;
  return space;
}

block_ptr alloc_new_nolock(size_t size)
{
  pthread_mutex_lock(&lock);
  block_ptr space=sbrk(sizeof(struct block)+size);
  pthread_mutex_unlock(&lock);
  if(space==(void*)-1)
    {return NULL;}
  else
    { space->size=size;
      space->prev=NULL;
      space->next=NULL;
      space->isfree=0;}
  heapsize+=sizeof(struct block)+size;
  return space;
}

void addfreelist(block_ptr op)
{// printf("op is %p\n",op);
  block_ptr location=head;
  // printf("head is %p\n",head);
  if(head==NULL)  // no head 
    {head=op;
     op->next=NULL;
     op->prev=NULL;
     return;}
  if(op<head)  // head update
    {op->prev=NULL;
     op->next=head;
     head->prev=op;
     head=op;
     return;}
  while(location->next!=NULL) // search through the free list, locate the new free block
    { 
      if(op>location)
	{location=location->next;
	  // printf("location is %p\n",location);
	}
      else if(op<location)
	{op->next=location;
	 op->prev=location->prev;
	 location->prev->next=op;
	 location->prev=op;
	 // printf("inserted op is %p\n",op);
	 return;
	 }
      else
	{return;}
    }
  if(op>location) // add to tail
    {op->next=NULL;
     op->prev=location;
     location->next=op;
     //printf("last op is %p\n",op);
    }
  if(op<location)
    {op->next=location;
      op->prev=location->prev;
      location->prev->next=op;
      location->prev=op;
     //printf("Second last op is %p\n",op);
    }
  
}

void addfreelist_nolock(block_ptr op)
{// printf("op is %p\n",op);
  block_ptr location=head_nolock;
  // printf("head is %p\n",head);
  if(head_nolock==NULL)  // no head 
    {head_nolock=op;
     op->next=NULL;
     op->prev=NULL;
     return;}
  if(op<head_nolock)  // head update
    {op->prev=NULL;
     op->next=head_nolock;
     head_nolock->prev=op;
     head_nolock=op;
     return;}
  while(location->next!=NULL) // search through the free list, locate the new free block
    { 
      if(op>location)
	{location=location->next;
	  // printf("location is %p\n",location);
	}
      else if(op<location)
	{op->next=location;
	 op->prev=location->prev;
	 location->prev->next=op;
	 location->prev=op;
	 // printf("inserted op is %p\n",op);
	 return;
	 }
      else
	{return;}
    }
  if(op>location) // add to tail
    {op->next=NULL;
     op->prev=location;
     location->next=op;
     //printf("last op is %p\n",op);
    }
  if(op<location)
    {op->next=location;
      op->prev=location->prev;
      location->prev->next=op;
      location->prev=op;
     //printf("Second last op is %p\n",op);
    }
  
}
		 
block_ptr split(block_ptr use, size_t size)
{
  block_ptr new=(block_ptr)((char*)use+use->size-size); // we utilize the new block as isfree=0
  use->size=use->size-size-sizeof(struct block);
  new->size=size;
  new->isfree=0;
  new->next=NULL;
  new->prev=NULL;
  return new;
}

block_ptr merge(block_ptr mer)  // single direction
{ 
  if(mer->next!=NULL)
    { block_ptr fus=mer->next;
      if((char*)mer+sizeof(struct block)+mer->size==(char*)fus) // check if fus is the physical neighbor of mer
	{ mer->size+=sizeof(struct block)+fus->size;
	  mer->next=fus->next;
	  if(mer->next)
	    { mer->next->prev=mer;}
	}
    }
  return mer;
}

void *bf_malloc(size_t size)
{
  block_ptr curr=NULL;
  size_t s=size;
  void *demo=NULL;
  if(head!=NULL)  // whether freelist is empty
    {
      curr=best_find(head,s);
      if(curr!=NULL) // find the free block we want
	{
	  if((curr->size-s)>=(sizeof(struct block)+8))
	    {curr=split(curr,s);}
	  else // find it but don't need to split 
	    { curr->isfree=0;
	      if(curr==head&&curr->next==NULL)
		{head=NULL;}
	      else if(curr==head&&curr->next!=NULL)
		{head=curr->next;
		  head->prev=NULL;
		  curr->next=NULL;}
	      else if(curr!=head&&curr->next==NULL)
		{curr->prev->next=NULL;
		  curr->prev=NULL;}
	      else
		{curr->prev->next=curr->next;
		 curr->next->prev=curr->prev;
		 curr->next=NULL;
		 curr->prev=NULL;} 
	    }
	}
      else // no block we want, alloc a new
	{curr=alloc_new(size);
	 if(curr==NULL)
	   {return NULL;}
	}
    }
  else
    {
      curr=alloc_new(size);
      if(curr==NULL)
	{return NULL;}    
    }


  demo=(void*)((char*)curr+sizeof(struct block));
  // printf("demo is %p\n",demo);

  return demo;
}

void *ts_malloc_nolock(size_t size)
{
  block_ptr curr=NULL;
  size_t s=size;
  void *demo=NULL;
  if(head_nolock!=NULL)  // whether freelist is empty
    {
      curr=best_find(head_nolock,s);
      if(curr!=NULL) // find the free block we want
	{
	  if((curr->size-s)>=(sizeof(struct block)+8))
	    {curr=split(curr,s);}
	  else // find it but don't need to split 
	    { curr->isfree=0;
	      if(curr==head_nolock&&curr->next==NULL)
		{head_nolock=NULL;}
	      else if(curr==head_nolock&&curr->next!=NULL)
		{head_nolock=curr->next;
		  head_nolock->prev=NULL;
		  curr->next=NULL;}
	      else if(curr!=head_nolock&&curr->next==NULL)
		{curr->prev->next=NULL;
		  curr->prev=NULL;}
	      else
		{curr->prev->next=curr->next;
		 curr->next->prev=curr->prev;
		 curr->next=NULL;
		 curr->prev=NULL;} 
	    }
	}
      else // no block we want, alloc a new
	{curr=alloc_new_nolock(size);
	 if(curr==NULL)
	   {return NULL;}
	}
    }
  else
    {
      curr=alloc_new_nolock(size);
      if(curr==NULL)
	{return NULL;}    
    }


  demo=(void*)((char*)curr+sizeof(struct block));
  // printf("demo is %p\n",demo);

  return demo;
}

void bf_free(void *ptr)
{
  if(ptr==NULL||ptr>=sbrk(0))
    {return;}
  block_ptr temp=(block_ptr)((char*)ptr-sizeof(struct block));
  temp->isfree=1;
  addfreelist(temp);
  if(temp->prev!=NULL && temp->prev->isfree==1)
    {temp=merge(temp->prev);}
  if(temp->next!=NULL && temp->next->isfree==1)
    {temp=merge(temp);}
}

void ts_free_nolock(void *ptr)
{
  pthread_mutex_lock(&lock);
  void *breakpoint=sbrk(0);
  pthread_mutex_unlock(&lock);
  if(ptr==NULL||ptr>=breakpoint)
    {return;}
  block_ptr temp=(block_ptr)((char*)ptr-sizeof(struct block));
  temp->isfree=1;
  addfreelist_nolock(temp);
  if(temp->prev!=NULL && temp->prev->isfree==1)
    {temp=merge(temp->prev);}
  if(temp->next!=NULL && temp->next->isfree==1)
    {temp=merge(temp);}
}


void *ts_malloc_lock(size_t size)
{ pthread_mutex_lock(&lock);
  void *ans=bf_malloc(size);
  pthread_mutex_unlock(&lock);
  return ans;
 
}

void ts_free_lock(void *ptr)
{
  pthread_mutex_lock(&lock);
  bf_free(ptr);
  pthread_mutex_unlock(&lock);
}
/*
unsigned long get_data_segment_size()
{
  return heapsize;
 }

unsigned long get_data_segment_free_space_size()
{
  unsigned long count=0;
  block_ptr iter=head;
  while(iter!=NULL)
    {
      count+=iter->size;
      count+=sizeof(struct block);
      iter=iter->next;
    }
  return count;
}
*/
