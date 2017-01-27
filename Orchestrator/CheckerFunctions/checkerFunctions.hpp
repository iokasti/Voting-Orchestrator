#ifndef CHECKERFUNCTIONS_CHECKERFUNCTIONS_HPP_
#define CHECKERFUNCTIONS_CHECKERFUNCTIONS_HPP_

#define CHECKERS_STATE_SEM_NAME "checkersStateSem"

#include <semaphore.h>

sem_t* createCheckersStateSemaphore(int); // creates semaphore used for access in access

#endif /* CHECKERFUNCTIONS_CHECKERFUNCTIONS_HPP_ */
