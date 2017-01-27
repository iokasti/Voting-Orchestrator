#include <iostream>
#include <sys/shm.h>
#include <cstring>
#include <cstdlib>
#include <signal.h>
#include <cstdio>
#include <unistd.h>
#include <ctime>
#include "CollectorFunctions/collectorFunctions.hpp"
#include "VotersRegistry/votersRegistry.hpp"
#include "ResultList/resultList.hpp"

using namespace std;

sem_t *collectorsAccessSem;
bool end;
 
void signalHandler(int signum)
{    
	end = true;
	sem_post(collectorsAccessSem);
}

int main(int argc, char** argv) {
	signal(SIGINT, signalHandler);	 
	end = false; 	
	/****************	READ COMMAND LINE ARGUMENTS	*********************************************/
	if (argc != 15) {
		cout << "Wrong number of given arguments!" << endl;
		return 1;
	}
	string registry = "";
	int period = -1;
	string statsfile = "";
	int votersRegistryShKey = -1;
	int registeredVoters = -1;
	int numVoters = -1;
	string choices = "";
	for (int i = 1; i < 15; i = i + 2) { // value 0 is program name so we don't need it
		if (strcmp(argv[i], "-i") == 0) {
			registry = argv[i + 1];
		} else if (strcmp(argv[i], "-d") == 0) {
			period = atoi(argv[i + 1]);
		} else if (strcmp(argv[i], "-o") == 0) {
			statsfile = argv[i + 1];
		} else if (strcmp(argv[i], "-s") == 0) {
			votersRegistryShKey = atoi(argv[i + 1]);
		} else if (strcmp(argv[i], "-r") == 0) {
			registeredVoters = atoi(argv[i + 1]);
		} else if (strcmp(argv[i], "-n") == 0) {
			numVoters = atoi(argv[i + 1]);
		} else if (strcmp(argv[i], "-c") == 0) {
			choices = argv[i + 1];
		}
	}
	if (registry == "" || period < 0 || statsfile == ""
			|| votersRegistryShKey < 0 || registeredVoters < 0
			|| numVoters < 0) {
		cout << "Collector: Something went wrong while reading given arguments, try again!"
				<< endl;
		return 1;
	}
	int orchestratorPid = getppid();
	/*******************************************************************************************/

	srand(time(NULL));

	/******** SHARED MEMORY SEGMENT FOR COLLECTOR	*********/
	// create shared memory segment
	int collectorsMemShId = createCollectorsMem();
	if (collectorsMemShId == -1) {
		return 1;
	}

	// attach shared memory segment
	void* collectorsMem = attachCollectorsMem(collectorsMemShId);
	if (collectorsMem == (void*) -1) {
		// cleanup before erroneous exit
		// destroy shared memory segments
		shmctl(collectorsMemShId, IPC_RMID, 0);

		return 1;
	}

	/********************************************************************/

	/********************	CREATING SEMAPHORE FOR VOTERS QUEUE	**************************/
	sem_t *votersQueueSem = createVotersQueueSemaphore();
	if (votersQueueSem == NULL) {
		// cleanup before erroneous exit
		// detach shared memory segments
		shmdt((void*) collectorsMem);
		// destroy shared memory segments
		shmctl(collectorsMemShId, IPC_RMID, 0);

		return 1;
	}
	sem_close(votersQueueSem);
	/*******************************************************************************************/

	/********************	CREATING SEMAPHORE FOR COLLECTORS ACCESS	**************************/
	collectorsAccessSem = createCollectorsAccessSemaphore();
	if (collectorsAccessSem == NULL) {
		// cleanup before erroneous exit
		// detach shared memory segments
		shmdt((void*) collectorsMem);
		// destroy shared memory segments
		shmctl(collectorsMemShId, IPC_RMID, 0);
		// close semaphores
		sem_close(votersQueueSem);
		// destroy semaphores
		sem_unlink(VOTERS_QUEUE_SEM_NAME);

		return 1;
	}
	/*******************************************************************************************/

	/********************	CREATING SEMAPHORE FOR COLLECTORS FREE	**************************/
	sem_t *collectorsFreeSem = createCollectorsFreeSemaphore();
	if (collectorsFreeSem == NULL) {
		// cleanup before erroneous exit
		// detach shared memory segments
		shmdt((void*) collectorsMem);
		// destroy shared memory segments
		shmctl(collectorsMemShId, IPC_RMID, 0);
		// close semaphores
		sem_close(votersQueueSem);
		sem_close(collectorsAccessSem);
		// destroy semaphores
		sem_unlink(VOTERS_QUEUE_SEM_NAME);
		sem_unlink(COLLECTORS_ACCESS_SEM_NAME);

		return 1;
	}
	sem_close(collectorsFreeSem);
	/*******************************************************************************************/

	// create and initialize list for results
	resultList results;
	results.init(choices);

	// main loop
	int totalVotes = 0;
	while (totalVotes != numVoters && end == false) {
		sem_wait(collectorsAccessSem);
		if(end == true){
			break;
		}
		int votersSelection = readVotersSelectionFromCollectorsMem(
				collectorsMem);
		results.add(votersSelection);
		// sleep for a random period from 1 to period
		sleep((rand() % period + 1));
		writeSuccess(collectorsMem);
		totalVotes++;
	}

	/**********	OPEN SHARED MEMORY SEGMENT	FOR VOTERS	REGISTRY	************/
	// get shared memory segment for access to voters' registry
	int votersRegistryShId = getVotersRegistry(registeredVoters,
			votersRegistryShKey);
	if (votersRegistryShId == -1) {
		// cleanup before erroneous exit
		// exited main loop terminate orchestrator 
		kill(orchestratorPid, SIGINT);
		// detach shared memory segments
		shmdt((void*) collectorsMem);
		// destroy shared memory segments
		shmctl(collectorsMemShId, IPC_RMID, 0);
		// close semaphores
		sem_close(votersQueueSem);
		sem_close(collectorsAccessSem);
		// destroy semaphores
		sem_unlink(VOTERS_QUEUE_SEM_NAME);
		sem_unlink(COLLECTORS_ACCESS_SEM_NAME);
		sem_unlink(COLLECTORS_FREE_SEM_NAME);

		return 1;
	}

	// attach shared memory segment
	void* votersRegistryMem = attachVotersRegistry(votersRegistryShId);
	if (votersRegistryMem == (void*) -1) {
		// cleanup before erroneous exit
		// exited main loop terminate orchestrator 
		kill(orchestratorPid, SIGINT);
		// detach shared memory segments
		shmdt((void*) collectorsMem);
		// destroy shared memory segments
		shmctl(collectorsMemShId, IPC_RMID, 0);
		// close semaphores
		sem_close(votersQueueSem);
		sem_close(collectorsAccessSem);
		// destroy semaphores
		sem_unlink(VOTERS_QUEUE_SEM_NAME);
		sem_unlink(COLLECTORS_ACCESS_SEM_NAME);
		sem_unlink(COLLECTORS_FREE_SEM_NAME);

		return 1;
	}
	/***********************************************************************/

	/**********	OPEN SEMAPHORE	FOR VOTERS	REGISTRY	************/
	sem_t *votersRegistrySem = openVotersRegistrySemaphore();
	if (votersRegistrySem == NULL) {
		// cleanup before erroneous exit
		// exited main loop terminate orchestrator 
		kill(orchestratorPid, SIGINT);
		// detach shared memory segments
		shmdt((void*) collectorsMem);
		shmdt((void*) votersRegistryMem);
		// destroy shared memory segments
		shmctl(collectorsMemShId, IPC_RMID, 0);
		// close semaphores
		sem_close(votersQueueSem);
		sem_close(collectorsAccessSem);
		// destroy semaphores
		sem_unlink(VOTERS_QUEUE_SEM_NAME);
		sem_unlink(COLLECTORS_ACCESS_SEM_NAME);
		sem_unlink(COLLECTORS_FREE_SEM_NAME);


		return 1;
	}
	/***********************************************************************/

	// print results
	results.print(statsfile, numVoters, votersRegistrySem, votersRegistryMem, registeredVoters);

	// exited main loop terminate orchestrator 
	kill(orchestratorPid, SIGINT);

	// detach shared memory segments
	shmdt((void*) collectorsMem);
	// destroy shared memory segments
	shmctl(collectorsMemShId, IPC_RMID, 0);

	// close semaphores
	sem_close(votersQueueSem);
	sem_close(collectorsAccessSem);

	// destroy semaphores
	sem_unlink(VOTERS_QUEUE_SEM_NAME);
	sem_unlink(COLLECTORS_ACCESS_SEM_NAME);
	sem_unlink(COLLECTORS_FREE_SEM_NAME);
}
