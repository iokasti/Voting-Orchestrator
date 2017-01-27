#ifndef RESULTLIST_RESULISTLIST_HPP
#define RESULTLIST_RESULISTLIST_HPP

#include <string>
#include <semaphore.h>

struct resultListNode {
	int id;
	std::string name;
	int votes;
	resultListNode* next;
	resultListNode* previous;
};

class resultList {
private:
	resultListNode* head;
public:
	resultList();
	~resultList();
	void init(std::string);
	void addInit(int, std::string);
	void add(int);
	int getVotes();
	int getValidVotes();
	int getInvalidVotes();
	int getWhiteVotes();
	void print(std::string, int, sem_t*, void*, int);
};

#endif /* RESULTLIST_RESULISTLIST_HPP */
