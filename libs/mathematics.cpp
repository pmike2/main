#include <iostream>
#include <math.h>

#include "mathematics.h"

using namespace std;


Polynomial::Polynomial() {

}


Polynomial::Polynomial(vector<float> coeffs) : _coeffs(coeffs) {

}


Polynomial::Polynomial(vector<glm::vec2> points) {
	
}


Polynomial::~Polynomial() {

}


float Polynomial::operator[](unsigned int idx) const {
	if (idx>= _coeffs.size()) {
		cerr << "Polynomial idx =" << idx << " >= coeffs.size = " << _coeffs.size() << "\n";
	}
	return _coeffs[idx];
}


float Polynomial::valueat(float x) {
	float res= 0.0;
	for (unsigned int idx=0; idx<_coeffs.size(); ++idx) {
		res+= pow(x, idx)* _coeffs[idx];
	}
	return res;
}


ostream & operator << (ostream & os, Polynomial & p) {
	os << "size=" << p._coeffs.size() << "\n";
	os << "coeffs= ";
	for (auto & it : p._coeffs) {
		os << it << " ; ";
	}
	os << "\n";
	
	return os;
}
