#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <thread>

#include <glm/gtx/transform.hpp>

#include "typedefs.h"
#include "bbox.h"


int main() {
	pt_3d vmin(0.0, 0.0, 0.0);
	pt_3d vmax(2.0, 1.0, 100.0);
	//pt_3d vmax(1.0, 1.0, 1.0);
	mat_4d model2world = glm::rotate(M_PI * 0.25, pt_3d(0.0, 0.0, 1.0));
	//mat_4d model2world(1.0);
	BBox * bbox = new BBox(vmin, vmax, model2world);
	BBox_2D * bbox_2d = bbox->bbox2d();
	std::cout << *bbox_2d << "\n";

	return 0;
}
