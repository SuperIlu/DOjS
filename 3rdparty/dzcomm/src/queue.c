/*
 * DZcomm : a serial port API.
 * file : queue.c
 *
 * Provides FIFO queues for dzcomm
 * 
 * By Dim Zegebart
 *
 * See readme.txt for copyright information.
 */

/* This #define tells dzcomm.h that this is source file for the library and
 * not to include things the are only for the users code.
 */
#define DZCOMM_LIB_SRC_FILE

#include "dzcomm.h"
#include "dzcomm/dzintern.h"

#define QH(q) q->queue+q->head
#define QT(q) q->queue+q->tail

/*------------------------- FIFO QUEUE -----------------------------*/
/* 'first in first out' queue functions */

inline void queue_reset(fifo_queue *q)
{
   dz_disable_interrupts();
   q->tail        = 0;
   q->head        = 0;
   q->full        = DZQ_NOT_FULL;
   q->nearly_full = DZQ_NOT_NEARLY_FULL;
   dz_enable_interrupts();
}
END_OF_FUNCTION(queue_reset);


int queue_resize(fifo_queue *q,unsigned int new_size)
{
   int *tmp;

   dz_disable_interrupts();
   if ((tmp=(int*)realloc(q->queue,sizeof(int)*new_size))==NULL) {
      dz_enable_interrupts();
      return DZQ_FAIL_NO_MEM;
   }

   if (new_size>q->size) q->resize_counter++;
   else if (new_size<=q->initial_size) q->resize_counter=0;
   else q->resize_counter--;

   q->queue=tmp;
   q->size=new_size;
   q->fill_level=3*(q->size>>2); // 3/4
   queue_reset(q);
   dz_enable_interrupts();

   return DZQ_SUCCESS;
}

/*-------------------- QUEUE_DELETE --------------------------*/

void queue_delete(fifo_queue *q)
{ if (q==NULL) return;
  free(q->queue);
  free(q);
}

/*--------------------- QUEUE_PUT ------------------------------*/

int queue_put(fifo_queue *q,int c)
{ return(queue_put_(q,&c));
}
END_OF_FUNCTION(queue_put);

int queue_put_(fifo_queue *q,void *c)
{
   if (!q)      return DZQ_FAIL_NO_Q;
   if (q->full) return DZQ_FAIL_FULL;

   dz_disable_interrupts();
   memcpy(QT(q),c,q->dsize);
   q->tail+=q->dsize;
   if(q->tail>(q->size-q->dsize)) q->tail=0;

   if (q->tail==q->head) {
      q->full        = DZQ_FULL;
      q->nearly_full = DZQ_NEARLY_FULL;
   }
   else {
      q->nearly_full = DZQ_NOT_NEARLY_FULL;
      if (q->head<q->tail) {
	 if ((q->tail-q->head)>=q->fill_level) {
	    q->nearly_full = DZQ_NEARLY_FULL;
	 }
      }
      else {
	 if ((q->head-q->tail)<=(q->size-q->fill_level)) {
	    q->nearly_full = DZQ_NEARLY_FULL;
	 }
      }
   }

   dz_enable_interrupts();

   return(q->nearly_full);
}
END_OF_FUNCTION(queue_put_);

/*---------------------- QUEUE GET ---------------------------*/

int queue_get(fifo_queue *q)
{ int c;
  queue_get_(q,&c);
  return(c);
}
END_OF_FUNCTION(queue_get);

int queue_get_(fifo_queue *q,void *c)
{
   if (!q) return DZQ_FAIL_NO_Q;

   dz_disable_interrupts();
   if ((q->head==q->tail) && !q->full) {
       dz_enable_interrupts();
       return DZQ_FAIL_EMPTY;
   }

   memcpy(c,QH(q),q->dsize);
   q->head+=q->dsize;

   if (q->head>(q->size-q->dsize)) q->head=0;
   if (q->head==q->tail) queue_reset(q);

   q->full        = DZQ_NOT_FULL;
   q->nearly_full = DZQ_NOT_NEARLY_FULL;
   if (q->head<q->tail) {
      if ((q->tail-q->head)>=q->fill_level) {
	 q->nearly_full = DZQ_NEARLY_FULL;
      }
   }
   else {
      if ((q->head-q->tail)<=(q->size-q->fill_level)) {
	 q->nearly_full = DZQ_NEARLY_FULL;
      }
   }
   dz_enable_interrupts();

   return(q->nearly_full);
}
END_OF_FUNCTION(queue_get_);

/*---------------------- QUEUE PEEK ----------------------------*/
int queue_peek(fifo_queue *q)
{
   int c;

   queue_peek_(q,&c);
   return c;
}

int queue_peek_(fifo_queue *q,void *c)
{
   int rv;
   dz_disable_interrupts();
   if ((q->head==q->tail) && !q->full) rv = DZQ_FAIL_EMPTY;
   else {
      memcpy(c,QH(q),q->dsize);
      rv = q->nearly_full;
   }
   dz_enable_interrupts();

   return(rv);
}

/*---------------------- QUEUE EMPTY ---------------------------*/

int queue_empty(fifo_queue *q)
{
   int rv = DZQ_NOT_EMPTY;

   if (q==NULL) return(-1);

   dz_disable_interrupts();
   if ((q->head==q->tail) && !q->full) {
      if (q->empty_handler!=NULL) rv = DZQ_FAIL_INTERNAL;
      else                        rv = DZQ_EMPTY;
   }
   dz_enable_interrupts();

   return(rv);
}
END_OF_FUNCTION(queue_empty);


/*---------------------- QUEUE (NEARLY) FULL ------------------------*/

int queue_nearly_full(fifo_queue *q)
{ 
   if (q==NULL) return(-1);
   return(q->nearly_full);
}
END_OF_FUNCTION(queue_nearly_full);

int queue_full(fifo_queue *q)
{
   if (q==NULL) return(-1);
   return (q->full);
}
END_OF_FUNCTION(queue_full);

/*------------------------------- QUEUE NEW --------------------------*/

fifo_queue* queue_new(unsigned int size)
{
  return(queue_new_(size,sizeof(int)));
}

/*------------------------- QUEUE_NEW_ --------------------------*/

fifo_queue* queue_new_(unsigned int size,unsigned int dsize)
{ fifo_queue *q=NULL;

  if (dsize<=0||dsize>4) return(NULL);
  if ((q=(fifo_queue*)malloc(sizeof(fifo_queue)))==NULL) return(NULL);

  if (!size) size=1024; //if illegal size, turn it to default size
  if ((q->queue=malloc(sizeof(int)*size*dsize))==NULL) {
     free(q);
     return(NULL);
  }

  q->dsize          = dsize;
  q->size           = size*dsize;
  q->initial_size   = size*dsize;//size;
  q->resize_counter = 0;
  q->nearly_full    = 0;
  q->fill_level     = 3*q->size/4; // 3/4
  q->empty_handler  = NULL;
  queue_reset(q);
  LOCK_DATA(q->queue,sizeof(int)*size*dsize);
  LOCK_DATA(q, sizeof(fifo_queue));

  return(q);
}

void lock_queue_functions(void)
{
  LOCK_FUNCTION(queue_empty);
  LOCK_FUNCTION(queue_nearly_full);
  LOCK_FUNCTION(queue_full);
  LOCK_FUNCTION(queue_put);
  LOCK_FUNCTION(queue_get);
  LOCK_FUNCTION(queue_put_);
  LOCK_FUNCTION(queue_get_);
  LOCK_FUNCTION(queue_reset);
}
