#ifndef COLLECTORFUNCTIONS_COLLECTORFUNCTIONS_HPP_
#define COLLECTORFUNCTIONS_COLLECTORFUNCTIONS_HPP_

#define VOTERS_QUEUE_SEM_NAME "votersCollectorQueue"


#include <semaphore.h>

sem_t* getVotersQueueSemaphore(); // get semaphore used as a buffer for access to the Collector

void wakeCollector();

#endif /* COLLECTORFUNCTIONS_COLLECTORFUNCTIONS_HPP_ */
