#ifndef VOTERSREGISTRY_VOTERSREGISTRY_HPP_
#define VOTERSREGISTRY_VOTERSREGISTRY_HPP_

#define VOTERS_REG_SHMEM_KEY 7777
#define VOTERS_REG_SEM_NAME "votersRegSem"

#include <semaphore.h>

int countRegisteredVoters(std::string);	// returns number of registered voters in file
int createVotersRegistry(int);	// shmget shared memory for voters registry
void* attachVotersRegistry(int);	// shmat shared memory for voters registry
void fillVotersRegistry(std::string, void*);// fill voters registry from file
sem_t* createVotersRegistrySemaphore();	// creates semaphore used for access in voters registry

#endif /* VOTERSREGISTRY_VOTERSREGISTRY_HPP_ */
