#pragma once
#ifndef USER_H
#define USER_H

#include<vector>
#include<string>
#include"Game.h"
using namespace std;

class User
{
public:
	User();
	User(int playerId);
	~User();

	int id = 0;
	string name = "";
	vector<Game> game;
};

#endif
