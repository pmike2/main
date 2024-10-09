#include <algorithm>
#include <cctype>
#include <fstream>
#include <cmath>
#include <tuple>

#include "dcel.h"
#include "utile.h"

#include <glm/gtx/norm.hpp>


// ----------------------------------------------------------------------
DCEL_Vertex::DCEL_Vertex() : _coords(glm::vec2(0.0f)), _incident_edge(NULL), _data(NULL) {

}


DCEL_Vertex::DCEL_Vertex(const glm::vec2 & coords) : _coords(coords), _incident_edge(NULL), _data(NULL) {

}


DCEL_Vertex::~DCEL_Vertex() {
	if (_data!= NULL) {
		delete _data;
	}
}


// renvoie les hedges dont l'origine est le vertex
std::vector<DCEL_HalfEdge *> DCEL_Vertex::get_incident_edges() {
	bool verbose= false;
	
	if (verbose) {
		std::cout << "DCEL_Vertex::get_incident_edges : v == " << *this << "\n";
	}

	std::vector<DCEL_HalfEdge *> result;
	if (_incident_edge== NULL) {
		if (verbose) {
			std::cout << "DCEL_Vertex::get_incident_edges : _incident_edge== NULL\n";
		}
		return std::vector<DCEL_HalfEdge *>{};
	}
	
	// _incident_edge est 1 des hedges concernés
	if (verbose) {
		std::cout << "DCEL_Vertex::get_incident_edges : init avec = " << *_incident_edge << "\n";
	}
	result.push_back(_incident_edge);
	DCEL_HalfEdge * current= _incident_edge;

	if (verbose) {
		std::cout << "DCEL_Vertex::get_incident_edges : current = " << *current << "\n";
	}
	while(true) {
		current= current->_next;

		// ne devrait pas arriver si DCEL::make_valid() a été appelé
		if (current== NULL) {
			std::cerr << "ERR : DCEL_Vertex::get_incident_edges : current == NULL\n";
			break;
		}

		if (verbose) {
			std::cout << "DCEL_Vertex::get_incident_edges : current = " << *current << "\n";
		}

		// on parcourt tous les edges de la face et quand destination() == this on prend le twin et on enchaine sur une autre face
		if (current->destination()== this) {
			if (verbose) {
				std::cout << "DCEL_Vertex::get_incident_edges : dst == this : " << *current << " ; ajout de : " << *current->_twin << "\n";
			}
			current= current->_twin;

			// on est revenu au 1er edge
			if (current== _incident_edge) {
				break;
			}
			result.push_back(current);
		}
	}

	return result;
}


std::ostream & operator << (std::ostream & os, DCEL_Vertex & v) {
	os << "(" << v._coords.x << " ; " << v._coords.y << ")";
	return os;
}


// ----------------------------------------------------------------------
DCEL_HalfEdge::DCEL_HalfEdge() : 
_origin(NULL), _twin(NULL), _next(NULL), _previous(NULL), _incident_face(NULL), _data(NULL) {

}


DCEL_HalfEdge::~DCEL_HalfEdge() {
	if (_data!= NULL) {
		delete _data;
	}
}


// renvoie le vertex destination
DCEL_Vertex * DCEL_HalfEdge::destination() {
	if (_twin== NULL) {
		std::cerr << "ERR : DCEL_HalfEdge::destination() : _twin == NULL\n";
		return NULL;
	}
	return _twin->_origin;
}


// renvoie la face opposée
DCEL_Face * DCEL_HalfEdge::opposite_face() {
	if (_twin== NULL) {
		std::cerr << "ERR : DCEL_HalfEdge::opposite_face() : _twin == NULL\n";
	}
	return _twin->_incident_face;
}


// set twin ; hedge peut valoir NULL
void DCEL_HalfEdge::set_twin(DCEL_HalfEdge * hedge) {
	_twin= hedge;
	if (hedge!= NULL) {
		hedge->_twin= this;
	}
	else {
		hedge->_twin= NULL;
	}
}


// set next ; hedge peut valoir NULL
void DCEL_HalfEdge::set_next(DCEL_HalfEdge * hedge) {
	//std::cout << "DEBUG__ : " << *this << " | " << *hedge << "\n";
	if (hedge!= NULL) {
		hedge->_previous= this;
	}
	else if (_next!= NULL) {
		_next->_previous= NULL;
	}
	_next= hedge;
}


// set previous ; hedge peut valoir NULL
void DCEL_HalfEdge::set_previous(DCEL_HalfEdge * hedge) {
	if (hedge!= NULL) {
		hedge->_next= this;
	}
	else if (_previous!= NULL) {
		_previous->_next= NULL;
	}
	_previous= hedge;
}


// set origin ; v peut valoir NULL
void DCEL_HalfEdge::set_origin(DCEL_Vertex * v) {
	if (_twin!= NULL && destination()== v) {
		std::cerr << "ERR : DCEL_HalfEdge::set_origin : v == destination : " << *v << "\n";
	}
	if (_origin!= NULL) {
		_origin->_incident_edge= NULL; // OK ?
	}
	_origin= v;
	v->_incident_edge= this; // OK ?
}


// set incident face
void DCEL_HalfEdge::set_incident_face(DCEL_Face * f) {
	bool verbose= false;

	// on ne sait pas si f.ccw() donc f->_outer_edge= this n'est pas forcément vrai
	DCEL_HalfEdge * he= this;
	do {
		if (verbose) {
			std::cout << "DCEL_HalfEdge::set_incident_face : he == " << *he << "\n";
		}

		he->_incident_face= f;
		he= he->_next;
		if (he== NULL) {
			std::cerr << "ERR : DCEL_HalfEdge::set_incident_face : face non close\n";
			return;
		}
	} while (he!= this);
}


std::ostream & operator << (std::ostream & os, DCEL_HalfEdge & e) {
	//os << &e << " : ";

	if (e._origin!= NULL) {
		os << *e._origin;
	}
	else {
		os << "NULL";
	}
	os << " -> ";
	if (e.destination()!= NULL) {
		os << *e.destination();
	}
	else {
		os << "NULL";
	}
	/*if (e._previous!= NULL) {
		os << " ; previous = " << *e._previous;
	}
	else {
		os << " ; previous = NULL";
	}
	if (e._next!= NULL) {
	 	os << " ; next = " << *e._next;
	}
	else {
		os << " ; next = NULL";
	}*/

	return os;
}


// ----------------------------------------------------------------------
DCEL_Face::DCEL_Face() : _outer_edge(NULL), _data(NULL) {

}


DCEL_Face::~DCEL_Face() {
	if (_data!= NULL) {
		delete _data;
	}
}


// renvoie la liste des vertices entourant la face
std::vector<DCEL_Vertex *> DCEL_Face::get_vertices() {
	bool verbose= false;

	std::vector<DCEL_Vertex *> result;

	std::vector<DCEL_HalfEdge *> edges= get_outer_edges();
	for (auto edge : edges) {
		if (edge->_origin== NULL) {
			std::cerr << "ERR : DCEL_Face::get_vertices() : edge->_origin== NULL\n";
			continue;
		}

		if (verbose) {
			std::cout << "DCEL_Face::get_vertices : ajout vertex : " << *edge->_origin << "\n";
		}

		result.push_back(edge->_origin);
	}

	return result;
}


// renvoie la liste des outer edges de la face
std::vector<DCEL_HalfEdge *> DCEL_Face::get_outer_edges() {
	bool verbose= false;

	if (_outer_edge== NULL) {
		return std::vector<DCEL_HalfEdge *>{};
	}

	std::vector<DCEL_HalfEdge *> result;
	DCEL_HalfEdge * first_edge= _outer_edge;
	DCEL_HalfEdge * current_edge= _outer_edge;
	do {
		if (current_edge== NULL) {
			std::cerr << "ERR : DCEL_Face::get_outer_edges : current_edge== NULL\n";
			break;
		}
		if (verbose) {
			std::cout << "DCEL_Face::get_outer_edges : ajout edge : " << *current_edge << "\n";
		}
		
		result.push_back(current_edge);
		current_edge= current_edge->_next;
	} while (current_edge!= first_edge);

	return result;
}


