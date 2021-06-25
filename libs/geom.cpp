#include <iostream>
#include <algorithm>

#include <glm/gtx/string_cast.hpp>

#include "geom.h"

using namespace std;


// http://web.stanford.edu/class/cs277/resources/papers/Moller1997b.pdf
bool triangle_intersects_triangle(glm::vec3 v[3], glm::vec3 w[3]) {
	// -------
	glm::vec3 nv= glm::cross(v[1]- v[0], v[2]- v[0]);
	float dv= -1.0f* glm::dot(nv, v[0]);
	float dvw[3];
	dvw[0]= glm::dot(nv, w[0])+ dv;
	dvw[1]= glm::dot(nv, w[1])+ dv;
	dvw[2]= glm::dot(nv, w[2])+ dv;
	if ((dvw[0]* dvw[1]> 0.0f) && (dvw[0]* dvw[2]> 0.0f)) {
		return false;
	}

	// -------
	glm::vec3 nw= glm::cross(w[1]- w[0], w[2]- w[0]);
	float dw= -1.0f* glm::dot(nw, w[0]);
	float dwv[3];
	dwv[0]= glm::dot(nw, v[0])+ dw;
	dwv[1]= glm::dot(nw, v[1])+ dw;
	dwv[2]= glm::dot(nw, v[2])+ dw;
	if ((dwv[0]* dwv[1]> 0.0f) && (dwv[0]* dwv[2]> 0.0f)) {
		return false;
	}

	// -------
	glm::vec3 d= glm::normalize(glm::cross(nv, nw));

	// -------
	unsigned int v_idx_alone, v_idx_couple_1, v_idx_couple_2;
	if (dvw[0]* dvw[1]< 0.0f) {
		if (dvw[0]* dvw[2]< 0.0f) {
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

	float v_dot_d[3];
	v_dot_d[0]= glm::dot(d, v[0]);
	v_dot_d[1]= glm::dot(d, v[1]);
	v_dot_d[2]= glm::dot(d, v[2]);

	float tv1= v_dot_d[v_idx_couple_1]+ (v_dot_d[v_idx_alone]- v_dot_d[v_idx_couple_1])* dvw[v_idx_couple_1]/ (dvw[v_idx_couple_1]- dvw[v_idx_alone]);
	float tv2= v_dot_d[v_idx_couple_2]+ (v_dot_d[v_idx_alone]- v_dot_d[v_idx_couple_2])* dvw[v_idx_couple_2]/ (dvw[v_idx_couple_2]- dvw[v_idx_alone]);

	// -------
	unsigned int w_idx_alone, w_idx_couple_1, w_idx_couple_2;
	if (dwv[0]* dwv[1]< 0.0f) {
		if (dwv[0]* dwv[2]< 0.0f) {
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

	float w_dot_d[3];
	w_dot_d[0]= glm::dot(d, w[0]);
	w_dot_d[1]= glm::dot(d, w[1]);
	w_dot_d[2]= glm::dot(d, w[2]);

	float tw1= w_dot_d[w_idx_couple_1]+ (w_dot_d[w_idx_alone]- w_dot_d[w_idx_couple_1])* dwv[w_idx_couple_1]/ (dwv[w_idx_couple_1]- dwv[w_idx_alone]);
	float tw2= w_dot_d[w_idx_couple_2]+ (w_dot_d[w_idx_alone]- w_dot_d[w_idx_couple_2])* dwv[w_idx_couple_2]/ (dwv[w_idx_couple_2]- dwv[w_idx_alone]);

	// -------
	cout << "v0=" << glm::to_string(v[0]) << " ; v1=" << glm::to_string(v[1]) << "v2=" << glm::to_string(v[2]) << "\n";
	cout << "w0=" << glm::to_string(w[0]) << " ; w1=" << glm::to_string(w[1]) << "w2=" << glm::to_string(w[2]) << "\n";
	cout << "nv=" << glm::to_string(nv) << "\n";
	cout << "nw=" << glm::to_string(nw) << "\n";
	cout << "d=" << glm::to_string(d) << "\n";
	cout << "v_idx_alone=" << v_idx_alone << " ; v_idx_couple_1=" << v_idx_couple_1 << " ; v_idx_couple_2=" << v_idx_couple_2 << "\n";
	cout << "w_idx_alone=" << w_idx_alone << " ; w_idx_couple_1=" << w_idx_couple_1 << " ; w_idx_couple_2=" << w_idx_couple_2 << "\n";
	cout << "v_dot_d[v_idx_alone]=" << v_dot_d[v_idx_alone] << " ; v_dot_d[v_idx_couple_1]=" << v_dot_d[v_idx_couple_1] << " ; v_dot_d[v_idx_couple_2]=" << v_dot_d[v_idx_couple_2] << "\n";
	cout << "w_dot_d[w_idx_alone]=" << w_dot_d[w_idx_alone] << " ; w_dot_d[w_idx_couple_1]=" << w_dot_d[w_idx_couple_1] << " ; w_dot_d[w_idx_couple_2]=" << w_dot_d[w_idx_couple_2] << "\n";
	cout << "tv1=" << tv1 << " ; tv2=" << tv2 << " ; tw1=" << tw1 << " ; tw2=" << tw2 << "\n";

	if ((max(tv1, tv2)< min(tw1, tw2)) || (min(tv1, tv2)> max(tw1, tw2))) {
		return false;
	}

	return true;
}

