#ifndef CHECKERFUNCTIONS_CHECKERFUNCTIONS_HPP_
#define CHECKERFUNCTIONS_CHECKERFUNCTIONS_HPP_

#define VOTERS_TBC_SHMEM_GENKEY 8888
#define CHECKERS_STATE_SEM_NAME "checkersStateSem"
#define FREE_CHECKER_SEM_NAME_GEN "freeCheckerSem"
#define CHECKER_ACCESS_SEM_NAME_GEN "accessCheckerSem"

#include <semaphore.h>
#include <string>

// same way we use at checker but we create them once more in order to search for checker through voter process
std::string uniqueFreeCheckerSemName(int); // creates a unique freeCheckerSem name using FREE_CHECKER_SEM_NAME_GEN and checkerNum
std::string uniqueAccessCheckerSemName(int); // creates a unique accesCheckerSem name using CHECKER_ACCESS_SEM_NAME_GEN and checkerNum

sem_t* openCheckersStateSemaphore(); // creates semaphore used for access in access
sem_t* getFreeChecker(int, int*); // trywait on all checkers, lock the successful one, and get its checkerNum

int getCheckersMemory(int);
void* attachCheckersMemory(int);

void writeVoterIDtoCheckerSharedMemory(long, void*);
void wakeFreeChecker(int);
bool getCheckerAnswer(void*);

#endif /* CHECKERFUNCTIONS_CHECKERFUNCTIONS_HPP_ */
