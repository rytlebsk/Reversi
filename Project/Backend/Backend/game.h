﻿#pragma once
#ifndef GAME_H
#define GAME_H

#include <vector>
#include <tuple>
#include <nlohmann/json.hpp>
using namespace std;

struct TupleHash {
	template <typename T1, typename T2>
	size_t operator()(const std::tuple<T1, T2>& t) const {
		size_t h1 = std::hash<T1>{}(std::get<0>(t));
		size_t h2 = std::hash<T2>{}(std::get<1>(t));
		return h1 ^ (h2 << 1);
	}
};

class Game {
public:
	Game() {};

	Game(const Game& g) {
		id = g.id;
		player = g.player;
		done = g.done;
		whiteId = g.whiteId;
		blackId = g.blackId;
		whiteTimer = g.whiteTimer;
		blackTimer = g.blackTimer;
		whiteScore = g.whiteScore;
		blackScore = g.blackScore;
		nonValidCount = g.nonValidCount;
		board = g.board;
		validSquare = g.validSquare;
		canEatSquare = g.canEatSquare;
		pathX = g.pathX;
		pathY = g.pathY;
	}

	int id = 0; //key -> id
	int player = 1; // 現在輪到的玩家 1:黑 2:白
	int done = 0;
	//player id
	int whiteId = 0;
	int blackId = 0;
	//timer
	double whiteTimer = 0;
	double blackTimer = 0;
	// score
	int whiteScore = 2;
	int blackScore = 2;
	int nonValidCount = 0;
	vector<vector<int>> board = vector<vector<int>>(8, vector<int>(8, 0));
	vector<pair<int, int>> validSquare; // 有效格子
	unordered_map<tuple<int, int>, vector<pair<int, int>>, TupleHash> canEatSquare;
	vector<int> pathX; //positionX -> pathX
	vector<int> pathY; //positionY -> pathY

	void initialGame();
	void place(int x, int y);



	//convert class into json
	//NLOHMANN_DEFINE_TYPE_INTRUSIVE(Game, id, player, whiteScore, blackScore, board, validSquare);
};

#endif