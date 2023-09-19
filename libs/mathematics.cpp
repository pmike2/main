#include <iostream>
#include <math.h>

#include <glm/gtc/type_ptr.hpp>

#include "mathematics.h"

using namespace std;


glm::vec3 sum_over_e(glm::vec3* e, glm::vec3* e_prime, int& i) {
	int k = 0;
	glm::vec3 result;
	while (k < i) {
		float e_prime_k_squared = glm::dot(e_prime[k], e_prime[k]);
		result += (glm::dot(e[i], e_prime[k]) / e_prime_k_squared) * e_prime[k];
		k++;
	}
	return result;
}


void gram_schmidt(float * mat) {
	glm::vec3 e[] = {
		glm::vec3(mat[0], mat[1], mat[2]),
		glm::vec3(mat[4], mat[5], mat[6]),
		glm::vec3(mat[8], mat[9], mat[10])
	};
	glm::vec3 e_prime[3];
	e_prime[0]= e[0];
	int i= 0;

	do {
		e_prime[i]= e[i]- sum_over_e(e, e_prime, i);
		i++;
	} while (i< 3);
	
	for (i=0; i<3; i++)
		e_prime[i]= glm::normalize(e_prime[i]);
	
	float * pt;
	pt= glm::value_ptr(e_prime[0]);
	memcpy(mat, pt, sizeof(float) * 3);
	pt= glm::value_ptr(e_prime[1]);
	memcpy(mat+ 4, pt, sizeof(float) * 3);
	pt= glm::value_ptr(e_prime[2]);
	memcpy(mat+ 8, pt, sizeof(float) * 3);
}



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
