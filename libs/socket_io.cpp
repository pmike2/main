	socket_io_struct() : _reload(false), _last_modified(chrono::system_clock::now()) {}
	bool sleep_complete() {
		chrono::system_clock::time_point now= chrono::system_clock::now();
		chrono::system_clock::duration d= now- _last_modified;
		unsigned int ms= chrono::duration_cast<chrono::milliseconds>(d).count();
		if (ms> 2000) {
			return true;
		}
		return false;
	}
