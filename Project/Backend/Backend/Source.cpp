#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>
#include <string>
#include <chrono>
#include <sstream>
#include "Game.h"
#include "User.h"
#include "DatabaseControl.h"
using namespace std;
using namespace uWS;
using namespace nlohmann;
using namespace chrono;
using webSocket = uWS::WebSocket<0, 1, struct Player>;//alias
struct Player {
	int id = 0;
	int gameId = 0;
	chrono::system_clock::time_point lastPlaceTime;
	webSocket* pWS = nullptr;
};
//Game game;

vector<User> onlineUser;
vector<Game> onlineGame;
vector<Player*> findMatch;

map<int, int> UserId;
map<int, int> MatchId;


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

//ai place
void ai_place(Game& game)
{
	if (game.blackId == game.whiteId)
	{
		if (game.player == 1 || !game.blackId)return;
		size_t maxLength = 0;
		int bestX, bestY;
		for (const auto& entry : game.canEatSquare)
		{
			// entry.first 是鍵 (tuple<int, int>)
			int x = std::get<0>(entry.first); // 獲取 x
			int y = std::get<1>(entry.first); // 獲取 y
			cout << x << y << endl;

			// entry.second 是值 (vector<pair<int, int>>)
			size_t length = entry.second.size(); // 獲取 vector 的長度

			if (length > maxLength)
			{
				maxLength = length;
				bestX = x;
				bestY = y;
			}
		}
		game.place(bestX, bestY);
		game.pathX.push_back(bestX);
		game.pathY.push_back(bestY);
	}
}

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

