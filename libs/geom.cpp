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
std::ostream & operator << (std::ostream & os, const Face & face) {
	os << "face idx=" << glm::to_string(face._idx);
	return os;
}


ConvexHull::ConvexHull() {
	
}


ConvexHull::~ConvexHull() {

}


bool ConvexHull::is_conflict(Pt * pt, Face * face) {
	if (glm::dot(face->_normal, pt->_coords - _pts[face->_idx[0]]->_coords) > 0.0) {
		return true;
	}
	return false;
}


void ConvexHull::add_face(glm::uvec3 idx, bool is_face_init) {
	Face * face = new Face();
	face->_idx = idx;
	face->_normal = glm::cross(_pts[face->_idx[1]]->_coords- _pts[face->_idx[0]]->_coords, _pts[face->_idx[2]]->_coords- _pts[face->_idx[0]]->_coords);
	_faces.push_back(face);

	if (is_face_init) {
		return;
	}

	//ccw(face);

	for (auto pt : _pts) {
		if (is_conflict(pt, face)) {
			face->_conflict.push_back(pt);
			pt->_conflict.push_back(face);
		}
	}
}


Pt * ConvexHull::get_pt_not_in_face(Face * face) {
	uint idx_pt = 0;
	for (auto face2 : _faces) {
		std::cout << *face2 << "\n";
		if (face2 == face) {
			continue;
		}
		for (uint i=0; i<3; ++i) {
			if (face->_idx[0] != face2->_idx[i] && face->_idx[1] != face2->_idx[i] && face->_idx[2] != face2->_idx[i]) {
				return _pts[face2->_idx[i]];
			}
		}
	}
	std::cerr << "ConvexHull::get_pt_not_in_face : pas de point trouvé : " << *face << ".\n";
	return NULL;
}


void ConvexHull::ccw(Face * face) {
	Pt * pt = get_pt_not_in_face(face);
	
	if (glm::dot(face->_normal, pt->_coords - _pts[face->_idx[0]]->_coords) < 0.0) {
		uint tmp = face->_idx[0];
		face->_idx[0] = face->_idx[1];
		face->_idx[1] = tmp;
		face->_normal*= -1.0;
	}
}


Face * ConvexHull::opposite_face(Face * face, uint idx_edge) {
	for (auto face2 : _faces) {
		if (face == face2) {
			continue;
		}

		uint idx0 = 0;
		uint idx1 = 0;
		bool found_idx0 = false;
		bool found_idx1 = false;
		for (uint i = 0; i<3; ++i) {
			if (face->_idx[idx_edge] == face2->_idx[i]) {
				idx0 = i;
				found_idx0 = true;
				break;
			}
		}
		for (uint i = 0; i<3; ++i) {
			if (face->_idx[(idx_edge + 1) % 3] == face2->_idx[i]) {
				idx1 = i;
				found_idx1 = true;
				break;
			}
		}
		if (found_idx0 && found_idx1) {
			return face2;
		}
	}

	std::cerr << "ConvexHull::opposite_face : non trouvé.\n";
	return NULL;
}


void ConvexHull::randomize(uint n_pts, number xmin, number xmax, number ymin, number ymax, number zmin, number zmax) {
	_pts.clear();
	for (uint i=0; i<n_pts; ++i) {
		Pt * pt = new Pt();
		pt->_coords = rand_pt_3d(xmin, xmax, ymin, ymax, zmin, zmax);
		_pts.push_back(pt);
	}
}


void ConvexHull::compute() {
	_faces.clear();

	if (_pts.size()< 5) {
		std::cerr << "ConvexHull pas assez de points : " << _pts.size() << "\n";
		return;
	}

	add_face(glm::uvec3(0, 1, 2), true);
	add_face(glm::uvec3(0, 1, 3), true);
	add_face(glm::uvec3(0, 2, 3), true);
	add_face(glm::uvec3(1, 2, 3), true);



	for (uint idx_pt=4; idx_pt<_pts.size(); ++idx_pt) {
		Pt * pt = _pts[idx_pt];
		if (pt->_conflict.empty()) {
			continue;
		}

		std::vector<glm::uvec2> horizon;
		for (auto face : pt->_conflict) {
			for (uint idx_edge = 0; idx_edge<3; ++idx_edge) {
				bool op_face_in_conflict = false;
				Face * op_face = opposite_face(face, idx_edge);
				for (auto face2 : pt->_conflict) {
					if (face2 == op_face) {
						op_face_in_conflict = true;
						break;
					}
				}
				if (!op_face_in_conflict) {
					horizon.push_back(glm::uvec2(face->_idx[idx_edge], face->_idx[(idx_edge + 1) % 3]));
				}
			}
		}

		for (auto edge : horizon) {
			add_face(glm::uvec3(edge[0], edge[1], idx_pt));
		}

	}
}

