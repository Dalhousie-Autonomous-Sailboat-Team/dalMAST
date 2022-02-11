#ifndef QUEUE_H
#define QUEUE_H

/* Queue structure */
#define QUEUE_ELEMENTS 100
#define QUEUE_SIZE (QUEUE_ELEMENTS + 1)

// definition of the object
typedef struct {
	char Queue[QUEUE_SIZE];
	int QueueIn, QueueOut;
	int QueueSize;
}QueueC;

// object interface
int GetSize(QueueC* Queue);
void QueueInit(QueueC* Queue);
int QueuePut(char new, QueueC* Queue);
int QueueGet(char *old, QueueC* Queue);

#endif