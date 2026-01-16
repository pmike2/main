
#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>


struct Test {
	int _x;
	std::vector<int> _v;
};


struct List {
	std::vector<Test> _tests;
};


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

	List l;

	Test t;

	t._x = 12;
	t._v.push_back(100);
	t._v.push_back(200);
	l._tests.push_back(t);

	t._x = 38;
	t._v.clear();
	t._v.push_back(1000);
	t._v.push_back(2000);
	l._tests.push_back(t);

	for (auto &t : l._tests) {
		std::cout << t._x << " / ";
		for (auto &x : t._v) {
			std::cout << x << " ; ";
		}
		std::cout << "\n";
	}

	return 0;
}
