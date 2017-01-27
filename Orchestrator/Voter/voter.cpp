#include "voter.hpp"
#include "fstream"
#include "ctime"
#include <cstdlib>
#include <unistd.h>
#include <sstream>

using namespace std;

long randomID(string registry, int registeredVoters) {
	srand(time(NULL) ^ (getpid() << 16));
	// pick an existing ID
	ifstream input(registry.c_str());
	string line = "";
	int linePos = rand() % registeredVoters;
	for (int i = 0; i < linePos; i++) {
		getline(input, line);
	}
	istringstream iss(line);
	long voterID, numVotes;
	iss >> voterID >> numVotes;
	input.close();
	return voterID;
}


int randomChoice(string choices, int numChoices) {
	srand(time(NULL) ^ (getpid() << 16));
	if ((rand() % 20) > 1) {
		// pick an existing choice
		ifstream input(choices.c_str());
		string line = "";
		int linePos = rand() % numChoices;
		for (int i = 0; i < linePos; i++) {
			getline(input, line);
		}
		istringstream iss(line);
		long choiceNum, choice;
		iss >> choiceNum >> choice;
		input.close();
		return choiceNum;
	} else {
		// pick random choice
		return rand() % 50 + 10;
	}
}

int countChoices(string choices) {
	int numLines = 0;
	ifstream input(choices.c_str());
	string unused;
	while (std::getline(input, unused))
		++numLines;
	input.close();
	return numLines;
}
