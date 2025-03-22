#include <vector>
#include <iostream>

using namespace std;

const int BOARD_SIZE = 8;  // 棋盤是 8x8

struct piece {
	int row;
	int col;
	int player;
};

class Game {
	int id; //key -> id
	bool priority;
	double hostTimer;
	double guestTimer;
	vector<vector<int>> board;
	vector<int> pathX; //positionX -> pathX
	vector<int> pathY; //positionY -> pathY
};


void initialBoard(vector<vector<int>>& board) {

	// 黑白棋初始位置  
	int startX[] = { 3, 4, 3, 4 };
	int startY[] = { 3, 4, 4, 3 };

	for (int i = 0; i < 4; i++) {
		board[startY[i]][startX[i]] = i / 2 == 0 ? 2 : 1;
	}
}

//step 2 & 3
bool checkPiece(vector<vector<int>>& board, pair<int, int> dir, piece opponent_piece, int player) {

	pair<int, int> direction = dir;
	int x = opponent_piece.col;
	int y = opponent_piece.row;

	while (x <= 7 && x >= 0 && y <= 7 && y >= 0) {
		x += direction.first;
		y += direction.second;
		if (x >= 8 || x < 0 || y >= 8 || y < 0) {
			break;
		}
		if (board[y][x] == player) {
			return true;  // 找到我方棋子
		}
		else if (board[y][x] == 0) {
			break;  // 遇到空格
		}
	}
	return false;  // 到邊界還沒找到我方棋子
}

//檢測有效格子
//method:
//1.從對方的棋子去檢查周圍8格有沒有空格
//2.取一個Vector2f的向量(對方棋子 - 空格)
//3.用上一步的向量取反向量的方向去找有沒有我方的棋子
//4.將該格設為有效格子(顯示為紅色)
void findValidSquare(vector<vector<int>>& board, vector<pair<int, int>>& validSquare, int player) {
	validSquare.clear();
	int opponent = 3 - player;
	vector<piece> opponentPiece;
	for (int i = 0; i < board.size(); i++) {
		for (int j = 0; j < board[i].size(); j++) {
			if (board[i][j] == opponent) {
				opponentPiece.push_back(piece{ i,j,opponent });
			}
		}
	}

	//step 1
	for (int i = 0; i < opponentPiece.size(); i++) {
		for (int j = -1; j <= 1; j++) {
			for (int k = -1; k <= 1; k++) {

				if (j == 0 && k == 0) continue; // 跳過 (0,0) 方向
				int row = opponentPiece[i].row + j;
				int col = opponentPiece[i].col + k;
				//檢查是否在範圍內
				if (row >= 8 || row < 0 || col >= 8 || col < 0) {
					continue;
				}
				//step 2
				if (board[row][col] == 0) {
					pair<int, int> direction; //(x,y)
					direction.first = k;
					direction.second = j;
					direction.first *= -1.0f;
					direction.second *= -1.0f;
					//cout << "direction: " << direction.x << ", " << direction.y << endl;
					if (checkPiece(board, direction, opponentPiece[i], player)) {
						bool isDuplicate = false;
						//step 4
						//檢查是否重複
						for (int i = 0; i < validSquare.size(); i++) {
							if (validSquare[i].first == col && validSquare[i].second == row) {
								isDuplicate = true;
								break;
							}
						}
						if (!isDuplicate)
							validSquare.push_back(pair{ col,row });
					}
				}
			}
		}
	}

	for (pair<int, int> coords : validSquare) {
		board[coords.second][coords.first] = 3; // can be placed
	}

}

void findCanEatSquare(vector<vector<int>>& board, vector<pair<int, int>>& validSquare, int& player) {
	for (int i = 0; i < validSquare.size(); i++) {
		for (int j = -1; j <= 1; j++) {
			for (int k = -1; k <= 1; k++) {
				if (j == 0 && k == 0) continue;
				int x = validSquare[i].first + k;
				int y = validSquare[i].second + j;
				if (x >= 8 || x < 0 || y >= 8 || y < 0) {
					continue;
				}
				if (board[y][x] == 3 - player) {
					pair<int, int> direction;
					direction.first = k;
					direction.second = j;
					int _x = validSquare[i].first;
					int _y = validSquare[i].second;
					if (checkPiece(board, direction, piece{ y,x,3 - player }, player)) {
						while (true) {
							_x += direction.first;
							_y += direction.second;

							// 先檢查範圍
							if (_x < 0 || _x >= 8 || _y < 0 || _y >= 8) {
								break;
							}

							if (board[_y][_x] == 3 - player) {
								board[_y][_x] = 4;  // 將board上可吃的棋子設定成4
							}
							else if (board[_y][_x] == player) {
								break;  // 遇到我方棋子，停止
							}
							else {
								break;  // 遇到空格，停止
							}
						}
					}
				}
			}
		}
	}
}

