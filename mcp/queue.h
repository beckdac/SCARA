#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef struct queue {
	void **q;
	unsigned int size;
	int first;
	int last;
	int count;
} queue;

#endif /* __QUEUE_H__ */
