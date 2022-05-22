#include <fstream>

#include "cv_in.h"


using namespace std;
using json = nlohmann::json;


CVIn::CVIn() {

}


CVIn::CVIn(string json_path) {

}

CVIn::~CVIn() {
	
}


void CVIn::load_json(string json_path) {
	ifstream istr(json_path);
	json js;
	istr >> js;
	load_json(js);
}


void CVIn::load_json(json js) {

}
