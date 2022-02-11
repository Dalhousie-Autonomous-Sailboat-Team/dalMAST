#include "Queue.h"

/******************************************************************************
******************************************************************************/
void QueueInit(QueueC* Queue)
{
	Queue->QueueIn = Queue->QueueOut = 0;
	Queue->QueueSize = 0;
}

/******************************************************************************
******************************************************************************/
int QueuePut(char new, QueueC* Queue)
{
	if(Queue->QueueIn == (( Queue->QueueOut - 1 + QUEUE_SIZE) % QUEUE_SIZE))
	{
		return -1; /* Queue Full*/
	}

	Queue->Queue[Queue->QueueIn] = new;
	

	Queue->QueueIn = (Queue->QueueIn + 1) % QUEUE_SIZE;
	
	Queue->QueueSize++;

	return 0; // No errors
}

/******************************************************************************
******************************************************************************/
int QueueGet(char *old, QueueC* Queue)
{
	if(Queue->QueueIn == Queue->QueueOut)
	{
		return -1; /* Queue Empty - nothing to get*/
	}

	*old = Queue->Queue[Queue->QueueOut];

	Queue->QueueOut = (Queue->QueueOut + 1) % QUEUE_SIZE;
	
	Queue->QueueSize--;

	return 0; // No errors
}

/******************************************************************************
******************************************************************************/
int GetSize(QueueC* Queue)
{
	return Queue->QueueSize;
}