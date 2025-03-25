#pragma once
#ifndef DB
#define DB

#include <iostream>
#include <vector>
#include <string>
#include <SQLiteCpp/SQLiteCpp.h>
#include "Game.h"
#include "User.h"
using namespace std;
using namespace SQLite;

class ReversiDB
{
public:
	ReversiDB() {};
	~ReversiDB() {};

	static void initDB();
	static User getUser(int playerId);
	static int regis(string name);
};
#endif