#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef struct queue {
	void **q;
	unsigned int size;
	int first;
	int last;
	int count;

	pthread_mutex_t mutex;
} queue;

void queueInit(queue *q, unsigned int size);
void queueEnqueue(queue *q, void *x);
void *queueDequeue(queue *q);
int queueEmpty(queue *q);
void queuePrint(queue *q);

#endif /* __QUEUE_H__ */
