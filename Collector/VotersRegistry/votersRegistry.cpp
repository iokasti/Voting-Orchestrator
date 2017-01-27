#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fstream>
#include <cstring>
#include "votersRegistry.hpp"

using namespace std;

int getVotersRegistry(int registeredVoters, int votersRegistryShKey) {
	int votersRegistrySize = registeredVoters * (sizeof(long) + sizeof(int)); // number of voters *times* (sizeof id + size of votes)
	int votersRegistryShId;
	votersRegistryShId = shmget(votersRegistryShKey,
			registeredVoters * (sizeof(int) + sizeof(long)), 0666);
	if (votersRegistryShId < 0) {
		cout << "Getting voters' registry shared memory failed!" << endl;
		return -1;
	}
	return votersRegistryShId;
}

void* attachVotersRegistry(int votersRegistryShId) {
	void* votersRegistryMem;
	votersRegistryMem = (void*) shmat(votersRegistryShId, (void*) 0, 0);
	if (votersRegistryMem == (void*) -1) {
		cout << "Attaching shared memory segment for voters' registry failed!"
				<< endl;
		return (void*) -1;
	}
	return votersRegistryMem;
}

sem_t* openVotersRegistrySemaphore() {
	sem_t *votersRegistrySem = sem_open(VOTERS_REG_SEM_NAME, 0);

	if (votersRegistrySem == SEM_FAILED) {
		cout << "Open of checkers' state semaphore failed!" << endl;
		return NULL;
	}

	return votersRegistrySem;
}
