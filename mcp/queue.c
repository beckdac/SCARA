#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "queue.h"

void queueInit(queue *q, unsigned int size) {
	q->q = (void **)malloc(sizeof(void *) * size);
	q->size = size;
	q->first = 0;
	q->last = size - 1;
	q->count = 0;
}

void queueEnqueue(queue *q, void *x) {
	if (q->count >= q->size) {
		warning("queue overflow in enqueue 0x%x\n",x);
	} else {
		q->last = (q->last+1) % q->size;
		q->q[ q->last ] = x;    
		q->count = q->count + 1;
	}
}

void *queueDequeue(queue *q) {
	void *x;

	if (q->count <= 0) {
		warning("queueEmpty queue in dequeue\n");
	} else {
		x = q->q[q->first];
		q->first = (q->first+1) % q->size;
		q->count = q->count - 1;
	}

	return(x);
}

int queueEmpty(queue *q) {
	if (q->count <= 0)
		return 1;
	else
		return 0;
}

void queuePrint(queue *q) {
	int i;

	i=q->first; 
	while (i != q->last) {
		printf("0x%x ", (unsigned int)q->q[i]);
		i = (i+1) % q->size;
	}
	printf("0x%x ", (unsigned int)q->q[i]);
	printf("\n");
}


