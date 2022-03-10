#ifndef LOOPER_H
#define LOOPER_H

#include <chrono>
#include <string>

enum SEQ_MODE {STOPPED, RUNNING, RECORDING};

typedef std::chrono::system_clock::duration time_type;
typedef unsigned int key_type;

struct sharedata_type {
	friend bool operator== (const sharedata_type & x, const sharedata_type & y);
	friend bool operator!= (const sharedata_type & x, const sharedata_type & y);
	friend std::ostream & operator << (std::ostream & os, const sharedata_type & e);
	sharedata_type & operator=(const sharedata_type & other);
	void set_null();
	bool is_null();

	key_type _key;
	time_type _t_start;
	time_type _t_end;
	unsigned int _amplitude;
};

const unsigned int N_MAX_EVENTS= 1024;
const unsigned int N_MAX_TRACKS= 16;
const key_type NULL_KEY= 0; 
const std::chrono::duration<int,std::milli> DEFAULT_TRACK_DURATION= std::chrono::milliseconds(1000);
const std::chrono::duration<int,std::milli> DEFAULT_EVENT_DURATION= std::chrono::milliseconds(50);
const unsigned int DATA_SIZE= sizeof(sharedata_type)* N_MAX_TRACKS;
const std::string SHARED_MEM_OBJ_NAME= "/shmem-looper";


unsigned int time_ms(time_type t);
std::string time_print(time_type t);


class Event {
public:
	Event();
	~Event();
	void set(key_type key, time_type t, unsigned int amplitude, Event * previous, Event * next, bool hold);
	void set_end(time_type t);
	void set_null();
	bool is_null();
	friend std::ostream & operator << (std::ostream & os, const Event & e);

	sharedata_type _data;
	Event * _previous;
	Event * _next;
	bool _hold;
};


class Track {
public:
	Track();
	~Track();
	time_type get_relative_t(time_type t);
	unsigned int get_cycle(time_type t);
	void set_duration(time_type t);
	Event * get_first_null_event();
	Event * get_first_not_null_event();
	Event * get_last_event_before(time_type t);
	Event * get_first_event_after(time_type t);
	Event * get_first_event();
	Event * get_last_event();
	Event * insert_event(key_type key, time_type t, unsigned int amplitude, bool hold);
	void delete_event(Event * event);
	void update(time_type t);
	void clear();

	time_type _duration;
	Event * _events[N_MAX_EVENTS];
	Event * _last_event;
	unsigned int _current_cycle;
	Track * _previous;
	Track * _next;
};


class Sequence {
public:
	Sequence();
	~Sequence();
	time_type now();
	Event * insert_event(key_type key, unsigned int amplitude, bool hold);
	void set_event_end(Event * event);
	void emit_track(unsigned int idx_track);
	void update();
	void clear();
	void toggle_start();
	void toggle_record();
	void set_next_track();
	void set_previous_track();
	void set_current_track_duration(time_type t);
	void set_current_track_duration_ratio(float ratio);
	void init_data2send();
	void close_data2send();
	void debug();

	Track * _tracks[N_MAX_TRACKS];
	Track * _current_track;
	std::chrono::system_clock::time_point _start_point;
	sharedata_type * _data2send;
	int _fd;
	SEQ_MODE _mode;
};


class Receiver {
public:
	Receiver();
	~Receiver();
	void init_data2receive();
	void close_data2receive();
	void update();
	virtual void on_new_data(sharedata_type data) = 0;

	sharedata_type * _data2receive;
	sharedata_type * _data2receive_current;
	int _fd;
};


#endif
