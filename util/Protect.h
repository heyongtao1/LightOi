#ifndef _PROTECT_H
#define _PROTECT_H
#include <iostream>
#include <unordered_set>
using namespace std;
#include "Singleton.h"
class Protect{
private:
	Protect() = default;
	~Protect(){	blacklist.clear(); };
public:
	bool isExistence(const string& clientIp)
	{
		return blacklist.count(clientIp) == 0 ? false : true;
	}
	void insertBlacklistSet(const string& clientIp)
	{
		blacklist.insert(clientIp);
	}
	friend class Singleton<Protect>;
private:
	unordered_set<string> blacklist; 
};

#endif