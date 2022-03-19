/*
 * DZcomm : a serial port API.
 * file : dzqueue.h
 *
 * Include file for the fifo queue stuff
 * 
 * See readme.txt for copyright information.
 */

#ifndef DZQUEUE_H
#define DZQUEUE_H

typedef struct
{ unsigned int    size;  /* size of queue.                         */
  unsigned int    dsize; /* size of data element (default 4 bytes) */
  unsigned int    initial_size;
  unsigned int    resize_counter;
  unsigned int    fill_level;
  int             full;
  int             nearly_full;
  int           (*empty_handler)();
  void           *queue; /* pointer to memory where the data is stored */

  /* Position in the queue of the head and tail elements:
   * head points to the first element in queue
   * tail points to the first element free element in queue. 
   * For example : we have ten elements in queue, then head==0, tail==10
   */
  unsigned int    head,tail;
} fifo_queue;

/* Return values */
#define DZQ_SUCCESS          1
#define DZQ_FULL             1
#define DZQ_NOT_FULL         0
#define DZQ_NEARLY_FULL      1
#define DZQ_NOT_NEARLY_FULL  0
#define DZQ_EMPTY            1
#define DZQ_NOT_EMPTY        0
#define DZQ_FAIL_NO_Q       -1
#define DZQ_FAIL_FULL       -2
#define DZQ_FAIL_EMPTY      -3
#define DZQ_FAIL_NO_MEM     -4
#define DZQ_FAIL_INTERNAL   -5

/* Set up queues. Return pointer to new queue on success, NULL on failure */
DZ_FUNC(fifo_queue *, queue_new,  (unsigned int size));
DZ_FUNC(fifo_queue *, queue_new_, (unsigned int size,unsigned int dsize));

/* Delete or reset existing queues */
DZ_FUNC(void, queue_delete, (fifo_queue *q));
DZ_FUNC(void, queue_reset,  (fifo_queue *q));

/* Resize existing queue: returns 0 on failure, 1 on success */
DZ_FUNC(int, queue_resize, (fifo_queue *q, unsigned int new_size));

/* Use queue_empty befor calling queue_get. queue_get doesn't test
 * queue's head and tail, it unquestioningly returns queue[head].
 * Returns 1 if empty (head == tail) or 0 if not empty (head != tail).
 */
DZ_FUNC(int, queue_empty, (fifo_queue *q));

/* Put things into, peek at and get things from the queue */
DZ_FUNC(int, queue_put,   (fifo_queue *q,int c));
DZ_FUNC(int, queue_peek,  (fifo_queue *q));
DZ_FUNC(int, queue_get,   (fifo_queue *q));
DZ_FUNC(int, queue_put_,  (fifo_queue *q,void *data));
DZ_FUNC(int, queue_peek_, (fifo_queue *q,void *data));
DZ_FUNC(int, queue_get_,  (fifo_queue *q,void *data));

/* Function to let you know about buffer fullness.
 * queue_nearly_full returns 1 if the queue is nearly full; 0 otherwise.
 * queue_full returns 1 if the queue is full; 0 otherwise.
 */
DZ_FUNC(int, queue_nearly_full, (fifo_queue *q));
DZ_FUNC(int, queue_full, (fifo_queue *q));

#endif /* DZQUEUE_H */
