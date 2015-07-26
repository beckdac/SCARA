#ifndef __PRIQ_H__
#define __PRIQ_H__

// convienence defines
#define PRIQ_HIGH	1
#define PRIQ_NORMAL	2
#define PRIQ_LOW	3

typedef struct { void * data; int pri; } q_elem_t;
typedef struct { q_elem_t *buf; int n, alloc; } pri_queue_t, *pri_queue;

pri_queue priq_new(int size);
void priq_push(pri_queue q, void *data, int pri);
void * priq_pop(pri_queue q, int *pri);
void* priq_top(pri_queue q, int *pri);
void priq_combine(pri_queue q, pri_queue q2);

#endif /* __PRIQ_H__ */