// renvoie la liste des inner edges de la face
std::vector<std::vector<DCEL_HalfEdge *> > DCEL_Face::get_inner_edges() {
	std::vector<std::vector<DCEL_HalfEdge *> > result;
	// on concatène tous les inner edges
	for (auto inner_edge : _inner_edges) {
		DCEL_HalfEdge * first_edge= inner_edge;
		DCEL_HalfEdge * current_edge= inner_edge;
		std::vector<DCEL_HalfEdge *> edges;
		do {
			if (current_edge== NULL) {
				std::cerr << "ERR : DCEL_Face::get_inner_edges() : current_edge== NULL\n";
				break;
			}
			edges.push_back(current_edge);
			current_edge= current_edge->_next;
		} while (current_edge!= first_edge);
		result.push_back(edges);
	}

	return result;
}


// renvoie les faces contigues
std::vector<DCEL_Face *> DCEL_Face::get_adjacent_faces() {
	std::vector<DCEL_Face *> result;
	DCEL_HalfEdge * first_edge= _outer_edge;
	DCEL_HalfEdge * current_edge= _outer_edge;
	do {
		if (current_edge== NULL) {
			std::cerr << "ERR : DCEL_Face::get_adjacent_faces() : current_edge== NULL\n";
			break;
		}
		DCEL_Face * face= current_edge->opposite_face();
		std::vector<DCEL_Face *>::iterator it= find(result.begin(), result.end(), face);
		if (it!= result.end()) {
			result.push_back(face);
		}
		current_edge= current_edge->_next;
	} while (current_edge!= first_edge);

	return result;
}



// renvoie le centre de gravité, utile pour le html
glm::vec2 DCEL_Face::get_gravity_center() {
	glm::vec2 result(0.0f);
	std::vector<DCEL_Vertex *> vertices= get_vertices();
	for (auto v : vertices) {
		result+= v->_coords;
	}
	result/= vertices.size();
	return result;
}


Polygon2D * DCEL_Face::get_polygon() {
	std::vector<glm::vec2> pts;
	for (auto v : get_vertices()) {
		pts.push_back(v->_coords);
	}
	Polygon2D * poly= new Polygon2D();
	poly->set_points(pts);
	return poly;
}


// renvoie True si la face est en counterclockwise
bool DCEL_Face::ccw() {
	bool verbose= false;

	std::vector<glm::vec2> pts;
	for (auto v : get_vertices()) {
		pts.push_back(v->_coords);
		if (verbose) {
			std::cout << "DCEL_Face::ccw : " << glm_to_string(v->_coords) << "\n";
		}
	}
	if (pts.size()< 3) {
		std::cerr << "ERR : DCEL_Face::ccw : moins de 3 pts\n";
		return false;
	}
	return is_ccw(pts);
}


std::ostream & operator << (std::ostream & os, DCEL_Face & f) {
	if (f._outer_edge== NULL) {
		os << "unbounded face\n";
	}

	if (f._outer_edge!= NULL) {
		os << "outer_edge = " << *f._outer_edge << "\n";
		std::vector<DCEL_HalfEdge *> outer_edges= f.get_outer_edges();
		os << "outer edges.size = " << outer_edges.size() << "\n";
		for (auto edge : outer_edges) {
			os << *edge;
			if (edge!= outer_edges.back()) {
				os << " | ";
			}
		}
		os << "\n";
	}

	os << "inner_edges.size = " << f._inner_edges.size() << "\n";
	std::vector<std::vector<DCEL_HalfEdge *> > inner_edges= f.get_inner_edges();
	for (auto edges : inner_edges) {
		for (auto edge : edges) {
			os << *edge;
			if (edge!= edges.back()) {
				os << " | ";
			}
		}
		os << "\n";
	}
	return os;
}


// ----------------------------------------------------------------------
DCEL::DCEL() {
}


DCEL::~DCEL() {
	for (auto & f : _faces) {
		delete f;
	}
	_faces.clear();

	for (auto & he : _half_edges) {
		delete he;
	}
	_half_edges.clear();

	for (auto & v : _vertices) {
		delete v;
	}
	_vertices.clear();
}


// ajout d'un sommet
DCEL_Vertex * DCEL::add_vertex(const glm::vec2 & coords) {
	bool verbose= true;

	DCEL_Vertex * v= get_vertex(coords);
	if (v== NULL) {
		v= new DCEL_Vertex(coords);
		_vertices.push_back(v);
	}
	else {
		if (verbose) {
			std::cout << "DCEL::add_vertex : point déjà présent : " << glm_to_string(coords) << "\n";
		}
	}
	return v;
}


// ajout de 2 hedges
DCEL_HalfEdge * DCEL::add_edge(DCEL_Vertex * v1, DCEL_Vertex * v2) {
	bool verbose= true;

	if (v1== NULL || v2== NULL) {
		std::cerr << "ERR : DCEL::add_edge : v1== NULL || v2== NULL\n";
		return NULL;
	}

	if (v1== v2) {
		std::cerr << "ERR : DCEL::add_edge : v1== v2\n";
		return NULL;
	}

	DCEL_HalfEdge * hedge1= new DCEL_HalfEdge();
	DCEL_HalfEdge * hedge2= new DCEL_HalfEdge();
	hedge1->set_origin(v1);
	hedge2->set_origin(v2);
	hedge1->set_twin(hedge2);
	_half_edges.push_back(hedge1);
	_half_edges.push_back(hedge2);
	if (v1->_incident_edge== NULL) {
		v1->_incident_edge= hedge1;
	}
	if (v2->_incident_edge== NULL) {
		v2->_incident_edge= hedge2;
	}
	// on renvoie celui qui va de v1 à v2
	return hedge1;
}


DCEL_HalfEdge * DCEL::add_edge(const glm::vec2 & ori, const glm::vec2 & dst) {
	DCEL_Vertex * v_ori= add_vertex(ori);
	DCEL_Vertex * v_dst= add_vertex(dst);
	return add_edge(v_ori, v_dst);
}


DCEL_HalfEdge * DCEL::add_edge(const glm::vec4 & ori_and_dst) {
	return add_edge(glm::vec2(ori_and_dst[0], ori_and_dst[1]), glm::vec2(ori_and_dst[2], ori_and_dst[3]));
}


// ajout d'une face
DCEL_Face * DCEL::add_face() {
	DCEL_Face * face= new DCEL_Face();
	_faces.push_back(face);
	return face;
}


// suppression d'un sommet
/*void DCEL::delete_vertex(DCEL_Vertex * v) {
	if (v== NULL) {
		std::cerr << "ERR : DCEL::delete_vertex  : v== NULL\n";
		return;
	}

	add2queue({VERTEX, v});
}


// suppression d'un edge, ie de 2 half-edges
void DCEL::delete_edge(DCEL_HalfEdge * he) {
	if (he== NULL) {
		std::cerr << "ERR : DCEL::delete_edge NULL\n";
		return;
	}

	add2queue({HALF_EDGE, he});
}


void DCEL::delete_face(DCEL_Face * face) {
	if (face== NULL) {
		std::cerr << "ERR : DCEL::delete_face NULL\n";
		return;
	}

	add2queue({FACE, face});
}*/


