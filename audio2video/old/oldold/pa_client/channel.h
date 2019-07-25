
#ifndef CHANNEL_H
#define CHANNEL_H

#include <sys/time.h>
#include <vector>

#include "constantes.h"
#include "config.h"



class Channel {
public:
	Channel();
	void update_triggered();
	
	struct timeval last_trigger;
	bool triggable;
};


#endif
