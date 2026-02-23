#include "socket_util.h"


SocketUtil::SocketUtil() {
	if ((_server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		std::cerr << "Erreur création socket\n";
		return;
	}
	
    _server_address.sin_family = AF_INET;
    _server_address.sin_port = htons(8080);
    _server_address.sin_addr.s_addr = INADDR_ANY;
	int one = 1;
	setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    
    if (bind(_server_socket, (struct sockaddr*) & _server_address, sizeof(_server_address)) < 0) {
		std::cerr <<"bind failed\n";
		close(_server_socket);
    	return;
	}
    
	if (listen(_server_socket, 10) < 0) {
		std::cerr << "listen failed\n";
		close(_server_socket);
    	return;
	}
	
	int addrlen = sizeof(_server_address);
    _client_socket = accept(_server_socket, (struct sockaddr *) & _server_address, (socklen_t *) & addrlen);
	if (_client_socket < 0) {
		std::cerr << "accept failed\n";
    	return;
	}

	_numbytes = 0;
    _buffer = new char[MAXDATASIZE];

	_t= std::thread(&SocketUtil::read_socket, this);
}


SocketUtil::~SocketUtil() {
    delete _buffer;
	_t.join();
}


int SocketUtil::receive_till_zero() {
	int i = 0;
	while(true) {
		// Check if we have a complete message
		for ( ; i < _numbytes; i++ ) {
			if (_buffer[i] == '\0') {
				// \0 indicate end of message! so we are done
				return i + 1; // return length of message
			}
		}
		int n = recv(_client_socket, _buffer + _numbytes, MAXDATASIZE - _numbytes, 0);
		if (n == -1) {
			return -1; // operation failed!
		}
		_numbytes += n;
	}
}


void SocketUtil::remove_message_from_buffer() {
	memmove(_buffer, _buffer + _msglen, _numbytes - _msglen);
	_numbytes -= _msglen;
}


void SocketUtil::read_socket() {
	bool verbose = false;

	while(true) {
		_msglen = receive_till_zero();
		if (_msglen == -1) {
			std::cerr << "error\n";
		}

		if (verbose) {
			std::cout << "Message received from server: " << _buffer << "\n";
		}
		_msgs.push(std::string(_buffer));
		remove_message_from_buffer();
	}
}
