#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>
#include <string>
#include <sstream>
#include "Game.h"
#include "User.h"
#include "DatabaseControl.h"
using namespace std;
using namespace uWS;
using namespace nlohmann;
using webSocket = uWS::WebSocket<0, 1, struct Player>;//alias
struct Player {
	int id = 0;
	int gameId = 0;
	webSocket* pWS = nullptr;
};
//using webSocket = uWS::WebSocket<0, 1, class basic_string<char, struct char_traits<char>, class allocator<char>>>;//alias

//Game game;

vector<User> onlineUser;
vector<Game> onlineGame;
vector<Player*> findMatch;

map<int, int> UserId;

struct Data {
	Data(webSocket* _ws, OpCode _opCode, json _data) {
		ws = _ws;
		opCode = _opCode;
		data = _data;
	}
	webSocket* ws;
	OpCode opCode;
	json data;
};

// 定義 pair<int, int> 的 JSON 轉換
void to_json(json& j, const pair<int, int>& p) {
	j = { p.first,p.second };
}

// 定義 tuple<int, int> 的 JSON 轉換
void to_json(json& j, const tuple<int, int>& p) {
	j = { get<0>(p) , get<1>(p) };
}

// 定義 Game 的 JSON 轉換
void to_json(json& j, const Game& g) {
	j = json{
		{"id", g.id},
		{"player", g.player},
		{"done",g.done},
		{"whiteId",g.whiteId},
		{"blackId",g.blackId},
		{"whiteTimer",g.whiteTimer},
		{"blackTimer",g.blackTimer},
		{"whiteScore", g.whiteScore},
		{"blackScore", g.blackScore},
		{"nonValidCount", g.nonValidCount},
		{"validSquare", g.validSquare},
		{"board", g.board},
		{"validSquare", g.validSquare},
		{"canEatSaquare", g.canEatSquare},
		{"pathX", g.pathX},
		{"pathY", g.pathY}
	};
}

json convertCanEatSquare(const unordered_map<tuple<int, int>, vector<pair<int, int>>, TupleHash>& canEatSquare) {
	json result;
	for (const auto& [key, value] : canEatSquare) {
		// 把 tuple<int, int> 轉成 "xy" 這種字串格式
		string keyStr = to_string(get<0>(key)) + to_string(get<1>(key));

		// 轉換 vector<pair<int, int>> 為 JSON 陣列
		json valueArray = json::array();
		for (const auto& p : value) {
			valueArray.push_back(to_string(p.first) + to_string(p.second));
		}

		result[keyStr] = valueArray;
	}
	return result;
}

// handle function
void place(Data datas) {
	Player* p = datas.ws->getUserData();
	Game& game = onlineGame[p->gameId];
	string position = datas.data["position"];
	int x = position[0] - '0';
	int y = position[1] - '0';
	game.place(x, y);
	game.pathX.push_back(x);
	game.pathY.push_back(y);
	try {
		json gameJson = game;
		//cout << "Game JSON: " << gameJson.dump(4) << endl;
		//datas.ws->send(gameJson.dump(), datas.opCode, false);
	}
	catch (const json::exception& e) {
		cerr << "JSON 轉換錯誤: " << e.what() << endl;
	}
}

void undo(Data datas) {
	Player* p = datas.ws->getUserData();
	Game& game = onlineGame[p->gameId];
	game.pathX.pop_back();
	game.pathY.pop_back();
	game.initialGame();

	for (int i = 0; i < game.pathX.size(); i++) {
		game.place(game.pathX[i], game.pathY[i]);
	}

	try {
		json gameJson = game;
		cout << "Game JSON: " << gameJson.dump(4) << endl;
		//datas.ws->send(gameJson.dump(), datas.opCode, false);
	}
	catch (const json::exception& e) {
		cerr << "JSON 轉換錯誤: " << e.what() << endl;
	}
}

