#ifndef SOCKET_UTIL_H
#define SOCKET_UTIL_H

#include <iostream>
#include <string>
#include <queue>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>

#include "thread.h"


const int MAXDATASIZE = 3000;


struct SocketUtil {
    SocketUtil();
    ~SocketUtil();
    int receive_till_zero();
    void remove_message_from_buffer();
    void read_socket();


    SafeQueue<std::string> _msgs;
    int _client_socket;
    int _server_socket;
    sockaddr_in _server_address;
    std::mutex _mtx;
    char * _buffer;
    int _numbytes, _msglen;
    std::thread _t;
};


#endif
