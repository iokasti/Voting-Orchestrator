#include "checkerFunctions.hpp"
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>

using namespace std;

sem_t* createCheckersStateSemaphore(int checkers_num) {
	sem_t *checkersStateSem = sem_open(CHECKERS_STATE_SEM_NAME,
	O_CREAT | O_EXCL,
	S_IRUSR | S_IWUSR, checkers_num);

	if (checkersStateSem == SEM_FAILED) {
		cout << "Creation of checkers' state semaphore failed!" << endl;
		return NULL;
	}

	return checkersStateSem;
}
