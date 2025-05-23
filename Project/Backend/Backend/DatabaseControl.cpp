#include "DatabaseControl.h"
#include <nlohmann/json.hpp>
using namespace nlohmann;

Database db("./db/reversi.db", OPEN_READWRITE | OPEN_CREATE);
int dbUserSize, dbGameSize;

vector<int> ReversiDB::existGame;

void ReversiDB::initDB() {
	try {
		// 創建資料表
		db.exec("CREATE TABLE IF NOT EXISTS users ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT, "
			"gameId TEXT)");

		db.exec("CREATE TABLE IF NOT EXISTS games ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT, "
			"player INTEGER, "
			"done INTERGER, "
			"whiteId INTERGER, "
			"blackId INTERGER, "
			"whiteTimer DOUBLE, "
			"blackTimer DOUBLE, "
			"whiteScore INTEGER, "
			"blackScore INTEGER, "
			"nonValidCount INTEGER, "
			"validSquare TEXT, "
			"canEatSquare TEXT, "
			"board TEXT, "
			"pathX TEXT, "
			"pathY TEXT)");

		Statement countUsersSize(db, "SELECT COUNT(*) FROM users");
		if (countUsersSize.executeStep())dbUserSize = countUsersSize.getColumn(0).getInt();

		Statement countGamesSize(db, "SELECT COUNT(*) FROM games");
		if (countGamesSize.executeStep())dbGameSize = countGamesSize.getColumn(0).getInt();
	}
	catch (const Exception& e) {
		cerr << "錯誤: " << e.what() << endl;
		return;
	}

	cout << "db initialize success. User:" << dbUserSize << " Game:" << dbGameSize << endl;
}

int ReversiDB::regis() {
	try {
		Statement registerUser(db, "INSERT INTO users (gameId) VALUES (?) ");

		json emptyGameId = { {"gameId", json::array()} };

		registerUser.bind(1, emptyGameId.dump());

		registerUser.exec();

		dbUserSize++;

		return dbUserSize;
	}
	catch (const Exception& e) {
		cerr << "register fail cause from " << e.what() << endl;
	}
}

