#ifndef COLLECTORFUNCTIONS_COLLECTORFUNCTIONS_HPP_
#define COLLECTORFUNCTIONS_COLLECTORFUNCTIONS_HPP_

#define COLLECTORS_SHMEM 9999
#define VOTERS_QUEUE_SEM_NAME "votersCollectorQueue"
#define COLLECTORS_ACCESS_SEM_NAME "accessCollectorSem"
#define COLLECTORS_FREE_SEM_NAME "freeCollectorSem"

#include <semaphore.h>

int getCollectorsMem(); // shmget shared memory for collectors mem
void* attachCollectorsMem(int);	// shmat shared memory for collectors mem

void getCollectorsAnswer(void*); // reads collectors answer from collectors mem
void writeVotersSelection(int, void*); // writes voters selection to collectors mem

sem_t* getVotersQueueSemaphore(); // get semaphore used as a buffer for access to the Collector
sem_t* getCollectorsFreeSemaphore(); // get semaphore used to check if collector is free

void wakeCollector();

#endif /* COLLECTORFUNCTIONS_COLLECTORFUNCTIONS_HPP_ */
