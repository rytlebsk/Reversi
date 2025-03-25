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
	~User();

	int id = 0;
	string name = "";
	vector<int> gameId;
	vector<Game> game;
	int playingId;
};

#endif
