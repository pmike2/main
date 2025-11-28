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
Pt::Pt() {

}


Pt::Pt(pt_type_3d coords) : _coords(coords) {

}

Pt::~Pt() {

}


std::ostream & operator << (std::ostream & os, const Pt & pt) {
	os << "pt coords=" << glm::to_string(pt._coords);
	os << " ; pt conflict=";
	for (auto face : pt._conflict) {
		os << glm::to_string(face->_idx) << " ; ";
	}
	return os;
}


// ----------------------------------
Face::Face() {

}


Face::Face(glm::uvec3 idx) : _idx(idx), _delete(false) {
	
}


Face::~Face() {

}


void Face::change_orientation() {
	uint tmp = _idx[0];
	_idx[0] = _idx[1];
	_idx[1] = tmp;
	_normal *= -1.0;
}


std::ostream & operator << (std::ostream & os, const Face & face) {
	os << "face idx=" << glm::to_string(face._idx);
	os << " ; face conflict=";
	for (auto pt : face._conflict) {
		os << glm::to_string(pt->_coords) << " ; ";
	}
	return os;
}


// ----------------------------------
ConvexHull::ConvexHull() {
	
}


ConvexHull::~ConvexHull() {
	clear();
}


void ConvexHull::clear() {
	for (auto pt : _pts) {
		delete pt;
	}
	_pts.clear();
	for (auto face : _faces) {
		delete face;
	}
	_faces.clear();
}


bool ConvexHull::is_conflict(Pt * pt, Face * face) {
	if (glm::dot(face->_normal, pt->_coords - _pts[face->_idx[0]]->_coords) > 0.0) {
		return true;
	}
	return false;
}


void ConvexHull::add_conflict(Pt * pt, Face * face) {
	if (std::find(face->_conflict.begin(), face->_conflict.end(), pt) == face->_conflict.end()) {
		face->_conflict.push_back(pt);
	}
	
	if (std::find(pt->_conflict.begin(), pt->_conflict.end(), face) == pt->_conflict.end()) {
		pt->_conflict.push_back(face);
	}
}


Pt * ConvexHull::add_pt(pt_type_3d coords) {
	Pt * pt = new Pt(coords);
	_pts.push_back(pt);
	return pt;
}


Pt * ConvexHull::add_pt(number x, number y, number z) {
	return add_pt(pt_type_3d(x, y, z));
}


Face * ConvexHull::add_face(glm::uvec3 idx) {
	Face * face = new Face(idx);
	face->_normal = glm::cross(_pts[face->_idx[1]]->_coords- _pts[face->_idx[0]]->_coords, _pts[face->_idx[2]]->_coords- _pts[face->_idx[0]]->_coords);
	_faces.push_back(face);
	return face;
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
	clear();
	for (uint i=0; i<n_pts; ++i) {
		Pt * pt = new Pt();
		pt->_coords = rand_pt_3d(xmin, xmax, ymin, ymax, zmin, zmax);
		_pts.push_back(pt);
	}
}


void ConvexHull::randomize(uint n_pts, pt_type_3d vmin, pt_type_3d vmax) {
	randomize(n_pts, vmin.x, vmax.x, vmin.y, vmax.y, vmin.z, vmax.z);
}