DCEL_HalfEdge * DCEL::split_edge(DCEL_HalfEdge * he, const glm::vec2 & coords) {
	DCEL_Vertex * v= add_vertex(coords);
	DCEL_HalfEdge * he2= add_edge(v, he->destination());
	he2->set_next(he->_next);
	he->set_next(he2);
	he->_twin->set_origin(v);
	he2->_twin->set_previous(he->_twin->_previous);
	he2->_twin->set_next(he->_twin);
	he2->_incident_face= he->_incident_face;
	he2->_twin->_incident_face= he->_twin->_incident_face;
	return he2;
}


DCEL_HalfEdge * DCEL::cut_face(DCEL_HalfEdge * he1, DCEL_HalfEdge * he2) {
	bool verbose= false;

	if (he1->_origin== he2->_origin) {
		if (verbose) {
			std::cout << "DCEL::cut_face : he1->_origin== he2->_origin\n";
		}
		return NULL;
	}

	DCEL_HalfEdge * cut_he= add_edge(he1->_origin, he2->_origin);

	if (verbose) {
		std::cout << "DCEL::cut_face : he1 = " << *he1 << " ; he2 = " << *he2 << "\n";
	}

	// il faut le stocker car modifié par la ligne suivante
	DCEL_HalfEdge * he2_previous= he2->_previous;
	
	cut_he->set_next(he2);
	cut_he->set_previous(he1->_previous);
	cut_he->_twin->set_next(he1);
	cut_he->_twin->set_previous(he2_previous);
	if (he1->_incident_face!= NULL) {
		add2queue({FACE, he1->_incident_face});
		he1->_incident_face= NULL;
		he2->_incident_face= NULL;
	}
	
	// pas nécessaire car à la fin on recalcule toutes les faces avec create_faces_from_half_edges
	/*DCEL_Face * face1= add_face();
	he1->set_incident_face(face1);
	face1->_outer_edge= he1;
	DCEL_Face * face2= add_face();
	he2->set_incident_face(face2);
	face2->_outer_edge= he2;*/

	return cut_he;
}


// suppression de tout
void DCEL::clear() {
	_vertices.clear();
	_half_edges.clear();
	_faces.clear();
}


void DCEL::create_nexts_from_twins() {
	bool verbose= true;

	// on cherche les hedges qui n'ont pas de next
	while (true) {
		bool found= false;
		for (auto he : _half_edges) {
			if (he->_next== NULL) {
				if (verbose) {
					std::cout << "DCEL::create_nexts_from_twins : he = " << *he << "\n";
				}

				found= true;
				DCEL_HalfEdge * first_he= he;
				DCEL_HalfEdge * current_he= he;
				DCEL_HalfEdge * next_he= NULL;
				do {
					if (current_he->_twin->_previous== NULL) {
						std::cerr << "ERR : DCEL::create_nexts_from_twins : current_he->_twin->_previous== NULL : current_he = " << *current_he << " ; twin = " << *current_he->_twin << "\n";
						return;
					}
					next_he= current_he->_twin->_previous->_twin;

					if (verbose) {
						std::cout << "DCEL::create_nexts_from_twins : next_he = " << *next_he << "\n";
					}

					while (next_he->_previous!= NULL) {
						next_he= next_he->_previous->_twin;
					}
					current_he->set_next(next_he);
					current_he= next_he;
				} while (current_he!= first_he);
				break;
			}
		}
		if (!found) {
			break;
		}
	}
}


// création des faces à partir des hedges qui n'ont pas de face associée
void DCEL::create_faces_from_half_edges() {
	bool verbose= true;

	for (auto he : _half_edges) {
		he->_incident_face= NULL;
	}
	_faces.clear();
	
	bool found_unattached_he= false;
	while (true) {
		found_unattached_he= false;
		for (auto he : _half_edges) {
			if (he->_incident_face== NULL) {
				if (verbose) {
					std::cout << "DCEL::create_faces_from_half_edges : he->_incident_face== NULL : " << *he << "\n";
				}
				found_unattached_he= true;
				DCEL_Face * face= add_face();
				he->set_incident_face(face);
				face->_outer_edge= he;
			}
		}
		if (!found_unattached_he) {
			break;
		}
	}

	// on gère enfin les trous et la face infinie
	check_ccw_faces();
}


// gestion trous et face infinie
void DCEL::check_ccw_faces() {
	bool verbose= true;
	
	for (auto f : _faces) {
		f->_inner_edges.clear();
	}
	
	DCEL_Face * unbounded_face= NULL;

	for (auto f : _faces) {
		if (f->ccw()) {
			continue;
		}

		if (verbose) {
			std::cout << "DCEL::check_ccw_faces face ccw =\n" << *f << "\n";
		}

		bool is_hole= false;
		Polygon2D * fpoly= f->get_polygon();
		for (auto f2 : _faces) {
			if (f2== f || !f2->ccw()) {
				continue;
			}
			Polygon2D * fpoly2= f2->get_polygon();
			if (is_poly_inside_poly(fpoly, fpoly2)) {
				if (verbose) {
					std::cout << "DCEL::check_ccw_faces face inside :\n" << *f2 << "\n";
				}
				is_hole= true;
				f2->_inner_edges.push_back(f->_outer_edge);
				f->_outer_edge->set_incident_face(f2);
				add2queue({FACE, f});
				break;
			}
		}

		// trou de la face infinie
		if (!is_hole) {
			if (verbose) {
				std::cout << "DCEL::check_ccw_faces face inside infinite face\n";
			}
			if (unbounded_face== NULL) {
				unbounded_face= f;
			}
			else {
				add2queue({FACE, f});
			}
			unbounded_face->_inner_edges.push_back(f->_outer_edge);
		}
	}

	if (unbounded_face== NULL) {
		std::cerr << "ERR : DCEL::check_ccw_faces : unbounded_face== NULL\n";
		return;
	}
	unbounded_face->_outer_edge= NULL;

	if (verbose) {
		std::cout << "DCEL::check_ccw_faces face inside : delete_queue\n";
	}
	delete_queue();
}


void DCEL::check_integrity() {
	for (auto e : _half_edges) {
		if (e->_previous== NULL) {
			std::cout << "ERR : DCEL::check_integrity : previous == NULL : " << *e << "\n";
		}
		if (e->_next== NULL) {
			std::cout << "ERR : DCEL::check_integrity : next == NULL : " << *e << "\n";
		}
		if (e->_twin== NULL) {
			std::cout << "ERR : DCEL::check_integrity : twin == NULL : " << *e << "\n";
		}
	}
}


void DCEL::delete_loop_edge() {
	for (auto e : _half_edges) {
		if (e->_origin!= NULL && e->destination()!= NULL && e->_origin== e->destination()) {
			add2queue({HALF_EDGE, e});
			if (e->_previous!= NULL) {
				e->_previous->set_next(e->_next);
			}
			else if (e->_next!= NULL) {
				e->_next->set_next(e->_previous);
			}
		}
	}
	delete_queue();
}


void DCEL::add2queue(DeleteEvent evt) {
	bool verbose= false;

	if (verbose) {
		std::cout << "DCEL::add2queue : evt type = " << evt._type << "\n";
	}

	if (std::find(_delete_queue.begin(), _delete_queue.end(), evt)== _delete_queue.end()) {
		_delete_queue.push_back(evt);
	}
}


