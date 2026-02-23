

#include "socket_util.h"


// ------------------------------------------------------------------------
int main(int argc, char * argv[]) {
	SocketUtil * socket_util = new SocketUtil();

	std::string msg;
	while (true) {
		if (socket_util->_msgs.next(msg)) {
			std::cout << msg << "\n";
		}
	}

	return 0;
}
