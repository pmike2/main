
#include <iostream>

#include "threebody.h"


int main() {
	ThreeBody * threebody= new ThreeBody();
	threebody->add_body(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f);
	threebody->add_body(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f);
	threebody->add_body(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f);
	for (unsigned int idx=0; idx<100; ++idx) {
		threebody->anim(1.0f);
		threebody->print();
	}
	delete threebody;

	return 0;
}
