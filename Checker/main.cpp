#include <iostream>
#include <sys/shm.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <signal.h>
#include "CheckerFunctions/checkerFunctions.hpp"
#include "VotersRegistry/votersRegistry.hpp"
#include "CollectorFunctions/collectorFunctions.hpp"

using namespace std;

bool end;
sem_t *checkerAccessSem;

void signalHandler(int signum)
{    
	end = true;
	sem_post(checkerAccessSem);
}

int main(int argc, char** argv) {
	signal(SIGINT, signalHandler);  
	end = false;
	/****************	READ COMMAND LINE ARGUMENTS	*********************************************/
	if (argc != 9) {
		cout << "Wrong number of given arguments!" << endl;
		return 1;
	}
	int period = -1;
	int votersRegistryShKey = -1;
	int registeredVoters = -1;
	int checkerNum = -1;
	for (int i = 1; i < 9; i = i + 2) {	// value 0 is program name so we don't need it
		if (strcmp(argv[i], "-d") == 0) {
			period = atoi(argv[i + 1]);
		} else if (strcmp(argv[i], "-s") == 0) {
			votersRegistryShKey = atoi(argv[i + 1]);
		} else if (strcmp(argv[i], "-r") == 0) {
			registeredVoters = atoi(argv[i + 1]);
		} else if (strcmp(argv[i], "-n") == 0) {
			checkerNum = atoi(argv[i + 1]);
		}
	}
	if (period < 0 || votersRegistryShKey < 0 || registeredVoters < 0
			|| checkerNum < 0) {
		cout << "Checker: Something went wrong while reading given arguments, try again!"
				<< endl;
		return 1;
	}
	/*****************************************************************/

	srand(time(NULL));

	/******** SHARED MEMORY SEGMENT FOR VOTERS	TO BE CHECKED	*********/
	// create shared memory segment
	int votersToBeCheckedShId = createVotersToBeChecked(checkerNum);
	if (votersToBeCheckedShId == -1) {
		return 1;
	}

	// attach shared memory segment
	void* voterToBeCheckedMem = attachVotersToBeChecked(votersToBeCheckedShId);
	if (voterToBeCheckedMem == (void*) -1) {
		// cleanup before erroneous exit
		// destroy shared memory segments
		shmctl(votersToBeCheckedShId, IPC_RMID, 0);

		return 1;
	}

	/********************************************************************/

	/**********	CREATING SEMAPHORE FOR WAKING UP CHECKER	***********/
	checkerAccessSem = createCheckersAccessSemaphore(checkerNum);
	if (checkerAccessSem == NULL) {
		// cleanup before erroneous exit
		// detach shared memory segments
		shmdt((void*) voterToBeCheckedMem);
		// destroy shared memory segments
		shmctl(votersToBeCheckedShId, IPC_RMID, 0);

		return 1;
	}
	/***************************************************************/

	/**********	CREATING SEMAPHORE TO CHECK IF CHECKER IS FREE	***********/
	sem_t *freeCheckerSem = createFreeCheckerSemaphore(checkerNum);
	if (freeCheckerSem == NULL) {
		// cleanup before erroneous exit
		// destroy semaphores
		sem_close(checkerAccessSem);
		sem_unlink(uniqueAccessCheckerSemName(checkerNum).c_str());
		// detach shared memory segments
		shmdt((void*) voterToBeCheckedMem);
		// destroy shared memory segments
		shmctl(votersToBeCheckedShId, IPC_RMID, 0);

		return 1;
	}
	// initialize semaphore to 1, voter waits and changes it to 0 and then posts it and changes to 1.
	sem_close(freeCheckerSem);
	/***************************************************************/

	/**********	OPEN SHARED MEMORY SEGMENT	FOR VOTERS	REGISTRY	************/
	// get shared memory segment for access to voters' registry
	int votersRegistryShId = getVotersRegistry(registeredVoters,
			votersRegistryShKey);
	if (votersRegistryShId == -1) {
		// cleanup before erroneous exit
		// destroy semaphores
		sem_close(checkerAccessSem);
		sem_unlink(uniqueAccessCheckerSemName(checkerNum).c_str());
		sem_close(freeCheckerSem);
		sem_unlink(uniqueFreeCheckerSemName(checkerNum).c_str());
		// detach shared memory segments
		shmdt((void*) voterToBeCheckedMem);
		// destroy shared memory segments
		shmctl(votersToBeCheckedShId, IPC_RMID, 0);

		return 1;
	}

	// attach shared memory segment
	void* votersRegistryMem = attachVotersRegistry(votersRegistryShId);
	if (votersRegistryMem == (void*) -1) {
		// cleanup before erroneous exit
		// destroy semaphores
		sem_close(checkerAccessSem);
		sem_unlink(uniqueAccessCheckerSemName(checkerNum).c_str());
		sem_close(freeCheckerSem);
		sem_unlink(uniqueFreeCheckerSemName(checkerNum).c_str());
		// detach shared memory segments
		shmdt((void*) votersRegistryMem);
		shmdt((void*) voterToBeCheckedMem);
		// destroy shared memory segments
		shmctl(votersToBeCheckedShId, IPC_RMID, 0);

		return 1;
	}
	/***********************************************************************/

	/**********	OPEN SEMAPHORE	FOR VOTERS	REGISTRY	************/
	sem_t *votersRegistrySem = openVotersRegistrySemaphore();
	if (votersRegistrySem == NULL) {
		// cleanup before erroneous exit
		// destroy semaphores
		sem_close(checkerAccessSem);
		sem_unlink(uniqueAccessCheckerSemName(checkerNum).c_str());
		sem_close(freeCheckerSem);
		sem_unlink(uniqueFreeCheckerSemName(checkerNum).c_str());
		// detach shared memory segments
		shmdt((void*) votersRegistryMem);
		shmdt((void*) voterToBeCheckedMem);
		// destroy shared memory segments
		shmctl(votersToBeCheckedShId, IPC_RMID, 0);

		return 1;
	}
	/***********************************************************************/

	/**********	OPEN SEMAPHORE	FOR VOTERS	QUEUE	************/
	sem_t *votersQueueSem = getVotersQueueSemaphore();
	if (votersQueueSem == NULL) {
		// cleanup before erroneous exit
		// destroy semaphores
		sem_close(votersRegistrySem);
		sem_close(checkerAccessSem);
		sem_unlink(uniqueAccessCheckerSemName(checkerNum).c_str());
		sem_close(freeCheckerSem);
		sem_unlink(uniqueFreeCheckerSemName(checkerNum).c_str());
		// detach shared memory segments
		shmdt((void*) votersRegistryMem);
		shmdt((void*) voterToBeCheckedMem);
		// destroy shared memory segments
		shmctl(votersToBeCheckedShId, IPC_RMID, 0);

		return 1;
	}
	/***********************************************************************/

	while (end == false) {
		sem_wait(checkerAccessSem);

		if(end == true){
			break;
		}

		long voterID = readVoterIDFromVotersToBeChecked(voterToBeCheckedMem);

		sem_wait(votersRegistrySem);
		if (addVote(registeredVoters, voterID, votersRegistryMem) == true) {
			// sleep for a random period from 1 to period
			int value = -1;
			sem_getvalue(votersQueueSem, &value);
			do {
				// repeat this step if collectors queue is full
				sem_getvalue(votersQueueSem, &value);
				sleep((rand() % period + 1));
			} while (value <= 1);
			// voter with voterID is a registered voter
			if (getVotes(registeredVoters, voterID, votersRegistryMem) == 1) {
				// voter has not voted before
				writeVotersToBeChecked(true, voterToBeCheckedMem);
			} else {
				// voter has voted before
				writeVotersToBeChecked(false, voterToBeCheckedMem);
			}

		} else {
			// sleep for a random period from 1 to period
			int value = -1;
			sem_getvalue(votersQueueSem, &value);
			do {
				// repeat this step if collectors queue is full
				sem_getvalue(votersQueueSem, &value);
				sleep((rand() % period + 1));
			} while (value <= 1);
			// voter with voterID is not a registered voter
			writeVotersToBeChecked(false, voterToBeCheckedMem);
		}
		sem_post(votersRegistrySem);
	}
// detach shared memory segments
	shmdt((void*) votersRegistryMem);
	shmdt((void*) voterToBeCheckedMem);

// destroy shared memory segments
	shmctl(votersToBeCheckedShId, IPC_RMID, 0);

// close semaphores
	sem_close(checkerAccessSem);
	sem_close(votersRegistrySem);
	sem_close(votersQueueSem);

// destroy semaphores
	sem_unlink(uniqueAccessCheckerSemName(checkerNum).c_str());
	sem_unlink(uniqueFreeCheckerSemName(checkerNum).c_str());

}
