#ifndef SOCKET_IO_H
#define SOCKET_IO_H

#include <thread>

#include "json.hpp"

class SocketIOUtil {
	SocketIOUtil();
	~SocketIOUtil();
	bool sleep_complete();

	mutex _mtx;
	json _js_config;
	bool _reload;
	std::chrono::system_clock::time_point _last_modified;
};

#endif
