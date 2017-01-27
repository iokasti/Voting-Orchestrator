#include "checkerFunctions.hpp"
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <cstring>

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

sem_t* openCheckersStateSemaphore() {
	sem_t *checkersStateSem = sem_open(CHECKERS_STATE_SEM_NAME, 0);

	if (checkersStateSem == SEM_FAILED) {
		cout << "Open of checkers' state semaphore failed!" << endl;
		return NULL;
	}

	return checkersStateSem;
}

sem_t* openFreeCheckerSemaphore(string freeCheckerSemName) {
	sem_t *freeCheckerSem = sem_open(freeCheckerSemName.c_str(), 0);
	if (freeCheckerSem == SEM_FAILED) {
		cout << "Open of free checkers' semaphore failed!" << endl;
		return NULL;
	}

	return freeCheckerSem;
}

sem_t* getFreeChecker(int checkersNum, int *freeCheckerNum) {
	for (int i = 1; i <= checkersNum; i++) {
		sem_t* freeCheckerSem; // used to find a checker available for use by voter
		freeCheckerSem = openFreeCheckerSemaphore(uniqueFreeCheckerSemName(i));
		if (freeCheckerSem == NULL) {	// in case something went wrong
			return NULL;
		}
		if (sem_trywait(freeCheckerSem) == 0) {	//	checker found free is successfully locked
			*freeCheckerNum = i;
			return freeCheckerSem;	// give the number of the one we found free
		}
		sem_close(freeCheckerSem);
	}
	return NULL;
}

int getCheckersMemory(int freeCheckerNum) {
	int checkersMemoryShId;
	checkersMemoryShId = shmget(uniqueCheckerKey(freeCheckerNum), sizeof(long),
			0666);
	if (checkersMemoryShId < 0) {
		cout << "Getting checkers' memory failed!" << endl;
		return -1;
	}
	return checkersMemoryShId;
}

void* attachCheckersMemory(int checkersMemoryShId) {
	void* checkersMem;
	checkersMem = (void*) shmat(checkersMemoryShId, (void*) 0, 0);
	if (checkersMem == (void*) -1) {
		cout << "Attaching shared memory segment for checkers failed!" << endl;
		return (void*) -1;
	}
	return checkersMem;
}

void writeVoterIDtoCheckerSharedMemory(long voterID, void* checkersMem) {
	memcpy(checkersMem, &voterID, sizeof(long));
}

sem_t* openCheckerAccessSemaphore(string checkerAccessSemName) {
	sem_t *checkerAccessSem = sem_open(checkerAccessSemName.c_str(), 0);

	if (checkerAccessSem == SEM_FAILED) {
		cout << "Open of checker's access semaphore failed!" << endl;
		return NULL;
	}

	return checkerAccessSem;
}

void wakeFreeChecker(int freeCheckerNum) {
	sem_t* checkerAccessSem = openCheckerAccessSemaphore(
			uniqueAccessCheckerSemName(freeCheckerNum));
	if (checkerAccessSem == NULL) {
		return;
	}
	sem_post(checkerAccessSem);	//	wake up checker
	sem_close(checkerAccessSem);
}

bool getCheckerAnswer(void* checkersMem) {
	long answer = -1;
	// in case voter tries to read from memory and checker hasn't written in it yet
	do {
		memcpy(&answer, checkersMem, sizeof(long));
		if (answer == 0) {
			return false;
		} else if (answer == 1) {
			return true;
		}
	} while (answer != 0 && answer != 1);

	// this code will never be reached..hopefully
	return false;
}
