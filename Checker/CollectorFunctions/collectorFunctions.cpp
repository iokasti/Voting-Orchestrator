#include "collectorFunctions.hpp"
#include <iostream>
#include <sys/shm.h>

using namespace std;

sem_t* getVotersQueueSemaphore() {
	sem_t *votersQueue = sem_open(VOTERS_QUEUE_SEM_NAME, 0);

	if (votersQueue == SEM_FAILED) {
		cout << "Opening voters queue semaphore failed!" << endl;
		return NULL;
	}

	return votersQueue;
}