void DCEL::delete_queue() {
	bool verbose= false;

	std::vector<DCEL_Vertex *> vertices2delete;
	std::vector<DCEL_HalfEdge *> edges2delete;
	std::vector<DCEL_Face *> faces2delete;

	while (!_delete_queue.empty()) {
		DeleteEvent evt= _delete_queue.front();
		_delete_queue.pop_front();

		if (evt._type== VERTEX) {
			DCEL_Vertex * v= (DCEL_Vertex *)evt._ptr;
			if (std::find(vertices2delete.begin(), vertices2delete.end(), v)!= vertices2delete.end()) {
				continue;
			}

			if (verbose) {
				std::cout << "DCEL::delete_queue : v= " << *v << "\n";
			}

			vertices2delete.push_back(v);
		}
		
		else if (evt._type== HALF_EDGE) {
			DCEL_HalfEdge * he= (DCEL_HalfEdge *)evt._ptr;
			if (std::find(edges2delete.begin(), edges2delete.end(), he)!= edges2delete.end()) {
				continue;
			}

			if (verbose) {
				std::cout << "DCEL::delete_queue : he= " << *he << "\n";
			}

			// on supprime le twin du coup
			add2queue({HALF_EDGE, he->_twin});

			edges2delete.push_back(he);
		}

		else if (evt._type== FACE) {
			DCEL_Face * f= (DCEL_Face *)evt._ptr;
			if (std::find(faces2delete.begin(), faces2delete.end(), f)!= faces2delete.end()) {
				continue;
			}

			if (verbose) {
				std::cout << "DCEL::delete_queue : f= " << *f;
			}

			faces2delete.push_back(f);
		}
	}

	_vertices.erase(std::remove_if(_vertices.begin(), _vertices.end(), [vertices2delete](DCEL_Vertex * v){
		return std::find(vertices2delete.begin(), vertices2delete.end(), v)!= vertices2delete.end();
	}), _vertices.end());
	_half_edges.erase(std::remove_if(_half_edges.begin(), _half_edges.end(), [edges2delete](DCEL_HalfEdge * e){
		return std::find(edges2delete.begin(), edges2delete.end(), e)!= edges2delete.end();
	}), _half_edges.end());
	_faces.erase(std::remove_if(_faces.begin(), _faces.end(), [faces2delete](DCEL_Face * f){
		return std::find(faces2delete.begin(), faces2delete.end(), f)!= faces2delete.end();
	}), _faces.end());
}


// est-ce vide
bool DCEL::is_empty() {
	return _vertices.empty() && _half_edges.empty();
}