void sync(Data datas) {
	/*try {
		json gameJson = game;
		datas.ws->send(gameJson.dump(), datas.opCode);
		cout << "Sync game state: " << gameJson.dump(4) << endl;
	}
	catch (const json::exception& e) {
		cerr << "同步錯誤: " << e.what() << endl;
	}*/


}

void login(Data datas) {
	try {
		cout << "login" << endl;

		string strId = datas.data["id"];
		User user = ReversiDB::getUser(stoi(strId));

		onlineUser.push_back(user);
		UserId.insert(pair<int, int>(user.id, onlineUser.size() - 1));

		Player* p = datas.ws->getUserData();
		p->id = user.id;

		json gameId = { {"gameId",user.gameId} };

		datas.ws->send(gameId.dump(), datas.opCode, false);
	}
	catch (Exception& e) {
		cerr << e.what() << endl;
		datas.ws->send("Wrong format.Please try again.");
	}
}

void logout(Data datas) {
	Player* p = datas.ws->getUserData();
	User u = onlineUser[UserId[p->id]];
	ReversiDB::save(u);

	onlineUser.erase(onlineUser.begin() + UserId[p->id]);
	p->id = 0;
}

void regis(Data datas) {
	int id = ReversiDB::regis();

	cout << id << endl;

	json repeat = {
		{"event","registered"},
		{"id",to_string(id)}
	};

	datas.ws->send(repeat.dump(), datas.opCode, false);
}

void join(Data datas) {
	try {
		string gameid = datas.data["id"];

		if (gameid == "new_game_b") {
			Player* p = datas.ws->getUserData();
			User* u = &onlineUser[UserId[p->id]];

			int gameId = ReversiDB::createGame(*u);
			u->gameTable[gameId].whiteId = p->id;
			u->gameTable[gameId].blackId = p->id;
			onlineGame.push_back(u->gameTable[gameId]);

			p->gameId = onlineGame.size() - 1;

			datas.ws->send(to_string(gameId), datas.opCode, false);

			Game& game = onlineGame[p->gameId];
			game.initialGame();
		}
		else if (gameid == "new_game_p") {
			Player* p = datas.ws->getUserData();
			p->pWS = datas.ws;
			findMatch.push_back(p);

			if (findMatch.size() >= 2) {
				Player* p1 = findMatch[0];
				Player* p2 = findMatch[1];
				User* u1 = &onlineUser[UserId[p1->id]];
				User* u2 = &onlineUser[UserId[p2->id]];

				int gameId = ReversiDB::createGame(*u1);
				u1->gameTable[gameId].whiteId = u1->id;
				u1->gameTable[gameId].blackId = u2->id;
				onlineGame.push_back(u1->gameTable[gameId]);
				p2->gameId = p1->gameId = onlineGame.size() - 1;

				u2->gameId.push_back(gameId);
				u2->gameTable.insert(pair<int, Game>(gameId, u1->gameTable[gameId]));

				p1->pWS->send("match found", datas.opCode, false);
				p2->pWS->send("match found", datas.opCode, false);

				for (int i = 0; i < 2; i++)findMatch.erase(findMatch.begin());

				Game& game = onlineGame[p1->gameId];
				game.initialGame();
			}
			else datas.ws->send("Waiting for match...", datas.opCode, false);
		}
		else {
			Player* p = datas.ws->getUserData();
			User* u = &onlineUser[UserId[p->id]];

			onlineGame.push_back(u->gameTable[stoi(gameid)]);
			p->gameId = onlineGame.size() - 1;

			cout << onlineGame[p->gameId].whiteId << " " << onlineGame[p->gameId].blackId << endl;
			datas.ws->send(gameid, datas.opCode, false);
		}

	}
	catch (const json::exception& e) {
		cerr << "加入遊戲錯誤: " << e.what() << endl;
	}
}

