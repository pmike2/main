#include <iostream>
#include <algorithm>

#include <glm/gtx/string_cast.hpp>

#include "utile.h"

#include "geom.h"


// Intersection de 2 triangles en 3D
// http://web.stanford.edu/class/cs277/resources/papers/Moller1997b.pdf
// ATTENTION : reste à gérer le cas ou les triangles sont coplanaires !
bool triangle_intersects_triangle(pt_type_3d v[3], pt_type_3d w[3]) {
	// -------
	pt_type_3d nv= glm::cross(v[1]- v[0], v[2]- v[0]);
	number dv= -1.0f* glm::dot(nv, v[0]);
	pt_type_3d dvw;
	dvw[0]= glm::dot(nv, w[0])+ dv;
	dvw[1]= glm::dot(nv, w[1])+ dv;
	dvw[2]= glm::dot(nv, w[2])+ dv;
	if ((dvw[0]* dvw[1]> 0.0f) && (dvw[0]* dvw[2]> 0.0f)) {
		return false;
	}

	// -------
	pt_type_3d nw= glm::cross(w[1]- w[0], w[2]- w[0]);
	number dw= -1.0f* glm::dot(nw, w[0]);
	pt_type_3d dwv;
	dwv[0]= glm::dot(nw, v[0])+ dw;
	dwv[1]= glm::dot(nw, v[1])+ dw;
	dwv[2]= glm::dot(nw, v[2])+ dw;
	if ((dwv[0]* dwv[1]> 0.0f) && (dwv[0]* dwv[2]> 0.0f)) {
		return false;
	}

	// -------
	//pt_type_3d d= glm::normalize(glm::cross(nv, nw));
	pt_type_3d d= glm::cross(nv, nw);

	// -------
	uint v_idx_alone, v_idx_couple_1, v_idx_couple_2;
	if (dwv[0]* dwv[1]< 0.0f) {
		if (dwv[0]* dwv[2]< 0.0f) {
			v_idx_alone= 0;
			v_idx_couple_1= 1;
			v_idx_couple_2= 2;
		}
		else {
			v_idx_alone= 1;
			v_idx_couple_1= 0;
			v_idx_couple_2= 2;
		}
	}
	else {
		v_idx_alone= 2;
		v_idx_couple_1= 0;
		v_idx_couple_2= 1;
	}

	pt_type_3d v_dot_d;
	v_dot_d[0]= glm::dot(d, v[0]);
	v_dot_d[1]= glm::dot(d, v[1]);
	v_dot_d[2]= glm::dot(d, v[2]);

	number tv1= v_dot_d[v_idx_couple_1]+ (v_dot_d[v_idx_alone]- v_dot_d[v_idx_couple_1])* dwv[v_idx_couple_1]/ (dwv[v_idx_couple_1]- dwv[v_idx_alone]);
	number tv2= v_dot_d[v_idx_couple_2]+ (v_dot_d[v_idx_alone]- v_dot_d[v_idx_couple_2])* dwv[v_idx_couple_2]/ (dwv[v_idx_couple_2]- dwv[v_idx_alone]);

	// -------
	uint w_idx_alone, w_idx_couple_1, w_idx_couple_2;
	if (dvw[0]* dvw[1]< 0.0f) {
		if (dvw[0]* dvw[2]< 0.0f) {
			w_idx_alone= 0;
			w_idx_couple_1= 1;
			w_idx_couple_2= 2;
		}
		else {
			w_idx_alone= 1;
			w_idx_couple_1= 0;
			w_idx_couple_2= 2;
		}
	}
	else {
		w_idx_alone= 2;
		w_idx_couple_1= 0;
		w_idx_couple_2= 1;
	}

	pt_type_3d w_dot_d;
	w_dot_d[0]= glm::dot(d, w[0]);
	w_dot_d[1]= glm::dot(d, w[1]);
	w_dot_d[2]= glm::dot(d, w[2]);

	number tw1= w_dot_d[w_idx_couple_1]+ (w_dot_d[w_idx_alone]- w_dot_d[w_idx_couple_1])* dvw[w_idx_couple_1]/ (dvw[w_idx_couple_1]- dvw[w_idx_alone]);
	number tw2= w_dot_d[w_idx_couple_2]+ (w_dot_d[w_idx_alone]- w_dot_d[w_idx_couple_2])* dvw[w_idx_couple_2]/ (dvw[w_idx_couple_2]- dvw[w_idx_alone]);

	// -------
	/*cout << "v0=" << glm::to_string(v[0]) << " ; v1=" << glm::to_string(v[1]) << "v2=" << glm::to_string(v[2]) << "\n";
	cout << "w0=" << glm::to_string(w[0]) << " ; w1=" << glm::to_string(w[1]) << "w2=" << glm::to_string(w[2]) << "\n";
	cout << "nv=" << glm::to_string(nv) << "\n";
	cout << "nw=" << glm::to_string(nw) << "\n";
	cout << "d=" << glm::to_string(d) << "\n";
	cout << "v_idx_alone=" << v_idx_alone << " ; v_idx_couple_1=" << v_idx_couple_1 << " ; v_idx_couple_2=" << v_idx_couple_2 << "\n";
	cout << "w_idx_alone=" << w_idx_alone << " ; w_idx_couple_1=" << w_idx_couple_1 << " ; w_idx_couple_2=" << w_idx_couple_2 << "\n";
	cout << "v_dot_d[v_idx_alone]=" << v_dot_d[v_idx_alone] << " ; v_dot_d[v_idx_couple_1]=" << v_dot_d[v_idx_couple_1] << " ; v_dot_d[v_idx_couple_2]=" << v_dot_d[v_idx_couple_2] << "\n";
	cout << "w_dot_d[w_idx_alone]=" << w_dot_d[w_idx_alone] << " ; w_dot_d[w_idx_couple_1]=" << w_dot_d[w_idx_couple_1] << " ; w_dot_d[w_idx_couple_2]=" << w_dot_d[w_idx_couple_2] << "\n";
	cout << "tv1=" << tv1 << " ; tv2=" << tv2 << " ; tw1=" << tw1 << " ; tw2=" << tw2 << "\n";*/

	if ((std::max(tv1, tv2)< std::min(tw1, tw2)) || (std::min(tv1, tv2)> std::max(tw1, tw2))) {
		return false;
	}

	return true;
}


// ----------------------------------
ConvexHull::ConvexHull() {
	_dcel = new DCEL();
}


ConvexHull::~ConvexHull() {
	_conflict.clear();
	delete _dcel;
}


void ConvexHull::randomize(uint n_pts, number xmin, number xmax, number ymin, number ymax, number zmin, number zmax) {
	_pts.clear();
	for (uint i=0; i<n_pts; ++i) {
		_pts.push_back(rand_pt_3d(xmin, xmax, ymin, ymax, zmin, zmax));
	}
}


void ConvexHull::compute() {
	_dcel->clear();
}