// ajout d'une bounding box qui peut rogner le DCEL
void DCEL::add_bbox(const glm::vec2 & bbox_min, const glm::vec2 & bbox_max) {
	bool verbose= true;

	// si vide on crée un carré de taille 1
	if (is_empty()) {
		if (verbose) {
			std::cout << "DCEL::add_bbox : DCEL vide -> carré 1x1\n";
		}
		DCEL_Vertex * v1= add_vertex(glm::vec2(0.0f, 0.0f));
		DCEL_Vertex * v2= add_vertex(glm::vec2(1.0f, 0.0f));
		DCEL_Vertex * v3= add_vertex(glm::vec2(1.0f, 1.0f));
		DCEL_Vertex * v4= add_vertex(glm::vec2(0.0f, 1.0f));
		DCEL_HalfEdge * he1= add_edge(v1, v2);
		DCEL_HalfEdge * he2= add_edge(v2, v3);
		DCEL_HalfEdge * he3= add_edge(v3, v4);
		DCEL_HalfEdge * he4= add_edge(v4, v1);
		he1->set_next(he2);
		he2->set_next(he3);
		he3->set_next(he4);
		he4->set_next(he1);
		return;
	}

	// un vertex est-il dans une emprise
	auto in_bbox= [bbox_min, bbox_max](DCEL_Vertex * v) -> bool{
		if ((v->_coords.x< bbox_min.x) || (v->_coords.x> bbox_max.x) || (v->_coords.y< bbox_min.y) || (v->_coords.y> bbox_max.y)) {
			return false;
		}
		return true;
	};

	if (verbose) {
		std::cout << "DCEL::add_bbox : bbox_min = " << glm_to_string(bbox_min) << " ; bbox_max = " << glm_to_string(bbox_max) << "\n";
	}

	// ------------------------------------------------------------------------------------------------------------------------
	if (verbose) {
		std::cout << "DCEL::add_bbox phase 1 : création des nouveaux sommets intersections des droites et 1/2 droites avec la bbox\n";
	}

	// ajout des sommets de la bbox
	DCEL_Vertex * bottom_left_corner= add_vertex(bbox_min);
	DCEL_Vertex * bottom_right_corner= add_vertex(glm::vec2(bbox_max.x, bbox_min.y));
	DCEL_Vertex * top_left_corner= add_vertex(glm::vec2(bbox_min.x, bbox_max.y));
	DCEL_Vertex * top_right_corner= add_vertex(bbox_max);

	std::vector<DCEL_HalfEdge *> origin_top, origin_bottom, origin_left, origin_right, ignore;

	// edges qui n'ont pas d'origine mais une destination et cherche l'intersection du twin avec la bbox
	// on ne fait pas (auto e : _half_edges) car on fait des push_back dans la boucle
	const int nhes= _half_edges.size();
	for (int i=0; i<nhes; ++i) {
		DCEL_HalfEdge * he= _half_edges[i];

		if (std::find(ignore.begin(), ignore.end(), he)!= ignore.end()) {
			continue;
		}

		bool ori_in_bbox= in_bbox(he->_origin);
		bool dst_in_bbox= in_bbox(he->destination());

		if (ori_in_bbox) {
			continue;
		}

		if (verbose) {
			std::cout << "DCEL::add_bbox phase 1 : he = " << *he << "\n";
		}

		add2queue({HALF_EDGE, he});
		add2queue({VERTEX, he->_origin});
		if (!dst_in_bbox) {
			add2queue({VERTEX, he->destination()});
		}
		ignore.push_back(he->_twin);

		bool is_inter_bottom= false, is_inter_top= false, is_inter_left= false, is_inter_right= false;
		glm::vec2 inter_bottom, inter_top, inter_left, inter_right;
		is_inter_bottom= segment_intersects_segment(he->_origin->_coords, he->destination()->_coords, bottom_left_corner->_coords , bottom_right_corner->_coords, &inter_bottom);
		is_inter_top   = segment_intersects_segment(he->_origin->_coords, he->destination()->_coords, top_left_corner->_coords    , top_right_corner->_coords   , &inter_top);
		is_inter_left  = segment_intersects_segment(he->_origin->_coords, he->destination()->_coords, bottom_left_corner->_coords , top_left_corner->_coords    , &inter_left);
		is_inter_right = segment_intersects_segment(he->_origin->_coords, he->destination()->_coords, bottom_right_corner->_coords, top_right_corner->_coords   , &inter_right);

		if (verbose) {
			std::cout << "bottom : " << is_inter_bottom << " ; pt1_begin = " << glm_to_string(he->_origin->_coords) << " ; pt1_end = " << glm_to_string(he->destination()->_coords) << " ; pt2_begin = " << glm_to_string(bottom_left_corner->_coords) << " ; pt2_end = " << glm_to_string(bottom_right_corner->_coords) << "\n";
			std::cout << "top : " << is_inter_bottom << " ; pt1_begin = " << glm_to_string(he->_origin->_coords) << " ; pt1_end = " << glm_to_string(he->destination()->_coords) << " ; pt2_begin = " << glm_to_string(top_left_corner->_coords) << " ; pt2_end = " << glm_to_string(top_right_corner->_coords) << "\n";
			std::cout << "left : " << is_inter_bottom << " ; pt1_begin = " << glm_to_string(he->_origin->_coords) << " ; pt1_end = " << glm_to_string(he->destination()->_coords) << " ; pt2_begin = " << glm_to_string(bottom_left_corner->_coords) << " ; pt2_end = " << glm_to_string(top_left_corner->_coords) << "\n";
			std::cout << "right : " << is_inter_bottom << " ; pt1_begin = " << glm_to_string(he->_origin->_coords) << " ; pt1_end = " << glm_to_string(he->destination()->_coords) << " ; pt2_begin = " << glm_to_string(bottom_right_corner->_coords) << " ; pt2_end = " << glm_to_string(top_right_corner->_coords) << "\n";
		}

		if (dst_in_bbox) {
			if (is_inter_bottom) {
				if (verbose) {
					std::cout << "DCEL::add_bbox phase 1 : inter_bottom et dst_in_bbox\n";
				}
				origin_bottom.push_back(split_edge(he, inter_bottom));
			}
			else if (is_inter_top) {
				if (verbose) {
					std::cout << "DCEL::add_bbox phase 1 : inter_top et dst_in_bbox\n";
				}
				origin_top.push_back(split_edge(he, inter_top));
			}
			else if (is_inter_left) {
				if (verbose) {
					std::cout << "DCEL::add_bbox phase 1 : inter_left et dst_in_bbox\n";
				}
				origin_left.push_back(split_edge(he, inter_left));
			}
			else if (is_inter_right) {
				if (verbose) {
					std::cout << "DCEL::add_bbox phase 1 : inter_right et dst_in_bbox\n";
				}
				origin_right.push_back(split_edge(he, inter_right));
			}
			else {
				// cas foireux où segment_intersects_segment n'a pas bien fonctionné car l'intersection est sur une extrémité de segment
				// c'est forcement un de ces 2 cas il me semble
				if (he->_origin->_coords.y<= bbox_min.y) {
					if (verbose) {
						std::cout << "DCEL::add_bbox phase 1 : inter_bottom (cas limite) et dst_in_bbox\n";
					}
					origin_bottom.push_back(split_edge(he, inter_bottom));
				}
				else if (he->_origin->_coords.y> bbox_max.y) {
					if (verbose) {
						std::cout << "DCEL::add_bbox phase 1 : inter_top (cas limite) et dst_in_bbox\n";
					}
					origin_top.push_back(split_edge(he, inter_top));
				}
				else {
					std::cerr << "ERR : cas limite\n";
				}
			}
			continue;
		}

		if (is_inter_bottom && he->_origin->_coords.y<= bbox_min.y) {
			if (verbose) {
				std::cout << "DCEL::add_bbox phase 1 : inter_bottom et !dst_in_bbox\n";
			}
			DCEL_HalfEdge * he2= split_edge(he, inter_bottom);
			origin_bottom.push_back(he2);
			if (is_inter_top) {
				add2queue({HALF_EDGE, split_edge(he2, inter_top)});
				origin_top.push_back(he2->_twin);
			}
			else if (is_inter_left) {
				add2queue({HALF_EDGE, split_edge(he2, inter_left)});
				origin_left.push_back(he2->_twin);
			}
			else if (is_inter_right) {
				add2queue({HALF_EDGE, split_edge(he2, inter_right)});
				origin_right.push_back(he2->_twin);
			}
		}
		
		else if (is_inter_top && he->_origin->_coords.y>= bbox_max.y) {
			if (verbose) {
				std::cout << "DCEL::add_bbox phase 1 : inter_top et !dst_in_bbox\n";
			}
			DCEL_HalfEdge * he2= split_edge(he, inter_top);
			origin_top.push_back(he2);
			if (is_inter_bottom) {
				add2queue({HALF_EDGE, split_edge(he2, inter_bottom)});
				origin_bottom.push_back(he2->_twin);
			}
			else if (is_inter_left) {
				add2queue({HALF_EDGE, split_edge(he2, inter_left)});
				origin_left.push_back(he2->_twin);
			}
			else if (is_inter_right) {
				add2queue({HALF_EDGE, split_edge(he2, inter_right)});
				origin_right.push_back(he2->_twin);
			}
		}
		
		else if (is_inter_left && he->_origin->_coords.x<= bbox_min.y) {
			if (verbose) {
				std::cout << "DCEL::add_bbox phase 1 : inter_left et !dst_in_bbox\n";
			}
			DCEL_HalfEdge * he2= split_edge(he, inter_left);
			origin_left.push_back(he2);
			if (is_inter_top) {
				add2queue({HALF_EDGE, split_edge(he2, inter_top)});
				origin_top.push_back(he2->_twin);
			}
			else if (is_inter_bottom) {
				add2queue({HALF_EDGE, split_edge(he2, inter_bottom)});
				origin_bottom.push_back(he2->_twin);
			}
			else if (is_inter_right) {
				add2queue({HALF_EDGE, split_edge(he2, inter_right)});
				origin_right.push_back(he2->_twin);
			}
		}
		
		else if (is_inter_right && he->_origin->_coords.x>= bbox_max.y) {
			if (verbose) {
				std::cout << "DCEL::add_bbox phase 1 : inter_right et !dst_in_bbox\n";
			}
			DCEL_HalfEdge * he2= split_edge(he, inter_right);
			origin_bottom.push_back(he2);
			if (is_inter_top) {
				add2queue({HALF_EDGE, split_edge(he2, inter_top)});
				origin_top.push_back(he2->_twin);
			}
			else if (is_inter_left) {
				add2queue({HALF_EDGE, split_edge(he2, inter_left)});
				origin_left.push_back(he2->_twin);
			}
			else if (is_inter_bottom) {
				add2queue({HALF_EDGE, split_edge(he2, inter_bottom)});
				origin_bottom.push_back(he2->_twin);
			}
		}
		
		else {
			if (verbose) {
				std::cout << "DCEL::add_bbox phase 1 : no inter\n";
			}
			add2queue({FACE, he->_incident_face});
		}
	}

	// ------------------------------------------------------------------------------------------------------------
	if (verbose) {
		std::cout << "DCEL::add_bbox phase 2 : delete_queue\n";
	}
	delete_queue();

	// ------------------------------------------------------------------------------------------------------------
	if (verbose) {
		std::cout << "DCEL::add_bbox phase 3 : tri des intersections\n";
	}

	// tris dans le sens trigo en partant par le bas des intersections sur la bbox
	sort(origin_bottom.begin(), origin_bottom.end(),
	[](const DCEL_HalfEdge * a, const DCEL_HalfEdge * b) -> bool {
		return a->_origin->_coords.x< b->_origin->_coords.x;
	});

	sort(origin_right.begin(), origin_right.end(),
	[](const DCEL_HalfEdge * a, const DCEL_HalfEdge * b) -> bool {
		return a->_origin->_coords.y< b->_origin->_coords.y;
	});

	sort(origin_top.begin(), origin_top.end(),
	[](const DCEL_HalfEdge * a, const DCEL_HalfEdge * b) -> bool {
		return a->_origin->_coords.x> b->_origin->_coords.x;
	});

	sort(origin_left.begin(), origin_left.end(),
	[](const DCEL_HalfEdge * a, const DCEL_HalfEdge * b) -> bool {
		return a->_origin->_coords.y> b->_origin->_coords.y;
	});

	// ------------------------------------------------------------------------------------------------------------------------
	if (verbose) {
		std::cout << "DCEL::add_bbox phase 4 : ajouts des edges sur la bbox\n";
	}
	DCEL_HalfEdge * bbox_he= NULL;
	DCEL_HalfEdge * last_he= NULL;
	DCEL_HalfEdge * first_he= NULL;

	std::vector<DCEL_HalfEdge * > unbounded_edges;

	if (!origin_bottom.empty()) {
		if (bottom_left_corner== origin_bottom[0]->_origin) {
			first_he= origin_bottom[0];
		}
		else {
			bbox_he= add_edge(bottom_left_corner, origin_bottom[0]->_origin);
			unbounded_edges.push_back(bbox_he->_twin);
			first_he= bbox_he;
			bbox_he->set_next(origin_bottom[0]);
		}

		for (int i=0; i<int(origin_bottom.size())- 1; ++i) {
			DCEL_HalfEdge * cut_edge= cut_face(origin_bottom[i]->_twin->_next, origin_bottom[i+ 1]);
			if (cut_edge!= NULL) {
				unbounded_edges.push_back(cut_edge->_twin);
			}
		}

		if (origin_bottom[origin_bottom.size()- 1]->_origin== bottom_right_corner) {
			bbox_he= origin_bottom[origin_bottom.size()- 1]->_twin;
		}
		else {
			bbox_he= add_edge(origin_bottom[origin_bottom.size()- 1]->_origin, bottom_right_corner);
			origin_bottom[origin_bottom.size()- 1]->_twin->set_next(bbox_he);
			unbounded_edges.push_back(bbox_he->_twin);
		}
	}
	else {
		bbox_he= add_edge(bottom_left_corner, bottom_right_corner);
		first_he= bbox_he;
		unbounded_edges.push_back(bbox_he->_twin);
	}
	last_he= bbox_he;

	if (!origin_right.empty()) {
		if (bottom_right_corner== origin_right[0]->_origin) {
			last_he->set_next(origin_right[0]);
		}
		else {
			bbox_he= add_edge(bottom_right_corner, origin_right[0]->_origin);
			unbounded_edges.push_back(bbox_he->_twin);
			last_he->set_next(bbox_he);
			bbox_he->set_next(origin_right[0]);
		}

		for (int i=0; i<int(origin_right.size())- 1; ++i) {
			DCEL_HalfEdge * cut_edge= cut_face(origin_right[i]->_twin->_next, origin_right[i+ 1]);
			if (cut_edge!= NULL) {
				unbounded_edges.push_back(cut_edge->_twin);
			}
		}

		if (origin_right[origin_right.size()- 1]->_origin== top_right_corner) {
			bbox_he= origin_right[origin_right.size()- 1]->_twin;
		}
		else {
			bbox_he= add_edge(origin_right[origin_right.size()- 1]->_origin, top_right_corner);
			origin_right[origin_right.size()- 1]->_twin->set_next(bbox_he);
			unbounded_edges.push_back(bbox_he->_twin);
		}
	}
	else {
		bbox_he= add_edge(bottom_right_corner, top_right_corner);
		last_he->set_next(bbox_he);
		unbounded_edges.push_back(bbox_he->_twin);
	}
	last_he= bbox_he;
	
	if (!origin_top.empty()) {
		if (top_right_corner== origin_top[0]->_origin) {
			last_he->set_next(origin_top[0]);
		}
		else {
			bbox_he= add_edge(top_right_corner, origin_top[0]->_origin);
			unbounded_edges.push_back(bbox_he->_twin);
			last_he->set_next(bbox_he);
			bbox_he->set_next(origin_top[0]);
		}

		for (int i=0; i<int(origin_top.size())- 1; ++i) {
			DCEL_HalfEdge * cut_edge= cut_face(origin_top[i]->_twin->_next, origin_top[i+ 1]);
			if (cut_edge!= NULL) {
				unbounded_edges.push_back(cut_edge->_twin);
			}
		}

		if (origin_top[origin_top.size()- 1]->_origin== top_left_corner) {
			bbox_he= origin_top[origin_top.size()- 1]->_twin;
		}
		else {
			bbox_he= add_edge(origin_top[origin_top.size()- 1]->_origin, top_left_corner);
			origin_top[origin_top.size()- 1]->_twin->set_next(bbox_he);
			unbounded_edges.push_back(bbox_he->_twin);
		}
	}
	else {
		bbox_he= add_edge(top_right_corner, top_left_corner);
		last_he->set_next(bbox_he);
		unbounded_edges.push_back(bbox_he->_twin);
	}
	last_he= bbox_he;
	
	if (!origin_left.empty()) {
		if (top_left_corner== origin_left[0]->_origin) {
			last_he->set_next(origin_left[0]);
		}
		else {
			bbox_he= add_edge(top_left_corner, origin_left[0]->_origin);
			unbounded_edges.push_back(bbox_he->_twin);
			last_he->set_next(bbox_he);
			bbox_he->set_next(origin_left[0]);
		}

		for (int i=0; i<int(origin_left.size())- 1; ++i) {
			DCEL_HalfEdge * cut_edge= cut_face(origin_left[i]->_twin->_next, origin_left[i+ 1]);
			if (cut_edge!= NULL) {
				unbounded_edges.push_back(cut_edge->_twin);
			}
		}

		if (origin_left[origin_left.size()- 1]->_origin== bottom_left_corner) {
			bbox_he= origin_left[origin_left.size()- 1]->_twin;
		}
		else {
			bbox_he= add_edge(origin_left[origin_left.size()- 1]->_origin, bottom_left_corner);
			origin_left[origin_left.size()- 1]->_twin->set_next(bbox_he);
			unbounded_edges.push_back(bbox_he->_twin);
		}
	}
	else {
		bbox_he= add_edge(top_left_corner, bottom_left_corner);
		last_he->set_next(bbox_he);
		unbounded_edges.push_back(bbox_he->_twin);
	}
	last_he= bbox_he;
	
	// on clot la boucle
	last_he->set_next(first_he);

	// ----------------------------------------------------------------------------------------------------------------
	if (verbose) {
		std::cout << "DCEL::add_bbox phase 5 : unbounded edges\n";
	}
	for (int i=0; i<int(unbounded_edges.size())- 1; ++i) {
		unbounded_edges[i]->set_previous(unbounded_edges[i+ 1]);
	}
	unbounded_edges[unbounded_edges.size()- 1]->set_previous(unbounded_edges[0]);

	// ----------------------------------------------------------------------------------------------------------------
	if (verbose) {
		std::cout << "DCEL::add_bbox phase 6 : creation des faces\n";
	}

	create_faces_from_half_edges();
}


