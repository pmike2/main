
#include <iostream>

#include "utile.h"
#include "typedefs.h"

#include "geom.h"


int main() {

	ConvexHull * hull = new ConvexHull();
	
	hull->randomize(10, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0);

	/*hull->add_pt(0.0, 0.0, 0.0);
	hull->add_pt(1.0, 0.0, 0.0);
	hull->add_pt(0.0, 1.0, 0.0);
	hull->add_pt(0.0, 0.0, 1.0);
	hull->add_pt(-1.0, 0.0, 0.0);*/

	hull->compute();

	delete hull;

	return 0;
}
