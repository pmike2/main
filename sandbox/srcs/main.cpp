
#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>


int main() {
	/*std::string s = "12";
	std::stringstream ss(s);
	std::string s2;
	std::vector<std::string> v;
	while(std::getline(ss, s2, '/')) {
		v.push_back(s2);
	}

	std::cout << v.size() << "\n";
	for (auto & x : v) {
		std::cout << x << " ; ";
	}
	std::cout << "\n";*/

	std::string s2 = "aaa:45:instanced";
	std::string s;
	std::string s3;
	std::stringstream ss(s2);
	std::getline(ss, s, ':');
	std::cout << s << "\n";
	std::getline(ss, s, ':');
	std::cout << s << "\n";
	std::getline(ss, s3, ':');
	std::cout << s3 << "\n";
	if (s3 == "instanced") {
		std::cout << "ok\n";
	}

	return 0;
}
