#include <stdio.h>
#include <stdlib.h>
#include<pthread.h>

#include "error.h"
#include "queue.h"

void queueInit(queue *q, unsigned int size) {
	q->q = (void **)malloc(sizeof(void *) * size);
	q->size = size;
	q->first = 0;
	q->last = size - 1;
	q->count = 0;

	if (pthread_mutex_init(&q->mutex, NULL) != 0) {
		perror(NULL);
		fatal_error("queueInit: mutex init failed\n");
	}
}

int queueCount(queue *q) {
	int count;
	pthread_mutex_lock(&q->mutex);
	count = q->count;
	pthread_mutex_unlock(&q->mutex);
	return count;
}

void queueEnqueue(queue *q, void *x) {
	pthread_mutex_lock(&q->mutex);

	if (q->count >= q->size) {
		warning("queue overflow in enqueue 0x%x\n",x);
	} else {
		q->last = (q->last+1) % q->size;
		q->q[ q->last ] = x;    
		q->count = q->count + 1;
	}

	pthread_mutex_unlock(&q->mutex);
}

void *queueDequeue(queue *q) {
	void *x;

	pthread_mutex_lock(&q->mutex);

	if (q->count <= 0) {
		warning("queue is empty for dequeue\n");
	} else {
		x = q->q[q->first];
		q->first = (q->first+1) % q->size;
		q->count = q->count - 1;
	}

	pthread_mutex_unlock(&q->mutex);

	return(x);
}

int queueEmpty(queue *q) {
	int isEmpty;

	pthread_mutex_lock(&q->mutex);
	if (q->count <= 0)
		isEmpty=1;
	else
		isEmpty=0;
	pthread_mutex_unlock(&q->mutex);

	return isEmpty;
}

void queuePrint(queue *q) {
	int i;

	pthread_mutex_lock(&q->mutex);

	i=q->first; 
	while (i != q->last) {
		printf("0x%x ", (unsigned int)q->q[i]);
		i = (i+1) % q->size;
	}
	printf("0x%x ", (unsigned int)q->q[i]);
	printf("\n");

	pthread_mutex_unlock(&q->mutex);
}


