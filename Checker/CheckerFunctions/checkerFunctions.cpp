#include "checkerFunctions.hpp"

#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fstream>
#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <cstdlib>

using namespace std;

// receive checkerNum and "add" it to the VOTERS_TBC_SHMEM_GENKEY
// not declared in hpp, we only use it here
int uniqueCheckerKey(int checkerNum) {
	ostringstream ssVOTERS_TBC_SHMEM_GENKEY;
	ssVOTERS_TBC_SHMEM_GENKEY << VOTERS_TBC_SHMEM_GENKEY;
	ostringstream ssCheckerNum;
	ssCheckerNum << checkerNum;
	string uniqueCheckerKey;
	uniqueCheckerKey = ssVOTERS_TBC_SHMEM_GENKEY.str() + ssCheckerNum.str(); // merge the above ss to one string
	return atoi(uniqueCheckerKey.c_str());
}

string uniqueFreeCheckerSemName(int checkerNum) {
	ostringstream ssCheckerNum;
	ssCheckerNum << checkerNum;
	string name = FREE_CHECKER_SEM_NAME_GEN + ssCheckerNum.str();
	return name;
}

string uniqueAccessCheckerSemName(int checkerNum) {
	ostringstream ssCheckerNum;
	ssCheckerNum << checkerNum;
	string name = CHECKER_ACCESS_SEM_NAME_GEN + ssCheckerNum.str();
	return name;
}

int createVotersToBeChecked(int checkerNum) {
	int votersToBeCheckedSize = sizeof(long); // sizeof id
	int votersToBeCheckedShId;

	votersToBeCheckedShId = shmget(uniqueCheckerKey(checkerNum),
			votersToBeCheckedSize,
			IPC_CREAT | IPC_EXCL | 0666);
	if (votersToBeCheckedShId < 0) {
		cout << "Creation of voter to be checked shared memory failed!" << endl;
		return -1;
	}
	return votersToBeCheckedShId;
}

void* attachVotersToBeChecked(int votersToBeCheckedShId) {
	void* voterToBeCheckedMem;
	voterToBeCheckedMem = (void*) shmat(votersToBeCheckedShId, (void*) 0, 0);
	if (voterToBeCheckedMem == (void*) -1) {
		cout
				<< "Attaching shared memory segment for voter to be checked failed!"
				<< endl;
		return (void*) -1;
	}
	return voterToBeCheckedMem;
}

sem_t* createFreeCheckerSemaphore(int checkerNum) {
	sem_t *freeCheckerSem = sem_open(
			uniqueFreeCheckerSemName(checkerNum).c_str(),
			O_CREAT | O_EXCL,
			S_IRUSR | S_IWUSR, 1);

	if (freeCheckerSem == SEM_FAILED) {
		cout << "Creation of free checkers' semaphore failed!" << endl;
		return NULL;
	}

	return freeCheckerSem;
}

sem_t* createCheckersAccessSemaphore(int checkerNum) {
	sem_t *votersToBeCheckedSem = sem_open(
			uniqueAccessCheckerSemName(checkerNum).c_str(),
			O_CREAT | O_EXCL,
			S_IRUSR | S_IWUSR, 0);

	if (votersToBeCheckedSem == SEM_FAILED) {
		cout << "Creation of checkers' access semaphore failed!" << endl;
		return NULL;
	}

	return votersToBeCheckedSem;
}

long readVoterIDFromVotersToBeChecked(void* voterToBeCheckedMem) {
	long voterID;
	memcpy(&voterID, voterToBeCheckedMem, sizeof(voterID));
	return voterID;
}

void writeVotersToBeChecked(bool success, void* voterToBeCheckedMem) {
	long status;
	if (success == true) {
		status = 1;
	} else {
		status = 0;
	}

	memcpy(voterToBeCheckedMem, &status, sizeof(long));
}
