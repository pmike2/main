#ifndef RECEIVER_TEST_H
#define RECEIVER_TEST_H

#include <chrono>

#include "shared_mem.h"



class ReceiverTest : public Receiver {
public:
	ReceiverTest();
	~ReceiverTest();
	void note_on(unsigned int idx_track);
	void note_off(unsigned int idx_track);
};

#endif
