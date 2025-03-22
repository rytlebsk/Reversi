#pragma once
#ifndef GAME_H
#define GAME_H

#include <vector>
using namespace std;

class Game {
public:
    int id; //key -> id
    int player = 1; // 現在輪到的玩家 1:黑 2:白
    // score
    int whiteScore = 2;
    int blackScore = 2;
    int nonValidCount = 0;
    vector<vector<int>> board = vector<vector<int>>(8, vector<int>(8, 0));
    vector<pair<int, int>> validSquare; // 有效格子
    vector<int> pathX; //positionX -> pathX
    vector<int> pathY; //positionY -> pathY

    void initialGame();
    void place(int x, int y);
};

#endif