bool placePiece(vector<vector<int>>& board, vector<pair<int, int>>& validSquare, pair<int, int> position, int& player) {
	int row = position.second;
	int col = position.first;

	for (int i = 0; i < validSquare.size(); i++) {
		if (validSquare[i].first == col && validSquare[i].second == row) {
			board[row][col] = player;
			// flip piece
			for (int j = -1; j <= 1; j++) {
				for (int k = -1; k <= 1; k++) {
					if (k == 0 && j == 0) continue;
					int x = col + k;
					int y = row + j;
					if (x >= 8 || x < 0 || y >= 8 || y < 0) {
						continue;
					}
					if (board[y][x] == 4) {
						pair<int, int> direction;
						direction.first = k;
						direction.second = j;
						int _x = col;
						int _y = row;
						if (checkPiece(board, direction, piece{ y,x,3 - player }, player)) {
							while (true) {
								_x += direction.first;
								_y += direction.second;
								// 先檢查範圍
								if (_x < 0 || _x >= 8 || _y < 0 || _y >= 8) {
									break;
								}

								if (board[_y][_x] == 4) {
									board[_y][_x] = player;  // 翻轉
								}
								else if (board[_y][_x] == player) {
									break;  // 遇到我方棋子，停止
								}
								else {
									break;  // 遇到空格，停止
								}
							}
						}
					}
				}
			}
			//remove validSquare from board
			for (int i = 0; i < board.size(); i++) {
				for (int j = 0; j < board[0].size(); j++) {
					board[i][j] = board[i][j] == 3 ? 0 : board[i][j];

				}
			}

			//return can eat square back to opponent's piece
			for (int i = 0; i < board.size(); i++) {
				for (int j = 0; j < board[0].size(); j++) {
					board[i][j] = board[i][j] == 4 ? 3 - player : board[i][j];
				}
			}
			//switch player
			player = 3 - player;


			return true;
		}
	}
	return false;
}

void calculateScore(vector<vector<int>>& board, int& whiteScore, int& blackScore) {
	whiteScore = 0;
	blackScore = 0;
	for (int i = 0; i < board.size(); i++) {
		for (int j = 0; j < board[i].size(); j++) {
			if (board[i][j] == 1) {
				blackScore++;
			}
			else if (board[i][j] == 2) {
				whiteScore++;
			}
		}
	}
}

void checkGameOver(vector<vector<int>>& board, int whiteScore, int blackScore) {
	for (int i = 0; i < board.size(); i++) {
		for (int j = 0; j < board[i].size(); j++) {
			if (board[i][j] == 0) {
				return;
			}
		}
	}
	// game over
}
/*
int main() {
	vector<vector<int>> board(BOARD_SIZE, vector<int>(BOARD_SIZE, 0)); // 棋盤狀態 0:空格 1:黑 2:白
	vector<pair<int, int>> validSquare; // 有效格子
	int player = 1; // 現在輪到的玩家 1:黑 2:白
	// score
	int whiteScore = 2;
	int blackScore = 2;
	int nonValidCount = 0;

	initialBoard(board); // 初始化棋盤

	findValidSquare(board, validSquare, player); // 找出初始有效格子

	findCanEatSquare(board, validSquare, player); // 找出初始可吃格子

	while (true) {

		if (nonValidCount >= 2) {
			//cout << "Game Over" << endl;
			checkGameOver(board, whiteScore, blackScore);
			continue;
		}

		// 遊戲結束
		checkGameOver(board, whiteScore, blackScore);

		cout << "current player = " << player << endl;

		for (int i = 0; i < board.size(); i++) {
			for (int j = 0; j < board[0].size(); j++) {
				cout << board[i][j] << " ";
			}
			cout << endl;
		}

		int x, y;
		cin >> x >> y;

		// 當有position進來後執行
		if (placePiece(board, validSquare, { x,y }, player)) {
			calculateScore(board, whiteScore, blackScore);
			findValidSquare(board, validSquare, player);
			findCanEatSquare(board, validSquare, player);
			if (validSquare.empty())
			{
				nonValidCount++;
				player = 3 - player;
				findValidSquare(board, validSquare, player);
				findCanEatSquare(board, validSquare, player);
				continue;
			}
			nonValidCount = 0;
		}

	}
	return 0;
}
*/