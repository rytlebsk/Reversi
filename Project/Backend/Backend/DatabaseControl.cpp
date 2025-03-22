#include"DatabaseControl.h"

void ReversiDB::initDB() {
	try {
		// ���}��Ʈw�]�Y���s�b�h�Ыء^
		Database db("test.db", OPEN_READWRITE | OPEN_CREATE);

		// �Ыظ�ƪ�
		db.exec("CREATE TABLE IF NOT EXISTS users ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT, "
			"name TEXT NOT NULL, "
			"age INTEGER)");
		std::cout << "��ƪ�Ыئ��\�I" << std::endl;

		// ���J���
		Statement insert(db, "INSERT INTO users (name, age) VALUES (?, ?)");
		insert.bind(1, "Alice");  // �j�w�Ĥ@�ӰѼ�
		insert.bind(2, 25);       // �j�w�ĤG�ӰѼ�
		insert.exec();            // ���洡�J
		std::cout << "��ƴ��J���\�I" << std::endl;

		// �d�߸��
		Statement query(db, "SELECT id, name, age FROM users");
		std::cout << "�d�ߵ��G�G" << std::endl;
		while (query.executeStep()) {  // �v��B�z���G
			int id = query.getColumn(0).getInt();
			std::string name = query.getColumn(1).getString();
			int age = query.getColumn(2).getInt();
			std::cout << "ID: " << id << ", Name: " << name << ", Age: " << age << std::endl;
		}
	}
	catch (const Exception& e) {
		std::cerr << "���~: " << e.what() << std::endl;
		return;
	}

	std::cout << "�ާ@�����C" << std::endl;
	return;
}

string ReversiDB::findName(int playerId) {

}

vector<Game> ReversiDB::findGame(int playerId) {

}


