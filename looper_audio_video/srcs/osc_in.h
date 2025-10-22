#ifndef OSC_IN_H
#define OSC_IN_H

#include <string>
#include <map>
#include <vector>
#include <iostream>

#include "json.hpp"

#include "shared_mem.h"


class OSCIn : public Emitter {
public:
	OSCIn();
	OSCIn(std::string json_path, unsigned int n_input_channels);
	~OSCIn();
};

#endif
