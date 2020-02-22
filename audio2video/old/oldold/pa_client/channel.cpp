
#include "channel.h"
#include "utile.h"

using namespace std;


Channel::Channel() {
	triggable= true;
	gettimeofday(&last_trigger, NULL);
}


void Channel::update_triggered() {
	if (triggable)
		return;
	
	unsigned int diff_ms= diff_time_ms_from_now(&last_trigger);
	if (diff_ms> MAX_TRIGGER_DURATION) {
		triggable= true;
	}
}

