#ifndef SIO_UTIL_H
#define SIO_UTIL_H

#include <chrono>
#include <thread>
#include <string>

#include "sio_client.h"


typedef void (* sio_callback)(sio::event & e);


class SocketIOUtil {
public:
	SocketIOUtil();
	SocketIOUtil(std::string url, std::string msg_id, uint sleep_duration);
	~SocketIOUtil();
	bool sleep_complete();
	void on_msg(sio::event & e);
	bool update();

	std::string _url;
	std::string _msg_id;
	uint _sleep_duration;
	std::mutex _mtx;
	std::string _last_msg;
	bool _reload;
	std::chrono::system_clock::time_point _last_modified;
	sio::client _io;
};


#endif
