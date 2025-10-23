#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <string>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "json.hpp"

#include "wav_in.h"
#include "sio_util.h"


//using json = nlohmann::json;


//SocketIOUtil * sio_util;
WavIn * wav_in;


void clean() {
	//delete sio_util;
	delete wav_in;
}


/*void interruption_handler(sig_atomic_t s) {
	clean();
	exit(1); 
}*/


void init(std::string json_path) {
	//signal(SIGINT, interruption_handler);

	wav_in= new WavIn(json_path);
	//sio_util= new SocketIOUtil("http://127.0.0.1:3003", "server2client_config_changed", 2000);
}


void main_loop(int clientSocket) {
	while (true) {
		//if (sio_util->update()) {
			//osc_in->load_json(json::parse(sio_util->_last_msg));
			//cout << *osc_in;
            //json js = sio_util->_last_msg;
            //std::cout << js << "\n";
            //std::cout << sio_util->_last_msg << "\n";
		//}

        char buffer[3000] = {0};
        recv(clientSocket, buffer, sizeof(buffer), 0);
        if (buffer[0] != 0) {
			std::cout << "-------------------------------------------\n";
            std::cout << buffer << "\n";
			wav_in->new_envelope(std::string(buffer));
        }
	}
}


int main(int argc, char **argv) {
	if (argc!= 2) {
		std::cout << "donner le chemin d'un json en entrée\n";
		return 1;
	}
	init(std::string(argv[1]));
	

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    listen(serverSocket, 5);
    int clientSocket = accept(serverSocket, nullptr, nullptr);

    main_loop(clientSocket);

	clean();

	return 0;
}
