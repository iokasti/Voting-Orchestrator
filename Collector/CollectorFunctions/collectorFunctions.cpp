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

int createCollectorsMem() {
	int collectorsMemShId = shmget(COLLECTORS_SHMEM, sizeof(int),
	IPC_CREAT | IPC_EXCL | 0666);
	if (collectorsMemShId < 0) {
		cout << "Creation of collectors mem shared memory failed!" << endl;
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

int readVotersSelectionFromCollectorsMem(void* collectorsMem) {
	int votersSelection;
	memcpy(&votersSelection, collectorsMem, sizeof(int));
	return votersSelection;
}

void writeSuccess(void* collectorsMem) {
	int status = 1;
	memcpy(collectorsMem, &status, sizeof(int));
}

sem_t* createVotersQueueSemaphore() {
	sem_t *votersQueue = sem_open(
	VOTERS_QUEUE_SEM_NAME,
	O_CREAT | O_EXCL,
	S_IRUSR | S_IWUSR, VOTERS_QUEUE_SEM_SIZE);

	if (votersQueue == SEM_FAILED) {
		cout << "Creation of voters queue semaphore failed!" << endl;
		return NULL;
	}

	return votersQueue;
}

sem_t* createCollectorsFreeSemaphore() {
	sem_t *collectorsFree = sem_open(
	COLLECTORS_FREE_SEM_NAME,
	O_CREAT | O_EXCL,
	S_IRUSR | S_IWUSR, 1);

	if (collectorsFree == SEM_FAILED) {
		cout << "Creation of collectors' free semaphore failed!" << endl;
		return NULL;
	}

	return collectorsFree;
}

sem_t* createCollectorsAccessSemaphore() {
	sem_t *collectorsAccess = sem_open(
	COLLECTORS_ACCESS_SEM_NAME,
	O_CREAT | O_EXCL,
	S_IRUSR | S_IWUSR, 0);

	if (collectorsAccess == SEM_FAILED) {
		cout << "Creation of collectors' access semaphore failed!" << endl;
		return NULL;
	}

	return collectorsAccess;
}

