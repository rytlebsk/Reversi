#pragma once
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
	int id; //key -> id
	int player = 1; // 現在輪到的玩家 1:黑 2:白
	int done = false;
	//player id
	int whiteId;
	int blackId;
	//timer
	double whiteTimer;
	double blackTimer;
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
	~Game() {};

	//convert class into json
	//NLOHMANN_DEFINE_TYPE_INTRUSIVE(Game, id, player, whiteScore, blackScore, board, validSquare);
};

#endif