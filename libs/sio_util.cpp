#include <functional>

#include "sio_util.h"


using namespace std;


SocketIOUtil::SocketIOUtil() {

}


SocketIOUtil::SocketIOUtil(string url, string msg_id, uint sleep_duration) :
	_url(url), _msg_id(msg_id), _sleep_duration(sleep_duration), _reload(false), _last_modified(chrono::system_clock::now()), _last_msg("")
{
	// a désactiver si on veux avoir des infos
	_io.set_logs_quiet();
	// sera exécuté lorsque connect sera établi
	_io.set_open_listener([&]() {
		_io.socket()->on(_msg_id, bind(&SocketIOUtil::on_msg, this, placeholders::_1));
	});
	_io.connect(_url);
}


SocketIOUtil::~SocketIOUtil() {

}


bool SocketIOUtil::sleep_complete() {
	chrono::system_clock::time_point now= chrono::system_clock::now();
	chrono::system_clock::duration d= now- _last_modified;
	uint ms= chrono::duration_cast<chrono::milliseconds>(d).count();
	if (ms> _sleep_duration) {
		return true;
	}
	return false;
}


void SocketIOUtil::on_msg(sio::event & e) {
	// ici on est pas dans le thread principal -> mutex
	_last_msg= e.get_message()->get_string();
	_mtx.lock();
	_reload= true;
	_last_modified= chrono::system_clock::now();
	_mtx.unlock();
}


bool SocketIOUtil::update() {
	bool result= false;
	_mtx.lock();
	if ((_reload) && (sleep_complete())) {
		_reload= false;
		result= true;
	}
	_mtx.unlock();
	return result;
}