// est-ce que le DCEL s'autointersecte
bool DCEL::is_valid() {
	bool verbose= true;

	const float EPS= 1e-7;
	glm::vec2 inter;
	
	for (auto he1 : _half_edges) {
		for (auto he2 : _half_edges) {
			if (he1== he2) {
				continue;
			}
			if ((he1->_origin== NULL) || (he2->_origin== NULL)) {
				continue;
			}
			DCEL_Vertex * he1_destination= he1->destination();
			DCEL_Vertex * he2_destination= he2->destination();
			if ((he1_destination== NULL) || (he2_destination== NULL)) {
				continue;
			}
			if (segment_intersects_segment(he1->_origin->_coords, he1_destination->_coords,	he2->_origin->_coords, he2_destination->_coords, &inter, true, true)) {
				if (
					glm::distance2(he1->_origin->_coords, inter)< EPS || glm::distance2(he2->_origin->_coords, inter)< EPS ||
					glm::distance2(he1_destination->_coords, inter)< EPS || glm::distance2(he2_destination->_coords, inter)< EPS
				) {
					if (verbose) {
						std::cout << "DCEL::is_valid autointersection : " << *he1 << " || " << *he2 << "\n";
					}
					return false;
				}
			}
		}
	}
	return true;
}


// import à partir d'un string du type "0,0 -> 1,0 -> 0,1 | 1,0 -> 1,3 -> 0,1"
void DCEL::import(std::string s) {
	bool verbose= false;

	const float EPS= 1e-5;

	// suppression des espaces
	s.erase(std::remove(s.begin(), s.end(), ' '), s.end());

	std::vector<std::string> polys= split(s, "|");
	for (auto poly : polys) {
		std::vector<std::string> pts= split(poly, "->");
		DCEL_Vertex * first_v= NULL;
		DCEL_Vertex * last_v= NULL;
		DCEL_HalfEdge * first_e= NULL;
		DCEL_HalfEdge * last_e= NULL;
		for (auto pt : pts) {
			std::vector<std::string> coords= split(pt, ",");
			float x= std::stof(coords[0]);
			float y= std::stof(coords[1]);
			DCEL_Vertex * v= NULL;
			for (auto vv : _vertices) {
				if ((vv->_coords.x- x)* (vv->_coords.x- x)+ (vv->_coords.y- y)* (vv->_coords.y- y)< EPS) {
					if (verbose) {
						std::cout << "import : found vertex : " << "x=" << x << " ; y=" << y << "\n";
					}
					v= vv;
					break;
				}
			}
			if (v== NULL) {
				if (verbose) {
					std::cout << "import : new vertex : " << "x=" << x << " ; y=" << y << "\n";
				}
				v= add_vertex(glm::vec2(x, y));
			}
			if (first_v== NULL) {
				first_v= v;
			}
			if (last_v!= NULL) {
				DCEL_HalfEdge * e= NULL;
				for (auto edge : _half_edges) {
					if (edge->_origin== last_v && edge->destination()== v) {
						e= edge;
						if (verbose) {
							std::cout << "import : found edge : " << last_v->_coords.x << " ; " << last_v->_coords.y << " -> " << v->_coords.x << " ; " << v->_coords.y << "\n";
						}
						break;
					}
				}
				if (e== NULL) {
					if (verbose) {
						std::cout << "import : new edge : " << last_v->_coords.x << " ; " << last_v->_coords.y << " -> " << v->_coords.x << " ; " << v->_coords.y << "\n";
					}
					e= add_edge(last_v, v);
				}
				if (first_e== NULL) {
					first_e= e;
				}
				if (last_e!= NULL) {
					if (verbose) {
						std::cout << "import : " << *last_e << " set_next " << *e << "\n";
					}
					last_e->set_next(e);
				}
				last_e= e;
			}
			last_v= v;
		}

		DCEL_HalfEdge * e= NULL;
		for (auto edge : _half_edges) {
			if (edge->_origin== last_v && edge->destination()== first_v) {
				e= edge;
				if (verbose) {
					std::cout << "import : found last edge : " << last_v->_coords.x << " ; " << last_v->_coords.y << " -> " << first_v->_coords.x << " ; " << first_v->_coords.y << "\n";
				}
				break;
			}
		}
		if (e== NULL) {
			if (verbose) {
				std::cout << "import : new last edge : " << last_v->_coords.x << " ; " << last_v->_coords.y << " -> " << first_v->_coords.x << " ; " << first_v->_coords.y << "\n";
			}
			e= add_edge(last_v, first_v);
		}
		if (verbose) {
			std::cout << "import : " << *last_e << " set_next " << *e << "\n";
		}
		last_e->set_next(e);
		if (verbose) {
			std::cout << "import : " << *e << " set_next " << *first_e << "\n";
		}
		e->set_next(first_e);
	}

	create_nexts_from_twins();
	create_faces_from_half_edges();
}


