#ifndef VOTERSREGISTRY_VOTERSREGISTRY_HPP_
#define VOTERSREGISTRY_VOTERSREGISTRY_HPP_

#define VOTERS_REG_SEM_NAME "votersRegSem"

#include <semaphore.h>

int getVotersRegistry(int, int);	// shmget shared memory for voters registry
void* attachVotersRegistry(int);	// shmat shared memory for voters registry
bool voterExists(int, int, void*);// returns true if voter with given voterID exists
bool addVote(int, int, void*);	// adds a vote for given voterID;
int getVotes(int, int, void*);	// returns number of times a voter has voted
sem_t* openVotersRegistrySemaphore();// opens semaphore for access to votersRegistry

#endif /* VOTERSREGISTRY_VOTERSREGISTRY_HPP_ */
