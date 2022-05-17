/*
Test de lecture de la config video actuelle
*/

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "json.hpp"


using namespace std;
using json = nlohmann::json;

json js_config;



void get() {
	string url= "http://localhost:3000/config";
	string curl_out= "curl_tmp.txt";
	// s : silent ; 
	// --noproxy '*' : pour ignorer http_proxy
	string cmd= "curl -s --noproxy '*' "+ url+ " > "+ curl_out;
	int status= system(cmd.c_str());
	/*cout << status << "\n";
	cout << ifstream(curl_out).rdbuf();*/

	ifstream istr(curl_out);
	istr >> js_config;
	
	cout << "js_config vaut maintenant : " << js_config.dump(4) << "\n";

	string clean_cmd= "rm "+ curl_out;
	system(clean_cmd.c_str());
}


/*void post() {
	string url= "http://localhost:3000/config";
	string cmd= "curl -X POST "+ url+ " -H 'Content-Type: application/json' -d '"+ js_config.dump()+ "'";
	//cout << cmd << "\n";
	int status= system(cmd.c_str());
	//cout << status << "\n";
}*/


// point d'entrée -----------------------------------------------------------------
int main(int argc, char **argv) {
	get();
	
	return 0;
}
