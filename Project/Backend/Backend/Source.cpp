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

// handle function
void place(webSocket* ws, OpCode opCode, json data) {
	int x = data["x"];
	int y = data["y"];
	game.place(x, y);
}

void undo(webSocket* ws, OpCode opCode, json data) {

}

void sync(webSocket* ws, OpCode opCode, json data) {

}

void save(webSocket* ws, OpCode opCode, json data) {

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
}

void leave(webSocket* ws, OpCode opCode, json data) {

}

void replay(webSocket* ws, OpCode opCode, json data) {

}

void update(webSocket* ws, OpCode opCode, json data) {

}

void replayed(webSocket* ws, OpCode opCode, json data) {

}

void joined(webSocket* ws, OpCode opCode, json data) {

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