void leave(Data datas) {
	/*save logic*/
	Player* p = datas.ws->getUserData();
	User* u = &onlineUser[UserId[p->id]];

	int gameId = onlineGame[p->gameId].id;
	u->gameTable[gameId] = onlineGame[p->gameId];

	//ReversiDB::saveGame(onlineGame[p->gameId]);

	ReversiDB::save(onlineUser[UserId[p->id]]);

	for (Game g : onlineGame)cout << g.id << endl;

	/*clear impliment*/
	onlineGame.erase(onlineGame.begin() + p->gameId);
	for (Game g : onlineGame)cout << g.id << endl;
	p->gameId = 0;
}

void replay(Data datas) {
	Player* p = datas.ws->getUserData();
	Game& game = onlineGame[p->gameId];
	game.initialGame();
	vector<vector<vector<int>>> historyGameBoard;
	historyGameBoard.push_back(game.board);
	int size = game.pathX.size();
	for (int i = 0; i < size; i++) {
		game.place(game.pathX[i], game.pathY[i]);
		historyGameBoard.push_back(game.board);
	}
	try {
		json response = {
				{"status","ok"},
				{"board",historyGameBoard},
		};
		datas.ws->send(response.dump(), datas.opCode, false);
	}
	catch (const json::exception& e) {
		json response = {
			{"status","error"},
		};
		datas.ws->send(response.dump(), datas.opCode, false);
		cerr << "JSON 轉換錯誤: " << e.what() << endl;
	}
}

void update(Data datas) {
	Player* p = datas.ws->getUserData();
	Game& game = onlineGame[p->gameId];
	try {
		json p = {
			{"status","ok"},
			{"board",game.board},
			{"canDo",convertCanEatSquare(game.canEatSquare)}
		};
		datas.ws->send(p.dump(), datas.opCode, false);
	}
	catch (const json::exception& e) {
		json p = {
			{"status","update error"}
		};
		datas.ws->send(p.dump(), datas.opCode, false);
		cerr << "更新錯誤: " << e.what() << endl;
	}
}

void create(Data datas) {
	Player* p = datas.ws->getUserData();
	int playerId = p->id;
	User& u = onlineUser[UserId[playerId]];

	int gameId = ReversiDB::createGame(u);
	for (int i : u.gameId)cout << i << endl;

	onlineGame.push_back(u.gameTable[gameId]);
	p->gameId = onlineGame.size() - 1;

	onlineGame[p->gameId].blackScore = 10;

	u.gameTable[gameId] = onlineGame[p->gameId];

	onlineGame.erase(onlineGame.begin() + p->gameId);
	p->gameId = 0;

	ReversiDB::save(u);

	onlineUser.erase(onlineUser.begin() + UserId[p->id]);
	UserId.erase(UserId.find(p->id));
	p->id = 0;
}

map<string, void(*)(Data)> EVENTMAP{
	{"place",place},
	{"replay",replay},
	{"undo", undo},
	{"sync",sync},
	{"login",login},
	{"logout",logout},
	{"register",regis},
	{"join",join},
	{"leave",leave},
	{"update",update},
	{"create",create}
};


int main() {
	ReversiDB::initDB();

	App().ws<Player>("/*", {
		/* WebSocket 事件處理 */
		.open = [](webSocket* ws) {
			cout << "WebSocket 連線成功!" << endl;
		},
		.message = [](webSocket* ws, string_view message, OpCode opCode) {
			cout << message << opCode << endl;
			json response = json::parse(message);
			string socketEvent = response["event"];
			if (EVENTMAP.find(socketEvent) != EVENTMAP.end()) {
				try {
					EVENTMAP[socketEvent](Data(ws, opCode, response));
				}
				catch (const Exception& e) {
					cerr << e.what() << endl;
				}
			}
			//ws->send(message, opCode, false);  // Echo 回傳訊息
		},
		.close = [](webSocket* ws, int c, string_view message) {
			json response;
			/*leave(Data(ws, opCode, response));
			logout(Data(ws, opCode, response));*/
			cout << "bye" << endl;
		}
		}).listen(9001, [](auto* token) {
			if (token) {
				cout << "伺服器運行在 ws://localhost:9001" << endl;
			}
			}).run();
}
