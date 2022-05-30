#ifndef LOOPER_H
#define LOOPER_H

#include <chrono>
#include <string>
#include <utility>

#include "constantes.h"
#include "shared_mem.h"


enum SEQ_MODE {STOPPED, RUNNING, RECORDING};

typedef std::pair<unsigned int, unsigned int> ratio_type;

const bool VERBOSE= false;
const unsigned int N_MAX_EVENTS= 1024;
const std::chrono::duration<int,std::milli> DEFAULT_TRACK_DURATION= std::chrono::milliseconds(2048);
const std::chrono::duration<int,std::milli> DEFAULT_EVENT_DURATION= std::chrono::milliseconds(50);
const std::chrono::duration<int,std::milli> DEFAULT_EVENT_MARGIN= std::chrono::milliseconds(5);



class Event {
public:
	Event();
	~Event();
	void set(key_type key, time_type t, amplitude_type amplitude, Event * previous, Event * next);
	void set_end(time_type t);
	void set_null();
	bool is_null();
	friend std::ostream & operator << (std::ostream & os, const Event & e);

	sharedata_type _data;
	Event * _previous;
	Event * _next;
};


class Track {
public:
	Track();
	Track(sharedata_type * track_data, bool infinite=false);
	~Track();
	time_type get_relative_t(time_type t);
	unsigned int get_cycle(time_type t);
	void set_duration(time_type t);
	void set_ratio_to_master_track(Track * master_track, ratio_type ratio);
	void update_duration_from_ratio_to_master_track(Track * master_track);
	Event * get_first_null_event();
	Event * get_first_not_null_event();
	Event * get_last_event_before(time_type t);
	Event * get_first_event_after(time_type t);
	Event * get_event_at(time_type t);
	Event * get_first_event();
	Event * get_last_event();
	void insert_event(key_type key, time_type t, amplitude_type amplitude);
	void set_inserted_event_end(time_type t);
	void delete_event(Event * event);
	void update(time_type t);
	void set_quantize(unsigned int quantize);
	bool update_current_quantize(time_type t);
	void emit_current();
	void emit_null();
	void clear();
	int get_event_idx(Event * event);

	time_type _duration;
	ratio_type _ratio_to_master_track;
	Event * _events[N_MAX_EVENTS];
	Event * _current_event;
	Event * _inserted_event;
	unsigned int _current_cycle;
	Track * _previous;
	Track * _next;
	sharedata_type * _data;
	bool _infinite;
	unsigned int _quantize;
	time_type _quantize_step;
	unsigned int _current_quantize;
	bool _repeat;
	bool _hold;
};


class Sequence : public Emitter {
public:
	Sequence();
	~Sequence();
	time_type now();
	void note_on(key_type key, amplitude_type amplitude);
	void note_off();
	void update();
	void clear();
	void toggle_start();
	void toggle_record();
	void set_next_track();
	void set_previous_track();
	void set_master_track_duration(time_type t);
	unsigned int get_current_track_index();
	void debug();

	Track * _tracks[N_TRACKS];
	Track * _current_track;
	std::chrono::system_clock::time_point _start_point;
	SEQ_MODE _mode;
};


#endif