void switchTimer(Game& game) {
	if ((game.blackId == game.whiteId) || !game.blackId)return;

	Player* white = findMatch[MatchId[game.whiteId]], * black = findMatch[MatchId[game.blackId]];
	int nowPointer = game.player;
	if (!nowPointer)return;

	if (nowPointer == 1) {
		black->lastPlaceTime = system_clock::now();
		game.whiteTimer -= static_cast<double>((duration_cast<milliseconds>(system_clock::now() - white->lastPlaceTime)).count()) / 1000;
	}
	else {
		white->lastPlaceTime = system_clock::now();
		game.blackTimer -= static_cast<double>((duration_cast<milliseconds>(system_clock::now() - black->lastPlaceTime)).count()) / 1000;
	}
	json timer = {
		{"event","sync"},
		{"timer",
			{
				{"black",game.blackTimer},
				{"white",game.whiteTimer}
			}
		}
	};

	black->pWS->send(timer.dump(), OpCode::TEXT, false);
	white->pWS->send(timer.dump(), OpCode::TEXT, false);
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

	switchTimer(game);

	//AI放置
	ai_place(game);

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
				{"event","replayed"},
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
			{"canDo",convertCanEatSquare(game.canEatSquare)},
			{"player",game.player == 1 ? "black" : "white"}
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

void regis(Data datas) {
	int id = ReversiDB::regis();

	cout << id << endl;

	json repeat = {
		{"event","registered"},
		{"id",to_string(id)}
	};

	datas.ws->send(repeat.dump(), datas.opCode, false);
}

void login(Data datas) {
	try {
		cout << "login" << endl;

		string strId = datas.data["id"];
		User user = ReversiDB::getUser(stoi(strId));

		onlineUser.push_back(user);
		UserId.emplace(user.id, onlineUser.size() - 1);

		Player* p = datas.ws->getUserData();
		p->id = user.id;

		vector<int> done;

		for (int i : user.gameId) {
			done.push_back(user.gameTable[i].done);
		}

		json gameId = {
			{"loginStatus","success"},
			{"saveGame",user.gameId} ,
			{"done",done}
		};

		datas.ws->send(gameId.dump(), datas.opCode, false);
	}
	catch (runtime_error& r) {
		json gameId = {
			{"loginStatus","fail"}
		};
		datas.ws->send(gameId.dump(), OpCode::TEXT, false);
	}
	catch (Exception& e) {
		cerr << e.what() << endl;
		datas.ws->send("Wrong format.Please try again.");
	}
}

void logout(Data datas) {
	Player* p = datas.ws->getUserData();
	User* u = &onlineUser[UserId[p->id]];

	ReversiDB::save(*u);

	onlineUser[UserId[p->id]].id = -1;
	UserId.erase(p->id);
	p->id = 0;
}

void join(Data datas) {
	try {
		string gameid = datas.data["id"];

		if (gameid == "new_game_b") {
			Player* p = datas.ws->getUserData();
			User* u = &onlineUser[UserId[p->id]];

			int gameId = ReversiDB::createGame(*u);
			u->gameTable[gameId].whiteId = u->gameTable[gameId].blackId = p->id;
			onlineGame.push_back(u->gameTable[gameId]);
			p->gameId = onlineGame.size() - 1;

			cout << "Create complete" << onlineGame[p->gameId].id << endl;

			json joined = {
				{"event","joined"},
				{"id","bot"},
				{"role","black"}
			};

			datas.ws->send(joined.dump(), datas.opCode, false);

			onlineGame[p->gameId].initialGame();
		}
		else if (gameid == "new_game_p") {
			Player* p = datas.ws->getUserData();
			p->pWS = datas.ws;
			findMatch.push_back(p);

			if (!(findMatch.size() % 2)) {
				Player* p1 = findMatch[findMatch.size() - 2];
				Player* p2 = findMatch[findMatch.size() - 1];
				User* u1 = &onlineUser[UserId[p1->id]];
				User* u2 = &onlineUser[UserId[p2->id]];

				int gameId = ReversiDB::createGame(*u1);
				u1->gameTable[gameId].blackId = u1->id;
				u1->gameTable[gameId].whiteId = u2->id;
				u1->gameTable[gameId].blackTimer = u1->gameTable[gameId].whiteTimer = 180;

				u2->gameId.push_back(gameId);
				u2->gameTable.emplace(gameId, u1->gameTable[gameId]);

				MatchId.emplace(p1->id, findMatch.size() - 2);
				MatchId.emplace(p2->id, findMatch.size() - 1);

				onlineGame.push_back(u1->gameTable[gameId]);

				json res1 = {
					{"event","joined"},
					{"id",to_string(p2->id)},
					{"role","black"}
				};
				json res2 = {
					{"event","joined"},
					{"id",to_string(p1->id)},
					{"role","white"}
				};

				p1->pWS->send(res1.dump(), OpCode::TEXT, false);
				p2->pWS->send(res2.dump(), OpCode::TEXT, false);

				p2->gameId = p1->gameId = onlineGame.size() - 1;
				p2->lastPlaceTime = chrono::system_clock::now();
				switchTimer(onlineGame[p1->gameId]);

				onlineGame[p1->gameId].initialGame();
			}
			else datas.ws->send("Waiting for match...", datas.opCode, false);
		}
		else if (gameid == "new_game_l") {
			Player* p = datas.ws->getUserData();
			User* u = &onlineUser[UserId[p->id]];

			int gameId = ReversiDB::createGame(*u);
			u->gameTable[gameId].whiteId = u->gameTable[gameId].blackId = 0;
			onlineGame.push_back(u->gameTable[gameId]);
			p->gameId = onlineGame.size() - 1;

			cout << "Create complete" << onlineGame[p->gameId].id << endl;

			json joined = {
				{"event","joined"},
				{"id",to_string(p->id)},
				{"role","black"}
			};

			datas.ws->send(joined.dump(), datas.opCode, false);

			onlineGame[p->gameId].initialGame();
		}
		else {
			Player* p = datas.ws->getUserData();
			User* u = &onlineUser[UserId[p->id]];

			onlineGame.push_back(u->gameTable[stoi(gameid)]);
			p->gameId = onlineGame.size() - 1;

			json joined = {
				{"event","joined"},
				{"id","bot"},
				{"role","black"}
			};

			datas.ws->send(joined.dump(), datas.opCode, false);
		}
	}
	catch (const json::exception& e) {
		cerr << "加入遊戲錯誤: " << e.what() << endl;
	}
}

void leave(Data datas) {
	Player* p = datas.ws->getUserData();
	User* u = &onlineUser[UserId[p->id]];
	Game& game = onlineGame[p->gameId];

	int gameId = onlineGame[p->gameId].id;

	if (game.blackId != game.whiteId) {
		Player* p1 = findMatch[MatchId[game.blackId]];
		Player* p2 = findMatch[MatchId[game.whiteId]];
		User* u1 = &onlineUser[UserId[p1->id]];
		User* u2 = &onlineUser[UserId[p2->id]];
		if (!game.done)game.done = (game.whiteScore > game.blackScore) ? 2 : (game.whiteScore == game.blackScore) ? 3 : 1;
		u1->gameTable[gameId] = u2->gameTable[gameId] = onlineGame[p->gameId];
		ReversiDB::save(*u1);
		ReversiDB::save(*u2);
		onlineGame[p->gameId].id = -1;

		json res = {
			{"event","left"},
			{"status",3},
			{"done",game.done}
		};
		p1->pWS->send(res.dump(), OpCode::TEXT, false);
		p2->pWS->send(res.dump(), OpCode::TEXT, false);

		p1->gameId = p2->gameId = 0;
	}
	else {
		u->gameTable[gameId] = game;

		ReversiDB::save(*u);

		if (!game.done) {
			json res = {
				{"event","left"},
				{"status",2},
				{"done",game.done}
			};
			datas.ws->send(res.dump(), OpCode::TEXT, false);
		}
		else {
			json res = {
				{"event","left"},
				{"status",1},
				{"done",game.done}
			};
			datas.ws->send(res.dump(), OpCode::TEXT, false);
		}

		onlineGame[p->gameId].id = -1;
		p->gameId = 0;
	}
}

void timeout(Data datas) {
	Player* p = datas.ws->getUserData();
	Game& game = onlineGame[p->gameId];

	game.done = (game.whiteScore > game.blackScore) ? 2 : (game.whiteScore == game.blackScore) ? 3 : 1;
	leave(datas);
}

void pong(Data datas) {
	json pong = { {"pong","pong"} };
	datas.ws->send(pong.dump(), OpCode::TEXT, false);
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
	{"timeout",timeout},
	{"ping",pong}
};

int main() {
	ReversiDB::initDB();

	App().ws<Player>("/*", {
		/* WebSocket 事件處理 */
		.open = [](webSocket* ws) {
			cout << "WebSocket connected" << endl;
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
			if (ws->getUserData()->gameId)leave(Data(ws, OpCode::TEXT, response));
			if (ws->getUserData()->id)logout(Data(ws, OpCode::TEXT, response));
			cout << "left" << endl;
		}
		}).listen(9001, [](auto* token) {
			if (token) {
				cout << "run on ws://localhost:9001" << endl;
			}
			}).run();
}
