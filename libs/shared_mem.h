#ifndef SHARED_MEM_H
#define SHARED_MEM_H

#include <iostream>
#include <chrono>


typedef std::chrono::system_clock::duration time_type;
typedef unsigned int key_type;
typedef float amplitude_type;

struct sharedata_type {
	friend bool operator== (const sharedata_type & x, const sharedata_type & y);
	friend bool operator!= (const sharedata_type & x, const sharedata_type & y);
	friend std::ostream & operator << (std::ostream & os, const sharedata_type & e);
	sharedata_type & operator=(const sharedata_type & other);
	void set_null();
	bool is_null();

	key_type _key;
	amplitude_type _amplitude;
	time_type _t_start;
	time_type _t_end;
};


const bool DEBUG= false;
const unsigned int N_DEBUG= 1000;
const unsigned int N_MAX_TRACKS= 8;
const unsigned int DATA_SIZE= sizeof(sharedata_type)* N_MAX_TRACKS;
const std::string SHARED_MEM_OBJ_NAME= "/shmem-looper";
const key_type NULL_KEY= 0; 
const amplitude_type NULL_AMPLITUDE= 0.0f;


unsigned int time_ms(time_type t);
std::string time_print(time_type t);


class Emitter {
public:
	Emitter();
	~Emitter();

	sharedata_type * _data;
	int _fd;
};


class Receiver {
public:
	Receiver();
	~Receiver();
	void init_data();
	void close_data();
	void update();
	virtual void note_on(unsigned int idx_track) = 0;
	virtual void note_off(unsigned int idx_track) = 0;

	sharedata_type * _data;
	sharedata_type * _data_current;
	int _fd;

	time_type _debug[N_DEBUG];
	unsigned int _compt_debug;
	std::chrono::system_clock::time_point _debug_start_point;
	std::string _debug_path;
};

#endif
