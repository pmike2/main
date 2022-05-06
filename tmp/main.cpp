/*
Partie C++
*/

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "json.hpp"


using namespace std;
using json = nlohmann::json;

// init de js_config
json js_config;



// GET et mise à jour de js_config
void get() {
	string url= "http://localhost:3000/config";
	string curl_out= "curl_tmp.txt";
	string cmd= "curl -s "+ url+ " > "+ curl_out;
	int status= system(cmd.c_str());
	/*cout << status << "\n";
	cout << ifstream(curl_out).rdbuf();*/

	ifstream istr(curl_out);
	istr >> js_config;
	
	cout << "js_config vaut maintenant : " << js_config.dump() << "\n";
}


// POST de la valeur actuelle de js_config
/*void post() {
	string url= "http://localhost:3000/config";
	string cmd= "curl -X POST "+ url+ " -H 'Content-Type: application/json' -d '"+ js_config.dump()+ "'";
	//cout << cmd << "\n";
	int status= system(cmd.c_str());
	//cout << status << "\n";
}*/


// point d'entrée -----------------------------------------------------------------
int main(int argc, char **argv) {
	/*string json_path= "/Volumes/Data/perso/dev/main/ffmpeg_mpeg2sdl/data/config_01.json";
	ifstream istr(json_path);
	istr >> js_config;*/

	/*if (string(argv[1])== "get") {
		get();
	}
	else if (string(argv[1])== "post") {
		post();
	}
	else {
		cout << "arg doit valoir get ou post\n";
	}*/

	get();
	
	return 0;
}
