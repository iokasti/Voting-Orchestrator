#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include "resultList.hpp"

using namespace std;

resultList::resultList() {
	head = NULL;
}

resultList::~resultList() {
	resultListNode *temp = head;
	while (head->next != NULL) {
		temp = head->next;
		delete head;
		head = temp;
	}
	delete head;
}
void resultList::init(string choices) {
	ifstream input(choices.c_str());
	int id;
	string name;
	while (input >> id >> name) {
		addInit(id, name);
	}
	input.close();
	// add invalid votes - id:-2, name:"invalid"
	addInit(-2, "invalid");
}

void resultList::addInit(int id, string name) {
	int votes = 0;
	if (head == NULL) {
		resultListNode* newNode = new resultListNode();
		newNode->id = id;
		newNode->name = name;
		newNode->votes = votes;
		newNode->next = NULL;
		newNode->previous = NULL;
		head = newNode;
	} else {
		resultListNode* newNode = new resultListNode();
		resultListNode* temp = head;
		while (temp->next != NULL) {
			temp = temp->next;
		}
		newNode->id = id;
		newNode->name = name;
		newNode->votes = votes;
		newNode->next = NULL;
		newNode->previous = temp;
		temp->next = newNode;
	}
}

void resultList::add(int id) {
	// search the list for name
	resultListNode *temp = head;
	while (temp != NULL) {
		if (temp->id == id) {
			// id found, add 1 vote and return
			temp->votes += 1;
			// rearrange list so it's sorted
			while (temp->previous != NULL
					&& (temp->votes > temp->previous->votes)) {
				int tempID = temp->previous->id;
				string tempName = temp->previous->name;
				int tempVotes = temp->previous->votes;
				temp->previous->id = temp->id;
				temp->previous->name = temp->name;
				temp->previous->votes = temp->votes;
				temp->id = tempID;
				temp->name = tempName;
				temp->votes = tempVotes;
				temp = temp->previous;
			}
			return;
		}
		temp = temp->next;
	}

	// id wasn't found so vote is invalid
	add(-2);	// add one vote to invalid votes
}

int resultList::getVotes(){
	resultListNode *temp=head;
	int votes=0;
	if (head != NULL) {
			while (temp != NULL) {
				votes+=temp->votes;
				temp = temp->next;
			}
	}
	return votes;
}

int resultList::getValidVotes(){
	resultListNode *temp=head;
		int validVotes=0;
		if (head != NULL) {
				while (temp != NULL) {
					if(temp->id!=-2){
						validVotes+=temp->votes;
					}
					temp = temp->next;
				}
		}
		return validVotes;
}

int resultList::getInvalidVotes(){
	resultListNode *temp=head;
		int invalidVotes=0;
		if (head != NULL) {
				while (temp != NULL) {
					if(temp->id == -2){
						invalidVotes+=temp->votes;
					}
					temp = temp->next;
				}
		}
		return invalidVotes;
}

int resultList::getWhiteVotes(){
	resultListNode *temp=head;
		int whiteVotes=0;
		if (head != NULL) {
				while (temp != NULL) {
					if(temp->id==0){
						whiteVotes+=temp->votes;
					}
					temp = temp->next;
				}
		}
		return whiteVotes;
}

int getNumVotersWhoDoubleVoted(void* votersRegistryMem, int registeredVoters) {
	int count = 0;
	for (int i = 0; i < registeredVoters; i++) {
		long vID;
		int timesVoted;
		memcpy(&vID, votersRegistryMem + i * (sizeof(long) + sizeof(int)),
				sizeof(vID));
		memcpy(&timesVoted,
				votersRegistryMem + i * (sizeof(long) + sizeof(int))
						+ sizeof(long), sizeof(timesVoted));
		if(timesVoted > 1){
			count++;
		}
	}
	return count;
}

void resultList::print(string fileName, int numVoters, sem_t* votersRegistrySem, void* votersRegistryMem, int registeredVoters) {
	ofstream myfile;
	myfile.open(fileName.c_str(), ios_base::app);

	myfile << "Number of voters who tried to participate: " << numVoters << endl;
	myfile << "Number of voters who successfully participated: " << getVotes() << endl;
	sem_wait(votersRegistrySem);
	myfile << "Number of voters who tried to vote more than once: " << getNumVotersWhoDoubleVoted(votersRegistryMem, registeredVoters) <<endl;
	sem_post(votersRegistrySem);
	myfile << "Number of valid votes: " << getValidVotes() << endl;
	myfile << "Number of white votes: " << getWhiteVotes() << endl;
	myfile << "Number of invalid votes: " << getInvalidVotes() << endl;

	resultListNode *temp = head;
	myfile << "Number of votes for each choice in the ballot:" <<endl;
	if (head != NULL) {
		while (temp != NULL) {
			myfile << temp->id << " " << temp->name << " " << temp->votes
					<< endl;
			temp = temp->next;
		}
	}

	myfile << "Percentages for each choice in the ballot: " << endl;
	int allVotes = getVotes();
	temp = head;
	if (head != NULL) {
		while (temp != NULL) {
			myfile << temp->id << " " << temp->name << " " << (double)((double)temp->votes/(double)allVotes)
					<< endl;
			temp = temp->next;
		}
	}
	myfile << "IDs of voters who tried to vote more than once: " << endl;
	sem_wait(votersRegistrySem);
	for (int i = 0; i < registeredVoters; i++) {
		long vID;
		int timesVoted;
		memcpy(&vID, votersRegistryMem + i * (sizeof(long) + sizeof(int)),
				sizeof(vID));
		memcpy(&timesVoted,
				votersRegistryMem + i * (sizeof(long) + sizeof(int))
						+ sizeof(long), sizeof(timesVoted));
		if(timesVoted > 1){
			myfile << vID << endl;
		}
	}	
	sem_post(votersRegistrySem);

	myfile.close();
}

