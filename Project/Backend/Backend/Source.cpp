#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>
#include "Game.h"
using namespace std;
using namespace uWS;
using namespace nlohmann;

Game game;

// handle function
void place(json data) {
	int x = data["x"];
	int y = data["y"];
	game.place(x, y);
}

void undo(json data) {

}

void sync(json data) {

}

void save(json data) {

}

void login(json data) {

}

void join(json data) {
	game.initialGame();
}

void leave(json data) {

}

void replay(json data) {

}

void update(json data) {

}

void replayed(json data) {

}

void joined(json data) {

}


map<string, void(*)(json data)> EVENTMAP{
	{"place",place},
	{"replay",replay},
	{"undo", undo},
	{"sync",sync},
	{"save",save},
	{"login",login},
	{"join",join},
	{"leave",leave},
	{"update",update},
	{"replayed",replayed},
	{"joined",joined},

};


int main() {
	struct MyStruct
	{

	};

	App().ws<string>("/*", {
		/* WebSocket 事件處理 */
		.open = [](auto* ws) {
			cout << "WebSocket 連線成功!" << endl;
		},
		.message = [](auto* ws, string_view message, OpCode opCode) {
			cout << message << opCode << endl;
			json response = json::parse(message);
			string socketEvent = response["event"];
			if (EVENTMAP.find(socketEvent) != EVENTMAP.end()) {
				EVENTMAP[socketEvent](response);
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
