#include <iostream>
#include <sys/shm.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <signal.h>
#include <ctime>
#include <unistd.h>
#include "VotersRegistry/votersRegistry.hpp"
#include "CheckerFunctions/checkerFunctions.hpp"
#include "Voter/voter.hpp"

/* USE THIS COMMAND TO FREE SEMAPHORES AND SHARED MEMORY SEGMENTS IN CASE SOMETHING GOES WRONG
 * ipcrm -M 7777; ipcrm -M 88881; ipcrm -M 88882; ipcrm -M 88883; rm -f /dev/shm/sem*; ipcrm -M 9999
 */

#define CHECKERS_NUM 3
#define CHECKERS_PATH "./checker"
#define COLLECTORS_PATH "./collector"
#define VOTERS_PATH "./voter"

using namespace std;

pid_t *checkersPIDs;

void signalHandler(int signum)
{
    cout << "Termination signal received..." << endl;
    cout << "Terminating checkers..." << endl;
    for(int i=0; i < CHECKERS_NUM; i++){
    	kill(checkersPIDs[i], SIGINT);
    }
}

int main(int argc, char** argv) {
	signal(SIGINT, signalHandler);  

	/****************	READ COMMAND LINE ARGUMENTS	*********************************************/
	if (argc != 13) {
		cout << "Wrong number of given arguments!" << endl;
		return 1;
	}
	string registry = "";
	int numvoters = -1;
	int period1 = -1;
	int period2 = -1;
	string choices = "";
	string statsfile = "";
	for (int i = 1; i < 13; i = i + 2) { // value 0 is program name so we don't need it
		if (strcmp(argv[i], "-i") == 0) { // used r instead of s to prevent messing things up
			registry = argv[i + 1];
		} else if (strcmp(argv[i], "-n") == 0) {
			numvoters = atoi(argv[i + 1]);
		} else if (strcmp(argv[i], "-d1") == 0) {
			period1 = atoi(argv[i + 1]);
		} else if (strcmp(argv[i], "-d2") == 0) {
			period2 = atoi(argv[i + 1]);
		} else if (strcmp(argv[i], "-c") == 0) {
			choices = argv[i + 1];
		} else if (strcmp(argv[i], "-o") == 0) {
			statsfile = argv[i + 1];
		}
	}
	if (registry == "" || numvoters < 0 || period1 < 0 || period2 < 0
			|| choices == "" || statsfile == "") {
		cout << "Something went wrong while reading given arguments, try again!"
				<< endl;
		return 1;

	}
	// find number of registered voters
	int registeredVoters = countRegisteredVoters(registry);
	cout << registeredVoters<<endl;
	/*******************************************************************************************/

	/**********	SHARED MEMORY SEGMENT	FOR VOTERS	REGISTRY	************/
	// create shared memory segment
	int votersRegistryShId = createVotersRegistry(registeredVoters);
	if (votersRegistryShId == -1) {
		return 1;
	}

	// attach shared memory segment
	void* votersRegistryMem = attachVotersRegistry(votersRegistryShId);
	if (votersRegistryMem == (void*) -1) {
		return 1;
	}

	// write voters registry file data in to shared memory
	fillVotersRegistry(registry, votersRegistryMem);

	// detach shared memory
	shmdt((void*) votersRegistryMem);
	/***********************************************************************/

	/********************	CREATING SEMAPHORE FOR VOTERS REGISTRY	**************************/
	sem_t *votersRegistrySem = createVotersRegistrySemaphore();
	if (votersRegistrySem == NULL) {
		// cleanup before erroneous exit
		// destroy shared memory segments
		shmctl(votersRegistryShId, IPC_RMID, 0);

		return 1;
	}
	sem_close(votersRegistrySem);
	/*******************************************************************************************/

	/**************	CREATING SEMAPHORE TO IDENTIFY STATE OF CHECKERS	********************/
	sem_t *checkersStateSem = createCheckersStateSemaphore(CHECKERS_NUM);
	if (checkersStateSem == NULL) {
		// cleanup before erroneous exit
		// destroy semaphores
		sem_unlink(VOTERS_REG_SEM_NAME);
		// destroy shared memory segments
		shmctl(votersRegistryShId, IPC_RMID, 0);

		return 1;
	}
	sem_close(checkersStateSem);
	/****************************************************************************************/

	/********************	FORK/EXEC 1 COLLLECTOR	**************************/
	int pid = fork();
	if (pid == -1) {
		cout << "Something went wrong while forking!" << endl;
		// cleanup before erroneous exit
		// destroy semaphores
		sem_unlink(VOTERS_REG_SEM_NAME);
		sem_unlink(CHECKERS_STATE_SEM_NAME);
		// destroy shared memory segments
		shmctl(votersRegistryShId, IPC_RMID, 0);
		return 1;
	} else if (pid == 0) {
		// child
		// pass all int arguments to string streams so we can later user them as a const char*
		ostringstream ssPeriod2;
		ssPeriod2 << period2;
		ostringstream ssVOTERS_REG_SHMEM_KEY;
		ssVOTERS_REG_SHMEM_KEY << VOTERS_REG_SHMEM_KEY;
		ostringstream ssRegisteredVoters;
		ssRegisteredVoters << registeredVoters;
		ostringstream ssNumVoters;
		ssNumVoters << numvoters;
		ostringstream ssPPID;
		ssPPID << getppid();
		// send arguments to Collector in form of const char*
		execlp(COLLECTORS_PATH, COLLECTORS_PATH, "-i", registry.c_str(), "-d",
				ssPeriod2.str().c_str(), "-o", statsfile.c_str(), "-s",
				ssVOTERS_REG_SHMEM_KEY.str().c_str(), "-r",
				ssRegisteredVoters.str().c_str(), "-n",
				ssNumVoters.str().c_str(), "-c", choices.c_str(), NULL);
		// should never reach code below here in case everything went ok
		cout << "Something went wrong while execing a collector!" << endl;
		// cleanup before erroneous exit
		// destroy semaphores
		sem_unlink(VOTERS_REG_SEM_NAME);
		sem_unlink(CHECKERS_STATE_SEM_NAME);
		// destroy shared memory segments
		shmctl(votersRegistryShId, IPC_RMID, 0);
		return 1;
	}
	/************************************************************************/

	sleep(5);

	/********************	FORK/EXEC CHECKERS_NUM CHECKERS	**************************/
	checkersPIDs = new pid_t[CHECKERS_NUM];
	for (int i = 0; i < CHECKERS_NUM; i++) {
		int pid = fork();
		if (pid == -1) {
			cout << "Something went wrong while forking!" << endl;

			// cleanup before erroneous exit
			// destroy semaphores
			sem_unlink(VOTERS_REG_SEM_NAME);
			sem_unlink(CHECKERS_STATE_SEM_NAME);
			// destroy shared memory segments
			shmctl(votersRegistryShId, IPC_RMID, 0);

			return 1;
		} else if (pid == 0) {
			// child
			// pass all int arguments to string streams so we can later user them as a const char*
			ostringstream ssPeriod1;
			ssPeriod1 << period1;
			ostringstream ssVOTERS_REG_SHMEM_KEY;
			ssVOTERS_REG_SHMEM_KEY << VOTERS_REG_SHMEM_KEY;
			ostringstream ssRegisteredVoters;
			ssRegisteredVoters << registeredVoters;
			ostringstream ssCheckerNum;	// used to create names for semaphores and shared memory segments for each checker
			ssCheckerNum << (i + 1); // i+1 so checkerNum starts from 1 and not 0
			// send arguments to Checkers in form of const char*
			execlp(CHECKERS_PATH, CHECKERS_PATH, "-d", ssPeriod1.str().c_str(),
					"-s", ssVOTERS_REG_SHMEM_KEY.str().c_str(), "-r",
					ssRegisteredVoters.str().c_str(), "-n",
					ssCheckerNum.str().c_str(), NULL);
			// should never reach code below here in case everything went ok
			cout << "Something went wrong while execing a checker!" << endl;

			// cleanup before erroneous exit
			// destroy semaphores
			sem_unlink(VOTERS_REG_SEM_NAME);
			sem_unlink(CHECKERS_STATE_SEM_NAME);
			// destroy shared memory segments
			shmctl(votersRegistryShId, IPC_RMID, 0);

			return 1;
		} else{
			// father
			checkersPIDs[i] = pid;
		}
	}
	/*********************************************************************************/

	sleep(5);

	/********************	FORK/EXEC NUMVOTERS VALID VOTERS AND RANDOM NUMBER OF INVALID	**************************/
	srand (time(NULL));
	int numChoices = countChoices(choices);
	for (int i = 0; i < numvoters; i++) {
		int pid = fork();
		if (pid == -1) {
			cout << "Something went wrong while forking!" << endl;

			// cleanup before erroneous exit
			// destroy semaphores
			sem_unlink(VOTERS_REG_SEM_NAME);
			sem_unlink(CHECKERS_STATE_SEM_NAME);
			// destroy shared memory segments
			shmctl(votersRegistryShId, IPC_RMID, 0);

			return 1;
		} else if (pid == 0) {
			// child
			// pass all int arguments to string streams so we can later user them as a const char*
			ostringstream ssRandomID;
			ssRandomID << randomID(registry, registeredVoters);
			ostringstream ssRandomChoice;
			ssRandomChoice << randomChoice(choices, numChoices);
			ostringstream ssCheckersNum;
			ssCheckersNum << CHECKERS_NUM;
			// send arguments to Checkers in form of const char*
			execlp(VOTERS_PATH, VOTERS_PATH, "-s", ssRandomID.str().c_str(),
					"-c", ssRandomChoice.str().c_str(), "-n",
					ssCheckersNum.str().c_str(),
					NULL);

			// should never reach code below here in case everything went ok
			cout << "Something went wrong while execing a voter!" << endl;

			// cleanup before erroneous exit
			// destroy semaphores
			sem_unlink(VOTERS_REG_SEM_NAME);
			sem_unlink(CHECKERS_STATE_SEM_NAME);
			// destroy shared memory segments
			shmctl(votersRegistryShId, IPC_RMID, 0);

			return 1;
		}
	}
	/*********************************************************************************/

	pause();

	// destroy semaphores
	sem_unlink(VOTERS_REG_SEM_NAME);
	sem_unlink(CHECKERS_STATE_SEM_NAME);
	// destroy shared memory segments
	shmctl(votersRegistryShId, IPC_RMID, 0);

	delete[] checkersPIDs;

	cout << "Exiting..." <<endl;	

	return 0;
}
