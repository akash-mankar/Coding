#pragma once

#include "barnesHut.h"
#include <pthread.h>

#define MAX_Q_SIZE 1000

struct qItem
{
  struct node* aBody;
  int procRank;
};

struct Queue
{
  struct qItem q_items[MAX_Q_SIZE];
  int q_head;
  int q_tail;
  int q_isInit;
  int q_size;
  pthread_mutex_t q_lock;
};

// This method initializes the queue - should be called only once per queue 
void q_init(struct Queue* q);

// This method inserts 'item' to the bottom of the queue
// If the queue is full, this method sleeps till space is
// available, and then inserts the item to the bottom of the queue
// This should be used by only the main thread
void q_insert(struct Queue* q, const struct qItem* item);

// This method removes the topmost entry from the queue
// The way this function should be called is the following:
// call acquire lock -> call q_remove() -> release lock
struct qItem* q_remove(struct Queue* q, int numToRemove);
