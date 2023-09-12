#ifndef MATHEMATICS_H
#define MATHEMATICS_H

#include <vector>
#include <iostream>

#include <glm/glm.hpp>


class Polynomial {
public:
	Polynomial();
	Polynomial(std::vector<float> coeffs);
	Polynomial(std::vector<glm::vec2> points);
	~Polynomial();
	float operator[](unsigned int idx) const;
	float valueat(float x);
	friend std::ostream & operator << (std::ostream & os, Polynomial & p);


	std::vector<float> _coeffs;
};

#endif
