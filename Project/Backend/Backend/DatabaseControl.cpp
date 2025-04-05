#include "DatabaseControl.h"
#include <nlohmann/json.hpp>
using namespace nlohmann;

Database db("reversi.db", OPEN_READWRITE | OPEN_CREATE);
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
			"hostTimer DOUBLE, "
			"guestTimer DOUBLE, "
			"whiteScore INTEGER, "
			"blackScore INTEGER, "
			"nonValidCount INTEGER, "
			"validSquare TEXT, "
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

		string query = "SELECT id, player, done, hostTimer, guestTimer, whiteScore, blackScore, nonValidCount, validSquare, board, pathX, pathY "
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
			game.hostTimer = searchGame.getColumn(3).getDouble();
			game.guestTimer = searchGame.getColumn(4).getDouble();
			game.whiteScore = searchGame.getColumn(5).getInt();
			game.blackScore = searchGame.getColumn(6).getInt();
			game.nonValidCount = searchGame.getColumn(7).getInt();
			game.validSquare = json::parse(searchGame.getColumn(8).getString())["validSquare"].get<vector<pair<int, int>>>();
			game.board = json::parse(searchGame.getColumn(9).getString())["board"].get<vector<vector<int>>>();
			game.pathX = json::parse(searchGame.getColumn(10).getString())["pathX"].get<vector<int>>();
			game.pathY = json::parse(searchGame.getColumn(11).getString())["pathY"].get<vector<int>>();

			player.gameTable.insert(pair<int, Game>(player.gameId[idIndex], game));

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

void ReversiDB::save(User player) {
	/*save user*/

	Statement saveUser(db, "UPDATE users SET gameId=? WHERE id=?;");

	saveUser.bind(2, player.id);

	json gameId = { {"gameId", player.gameId} };

	saveUser.bind(1, gameId.dump());

	saveUser.exec();

	/*save game*/
	for (int i : player.gameId) {
		Statement saveGame(db,
			"UPDATE games SET player=?, done=?, hostTimer=?, guestTimer=?, "
			"whiteScore=?, blackScore=?, nonValidCount=?, validSquare=?, board=?, "
			"pathX=?, pathY=? WHERE id=?;");

		Game thisGame = player.gameTable[i];

		saveGame.bind(1, thisGame.id);
		saveGame.bind(2, thisGame.done);
		saveGame.bind(3, thisGame.hostTimer);
		saveGame.bind(4, thisGame.guestTimer);
		saveGame.bind(5, thisGame.whiteScore);
		saveGame.bind(6, thisGame.blackScore);
		saveGame.bind(7, thisGame.nonValidCount);

		json validSquare = { {"validSquare", thisGame.validSquare} };
		json board = { {"board", thisGame.board} };
		json pathX = { {"pathX", thisGame.pathX} };
		json pathY = { {"pathY", thisGame.pathY} };

		saveGame.bind(8, validSquare.dump());
		saveGame.bind(9, board.dump());
		saveGame.bind(10, pathX.dump());
		saveGame.bind(11, pathY.dump());

		saveGame.bind(12, i);

		saveGame.exec();
	}
}

int ReversiDB::createGame(User& player) {
	try {
		Statement createGame(db, "INSERT INTO games (player, done, hostTimer, guestTimer, whiteScore, blackScore, nonValidCount, validSquare, board, pathX, pathY) "
			"VALUES (?,?,?,?,?,?,?,?,?,?,?);");

		json emptyValidSquare = { {"validSquare", json::array()} };
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
		createGame.bind(8, emptyValidSquare.dump());
		createGame.bind(9, emptyBoard.dump());
		createGame.bind(10, emptyPathX.dump());
		createGame.bind(11, emptyPathY.dump());

		createGame.exec();

		Game emptyGame;

		dbGameSize++;

		player.gameId.push_back(dbGameSize);
		player.gameTable.insert(pair<int, Game>(player.gameId[player.gameId.size() - 1], emptyGame));

		return dbGameSize;
	}
	catch (const Exception& e) {
		cerr << "register fail cause from " << e.what() << endl;
	}
}