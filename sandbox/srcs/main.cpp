

#include <iostream>
#include <string>
#include <vector>

#include "mathematics.h"

using namespace std;



int main() {
	vector<float> coeffs{0.1, 4.3, -8.1};
	Polynomial p(coeffs);
	cout << p;
	cout << p[2] << "\n";
	cout << p.valueat(2.0) << "\n";
	
	return 0;
}