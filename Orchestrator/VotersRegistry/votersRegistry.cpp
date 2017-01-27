#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fstream>
#include <cstring>
#include <fcntl.h>
#include "votersRegistry.hpp"

using namespace std;

int countRegisteredVoters(string registry) {
	int numLines = 0;
	ifstream input(registry.c_str());
	string unused;
	while (std::getline(input, unused))
		++numLines;
	input.close();
	return numLines;
}

int createVotersRegistry(int registeredVoters) {
	int votersRegistrySize = registeredVoters * (sizeof(long) + sizeof(int)); // number of voters *times* (sizeof id + size of votes)
	int votersRegistryShId;
	votersRegistryShId = shmget(VOTERS_REG_SHMEM_KEY, votersRegistrySize,
	IPC_CREAT | IPC_EXCL | 0666);
	if (votersRegistryShId < 0) {
		cout << "Creation of voters' registry shared memory failed!" << endl;
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

void fillVotersRegistry(string registry, void* votersRegistryMem) {
	long voterID;
	int timesVoted;
	ifstream input(registry.c_str());
	int i = 0;
	while (input >> voterID >> timesVoted) {
		memcpy(votersRegistryMem + (i * (sizeof(long) + sizeof(int))), &voterID,
				sizeof(voterID));
		memcpy(
				votersRegistryMem + (i * (sizeof(long) + sizeof(int)))
						+ sizeof(long), &timesVoted, sizeof(timesVoted));
		i++;
	}

	// used to count not registered voters
	voterID = -1;
	timesVoted = 0;
	memcpy(votersRegistryMem + (i * (sizeof(long) + sizeof(int))), &voterID,
			sizeof(voterID));
	memcpy(
			votersRegistryMem + (i * (sizeof(long) + sizeof(int)))
					+ sizeof(long), &timesVoted, sizeof(timesVoted));

	input.close();
}

sem_t* createVotersRegistrySemaphore() {
	sem_t *votersRegistrySem = sem_open(VOTERS_REG_SEM_NAME, O_CREAT | O_EXCL,
	S_IRUSR | S_IWUSR, 1);

	if (votersRegistrySem == SEM_FAILED) {
		cout << "Creation of voters' registry semaphore failed!" << endl;
		return NULL;
	}

	return votersRegistrySem;
}

