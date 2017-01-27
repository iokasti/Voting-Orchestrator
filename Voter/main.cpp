#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/shm.h>
#include "CheckerFunctions/checkerFunctions.hpp"
#include "CollectorFunctions/collectorFunctions.hpp"

using namespace std;

int main(int argc, char** argv) {
	/****************	READ COMMAND LINE ARGUMENTS	*********************************************/
	if (argc != 7) {
		cout << "Wrong number of given arguments!" << endl;
		return 1;
	}
	int voterID = -1;	// regnum
	int choice = -1;
	int checkersNum = -1;
	for (int i = 1; i < 7; i = i + 2) {	// value 0 is program name so we don't need it
		if (strcmp(argv[i], "-s") == 0) {
			voterID = atoi(argv[i + 1]);
		} else if (strcmp(argv[i], "-c") == 0) {
			choice = atoi(argv[i + 1]);
		} else if (strcmp(argv[i], "-n") == 0) {
			checkersNum = atoi(argv[i + 1]);
		}
	}
	if (voterID < 0 || choice < 0 || checkersNum < 0) {
		cout << "Something went wrong while reading given arguments, try again!"
				<< endl;
		return 1;
	}
	/*******************************************************************************************/

	/*****************	OPEN SEMAPHORE TO IDENTIFY STATE OF CHECKERS	**********************/
	sem_t *checkersStateSem = openCheckersStateSemaphore();
	if (checkersStateSem == NULL) {
		return 1;
	}
	/****************************************************************************************/

	/**************	PASS CHECKER CHECK	********************/
	cout << "Waiting to be checked from a checker." << endl;
	sem_wait(checkersStateSem);

	int freeCheckerNum;
	sem_t* freeCheckerSem = getFreeChecker(checkersNum, &freeCheckerNum);
	if (freeCheckerSem == NULL) {
		// cleanup before erroneous exit
		// post to semaphores
		sem_post(checkersStateSem);
		// close semaphores
		sem_close(checkersStateSem);

		return 1;
	}

	/**********	OPEN SHARED MEMORY SEGMENT	FOR CHECKERS MEMORY	************/
	// get shared memory segment for access to voters' registry
	int checkersMemShId = getCheckersMemory(freeCheckerNum);
	if (checkersMemShId == -1) {
		// cleanup before erroneous exit
		// post to semaphores
		sem_post(freeCheckerSem);
		sem_post(checkersStateSem);
		// close semaphores
		sem_close(checkersStateSem);
		sem_close(freeCheckerSem);

		return 1;
	}

	// attach shared memory segment
	void* checkersMem = attachCheckersMemory(checkersMemShId);
	if (checkersMem == (void*) -1) {
		// cleanup before erroneous exit
		// post to semaphores
		sem_post(freeCheckerSem);
		sem_post(checkersStateSem);
		// close semaphores
		sem_close(checkersStateSem);
		sem_close(freeCheckerSem);

		return 1;
	}

	/***********************************************************************/
	writeVoterIDtoCheckerSharedMemory(voterID, checkersMem);

	wakeFreeChecker(freeCheckerNum);

	bool checkerAnswer = getCheckerAnswer(checkersMem);

	sem_post(freeCheckerSem);
	sem_post(checkersStateSem);

	if (checkerAnswer == false) {
		// close semaphores
		sem_close(checkersStateSem);
		sem_close(freeCheckerSem);
		// detach shared memory segments
		shmdt((void*) checkersMem);
		cout << "Rejected from checker!" << endl;
		return 0;
	}
	/*******************************************************/
	// close semaphores
	sem_close(checkersStateSem);
	sem_close(freeCheckerSem);
	// detach shared memory segments
	shmdt((void*) checkersMem);

	cout << "Accepted from checker, proceeding to vote collector now." << endl;

	/*****************	OPEN SEMAPHORE TO COLLECTORS VOTERS QUEUE	**********************/
	sem_t *votersQueueSem = getVotersQueueSemaphore();
	if (votersQueueSem == NULL) {
		return 1;
	}
	/****************************************************************************************/

	/*****************	OPEN SEMAPHORE COLLECTORS FREE	**********************/
	sem_t *collectorsFreeSem = getCollectorsFreeSemaphore();
	if (collectorsFreeSem == NULL) {
		// close semaphores
		sem_close(votersQueueSem);
		return 1;
	}
	/**************************************************************************/

	/************** VOTE ********************/
	cout << "Waiting to vote." << endl;
	sem_wait(votersQueueSem);
	sem_wait(collectorsFreeSem);
	/**********	OPEN SHARED MEMORY SEGMENT	FOR COLLECTORS MEMORY	************/
	// get shared memory segment for access to voters' registry
	int collectorsMemShId = getCollectorsMem();
	if (collectorsMemShId == -1) {
		// cleanup before erroneous exit
		sem_post(votersQueueSem);
		// close semaphores
		sem_close(votersQueueSem);

		return 1;
	}

	// attach shared memory segment
	void* collectorsMem = attachCollectorsMem(collectorsMemShId);
	if (collectorsMem == (void*) -1) {
		// cleanup before erroneous exit
		sem_post(votersQueueSem);
		// close semaphores
		sem_close(votersQueueSem);

		return 1;
	}

	/***********************************************************************/
	writeVotersSelection(choice, collectorsMem);

	wakeCollector();

	getCollectorsAnswer(collectorsMem);

	sem_post(collectorsFreeSem);
	sem_post(votersQueueSem);

	cout << "Successfully voted." << endl;
	/*******************************************************/

	// close semaphores
	sem_close(votersQueueSem);
	sem_close(collectorsFreeSem);

	return 0;
}
