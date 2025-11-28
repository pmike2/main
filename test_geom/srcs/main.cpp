
#include <iostream>

#include "utile.h"
#include "typedefs.h"

#include "geom.h"


int main() {

	ConvexHull * hull = new ConvexHull();
	
	//hull->randomize(10, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0);

	hull->add_pt(4.0, 10.0, 1.0);
	hull->add_pt(3.0, 8.0, 3.0);
	hull->add_pt(2.0, 1.0, 8.0);
	hull->add_pt(9.0, 7.0, 2.0);
	hull->add_pt(2.0, 1.7, 6.0);
	hull->add_pt(9.0, 4.0, 10.0);

	hull->compute();

	delete hull;

	return 0;
}
