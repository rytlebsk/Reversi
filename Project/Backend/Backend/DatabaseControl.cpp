#include"DatabaseControl.h"

void ReversiDB::initDB() {
	try {
		// 打開資料庫（若不存在則創建）
		Database db("test.db", OPEN_READWRITE | OPEN_CREATE);

		// 創建資料表
		db.exec("CREATE TABLE IF NOT EXISTS users ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT, "
			"name TEXT NOT NULL, "
			"age INTEGER)");
		std::cout << "資料表創建成功！" << std::endl;

		// 插入資料
		Statement insert(db, "INSERT INTO users (name, age) VALUES (?, ?)");
		insert.bind(1, "Alice");  // 綁定第一個參數
		insert.bind(2, 25);       // 綁定第二個參數
		insert.exec();            // 執行插入
		std::cout << "資料插入成功！" << std::endl;

		// 查詢資料
		Statement query(db, "SELECT id, name, age FROM users");
		std::cout << "查詢結果：" << std::endl;
		while (query.executeStep()) {  // 逐行處理結果
			int id = query.getColumn(0).getInt();
			std::string name = query.getColumn(1).getString();
			int age = query.getColumn(2).getInt();
			std::cout << "ID: " << id << ", Name: " << name << ", Age: " << age << std::endl;
		}
	}
	catch (const Exception& e) {
		std::cerr << "錯誤: " << e.what() << std::endl;
		return;
	}

	std::cout << "操作完成。" << std::endl;
	return;
}

string ReversiDB::findName(int playerId) {

}

vector<Game> ReversiDB::findGame(int playerId) {

}


