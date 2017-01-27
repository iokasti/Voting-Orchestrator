#include "collectorFunctions.hpp"

#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fstream>
#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <cstdlib>

using namespace std;

int getCollectorsMem() {
	int collectorsMemShId = shmget(COLLECTORS_SHMEM, sizeof(int), 0666);
	if (collectorsMemShId < 0) {
		cout << "Getting collectors mem shared memory failed!" << endl;
		return -1;
	}
	return collectorsMemShId;
}

void* attachCollectorsMem(int collectorsMemShId) {
	void* collectorsMem;
	collectorsMem = (void*) shmat(collectorsMemShId, (void*) 0, 0);
	if (collectorsMem == (void*) -1) {
		cout << "Attaching shared memory segment for collectors mem failed!"
				<< endl;
		return (void*) -1;
	}
	return collectorsMem;
}

void getCollectorsAnswer(void* collectorsMem) {
	int answer;
	do {
		memcpy(&answer, collectorsMem, sizeof(int));
	} while (answer != 1);
}

void writeVotersSelection(int votersSelection, void* collectorsMem) {
	memcpy(collectorsMem, &votersSelection, sizeof(int));
}

sem_t* getVotersQueueSemaphore() {
	sem_t *votersQueue = sem_open(VOTERS_QUEUE_SEM_NAME, 0);

	if (votersQueue == SEM_FAILED) {
		cout << "Opening voters queue semaphore failed!" << endl;
		return NULL;
	}

	return votersQueue;
}

sem_t* getCollectorsFreeSemaphore() {
	sem_t *collectorsFree = sem_open(COLLECTORS_FREE_SEM_NAME, 0);

	if (collectorsFree == SEM_FAILED) {
		cout << "Opening Collectors Free semaphore failed!" << endl;
		return NULL;
	}

	return collectorsFree;
}

sem_t* getcollectorsAccessSemaphore() {
	sem_t *collectorsAccess = sem_open(COLLECTORS_ACCESS_SEM_NAME, 0);

	if (collectorsAccess == SEM_FAILED) {
		cout << "Opening collectors' access semaphore failed!" << endl;
		return NULL;
	}

	return collectorsAccess;
}

void wakeCollector() {
	sem_t* collectorsAccess = getcollectorsAccessSemaphore();
	if (collectorsAccess == NULL) {
		return;
	}
	sem_post(collectorsAccess);	//	wake up collector
	sem_close(collectorsAccess);
}

