#include <stdio.h>
#include <fcntl.h>
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
#include <thread>
#include <errno.h>

#include "json.hpp"

#include "wav_in.h"
#include "sio_util.h"


//using json = nlohmann::json;


//SocketIOUtil * sio_util;
WavIn * wav_in;

std::mutex mtx;
void read_socket();
int client_socket;
int server_socket;
sockaddr_in serverAddress;


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


void read_socket() {
	int n_bytes_read;
	char buffer[3000] = {0};
	while ((n_bytes_read = recv(client_socket, buffer, sizeof(buffer), 0))) {

		if (n_bytes_read == -1 && errno != EAGAIN) {
			printf("Erreur lors de la réception du message. %s (%d)\n", strerror(errno), errno);
			exit(EXIT_FAILURE);
		} else if (n_bytes_read == 0) {
			printf("Le client s'est déconnecté (extrémité de la socket fermée)\n");
			exit(EXIT_FAILURE);
		} else if (n_bytes_read > 0) {
			/*std::cout << "OK recu :" << n_bytes_read << "\n";
			std::cout << "-------------------------------------------\n";
			std::cout << buffer << "\n";*/

			mtx.lock();
			wav_in->new_envelope(std::string(buffer).substr(0, n_bytes_read));
			mtx.unlock();
		}
	}
}


void main_loop() {

	while (true) {
		//if (sio_util->update()) {
			//osc_in->load_json(json::parse(sio_util->_last_msg));
			//cout << *osc_in;
            //json js = sio_util->_last_msg;
            //std::cout << js << "\n";
            //std::cout << sio_util->_last_msg << "\n";
		//}

		mtx.lock();
		wav_in->main_loop();
		mtx.unlock();
	}
}


int main(int argc, char **argv) {
	if (argc!= 2) {
		std::cout << "donner le chemin d'un json en entrée\n";
		return 1;
	}
	init(std::string(argv[1]));
	
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Erreur création socket");
		exit(EXIT_FAILURE);
	}
	
	/*int flags = fcntl(server_socket, F_GETFL, 0);
    if (fcntl(server_socket, F_SETFL, flags | O_NONBLOCK) < 0) {
		perror("fcntl failed");
        close(server_socket);
        exit(EXIT_FAILURE);
	}*/
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket, (struct sockaddr*) & serverAddress, sizeof(serverAddress)) < 0) {
		perror("bind failed");
		close(server_socket);
    	exit(EXIT_FAILURE);
	}
    if (listen(server_socket, 10) < 0) {
		perror("listen failed");
		close(server_socket);
    	exit(EXIT_FAILURE);
	}
	int addrlen = sizeof(serverAddress);
    client_socket = accept(server_socket, (struct sockaddr *)&serverAddress, (socklen_t *)&addrlen);
	if (client_socket < 0) {
		perror("accept failed");
    	exit(EXIT_FAILURE);		
	}
	
	std::thread t(read_socket);
    main_loop();

	clean();

	return 0;
}
