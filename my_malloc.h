#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct block *block_ptr;
typedef struct block myblock;
struct block{
  size_t size;
  block_ptr prev;
  block_ptr next;
  int isfree;
  };
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;



block_ptr first_find(block_ptr curr, size_t size);
block_ptr best_find(block_ptr curr, size_t size);
block_ptr alloc_new(size_t size);
block_ptr alloc_new_nolock(size_t size);
block_ptr merge(block_ptr mer);
void addfreelist(block_ptr op);
void addfreelist_nolock(block_ptr op);
block_ptr split(block_ptr use, size_t size);
void *bf_malloc(size_t size);
void bf_free(void *ptr);
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();
