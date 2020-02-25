
#include "config.h"

using namespace std;


// --------------------------------------------------------------------------------------------
ActionConfig::ActionConfig() {
	active= true;
	type= "";
	idx= -1;
	target= "";
	target_idx= -1;
	mult_offset= 1.;
	add_offset= 0.;
}


void ActionConfig::randomize() {
}


void ActionConfig::defaultize() {
}


// --------------------------------------------------------------------------------------------
ChannelConfig::ChannelConfig() {
}


void ChannelConfig::randomize() {
	release();
	unsigned int n_actions= rand_int(1, 4);
	unsigned int i;
	unsigned int idx_type;
	for (i=0; i< n_actions; i++) {
		idx_type= rand_int(0, 2);
		if (idx_type== 0) {
		}
		else if (idx_type== 1) {
		}
		else if (idx_type== 2) {
		}
	}
}


void ChannelConfig::release() {
	unsigned int i;
	for (i=0; i< actions.size(); i++) {
		delete actions[i];
	}
	actions.clear();
}

