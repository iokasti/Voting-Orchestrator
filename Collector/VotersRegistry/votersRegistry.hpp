#ifndef VOTERSREGISTRY_VOTERSREGISTRY_HPP_
#define VOTERSREGISTRY_VOTERSREGISTRY_HPP_

#define VOTERS_REG_SEM_NAME "votersRegSem"

#include <semaphore.h>

int getVotersRegistry(int, int);	// shmget shared memory for voters registry
void* attachVotersRegistry(int);	// shmat shared memory for voters registry
sem_t* openVotersRegistrySemaphore();// opens semaphore for access to votersRegistry

#endif /* VOTERSREGISTRY_VOTERSREGISTRY_HPP_ */