void ConvexHull::compute() {
	_faces.clear();

	if (_pts.size()< 4) {
		std::cerr << "ConvexHull pas assez de points : " << _pts.size() << "\n";
		return;
	}

	// création tétraèdre initial
	Face * face1 = add_face(glm::uvec3(0, 1, 2));
	Face * face2 = add_face(glm::uvec3(0, 1, 3));
	Face * face3 = add_face(glm::uvec3(0, 2, 3));
	Face * face4 = add_face(glm::uvec3(1, 2, 3));
	if (is_conflict(_pts[3], face1)) {
		face1->change_orientation();
	}
	if (is_conflict(_pts[2], face2)) {
		face2->change_orientation();
	}
	if (is_conflict(_pts[1], face3)) {
		face3->change_orientation();
	}
	if (is_conflict(_pts[0], face4)) {
		face4->change_orientation();
	}
	for (auto face : _faces) {
		for (uint idx_pt=4; idx_pt<_pts.size(); ++idx_pt) {
			if (is_conflict(_pts[idx_pt], face)) {
				add_conflict(_pts[idx_pt], face);
			}
		}	
	}

	// boucle sur les pts restants
	for (uint idx_pt=4; idx_pt<_pts.size(); ++idx_pt) {
		Pt * pt = _pts[idx_pt];

		if (VERBOSE) {
			std::cout << "idx_pt = " << idx_pt << "-----------------------\n";
			std::cout << "pt = " << *pt << "\n";
			std::cout << *this;
		}

		// pt->_conflict vide => pt situé à l'intérieur de l'enveloppe
		if (pt->_conflict.empty()) {
			continue;
		}

		// construction de l'horizon ; pour chaque face en conflit avec le pt, pour chaque edge de la face
		// si la face opposée par rapport au edge n'est pas en conflit avec le pt, alors edge est dans l'horizon
		std::vector<Horizon> horizon;
		for (auto face : pt->_conflict) {
			for (uint idx_edge = 0; idx_edge<3; ++idx_edge) {
				bool op_face_in_conflict = false;
				Face * op_face = opposite_face(face, idx_edge);
				
				if (op_face == NULL) {
					std::cout << "DEBUG----------------------\n";
					std::cout << *this << "\n";
					std::cout << "----\n";
					std::cout << "pt = " << *pt << "\n";
					std::cout << "face = " << *face << "\n";
					std::cout << "idx_edge = " << idx_edge << "\n";
					std::cout << "DEBUG----------------------\n";
					return;
				}
				
				for (auto face2 : pt->_conflict) {
					if (face2 == op_face) {
						op_face_in_conflict = true;
						break;
					}
				}
				
				if (!op_face_in_conflict) {
					horizon.push_back({face, op_face, idx_edge});
					if (VERBOSE) {
						std::cout << "ajout horizon : face = " << *face << " ; op_face = " << *op_face << " ; idx_edge = " << idx_edge << "\n";
					}
				}
			}
		}

		// pour chaque edge de l'horizon on crée une nouvelle face edge-pt; les points en conflit avec cette nouvelle face
		// sont inclus dans les conflits de la face à supprimer et de la face opposée
		for (auto h : horizon) {
			Face * face = add_face(glm::uvec3(h._face->_idx[h._idx_edge], h._face->_idx[(h._idx_edge + 1) % 3], idx_pt));
			std::vector<Pt * > possible_conflict_points;
			for (auto pt2 : h._face->_conflict) {
				if (pt2 == pt) { // on ne prend pas en compte pt évidemment
					continue;
				}
				possible_conflict_points.push_back(pt2);
			}
			for (auto pt2 : h._opposite_face->_conflict) {
				if (pt2 == pt) {
					continue;
				}
				possible_conflict_points.push_back(pt2);
			}
			for (auto pt2 : possible_conflict_points) {
				if (is_conflict(pt2, face)) {
					add_conflict(pt2, face); // add_conflict empeche les doublons
				}
			}
		}

		// suppression des faces en conflit
		for (auto face : pt->_conflict) {
			if (VERBOSE) {
				std::cout << "delete face : " << *face << "\n";
			}
			face->_delete = true;
		}

		for (auto pt2 : _pts) {
			pt2->_conflict.erase(std::remove_if(pt2->_conflict.begin(), pt2->_conflict.end(), [](Face * face) {
				return face->_delete;
			}), pt2->_conflict.end());
		}

		_faces.erase(std::remove_if(_faces.begin(), _faces.end(), [](Face * face) {
			return face->_delete;
		}), _faces.end());
	}

	if (VERBOSE) {
		std::cout << "FIN\n\n";
		std::cout << *this;
	}	
}


std::ostream & operator << (std::ostream & os, const ConvexHull & hull) {
	os << "pts :\n";
	for (auto pt : hull._pts) {
		os << *pt << "\n";
	}
	os << "faces :\n";
	for (auto face : hull._faces) {
		os << *face << "\n";
	}
	os << "\n";
	return os;
}
