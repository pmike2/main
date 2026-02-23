#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include <unordered_set>
#include <algorithm>
#include <iterator>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <errno.h>

#include <glm/gtx/transform.hpp>

#include "typedefs.h"
#include "bbox.h"
#include "bbox_2d.h"
#include "geom.h"
#include "geom_2d.h"


const int MAXDATASIZE = 3000;

int client_socket;
int server_socket;
sockaddr_in serverAddress;
std::mutex mtx;
char buffer[MAXDATASIZE];
int numbytes, msglen;



/*void read_socket() {
	int n_bytes_read;
	while ((n_bytes_read = recv(client_socket, buffer, sizeof(buffer), 0))) {
		if (n_bytes_read == -1 && errno != EAGAIN) {
			printf("Erreur lors de la réception du message. %s (%d)\n", strerror(errno), errno);
			exit(EXIT_FAILURE);
		} else if (n_bytes_read == 0) {
			printf("Le client s'est déconnecté (extrémité de la socket fermée)\n");
			exit(EXIT_FAILURE);
		} else if (n_bytes_read > 0) {
			std::cout << "OK recu :" << n_bytes_read << "\n";
			std::cout << "-------------------------------------------\n";
			std::cout << buffer << "\n";

			mtx.lock();
			//wav_in->new_envelope(std::string(buffer).substr(0, n_bytes_read));
			mtx.unlock();
		}
	}
}*/


int receive_till_zero(char* tmpbuf, int& numbytes) {
	int i = 0;
	while(true) {
		// Check if we have a complete message
		for ( ; i < numbytes; i++ ) {
			if (buffer[i] == '\0') {
				// \0 indicate end of message! so we are done
				return i + 1; // return length of message
			}
		}
		int n = recv(client_socket, buffer + numbytes, MAXDATASIZE - numbytes, 0);
		if (n == -1) {
			return -1; // operation failed!
		}
		numbytes += n;
	}
}


void remove_message_from_buffer( char* buf, int& numbytes, int msglen ) {
	memmove(buf, buf + msglen, numbytes - msglen);
	numbytes -= msglen;
}


void read_socket() {
	 while(true) {
		msglen = receive_till_zero(buffer, numbytes);
		if( msglen == -1 ) {
			std::cerr << "error\n";
		}

		std::cout << "Message received from server: " << buffer << "\n";
		remove_message_from_buffer(buffer, numbytes, msglen);
	}
}


void main_loop() {
	while (true) {
		mtx.lock();
		sleep(1);
		mtx.unlock();
	}
}

 
int main() {
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Erreur création socket");
		exit(EXIT_FAILURE);
	}
	
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
	int one = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
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

	numbytes = 0;
	
	std::thread t(read_socket);
	main_loop();

	return 0;
}
