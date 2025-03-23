#include "DatabaseControl.h"
#include <nlohmann/json.hpp>
using namespace nlohmann;

Database db("test.db", OPEN_READWRITE | OPEN_CREATE);
int dbSize;

void ReversiDB::initDB() {
	try {
		// 創建資料表
		db.exec("CREATE TABLE IF NOT EXISTS users ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT, "
			"name TEXT NOT NULL, "
			"gameId TEXT)");

		db.exec("CREATE TABLE IF NOT EXISTS games ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT, "
			"player INTEGER NOT NULL, "
			"whiteScore INTEGER, "
			"blackScore INTEGER, "
			"nonValidCount INTEGER, "
			"board TEXT, "
			"validSquare TEXT, "
			"pathX TEXT, "
			"pathY TEXT)");

		Statement countUsersSize(db, "SELECT COUNT(*) FROM users");

		if (countUsersSize.executeStep())dbSize = countUsersSize.getColumn(0).getInt();
	}
	catch (const Exception& e) {
		cerr << "錯誤: " << e.what() << endl;
		return;
	}

	cout << "操作完成。" << endl;
}

void ReversiDB::regis(string name) {
	try {
		Statement registerUser(db, "INSERT INTO users (name, gameId) VALUES (?, ?) ");

		json emptyGameId = { {"gameId", json::array()} };
		string str = emptyGameId.dump();

		registerUser.bind(1, name);
		registerUser.bind(2, str);

		registerUser.exec();

		cout << "register " << name << " success" << endl;

		dbSize++;
	}
	catch (const Exception& e) {
		cerr << "register " << name << " fail cause from " << e.what() << endl;
	}
}

User ReversiDB::getUser(int playerId) {
	User player;

	try {
		Statement searchUser(db, "SELECT * FROM users WHERE id = ?");
		searchUser.bind(1, playerId);

		if (searchUser.executeStep()) {
			player.id = searchUser.getColumn(0).getInt();
			player.name = searchUser.getColumn(1).getString();
			player.gameId = json::parse(searchUser.getColumn(2).getString())["gameId"].get<vector<int>>();
		}
		else {
			throw runtime_error("User not found");
		}

		if (!player.gameId.size())return player;

		string query = "SELECT id, player, whiteScore, blackScore, nonValidCount, board, validSquare, pathX, pathY "
			"FROM games WHERE id IN (";
		for (size_t i = 0; i < player.gameId.size(); ++i) {
			query += "?";
			if (i < player.gameId.size() - 1) query += ",";
		}
		query += ")";

		Statement searchGame(db, query);
		for (size_t i = 0; i < player.gameId.size(); ++i) {
			searchGame.bind(static_cast<int>(i + 1), player.gameId[i]); // 綁定每個 gameId
		}

		while (searchGame.executeStep()) {
			Game game;
			game.id = searchGame.getColumn("id").getInt();
			game.player = searchGame.getColumn("player").getInt();
			game.whiteScore = searchGame.getColumn("whiteScore").getInt();
			game.blackScore = searchGame.getColumn("blackScore").getInt();
			game.nonValidCount = searchGame.getColumn("nonValidCount").getInt();
			game.board = json::parse(searchUser.getColumn("board").getString())["board"].get<vector<vector<int>>>();
			game.validSquare = json::parse(searchUser.getColumn("validSquare").getString())["validSquare"].get<vector<pair<int, int>>>();
			game.pathX = json::parse(searchUser.getColumn("pathX").getString())["pathX"].get<vector<int>>();
			game.pathY = json::parse(searchUser.getColumn("pathY").getString())["pathY"].get<vector<int>>();

			player.game.push_back(game);
		}

		return player;
	}
	catch (const SQLite::Exception& e) {
		throw std::runtime_error("Database error: " + std::string(e.what()));
	}
	catch (const json::exception& e) {
		throw std::runtime_error("JSON parse error: " + std::string(e.what()));
	}
}


