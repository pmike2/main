#ifndef CV_IN_H
#define CV_IN_H

#include <string>
#include <map>
#include <vector>

#include "json.hpp"

#include "shared_mem.h"


class CVIn : public Emitter {
public:
	CVIn();
	CVIn(std::string json_path);
	~CVIn();
	void load_json(std::string json_path);
	void load_json(nlohmann::json js);


	std::map<unsigned int, std::vector<unsigned int> > _mapping;
};


#endif
