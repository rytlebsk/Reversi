﻿#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>
#include <string>
#include <sstream>
#include "Game.h"
#include "User.h"
#include "DatabaseControl.h"
using namespace std;
using namespace uWS;
using namespace nlohmann;
using webSocket = uWS::WebSocket<0, 1, class std::basic_string<char, struct std::char_traits<char>, class std::allocator<char>>>;//alias

Game game;

vector<User> onlineUser(1, User());
vector<Game> onlineGame(1, Game());

// 定義 std::pair<int, int> 的 JSON 轉換
void to_json(json& j, const pair<int, int>& p) {
	j = { p.first,p.second };
}

// 定義 Game 的 JSON 轉換
void to_json(json& j, const Game& g) {
	j = json{
		{"id", g.id},
		{"player", g.player},
		{"whiteScore", g.whiteScore},
		{"blackScore", g.blackScore},
		{"nonValidCount", g.nonValidCount},
		{"board", g.board},
		{"validSquare", g.validSquare},
		{"pathX", g.pathX},
		{"pathY", g.pathY}
	};
}

// handle function
void place(webSocket* ws, OpCode opCode, json data) {
	int x = data["x"];
	int y = data["y"];
	game.place(x, y);
	game.pathX.push_back(x);
	game.pathY.push_back(y);
	try {
		json gameJson = game;
		std::cout << "Game JSON: " << gameJson.dump(4) << std::endl;
		ws->send(gameJson.dump(), opCode);
	}
	catch (const nlohmann::json::exception& e) {
		std::cerr << "JSON 轉換錯誤: " << e.what() << std::endl;
	}
}

void undo(webSocket* ws, OpCode opCode, json data) {
	game.pathX.pop_back();
	game.pathY.pop_back();
	game.initialGame();

	for (int i = 0; i < game.pathX.size(); i++) {
		game.place(game.pathX[i], game.pathY[i]);
	}

	try {
		json gameJson = game;
		std::cout << "Game JSON: " << gameJson.dump(4) << std::endl;
		ws->send(gameJson.dump(), opCode);
	}
	catch (const nlohmann::json::exception& e) {
		std::cerr << "JSON 轉換錯誤: " << e.what() << std::endl;
	}
}

void sync(webSocket* ws, OpCode opCode, json data) {
	try {
		json gameJson = game;
		ws->send(gameJson.dump(), opCode);
		std::cout << "Sync game state: " << gameJson.dump(4) << std::endl;
	}
	catch (const nlohmann::json::exception& e) {
		std::cerr << "同步錯誤: " << e.what() << std::endl;
	}
}

void save(webSocket* ws, OpCode opCode, json data) {
	game.id = onlineGame.size() + 1; // 手動生成遊戲 ID
	onlineGame.push_back(game);
	try {
		json response = { {"status", "success"}, {"gameId", game.id} };
		ws->send(response.dump(), opCode);
		std::cout << "Game saved in memory with ID: " << game.id << std::endl;
	}
	catch (const nlohmann::json::exception& e) {
		std::cerr << "保存錯誤: " << e.what() << std::endl;
	}
}

void login(webSocket* ws, OpCode opCode, json data) {

	cout << "login" << endl;

	string strId = data["id"];
	User user = ReversiDB::getUser(stoi(strId));

	onlineUser.push_back(user);

	stringstream ss;
	ss << onlineUser.size();
	string a = ss.str();

	ws->send(a);
}

void regis(webSocket* ws, OpCode opCode, json data) {
	int id = ReversiDB::regis(data["name"]);

	stringstream ss;
	ss << id;
	string a = ss.str();

	ws->send(a, opCode, false);
}

void join(webSocket* ws, OpCode opCode, json data) {
	game.initialGame();
	try {
		json gameJson = game;
		ws->send(gameJson.dump(), opCode);
		std::cout << "User joined game" << std::endl;
	}
	catch (const nlohmann::json::exception& e) {
		std::cerr << "加入遊戲錯誤: " << e.what() << std::endl;
	}
}

void leave(webSocket* ws, OpCode opCode, json data) {
	// 假設什麼都不做，只發送確認訊息
	try {
		json response = { {"status", "success"}, {"message", "Left game"} };
		ws->send(response.dump(), opCode);
		std::cout << "User left game" << std::endl;
	}
	catch (const nlohmann::json::exception& e) {
		std::cerr << "離開遊戲錯誤: " << e.what() << std::endl;
	}
}

void replay(webSocket* ws, OpCode opCode, json data) {
	int gameId = data["gameId"];
	for (const auto& g : onlineGame) {
		if (g.id == gameId) {
			game = g; // 從 onlineGame 中重播
			try {
				json gameJson = game;
				ws->send(gameJson.dump(), opCode);
				std::cout << "Replaying game " << gameId << std::endl;
			}
			catch (const nlohmann::json::exception& e) {
				std::cerr << "重播錯誤: " << e.what() << std::endl;
			}
			return;
		}
	}
	json response = { {"status", "error"}, {"message", "Game not found"} };
	ws->send(response.dump(), opCode);
}

void update(webSocket* ws, OpCode opCode, json data) {
	game.player = (game.player == 1) ? 2 : 1; // 切換玩家
	try {
		json gameJson = game;
		ws->send(gameJson.dump(), opCode);
		std::cout << "Game updated, current player: " << game.player << std::endl;
	}
	catch (const nlohmann::json::exception& e) {
		std::cerr << "更新錯誤: " << e.what() << std::endl;
	}
}

void replayed(webSocket* ws, OpCode opCode, json data) {
	int gameId = data["gameId"];
	try {
		json response = { {"status", "replayed"}, {"gameId", gameId} };
		ws->send(response.dump(), opCode);
		std::cout << "Replay confirmed for game " << gameId << std::endl;
	}
	catch (const nlohmann::json::exception& e) {
		std::cerr << "重播確認錯誤: " << e.what() << std::endl;
	}
}

void joined(webSocket* ws, OpCode opCode, json data) {
	try {
		json response = { {"status", "joined"}, {"gameId", game.id} };
		ws->send(response.dump(), opCode);
		std::cout << "Join confirmed" << std::endl;
	}
	catch (const nlohmann::json::exception& e) {
		std::cerr << "加入確認錯誤: " << e.what() << std::endl;
	}
}


map<string, void(*)(webSocket* ws, OpCode opCode, json data)> EVENTMAP{
	{"place",place},
	{"replay",replay},
	{"undo", undo},
	{"sync",sync},
	{"save",save},
	{"login",login},
	{"register",regis},
	{"join",join},
	{"leave",leave},
	{"update",update},
	{"replayed",replayed},
	{"joined",joined},
};


int main() {
	App().ws<string>("/*", {
		/* WebSocket 事件處理 */
		.open = [](auto* ws) {
			cout << "WebSocket 連線成功!" << endl;
			ReversiDB::initDB();
		},
		.message = [](webSocket* ws, string_view message, OpCode opCode) {
			cout << message << opCode << endl;
			json response = json::parse(message);
			string socketEvent = response["event"];
			if (EVENTMAP.find(socketEvent) != EVENTMAP.end()) {
				try {
					EVENTMAP[socketEvent](ws, opCode, response);
				}
				catch (const Exception& e) {
					cerr << e.what() << endl;
				}
			}
			ws->send(message, opCode, false);  // Echo 回傳訊息
		},
		.close = [](auto* ws, int /*code*/, std::string_view message) {
			cout << "bye" << endl;
		}
		}).listen(9001, [](auto* token) {
			if (token) {
				cout << "伺服器運行在 ws://localhost:9001" << endl;
			}
			}).run();
}
