/*

Lib utilisée : https://github.com/socketio/socket.io-client-cpp

*/


#include <string>
#include <iostream>

#include "sio_client.h"

using namespace std;
using namespace sio;


void config_changed(sio::event & e) {
	cout << e.get_message()->get_string() << "\n";
}


// point d'entrée -----------------------------------------------------------------
int main(int argc, char **argv) {
	sio::client io;

	// sera appelé quand la connection sera établie
	io.set_open_listener([&]() {
		io.socket()->on("send2client_cpp", &config_changed);
	});

	io.connect("http://127.0.0.1:3000");
	
	
	while (true) {}

	return 0;
}
