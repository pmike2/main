#ifndef RECEIVER_TEST_H
#define RECEIVER_TEST_H

#include "keyseq.h"


class ReceiverTest: public Receiver {
public:
	ReceiverTest();
	~ReceiverTest();
	void on_new_data(sharedata_type data);

};

#endif
