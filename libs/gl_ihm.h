#ifndef GLIHM
#define GLIHM

#include <vector>
#include <map>
#include <iostream>
#include <string>

#include "json.hpp"


using json = nlohmann::json;


struct GLIHMElement {
	GLIHMElement();
	~GLIHMElement();


	
};


struct GLIHMButton : public GLIHMElement {
	GLIHMButton();
	~GLIHMButton();



};


struct GLIHMGroup {
	GLIHMGroup();
	~GLIHMGroup();


	std::string _name;

};


struct GLIHM {
	GLIHM();
	GLIHM(std::string json_path);
	~GLIHM();


	std::map<std::string, GLIHMGroup *> _groups;
};


#endif