// renvoie si existe le vertex à coords
DCEL_Vertex * DCEL::get_vertex(const glm::vec2 & coords) {
	const float EPS= 1e-9;
	for (auto v : _vertices) {
		if (glm::distance2(v->_coords, coords)< EPS) {
			return v;
		}
	}
	return NULL;
}


// renvoie si possible le edge allant de ori à dst
DCEL_HalfEdge * DCEL::get_edge(const glm::vec2 & ori, const glm::vec2 & dst) {
	DCEL_Vertex * origin= get_vertex(ori);
	DCEL_Vertex * destination= get_vertex(dst);
	if (origin== NULL || destination== NULL) {
		return NULL;
	}
	for (auto edge : _half_edges) {
		if (edge->_origin== origin && edge->destination()== destination) {
			return edge;
		}
	}
	return NULL;
}


// calcul des limites de vertices
void DCEL::get_bbox(glm::vec2 * bbox_min, glm::vec2 * bbox_max) {
	bbox_min->x= 1e8; bbox_min->y= 1e8; bbox_max->x= -1e8; bbox_max->y= -1e8;
	for (auto v : _vertices) {
		if (v->_coords.x< bbox_min->x) {
			bbox_min->x= v->_coords.x;
		}
		if (v->_coords.x> bbox_max->x) {
			bbox_max->x= v->_coords.x;
		}
		if (v->_coords.y< bbox_min->y) {
			bbox_min->y= v->_coords.y;
		}
		if (v->_coords.y> bbox_max->y) {
			bbox_max->y= v->_coords.y;
		}
	}
}


