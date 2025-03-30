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
	static User getUser(int);
	static int regis();
	static void save(User);
};
#endif