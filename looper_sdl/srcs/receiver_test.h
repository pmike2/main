#ifndef RECEIVER_TEST_H
#define RECEIVER_TEST_H

#include "looper.h"


const unsigned int N_DEBUG= 1000;

class ReceiverTest : public Receiver {
public:
	ReceiverTest();
	~ReceiverTest();
	void note_on(unsigned int idx_track);
	void note_off(unsigned int idx_track);

	time_type _debug[N_DEBUG];
	unsigned int _compt_debug;
	std::chrono::system_clock::time_point _debug_start_point;
};

#endif