User ReversiDB::getUser(int playerId) {
	User player;

	try {
		Statement searchUser(db, "SELECT * FROM users WHERE id = ?");
		searchUser.bind(1, playerId);

		if (searchUser.executeStep()) {
			player.id = searchUser.getColumn(0).getInt();
			player.gameId = json::parse(searchUser.getColumn(1).getString())["gameId"].get<vector<int>>();
		}
		else {
			throw runtime_error("User not found");
		}

		if (!player.gameId.size())return player;

		string query = "SELECT id, player, done, whiteId, blackId, whiteTimer, blackTimer, whiteScore, blackScore, nonValidCount, validSquare, canEatSquare, board, pathX, pathY "
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

		int idIndex = 0;

		while (searchGame.executeStep()) {
			Game game;
			game.id = searchGame.getColumn(0).getInt();
			game.player = searchGame.getColumn(1).getInt();
			game.done = searchGame.getColumn(2).getInt();
			game.whiteId = searchGame.getColumn(3).getInt();
			game.blackId = searchGame.getColumn(4).getInt();
			game.whiteTimer = searchGame.getColumn(5).getDouble();
			game.blackTimer = searchGame.getColumn(6).getDouble();
			game.whiteScore = searchGame.getColumn(7).getInt();
			game.blackScore = searchGame.getColumn(8).getInt();
			game.nonValidCount = searchGame.getColumn(9).getInt();
			game.validSquare = json::parse(searchGame.getColumn(10).getString())["validSquare"].get<vector<pair<int, int>>>();
			game.canEatSquare = json::parse(searchGame.getColumn(11).getString())["canEatSquare"].get<unordered_map<tuple<int, int>, vector<pair<int, int>>, TupleHash>>();
			game.board = json::parse(searchGame.getColumn(12).getString())["board"].get<vector<vector<int>>>();
			game.pathX = json::parse(searchGame.getColumn(13).getString())["pathX"].get<vector<int>>();
			game.pathY = json::parse(searchGame.getColumn(14).getString())["pathY"].get<vector<int>>();

			player.gameTable.emplace(player.gameId[idIndex], game);

			idIndex++;
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

void ReversiDB::save(const User& player) {
	/*save user*/

	Statement saveUser(db, "UPDATE users SET gameId=? WHERE id=?;");

	saveUser.bind(2, player.id);

	json gameId = { {"gameId", player.gameId} };

	saveUser.bind(1, gameId.dump());

	saveUser.exec();

	/*save game*/
	for (int i : player.gameId) {
		Statement saveGame(db,
			"UPDATE games SET player=?, done=?, whiteId=?, blackId=?, whiteTimer=?, blackTimer=?, "
			"whiteScore=?, blackScore=?, nonValidCount=?, validSquare=?, canEatSquare=?, board=?, "
			"pathX=?, pathY=? WHERE id=?;");

		auto it = player.gameTable.find(i);
		if (it == player.gameTable.end())continue;
		Game thisGame(it->second);

		saveGame.bind(1, thisGame.player);
		saveGame.bind(2, thisGame.done);
		saveGame.bind(3, thisGame.whiteId);
		saveGame.bind(4, thisGame.blackId);
		saveGame.bind(5, thisGame.whiteTimer);
		saveGame.bind(6, thisGame.blackTimer);
		saveGame.bind(7, thisGame.whiteScore);
		saveGame.bind(8, thisGame.blackScore);
		saveGame.bind(9, thisGame.nonValidCount);

		json validSquare = { {"validSquare", thisGame.validSquare} };
		json canEatSquare = { {"canEatSquare",thisGame.canEatSquare} };
		json board = { {"board", thisGame.board} };
		json pathX = { {"pathX", thisGame.pathX} };
		json pathY = { {"pathY", thisGame.pathY} };

		saveGame.bind(10, validSquare.dump());
		saveGame.bind(11, canEatSquare.dump());
		saveGame.bind(12, board.dump());
		saveGame.bind(13, pathX.dump());
		saveGame.bind(14, pathY.dump());

		saveGame.bind(15, i);

		saveGame.exec();

		cout << "Save:" << i << endl;
	}
}

int ReversiDB::createGame(User& player) {
	try {
		Statement createGame(db, "INSERT INTO games (player, done, whiteId, blackId, whiteTimer, blackTimer, whiteScore, blackScore, nonValidCount, validSquare, canEatSquare, board, pathX, pathY) "
			"VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?);");

		json emptyValidSquare = { {"validSquare", json::array()} };
		json emptyCanEatSquare = { {"canEatSquare",json::array()} };
		json emptyBoard = { {"board", json::array()} };
		json emptyPathX = { {"pathX", json::array()} };
		json emptyPathY = { {"pathY", json::array()} };

		createGame.bind(1, 0);
		createGame.bind(2, 0);
		createGame.bind(3, 0);
		createGame.bind(4, 0);
		createGame.bind(5, 0);
		createGame.bind(6, 0);
		createGame.bind(7, 0);
		createGame.bind(8, 0);
		createGame.bind(9, 0);
		createGame.bind(10, emptyValidSquare.dump());
		createGame.bind(11, emptyCanEatSquare.dump());
		createGame.bind(12, emptyBoard.dump());
		createGame.bind(13, emptyPathX.dump());
		createGame.bind(14, emptyPathY.dump());

		createGame.exec();

		dbGameSize++;

		Game emptyGame;

		emptyGame.id = dbGameSize;

		player.gameId.push_back(dbGameSize);
		player.gameTable.emplace(dbGameSize, emptyGame);

		cout << "Create:" << player.gameTable.at(dbGameSize).id << endl;

		return dbGameSize;
	}
	catch (const Exception& e) {
		cerr << "register fail cause from " << e.what() << endl;
	}
}