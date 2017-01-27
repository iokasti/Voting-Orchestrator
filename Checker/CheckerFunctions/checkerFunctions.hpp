#ifndef CHECKERFUNCTIONS_CHECKERFUNCTIONS_HPP_
#define CHECKERFUNCTIONS_CHECKERFUNCTIONS_HPP_

#define VOTERS_TBC_SHMEM_GENKEY 8888
#define FREE_CHECKER_SEM_NAME_GEN "freeCheckerSem"
#define CHECKER_ACCESS_SEM_NAME_GEN "accessCheckerSem"

#include <string>
#include <semaphore.h>

std::string uniqueFreeCheckerSemName(int); // creates a unique freeCheckerSem name using FREE_CHECKER_SEM_NAME_GEN and checkerNum
std::string uniqueAccessCheckerSemName(int); // creates a unique accesCheckerSem name using CHECKER_ACCESS_SEM_NAME_GEN and checkerNum
int createVotersToBeChecked(int); // shmget shared memory for voters to be checked
void* attachVotersToBeChecked(int);	// shmat shared memory for voters to be checked
long readVoterIDFromVotersToBeChecked(void*); // reads voterID from votersToBeChecked shared memory
void writeVotersToBeChecked(bool, void*); // writes 1 or 0 in votersToBeChecked shared memory according to a bool
sem_t* createFreeCheckerSemaphore(int); // creates semaphore used to check if checker is being used or not
sem_t* createCheckersAccessSemaphore(int); // creates semaphore used for access in checker

#endif /* CHECKERFUNCTIONS_CHECKERFUNCTIONS_HPP_ */
