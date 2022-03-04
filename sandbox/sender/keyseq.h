#ifndef KEYSEQ_H
#define KEYSEQ_H

#include <chrono>

typedef unsigned int key_type;

const unsigned int N_MAX_EVENTS= 1024;
const unsigned int N_MAX_TRACKS= 16;
const std::chrono::duration<int,std::milli> DEFAULT_TRACK_DURATION= std::chrono::milliseconds(1000);


class Event {
public:
	Event();
	~Event();
	void set(std::chrono::system_clock::duration t, Event * previous, Event * next, key_type key);
	void set_null();

	std::chrono::system_clock::duration _t;
	Event * _previous;
	Event * _next;
	key_type _key;
};


class Track {
public:
	Track();
	~Track();
	std::chrono::system_clock::duration get_relative_t(std::chrono::system_clock::duration t);
	unsigned int get_cycle(std::chrono::system_clock::duration t);
	void set_duration(std::chrono::system_clock::duration duration);
	Event * get_first_null_event();
	Event * get_first_not_null_event();
	Event * get_last_event_before(std::chrono::system_clock::duration t);
	Event * get_first_event_after(std::chrono::system_clock::duration t);
	Event * get_first_event();
	void insert_event(key_type key, std::chrono::system_clock::duration t);
	void delete_event(Event * event);
	void update(std::chrono::system_clock::duration t);

	std::chrono::system_clock::duration _duration;
	Event * _events[N_MAX_EVENTS];
	Event * _last_event;
	unsigned int _current_cycle;
};


class Sequence {
public:
	Sequence();
	~Sequence();
	std::chrono::system_clock::duration now();
	void insert_event(key_type key);
	void update();
	void set_track(Track * track);

	Track * _tracks[N_MAX_TRACKS];
	Track * _current_track;
	std::chrono::system_clock::time_point _start_point;
};

#endif
