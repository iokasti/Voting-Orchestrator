#ifndef COLLECTORFUNCTIONS_COLLECTORFUNCTIONS_HPP_
#define COLLECTORFUNCTIONS_COLLECTORFUNCTIONS_HPP_

#define COLLECTORS_SHMEM 9999
#define VOTERS_QUEUE_SEM_SIZE 30
#define VOTERS_QUEUE_SEM_NAME "votersCollectorQueue"
#define COLLECTORS_ACCESS_SEM_NAME "accessCollectorSem"
#define COLLECTORS_FREE_SEM_NAME "freeCollectorSem"

#include <semaphore.h>

int createCollectorsMem(); // shmget shared memory for collectors mem
void* attachCollectorsMem(int);	// shmat shared memory for collectors mem

int readVotersSelectionFromCollectorsMem(void*); // reads voters selection from CollectorsMem shared memory and returns it
void writeSuccess(void*); // writes 1 in collectorsMem

sem_t* createVotersQueueSemaphore(); // creates semaphore used as a buffer for access to the Collector
sem_t* createCollectorsFreeSemaphore(); // creates semaphore used to check if collector is free
sem_t* createCollectorsAccessSemaphore(); // creates semaphore used for access in collector

#endif /* COLLECTORFUNCTIONS_COLLECTORFUNCTIONS_HPP_ */
