#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>
using namespace std;
using namespace uWS;
using namespace nlohmann;

// handle function
void place() {

}

void undo() {

}

void sync() {

}

void save() {

}

void login() {

}

void join() {

}

void leave() {

}

void replay() {

}

void update() {

}

void replayed() {

}

void joined() {

}


map<string, void(*)()> EVENTMAP{
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
				EVENTMAP[socketEvent]();
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
