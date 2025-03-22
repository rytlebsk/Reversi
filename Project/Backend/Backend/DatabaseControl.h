#pragma once
#ifndef DB
#define DB

#include <iostream>
#include <vector>
#include <string>
#include <SQLiteCpp/SQLiteCpp.h>
#include"Game.h"
using namespace std;
using namespace SQLite;

class ReversiDB
{
public:
	ReversiDB() {};
	~ReversiDB() {};

	void initDB();
	string findName(int playerId);
	vector<Game> findGame(int playerId);
};

#endif