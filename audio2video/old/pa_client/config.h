
#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <string>
#include <sstream>
#include <stack>
#include <iostream>
#include <fstream>

#include "json.hpp"

#include "constantes.h"
#include "utile.h"



// ------------------------------------------------------------------------------------------
class ActionConfig {
public:
	ActionConfig();
	void randomize();
	void defaultize();
	
	bool active;
	std::string type;
	unsigned int idx;
	std::string target;
	unsigned int target_idx;
	float mult_offset, add_offset;
	
};



// ------------------------------------------------------------------------------------------
class ChannelConfig {
public:
	ChannelConfig();
	void randomize();
	void release();
	
	std::vector<ActionConfig *> actions;
};


#endif