// export sous forme de html
void DCEL::export_html(std::string html_path, bool simple, const glm::vec2 & bbox_min, const glm::vec2 & bbox_max, const std::vector<glm::vec2> & sites) {
	bool verbose= false;

	const unsigned int SVG_WIDTH= 1200;
	const unsigned int SVG_HEIGHT= 700;
	const float MARGIN_FACTOR= 1.5f;
	const glm::vec2 VIEW_MIN= (bbox_min- 0.5f* (bbox_min+ bbox_max))* MARGIN_FACTOR+ 0.5f* (bbox_min+ bbox_max);
	const glm::vec2 VIEW_MAX= (bbox_max- 0.5f* (bbox_min+ bbox_max))* MARGIN_FACTOR+ 0.5f* (bbox_min+ bbox_max);
	const float SIZE= std::max((VIEW_MAX- VIEW_MIN).x, (VIEW_MAX- VIEW_MIN).y);
	const float POINT_RADIUS= 0.002f* SIZE;
	const float SITE_POINT_RADIUS= 0.003f* SIZE;
	const float DELTA_BUFFER= 0.006f* SIZE;
	const float ARROW_SIZE= 0.02f* SIZE;
	const float ARROW_SIZE_2= 0.005f* SIZE;
	const float STROKE_WIDTH= 0.001f* SIZE;

	auto y_html= [VIEW_MIN, VIEW_MAX](float y) -> float {return VIEW_MIN.y+ VIEW_MAX.y- y;};

	std::ofstream f;
	f.open(html_path);
	f << "<!DOCTYPE html>\n<html>\n<head>\n";
	f << "<style>\n";
	f << ".point_class {fill: red;}\n";
	f << ".site_point_class {fill: black;}\n";
	f << ".line_class {fill: transparent; stroke: black; stroke-width: " << STROKE_WIDTH << "; stroke-opacity: 0.6;}\n";
	f << ".repere_line_class {fill: transparent; stroke: red; stroke-width: " << STROKE_WIDTH << "; stroke-opacity: 0.2;}\n";
	f << ".arrow_line_class {fill: transparent; stroke: rgb(100,100,100); stroke-width: " << STROKE_WIDTH << "; stroke-opacity: 0.6;}\n";
	f << "</style>\n</head>\n<body>\n";
	f << "<svg width=\"" << SVG_WIDTH << "\" height=\"" << SVG_HEIGHT << "\" ";
	f << "viewbox=\"" << VIEW_MIN.x << " " << VIEW_MIN.y << " " << VIEW_MAX.x- VIEW_MIN.x << " " << VIEW_MAX.y- VIEW_MIN.y << "\" ";
	f << "style=\"background-color:rgb(220,240,230)\"";
	f << "\">\n";

	// repère
	if (!simple) {
		f << "<line class=\"repere_line_class\" x1=\"" << 0 << "\" y1=\"" << y_html(0) << "\" x2=\"" << 1 << "\" y2=\"" << y_html(0) << "\" />\n";
		f << "<line class=\"repere_line_class\" x1=\"" << 0 << "\" y1=\"" << y_html(0) << "\" x2=\"" << 0 << "\" y2=\"" << y_html(1) << "\" />\n";
	}

	// les sommets
	for (auto v : _vertices) {
		f << "<circle class=\"point_class\" cx=\"" << v->_coords.x << "\" cy=\"" << y_html(v->_coords.y) << "\" r=\"" << POINT_RADIUS << "\" />\n";
	}

	for (auto face : _faces) {
		if (verbose) {
			std::cout << "DCEL::export_html : " << *face << "\n";
		}

		std::string str_color;
		if (!simple) {
			str_color= "rgb("+ std::to_string(rand_int(0, 120))+ ", "+ std::to_string(rand_int(0, 120))+ ", "+ std::to_string(rand_int(0, 120))+ ")";
		}
		else {
			str_color= "rgb(100, 100, 100)";
		}

		// cas simple : on ignore les inner_edges
		if (simple) {
			for (auto edge : face->get_outer_edges()) {
				float x1= edge->_origin->_coords.x;
				float y1= edge->_origin->_coords.y;
				float x2= edge->destination()->_coords.x;
				float y2= edge->destination()->_coords.y;
				f << "<line class=\"line_class\" x1=\"" << x1 << "\" y1=\"" << y_html(y1) << "\" x2=\"" << x2 << "\" y2=\"" << y_html(y2) << "\" style=\"stroke:" << str_color << "\" />\n";
			}
			continue;
		}
		
		// cas complexe : on applique un buffer pour séparer les twins
		// on concatène le groupe des outer_edges avec les groupes des inner_edges
		std::vector<std::vector<DCEL_HalfEdge *> > edges_groups= face->get_inner_edges();
		edges_groups.push_back(face->get_outer_edges());
		std::vector<std::vector<glm::vec4> > vecs;
		for (int idx_group=0; idx_group<edges_groups.size(); ++idx_group) {
			vecs.push_back(std::vector<glm::vec4>{});
	
			for (int i=0; i<edges_groups[idx_group].size(); ++i) {
				// on considère 2 edges consécutifs pour trouver l'intersection des edges avec application de buffer
				DCEL_Vertex * v1= edges_groups[idx_group][i]->_origin;
				DCEL_Vertex * v2= edges_groups[idx_group][i]->destination();
				DCEL_Vertex * v3;
				if (i< edges_groups[idx_group].size()- 1) {
					v3= edges_groups[idx_group][i+ 1]->destination();
				}
				else {
					v3= edges_groups[idx_group][0]->destination();
				}

				float x1= v1->_coords.x;
				float y1= v1->_coords.y;
				float x2= v2->_coords.x;
				float y2= v2->_coords.y;
				float x3= v2->_coords.x;
				float y3= v2->_coords.y;
				float x4= v3->_coords.x;
				float y4= v3->_coords.y;
				float norm12= sqrt((x2- x1)* (x2- x1)+ (y2- y1)* (y2- y1));
				float u12= (x2- x1)/ norm12;
				float v12= (y2- y1)/ norm12;
				float norm34= sqrt((x4- x3)* (x4- x3)+ (y4- y3)* (y4- y3));
				float u34= (x4- x3)/ norm34;
				float v34= (y4- y3)/ norm34;

				// on applique le buffer aux positions
				x1-= DELTA_BUFFER* v12;
				y1+= DELTA_BUFFER* u12;
				x2-= DELTA_BUFFER* v12;
				y2+= DELTA_BUFFER* u12;
				x3-= DELTA_BUFFER* v34;
				y3+= DELTA_BUFFER* u34;
				x4-= DELTA_BUFFER* v34;
				y4+= DELTA_BUFFER* u34;

				// on cherche l'intersection des 2 edges avec buffer appliqué
				glm::vec2 inter;
				if (!ray_intersects_ray(
					glm::vec2(x1, y1), glm::vec2(u12, v12),
					glm::vec2(x4, y4), -glm::vec2(u34, v34),
					&inter
				)) {
					//std::cerr << "ERR : DCEL::export_html : ray_intersects_ray false\n";
					// ca peut arriver si 2 edges consécutifs sont alignés
					inter.x= x2;
					inter.y= y2;
				}

				// on stocke dans vecs l'intersection et la direction qui part de cette intersection
				vecs[idx_group].push_back(glm::vec4(inter.x, inter.y, u34, v34));
			}
		}

		for (auto vec : vecs) {
			for (int i=0; i<vec.size(); ++i) {
				float x1= vec[i][0];
				float y1= vec[i][1];
				float u= vec[i][2];
				float v= vec[i][3];
				float x2, y2;
				if (i< vec.size()- 1) {
					x2= vec[i+ 1][0];
					y2= vec[i+ 1][1];
				}
				else {
					x2= vec[0][0];
					y2= vec[0][1];
				}

				// fleche
				float x_arrow_1= 0.5f* (x1+ x2)+ 0.5f* ARROW_SIZE* u;
				float y_arrow_1= 0.5f* (y1+ y2)+ 0.5f* ARROW_SIZE* v;
				float x_arrow_2= x_arrow_1- ARROW_SIZE* u- ARROW_SIZE_2* v;
				float y_arrow_2= y_arrow_1- ARROW_SIZE* v+ ARROW_SIZE_2* u;
				float x_arrow_3= x_arrow_1- ARROW_SIZE* u+ ARROW_SIZE_2* v;
				float y_arrow_3= y_arrow_1- ARROW_SIZE* v- ARROW_SIZE_2* u;
				f << "<line class=\"arrow_line_class\" x1=\"" << x_arrow_1 << "\" y1=\"" << y_html(y_arrow_1) << "\" x2=\"" << x_arrow_2 << "\" y2=\"" << y_html(y_arrow_2) << "\" style=\"stroke:" << str_color << "\" />\n";
				f << "<line class=\"arrow_line_class\" x1=\"" << x_arrow_2 << "\" y1=\"" << y_html(y_arrow_2) << "\" x2=\"" << x_arrow_3 << "\" y2=\"" << y_html(y_arrow_3) << "\" style=\"stroke:" << str_color << "\" />\n";
				f << "<line class=\"arrow_line_class\" x1=\"" << x_arrow_3 << "\" y1=\"" << y_html(y_arrow_3) << "\" x2=\"" << x_arrow_1 << "\" y2=\"" << y_html(y_arrow_1) << "\" style=\"stroke:" << str_color << "\" />\n";

				f << "<line class=\"line_class\" x1=\"" << x1 << "\" y1=\"" << y_html(y1) << "\" x2=\"" << x2 << "\" y2=\"" << y_html(y2) << "\" style=\"stroke:" << str_color << "\" />\n";
			}
		}
	}

	// on affiche les sites s'il y en a
	for (auto & site : sites) {
		f << "<circle class=\"site_point_class\" cx=\""+ std::to_string(site.x)+ "\" cy=\""+ std::to_string(y_html(site.y))+ "\" r=\""+ std::to_string(SITE_POINT_RADIUS)+ "\" />\n";
	}

	f << "</svg>\n";
	f << "</body>\n</html>\n";
	f.close();
}


std::ostream & operator << (std::ostream & os, DCEL & d) {
	os << "********************************************\n";
	os << "n_vertices = " << d._vertices.size() << " ; n_half_edges = " << d._half_edges.size() << " ; n_faces = " << d._faces.size() << "\n";
	os << "faces ============================\n";

	for (auto face : d._faces) {
		if (face!= d._faces.front()) {
			os << "----------------------\n";
		}
		os << *face;
	}

	os << "edges ============================\n";
	for (auto he : d._half_edges) {
		os << *he;
		if (he->_previous!= NULL) {
			os << " | previous = " << *he->_previous;
		}
		if (he->_next!= NULL) {
			os << " | next = " << *he->_next;
		}
		os << "\n";
	}

	os << "vertices ============================\n";
	for (auto v : d._vertices) {
		os << *v << "\n";
	}

	os << "********************************************\n";

	return os;
}

