#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>

#include <glm/gtx/transform.hpp>

#include "typedefs.h"
#include "bbox.h"
#include "bbox_2d.h"
#include "geom.h"
#include "geom_2d.h"
#include "utile.h"


int main() {
	/*BBox * bbox = new BBox(pt_3d(-1.0), pt_3d(1.0));
	std::vector<pt_3d> norms = bbox->normals();
	for (auto & n : norms) {
		std::cout << glm_to_string(n) << "\n";
	}
	delete bbox;*/

	//quat const Q = glm::identity<quat>();

	quat q = glm::angleAxis(M_PI * 0.5, pt_3d(0.0, 0.0, 1.0));
	pt_3d euler = glm::eulerAngles(q);
	std::cout << glm_to_string(euler) << "\n";


	return 0;
}
