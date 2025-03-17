#include <uwebsockets/App.h>
using namespace std;
using namespace uWS;

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
