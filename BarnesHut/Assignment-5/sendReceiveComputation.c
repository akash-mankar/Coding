#include <stdio.h>
#include <stdlib.h>
#include "sendReceiveComputation.h"

struct Queue computeQ;
pthread_mutex_t q_lock = PTHREAD_MUTEX_INITIALIZER;

/* void q_init(struct Queue* q) */
/* { */
/*   memset(q->items, 0, MAX_Q_SIZE * sizeof(struct qItem)); */
/*   q->q_head = 0; */
/*   q->q_tail = -1; */
/*   q->q_isInit = 1; */
/*   q->q_size = 0; */
/*   q->q_lock =  */
/* } */

/* void q_insert(struct Queue* q, const struct qItem* item) */
/* { */
  
/*   if (q->q_size < MAX_Q_SIZE) */
/*   { */
/*     q->q_tail = (q_tail + 1) % MAX_Q_SIZE; */
/*     q->q_items[q->q_tail] = *item; */
/*     q->q_size++; */
/*   } */
/* } */


