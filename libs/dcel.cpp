#include <algorithm>
#include <cctype>
#include <fstream>
#include <cmath>
#include <tuple>

#include "dcel.h"
#include "utile.h"

#include <glm/gtx/norm.hpp>


// ----------------------------------------------------------------------
DCEL_Vertex::DCEL_Vertex() : _coords(glm::vec2(0.0f)), _incident_edge(NULL), _delete_mark(false) {

}


DCEL_Vertex::DCEL_Vertex(const glm::vec2 & coords) : _coords(coords), _incident_edge(NULL), _delete_mark(false) {

}


DCEL_Vertex::~DCEL_Vertex() {

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
_origin(NULL), _twin(NULL), _next(NULL), _previous(NULL), _incident_face(NULL), _tmp_direction(glm::vec2(0.0f)), _tmp_position(glm::vec2(0.0f)), _delete_mark(false)
{

}


DCEL_HalfEdge::~DCEL_HalfEdge() {

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
	// si this est l incident edge de v il faut chercher un autre hedge pour devenir le nouveau incident edge de v
	if (_origin!= NULL && _origin->_incident_edge== this) {
		std::vector<DCEL_HalfEdge * > edges= _origin->get_incident_edges();
		if (edges.size()> 1) {
			_origin->_incident_edge= edges[1];
		}
		else {
			_origin->_incident_edge= NULL;
		}
	}
	_origin= v;
}


// maj des données temporaires
void DCEL_HalfEdge::set_tmp_data(const glm::vec2 & direction, const glm::vec2 & position) {
	_tmp_direction= direction;
	_tmp_position= position;
	if (_twin!= NULL) {
		_twin->_tmp_direction= -_tmp_direction;
		_twin->_tmp_position= -_tmp_position;
	}
}


void DCEL_HalfEdge::set_tmp_data(const glm::vec2 & direction) {
	_tmp_direction= direction;
	if (_twin!= NULL) {
		_twin->_tmp_direction= -_tmp_direction;
	}
}


void DCEL_HalfEdge::set_tmp_data() {
	if (_origin== NULL || destination()== NULL) {
		std::cerr << "ERR : DCEL_HalfEdge::set_tmp_data() impossible !\n";
		return;
	}
	_tmp_direction= destination()->_coords- _origin->_coords;
	if (_twin!= NULL) {
		_twin->_tmp_direction= -_tmp_direction;
	}
}


std::ostream & operator << (std::ostream & os, DCEL_HalfEdge & e) {
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
	//os << " ; dx = " << e._dx << " ; dy = " << e._dy << " ; _tmp_x = " << e._tmp_x << " ; _tmp_y = " << e._tmp_y;
	//os << " ; previous = " << e._previous << " ; next = " << e._next;

	return os;
}


// ----------------------------------------------------------------------
DCEL_Face::DCEL_Face() : _outer_edge(NULL), _unbounded(false), _delete_mark(false) {

}


DCEL_Face::~DCEL_Face() {

}


// renvoie la liste des vertices entourant la face
std::vector<DCEL_Vertex *> DCEL_Face::get_vertices() {
	std::vector<DCEL_Vertex *> result;

	std::vector<DCEL_HalfEdge *> edges= get_outer_edges();
	for (auto edge : edges) {
		if (edge->_origin== NULL) {
			std::cerr << "ERR : DCEL_Face::get_vertices() : edge->_origin== NULL\n";
			continue;
		}
		result.push_back(edge->_origin);
	}

	return result;
}


// renvoie la liste des outer edges de la face
std::vector<DCEL_HalfEdge *> DCEL_Face::get_outer_edges() {
	if (_unbounded) {
		return std::vector<DCEL_HalfEdge *>{};
	}

	std::vector<DCEL_HalfEdge *> result;
	DCEL_HalfEdge * first_edge= _outer_edge;
	DCEL_HalfEdge * current_edge= _outer_edge;
	do {
		if (current_edge== NULL) {
			std::cerr << "ERR : DCEL_Face::get_outer_edges() : current_edge== NULL\n";
			break;
		}
		result.push_back(current_edge);
		current_edge= current_edge->_next;
	} while (current_edge!= first_edge);

	return result;
}


// renvoie la liste des inner edges de la face
std::vector<DCEL_HalfEdge *> DCEL_Face::get_inner_edges() {
	std::vector<DCEL_HalfEdge *> result;
	// on concatène tous les inner edges
	for (auto inner_edge : _inner_edges) {
		DCEL_HalfEdge * first_edge= inner_edge;
		DCEL_HalfEdge * current_edge= inner_edge;
		do {
			if (current_edge== NULL) {
				std::cerr << "ERR : DCEL_Face::get_inner_edges() : current_edge== NULL\n";
				break;
			}
			result.push_back(current_edge);
			current_edge= current_edge->_next;
		} while (current_edge!= first_edge);
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
	std::vector<glm::vec2> pts;
	for (auto v : get_vertices()) {
		pts.push_back(v->_coords);
	}
	return is_ccw(pts);
}


std::ostream & operator << (std::ostream & os, DCEL_Face & f) {
	os << "face _unbounded = " << f._unbounded << "\n";
	if (!f._unbounded) {
		os << "outer_edge = " << *f._outer_edge << "\n";
		std::vector<DCEL_HalfEdge *> outer_edges= f.get_outer_edges();
		os << "outer edges.size = " << outer_edges.size() << "\n";
		for (auto edge : outer_edges) {
			os << *edge;
			if (edge!= outer_edges.back()) {
				os << " | ";
			}
		}
	}
	else {
		os << "inner_edges.size = " << f._inner_edges.size() << "\n";
		std::vector<DCEL_HalfEdge *> inner_edges= f.get_inner_edges();
		os << "inner_edges.size (total) = " << inner_edges.size() << "\n";
		for (auto edge : inner_edges) {
			os << *edge;
			if (edge!= inner_edges.back()) {
				os << " | ";
			}
		}
	}
	return os;
}


// ----------------------------------------------------------------------
DCEL::DCEL() {
	// on crée dès le départ la face infinie
	_unbounded_face= add_face();
	_unbounded_face->_unbounded= true;
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
	DCEL_Vertex * v= new DCEL_Vertex(coords);
	_vertices.push_back(v);
	return v;
}


// ajout de 2 hedges
DCEL_HalfEdge * DCEL::add_edge(DCEL_Vertex * v1, DCEL_Vertex * v2) {
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


// ajout d'une face
DCEL_Face * DCEL::add_face(DCEL_HalfEdge * outer_edge) {
	DCEL_Face * face= new DCEL_Face();
	if (outer_edge!= NULL) {
		face->_outer_edge= outer_edge;
		if (outer_edge->_incident_face!= NULL) {
			std::cerr << "ERR : DCEL::add_face : outer_edge->_incident_face!= NULL\n";
		}
	}
	_faces.push_back(face);
	return face;
}


// suppression d'un sommet
void DCEL::delete_vertex(DCEL_Vertex * v) {
	if (v== NULL) {
		std::cerr << "ERR : DCEL::delete_vertex  : v== NULL\n";
		return;
	}

	std::vector<DCEL_HalfEdge *> edges= v->get_incident_edges();
	std::for_each(edges.begin(), edges.end(), [this](DCEL_HalfEdge * e){delete_edge(e);});
	v->_delete_mark= true;
}


// suppression d'un edge, ie de 2 half-edges
void DCEL::delete_edge(DCEL_HalfEdge * he) {
	if (he== NULL) {
		std::cerr << "ERR : DCEL::delete_edge NULL\n";
		return;
	}

	// si he ou he->_twin délimite la face infinie on cherche le edge représentant he parmi _inner_edges et on le supprime
	if (he->_incident_face== _unbounded_face) {
		for (auto edge : he->_incident_face->_inner_edges) {
			if (edge->_twin->_incident_face== he->_twin->_incident_face) {
				he->_incident_face->_inner_edges.erase(std::remove(he->_incident_face->_inner_edges.begin(), he->_incident_face->_inner_edges.end(), edge), he->_incident_face->_inner_edges.end());
				break;
			}
		}
	}
	if (he->_twin->_incident_face== _unbounded_face) {
		for (auto edge : he->_twin->_incident_face->_inner_edges) {
			if (edge->_twin->_incident_face== he->_incident_face) {
				he->_twin->_incident_face->_inner_edges.erase(std::remove(he->_twin->_incident_face->_inner_edges.begin(), he->_twin->_incident_face->_inner_edges.end(), edge), he->_twin->_incident_face->_inner_edges.end());
				break;
			}
		}
	}

	// si he ou he->_twin délimite une face finie on supprime cette face et on assigne NULL au _incident_face de tous les edges de cette face
	if ((he->_incident_face!= NULL) && (he->_incident_face!= _unbounded_face)) {
		delete_face_without_edges(he->_incident_face);
	}
	if ((he->_twin->_incident_face!= NULL) && (he->_twin->_incident_face!= _unbounded_face)) {
		delete_face_without_edges(he->_twin->_incident_face);
	}

	// mise à NULL des extrémités
	he->set_origin(NULL);
	he->_twin->set_origin(NULL);
	
	/*std::cout << "he= " << *he << "\n";
	std::cout << "he->_previous= " << *he->_previous << "\n";
	std::cout << "he->_next= " << *he->_next << "\n";
	std::cout << "he->_twin= " << *he->_twin << "\n";
	std::cout << "he->_twin->_previous= " << *he->_twin->_previous << "\n";
	std::cout << "he->_twin->_next= " << *he->_twin->_next << "\n";*/

	// on s'occupe de he->_previous et he->_twin->_previous
	he->_previous->set_next(he->_twin->_next);
	he->_twin->_previous->set_next(he->_next);

	// on supprime he et he->_twin
	he->_delete_mark= true;
	he->_twin->_delete_mark= true;
}


// a mieux faire, ne gère pas les trous ...
void DCEL::delete_face(DCEL_Face * face) {
	if (face== NULL) {
		std::cerr << "ERR : DCEL::delete_face NULL\n";
		return;
	}

	if (face->_unbounded) {
		std::cerr << "ERR : DCEL::delete_face : on ne peut pas supprimer la face infinie\n";
		return;
	}

	for (auto edge : _half_edges) {
		if (edge->_incident_face== face) {
			if (edge->_twin->_incident_face== _unbounded_face) {
				delete_edge(edge);
			}
			else {
				edge->_incident_face= NULL;
			}
			break;
		}
	}
	
	face->_delete_mark= true;
}


// suppression d'une face en laissant les edges intacts
void DCEL::delete_face_without_edges(DCEL_Face * face) {
	if (face== NULL) {
		std::cerr << "ERR : DCEL::delete_face_without_edges NULL\n";
		return;
	}
	
	if (face->_unbounded) {
		std::cerr << "ERR : DCEL::delete_face_without_edges : on ne peut pas supprimer la face infinie\n";
		return;
	}

	face->_delete_mark= true;

	std::vector<DCEL_HalfEdge *> outer_edges= face->get_outer_edges();
	for (auto edge : outer_edges) {
		edge->_incident_face= NULL;
	}
	std::vector<DCEL_HalfEdge *> inner_edges= face->get_inner_edges();
	for (auto edge : inner_edges) {
		edge->_incident_face= NULL;
	}
}


// suppression de tout
void DCEL::clear() {
	_vertices.clear();
	_half_edges.clear();
	_faces.clear();
}


/*void DCEL::clear_unbounded_face() {
	for (auto edge : _unbounded_face->get_edges()) {
		edge->_incident_face= NULL;
	}
	_unbounded_face->_inner_edges.clear();
}*/


void DCEL::clear_next_equals_twin_edges() {
	while (true) {
		bool found_bad_edge= false;
		for (auto edge : _half_edges) {
			if (edge->_twin== edge->_next && !edge->_delete_mark) {
				found_bad_edge= true;
				delete_edge(edge);
			}
		}
		if (!found_bad_edge) {
			break;
		}
	}
}


void DCEL::clear_unconnected_vertices() {
	bool verbose= false;

	for (auto v : _vertices) {
		if (verbose) {
			std::cout << "DCEL::clear_unconnected_vertices : v = " << *v << "\n";
		}
		std::vector<DCEL_HalfEdge * > edges= v->get_incident_edges();
		bool connected= false;
		for (auto e : edges) {
			if (e->_delete_mark== false) {
				connected= true;
				break;
			}
		}
		if (!connected) {
			if (verbose) {
				std::cout << "DCEL::clear_unconnected_vertices : v->_delete_mark= true : " << *v << "\n";
			}
			v->_delete_mark= true;
		}
	}
}


bool DCEL::create_nexts_from_twins() {
	// on cherche les hedges qui n'ont pas de next
	while (true) {
		bool found= false;
		for (auto he : _half_edges) {
			if (he->_next== NULL) {
				found= true;
				DCEL_HalfEdge * first_he= he;
				DCEL_HalfEdge * current_he= he;
				DCEL_HalfEdge * next_he= NULL;
				do {
					if (current_he->_twin->_previous== NULL) {
						std::cerr << "ERR : DCEL::create_nexts_from_twins : current_he->_twin->_previous== NULL : current_he = " << *current_he << " ; twin = " << *current_he->_twin << "\n";
						return false;
					}
					next_he= current_he->_twin->_previous->_twin;
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
	return true;
}


// création des faces à partir des hedges qui n'ont pas de face associée
bool DCEL::create_faces_from_half_edges() {
	bool verbose= true;

	bool found_unattached_he= false;
	
	while (true) {
		found_unattached_he= false;
		for (auto he : _half_edges) {
			if (he->_incident_face== NULL) {
				if (verbose) {
					std::cout << "DCEL::create_faces_from_half_edges : he->_incident_face== NULL : " << *he << "\n";
				}
				found_unattached_he= true;
				DCEL_Face * face= add_face(he);
				DCEL_HalfEdge * he2= he;
				do {
					if (he2->_next== NULL) {
						std::cerr << "ERR : DCEL::create_faces_from_half_edges : face non close\n";
						return false;
					}
					he2->_incident_face= face;
					he2= he2->_next;
				} while (he2!= he);
			}
		}
		if (!found_unattached_he) {
			break;
		}
	}
	return true;
}


// gestion trous et face infinie
void DCEL::check_ccw_faces() {
	bool verbose= true;
	
	for (auto f : _faces) {
		f->_inner_edges.clear();
	}

	for (auto f : _faces) {
		if (f== _unbounded_face) {
			continue;
		}

		if (!f->ccw()) {
			if (verbose) {
				std::cout << "DCEL::check_ccw_faces face ccw =\n" << *f << "\n";
			}
			bool is_hole= false;
			Polygon2D * fpoly= f->get_polygon();
			for (auto f2 : _faces) {
				if (f2== _unbounded_face || f2== f || !f2->ccw()) {
					continue;
				}
				Polygon2D * fpoly2= f2->get_polygon();
				if (is_poly_inside_poly(fpoly, fpoly2)) {
					if (verbose) {
						std::cout << "DCEL::check_ccw_faces face inside :\n" << *f << "\n";
					}
					is_hole= true;
					f2->_inner_edges.push_back(f->_outer_edge);
					break;
				}
			}
			// trou de la face infinie
			if (!is_hole) {
				if (verbose) {
					std::cout << "DCEL::check_ccw_faces face inside infinite face:\n";
				}
				_unbounded_face->_inner_edges.push_back(f->_outer_edge);
			}
			
			f->_delete_mark= true;
		}
	}
}


// suppression des _delete_mark à True
void DCEL::delete_markeds() {
	_vertices.erase(std::remove_if(_vertices.begin(), _vertices.end(), [](DCEL_Vertex * v){return v->_delete_mark== true;}), _vertices.end());
	_half_edges.erase(std::remove_if(_half_edges.begin(), _half_edges.end(), [](DCEL_HalfEdge * e){return e->_delete_mark== true;}), _half_edges.end());
	_faces.erase(std::remove_if(_faces.begin(), _faces.end(), [](DCEL_Face * f){return f->_delete_mark== true;}), _faces.end());
}


// on rend valide le DCEL après l'avoir modifié avec des ajouts ou des suppressions
void DCEL::make_valid() {
	bool verbose= true;

	// on renseigne les hedges qui n'ont pas de next à partir de leurs twins
	if (verbose) {
		std::cout << "make_valid : create_nexts_from_twins\n";
	}
	create_nexts_from_twins();

	// on supprime les edges e où e->_next == e->_twin qui ont pu être créés par delete_edge
	if (verbose) {
		std::cout << "make_valid : clear_next_equals_twin_edges\n";
	}
	clear_next_equals_twin_edges();

	// on supprime les sommets qui ne sont connectés à aucun edge
	if (verbose) {
		std::cout << "make_valid : clear_unconnected_vertices\n";
	}
	clear_unconnected_vertices();
	
	// on vide la face infinie
	/*if (verbose) {
		std::cout << "make_valid : clear_unbounded_face\n";
	}
	clear_unbounded_face();*/

	// suppression réelle, jusqu'ici on ne faisait que _delete_mark= true
	if (verbose) {
		std::cout << "make_valid : delete_markeds\n";
	}
	delete_markeds();

	// on crée les faces des edges qui n'ont pas encore de face
	if (verbose) {
		std::cout << "make_valid : create_faces_from_half_edges\n";
	}
	create_faces_from_half_edges();

	if (verbose) {
		std::cout << "make_valid : check_ccw_faces\n";
	}
	check_ccw_faces();

	if (verbose) {
		std::cout << "make_valid : FIN : n_vertices = " << _vertices.size() << " ; n_edges = " << _half_edges.size() << " ; n_faces = " << _faces.size() << "\n";
	}
}


// est-ce vide
bool DCEL::is_empty() {
	return _vertices.empty() && _half_edges.empty();
}


// ajout d'une bounding box qui peut en fait rogner sur le DCEL
bool DCEL::add_bbox(const glm::vec2 & bbox_min, const glm::vec2 & bbox_max) {
	bool verbose= false;

	// si vide on crée un carré de taille 1
	if (is_empty()) {
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
		return true;
	}

	// un vertex est-il dans une emprise
	auto in_bbox= [bbox_min, bbox_max](DCEL_Vertex * v) -> bool{
		if ((v->_coords.x< bbox_min.x) || (v->_coords.x> bbox_max.x) || (v->_coords.y< bbox_min.y) || (v->_coords.y> bbox_max.y)) {
			return false;
		}
		return true;
	};

	// la nouvelle face infinie va devenir la bbox, donc on reinit l'ancienne mais on garde les liens previous / next
	for (auto edge : _unbounded_face->get_inner_edges()) {
		edge->_incident_face= NULL;
	}
	_unbounded_face->_inner_edges.clear();

	// ------------------------------------------------------------------------------------------------------------------------
	// phase 1 : on supprime les hedges et vertices en dehors de la bbox et on transforme en demi-droite ceux qui ont 1 des 2
	// extremités en dehors
	for (auto he : _half_edges) {
		if (he->_origin!= NULL && he->destination()!= NULL && !in_bbox(he->_origin)) {
			if (verbose) {
				std::cout << "DCEL::add_bbox phase1 : he = " << *he << "\n";
			}
			// on transforme en 1/2 droite
			if (in_bbox(he->destination())) {
				if (verbose) {
					std::cout << "DCEL::add_bbox phase1 : dst in box\n";
				}
				he->set_tmp_data();
				delete_face_without_edges(he->_incident_face);
				if (he->_previous!= NULL) {
					he->_previous->set_tmp_data();
					he->set_previous(NULL);
				}
				he->set_origin(NULL);
			}
			else {
				std::vector<glm::vec2> result;
				Polygon2D * poly= new Polygon2D();
				poly->set_rectangle(bbox_min, bbox_max- bbox_min);
				// le segment a ses extrémités en dehors de la bbox mais intersecte la bbox
				if (segment_intersects_poly_multi(he->_origin->_coords, he->destination()->_coords, poly, &result) && result.size()> 1) {
					if (verbose) {
						std::cout << "DCEL::add_bbox phase1 : intersection\n";
					}
					// on prend comme centre temporaire le milieu des 2 intersections avec la bbox
					he->set_tmp_data(he->destination()->_coords- he->_origin->_coords, 0.5f* (result[0]+ result[1]));

					if (he->_previous!= NULL) {
						he->_previous->set_tmp_data();
						he->set_previous(NULL);
					}
					he->set_origin(NULL);

				// pas d'intersection
				} else {
					if (verbose) {
						std::cout << "add_bbox phase1 : delete_edge\n";
					}
					delete_edge(he);
				}
			}
		}
	}

	// ------------------------------------------------------------------------------------------------------------------------
	// phase 2 : création des nouveaux sommets intersections des droites et 1/2 droites avec la bbox
	// ajout des sommets de la bbox
	DCEL_Vertex * bottom_left_corner= add_vertex(bbox_min);
	DCEL_Vertex * bottom_right_corner= add_vertex(glm::vec2(bbox_max.x, bbox_min.y));
	DCEL_Vertex * top_left_corner= add_vertex(glm::vec2(bbox_min.x, bbox_max.y));
	DCEL_Vertex * top_right_corner= add_vertex(bbox_max);

	std::vector<DCEL_HalfEdge *> origin_top;
	std::vector<DCEL_HalfEdge *> origin_bottom;
	std::vector<DCEL_HalfEdge *> origin_left;
	std::vector<DCEL_HalfEdge *> origin_right;
	glm::vec2 inter;
	bool is_inter;

	// boucle sur les edges qui n'ont pas d'origine mais une destination et cherche l'intersection du twin avec la bbox
	for (auto he : _half_edges) {
		if ((he->_origin== NULL) && (he->_twin->_origin!= NULL)) {
			if (verbose) {
				std::cout << "DCEL::add_bbox phase2 : he = " << *he << "\n";
			}

			is_inter= ray_intersects_segment(he->_twin->_origin->_coords, he->_twin->_tmp_direction,
				bottom_left_corner->_coords, bottom_right_corner->_coords,
				&inter);
			if (is_inter) {
				he->_origin= add_vertex(inter);
				origin_bottom.push_back(he);
				continue;
			}

			is_inter= ray_intersects_segment(he->_twin->_origin->_coords, he->_twin->_tmp_direction,
				bottom_right_corner->_coords, top_right_corner->_coords,
				&inter);
			if (is_inter) {
				he->_origin= add_vertex(inter);
				origin_right.push_back(he);
				continue;
			}

			is_inter= ray_intersects_segment(he->_twin->_origin->_coords, he->_twin->_tmp_direction,
				top_right_corner->_coords, top_left_corner->_coords,
				&inter);
			if (is_inter) {
				he->_origin= add_vertex(inter);
				origin_top.push_back(he);
				continue;
			}

			is_inter= ray_intersects_segment(he->_twin->_origin->_coords, he->_twin->_tmp_direction,
				top_left_corner->_coords, bottom_left_corner->_coords,
				&inter);
			if (is_inter) {
				he->_origin= add_vertex(inter);
				origin_left.push_back(he);
				continue;
			}

			std::cerr<< "ERR : DCEL::add_bbox phase2 pas d'intersection trouvée he = " << *he << "\n";
			return false;
		}
	}

	// boucle sur les edges qui n'ont ni origine ni destination et cherche les intersections de la droite dans les 2 sens avec la bbox
	for (auto he : _half_edges) {
		if ((he->_origin== NULL) && (he->_twin->_origin== NULL)) {
			is_inter= ray_intersects_segment(he->_tmp_position, he->_tmp_direction,
				bottom_left_corner->_coords, bottom_right_corner->_coords,
				&inter);
			if (is_inter) {
				he->_twin->_origin= add_vertex(inter);
				origin_bottom.push_back(he->_twin);
			}
			is_inter= ray_intersects_segment(he->_tmp_position, he->_twin->_tmp_direction,
				bottom_left_corner->_coords, bottom_right_corner->_coords,
				&inter);
			if (is_inter) {
				he->_origin= add_vertex(inter);
				origin_bottom.push_back(he);
			}

			is_inter= ray_intersects_segment(he->_tmp_position, he->_tmp_direction,
				bottom_right_corner->_coords, top_right_corner->_coords,
				&inter);
			if (is_inter) {
				he->_twin->_origin= add_vertex(inter);
				origin_right.push_back(he->_twin);
			}
			is_inter= ray_intersects_segment(he->_tmp_position, he->_twin->_tmp_direction,
				bottom_right_corner->_coords, top_right_corner->_coords,
				&inter);
			if (is_inter) {
				he->_origin= add_vertex(inter);
				origin_right.push_back(he);
			}

			is_inter= ray_intersects_segment(he->_tmp_position, he->_tmp_direction,
				top_right_corner->_coords, top_left_corner->_coords,
				&inter);
			if (is_inter) {
				he->_twin->_origin= add_vertex(inter);
				origin_top.push_back(he->_twin);
			}
			is_inter= ray_intersects_segment(he->_tmp_position, he->_twin->_tmp_direction,
				top_right_corner->_coords, top_left_corner->_coords,
				&inter);
			if (is_inter) {
				he->_origin= add_vertex(inter);
				origin_top.push_back(he);
			}

			is_inter= ray_intersects_segment(he->_tmp_position, he->_tmp_direction,
				top_left_corner->_coords, bottom_left_corner->_coords,
				&inter);
			if (is_inter) {
				he->_twin->_origin= add_vertex(inter);
				origin_left.push_back(he->_twin);
			}
			is_inter= ray_intersects_segment(he->_tmp_position, he->_twin->_tmp_direction,
				top_left_corner->_coords, bottom_left_corner->_coords,
				&inter);
			if (is_inter) {
				he->_origin= add_vertex(inter);
				origin_left.push_back(he);
			}
		}
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

	DCEL_HalfEdge * bbox_he= NULL;
	DCEL_HalfEdge * last_he= NULL;
	DCEL_HalfEdge * first_he= NULL;

	// ------------------------------------------------------------------------------------------------------------------------
	// phase 3 : ajout des edges sur la bbox
	for (int i=0; i<int(origin_bottom.size())- 1; ++i) {
		bbox_he= add_edge(origin_bottom[i]->_origin, origin_bottom[i+ 1]->_origin);
		origin_bottom[i]->_twin->set_next(bbox_he);
		bbox_he->set_next(origin_bottom[i+ 1]);
	}
	for (int i=0; i<int(origin_top.size())- 1; ++i) {
		bbox_he= add_edge(origin_top[i]->_origin, origin_top[i+ 1]->_origin);
		origin_top[i]->_twin->set_next(bbox_he);
		bbox_he->set_next(origin_top[i+ 1]);
	}
	for (int i=0; i<int(origin_right.size())- 1; ++i) {
		bbox_he= add_edge(origin_right[i]->_origin, origin_right[i+ 1]->_origin);
		origin_right[i]->_twin->set_next(bbox_he);
		bbox_he->set_next(origin_right[i+ 1]);
	}
	for (int i=0; i<int(origin_left.size())- 1; ++i) {
		bbox_he= add_edge(origin_left[i]->_origin, origin_left[i+ 1]->_origin);
		origin_left[i]->_twin->set_next(bbox_he);
		bbox_he->set_next(origin_left[i+ 1]);
	}

	// ajouts des edges entre les coins de la bbox et les segments de bbox déjà ajoutés
	if (!origin_bottom.empty()) {
		bbox_he= add_edge(bottom_left_corner, origin_bottom[0]->_origin);
		//last_he->set_next(bbox_he); // ici non last_he==NULL
		first_he= bbox_he;
		bbox_he->set_next(origin_bottom[0]);
		bbox_he= add_edge(origin_bottom[origin_bottom.size()- 1]->_origin, bottom_right_corner);
		origin_bottom[origin_bottom.size()- 1]->_twin->set_next(bbox_he);
	}
	else {
		bbox_he= add_edge(bottom_left_corner, bottom_right_corner);
		//last_he->set_next(bbox_he); // ici non last_he==NULL
		first_he= bbox_he;
	}
	last_he= bbox_he;
	
	if (!origin_right.empty()) {
		bbox_he= add_edge(bottom_right_corner, origin_right[0]->_origin);
		last_he->set_next(bbox_he);
		bbox_he->set_next(origin_right[0]);
		bbox_he= add_edge(origin_right[origin_right.size()- 1]->_origin, top_right_corner);
		origin_right[origin_right.size()- 1]->_twin->set_next(bbox_he);
	}
	else {
		bbox_he= add_edge(bottom_right_corner, top_right_corner);
		last_he->set_next(bbox_he);
	}
	last_he= bbox_he;
	
	if (!origin_top.empty()) {
		bbox_he= add_edge(top_right_corner, origin_top[0]->_origin);
		last_he->set_next(bbox_he);
		bbox_he->set_next(origin_top[0]);
		bbox_he= add_edge(origin_top[origin_top.size()- 1]->_origin, top_left_corner);
		origin_top[origin_top.size()- 1]->_twin->set_next(bbox_he);
	}
	else {
		bbox_he= add_edge(top_right_corner, top_left_corner);
		last_he->set_next(bbox_he);
	}
	last_he= bbox_he;
	
	if (!origin_left.empty()) {
		bbox_he= add_edge(top_left_corner, origin_left[0]->_origin);
		last_he->set_next(bbox_he);
		bbox_he->set_next(origin_left[0]);
		bbox_he= add_edge(origin_left[origin_left.size()- 1]->_origin, bottom_left_corner);
		origin_left[origin_left.size()- 1]->_twin->set_next(bbox_he);
	}
	else {
		bbox_he= add_edge(top_left_corner, bottom_left_corner);
		last_he->set_next(bbox_he);
	}
	last_he= bbox_he;
	
	// on clos la boucle
	last_he->set_next(first_he);

	return true;
}


// est-ce que le DCEL s'autointersecte
bool DCEL::is_valid() {
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
					std::cout << "DCEL not_valid : " << *he1 << " || " << *he2 << "\n";
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
}


// renvoie si existe le vertex à coords
DCEL_Vertex * DCEL::get_vertex(const glm::vec2 & coords) {
	const float EPS= 1e-7;
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
void DCEL::bbox(glm::vec2 * bbox_min, glm::vec2 * bbox_max) {
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
	const unsigned int SVG_WIDTH= 1200;
	const unsigned int SVG_HEIGHT= 700;
	const float MARGIN_FACTOR= 1.5f;
	const glm::vec2 VIEW_MIN= (bbox_min- 0.5f* (bbox_min+ bbox_max))* MARGIN_FACTOR+ 0.5f* (bbox_min+ bbox_max);
	const glm::vec2 VIEW_MAX= (bbox_max- 0.5f* (bbox_min+ bbox_max))* MARGIN_FACTOR+ 0.5f* (bbox_min+ bbox_max);
	// (xmin- 0.5f* (xmin+ xmax))* MARGIN_FACTOR+ 0.5f* (xmin+ xmax);
	/*const float VIEW_YMIN= (ymin- 0.5f* (ymin+ ymax))* MARGIN_FACTOR+ 0.5f* (ymin+ ymax);
	const float VIEW_XMAX= (xmax- 0.5f* (xmin+ xmax))* MARGIN_FACTOR+ 0.5f* (xmin+ xmax);
	const float VIEW_YMAX= (ymax- 0.5f* (ymin+ ymax))* MARGIN_FACTOR+ 0.5f* (ymin+ ymax);*/
	const float SIZE= std::max((VIEW_MAX- VIEW_MIN).x, (VIEW_MAX- VIEW_MIN).y);
	const float POINT_RADIUS= 0.002f* SIZE;
	const float SITE_POINT_RADIUS= 0.003f* SIZE;
	const float DELTA_BUFFER= 0.01f* SIZE;
	const float ARROW_SIZE= 0.05f* SIZE;
	const float ARROW_SIZE_2= 0.01f* SIZE;
	const float STROKE_WIDTH= 0.002f* SIZE;

	auto y_html= [VIEW_MIN, VIEW_MAX](float y) -> float {return VIEW_MIN.y+ VIEW_MAX.y- y;};

	std::ofstream f;
	f.open(html_path);
	f << "<!DOCTYPE html>\n<html>\n<head>\n";
	f << "<style>\n";
	f << ".point_class {fill: red;}\n";
	f << ".site_point_class {fill: black;}\n";
	f << ".line_class {fill: transparent; stroke: black; stroke-width: " << STROKE_WIDTH << "; stroke-opacity: 0.6;}\n";
	f << ".repere_line_class {fill: transparent; stroke: red; stroke-width: " << STROKE_WIDTH << "; stroke-opacity: 0.6;}\n";
	f << ".arrow_line_class {fill: transparent; stroke: rgb(100,100,100); stroke-width: " << STROKE_WIDTH << "; stroke-opacity: 0.6;}\n";
	f << "</style>\n</head>\n<body>\n";
	f << "<svg width=\"" << SVG_WIDTH << "\" height=\"" << SVG_HEIGHT << "\" ";
	// viewbox = xmin, ymin, width, height
	f << "viewbox=\"" << VIEW_MIN.x << " " << VIEW_MIN.y << " " << VIEW_MAX.x- VIEW_MIN.x << " " << VIEW_MAX.y- VIEW_MIN.y << "\" ";
	f << "style=\"background-color:rgb(220,240,230)\"";
	f << "\">\n";

	// repère
	if (!simple) {
		f << "<line class=\"repere_line_class\" x1=\"" << 0 << "\" y1=\"" << y_html(0) << "\" x2=\"" << 1 << "\" y2=\"" << y_html(0) << "\" />\n";
		f << "<line class=\"repere_line_class\" x1=\"" << 0 << "\" y1=\"" << y_html(0) << "\" x2=\"" << 0 << "\" y2=\"" << y_html(1) << "\" />\n";
	}

	for (auto face : _faces) {
		std::vector<DCEL_HalfEdge *> edges= face->get_outer_edges();
		std::vector<DCEL_HalfEdge *> inner_edges= face->get_inner_edges();
		edges.insert(edges.end(), inner_edges.begin(), inner_edges.end());

		std::string str_color;
		if (!simple) {
			str_color= "rgb("+ std::to_string(rand_int(0, 120))+ ", "+ std::to_string(rand_int(0, 120))+ ", "+ std::to_string(rand_int(0, 120))+ ")";
		}
		else {
			str_color= "rgb(100, 100, 100)";
		}
		
		for (auto edge : edges) {
			DCEL_Vertex * v1= edge->_origin;
			DCEL_Vertex * v2= edge->destination();

			float x1= v1->_coords.x;
			float y1= v1->_coords.y;
			float x2= v2->_coords.x;
			float y2= v2->_coords.y;

			// buffer inversé vers centre de gravité (moche mais bon)
			if (!simple) {
				glm::vec2 gravity_center= face->get_gravity_center();
				float xg= gravity_center.x;
				float yg= gravity_center.y;
				float d1= sqrt((xg- x1)* (xg- x1)+ (yg- y1)* (yg- y1));
				float d2= sqrt((xg- x2)* (xg- x2)+ (yg- y2)* (yg- y2));
				if (!face->_unbounded) {
					if (d1> 1e-3) {
						x1+= DELTA_BUFFER* (xg- x1)/ d1;
						y1+= DELTA_BUFFER* (yg- y1)/ d1;
					}
					if (d2> 1e-3) {
						x2+= DELTA_BUFFER* (xg- x2)/ d2;
						y2+= DELTA_BUFFER* (yg- y2)/ d2;
					}
				}
				else {
					if (d1> 1e-3) {
						x1-= DELTA_BUFFER* (xg- x1)/ d1;
						y1-= DELTA_BUFFER* (yg- y1)/ d1;
					}
					if (d2> 1e-3) {
						x2-= DELTA_BUFFER* (xg- x2)/ d2;
						y2-= DELTA_BUFFER* (yg- y2)/ d2;
					}
				}
			}

			f << "<circle class=\"point_class\" cx=\"" << x1 << "\" cy=\"" << y_html(y1) << "\" r=\"" << POINT_RADIUS << "\" />\n";
			f << "<circle class=\"point_class\" cx=\"" << x2 << "\" cy=\"" << y_html(y2) << "\" r=\"" << POINT_RADIUS << "\" />\n";
			f << "<line class=\"line_class\" x1=\"" << x1 << "\" y1=\"" << y_html(y1) << "\" x2=\"" << x2 << "\" y2=\"" << y_html(y2) << "\" style=\"stroke:" << str_color << "\" />\n";

			// fleche
			if (!simple) {
				float u= (x2- x1)/ sqrt((x2- x1)* (x2- x1)+ (y2- y1)* (y2- y1));
				float v= (y2- y1)/ sqrt((x2- x1)* (x2- x1)+ (y2- y1)* (y2- y1));
				float x_arrow_1= 0.5f* (x1+ x2)+ 0.5f* ARROW_SIZE* u;
				float y_arrow_1= 0.5f* (y1+ y2)+ 0.5f* ARROW_SIZE* v;
				float x_arrow_2= x_arrow_1- ARROW_SIZE* u- ARROW_SIZE_2* v;
				float y_arrow_2= y_arrow_1- ARROW_SIZE* v+ ARROW_SIZE_2* u;
				float x_arrow_3= x_arrow_1- ARROW_SIZE* u+ ARROW_SIZE_2* v;
				float y_arrow_3= y_arrow_1- ARROW_SIZE* v- ARROW_SIZE_2* u;
				f << "<line class=\"arrow_line_class\" x1=\"" << x_arrow_1 << "\" y1=\"" << y_html(y_arrow_1) << "\" x2=\"" << x_arrow_2 << "\" y2=\"" << y_html(y_arrow_2) << "\" style=\"stroke:" << str_color << "\" />\n";
				f << "<line class=\"arrow_line_class\" x1=\"" << x_arrow_2 << "\" y1=\"" << y_html(y_arrow_2) << "\" x2=\"" << x_arrow_3 << "\" y2=\"" << y_html(y_arrow_3) << "\" style=\"stroke:" << str_color << "\" />\n";
				f << "<line class=\"arrow_line_class\" x1=\"" << x_arrow_3 << "\" y1=\"" << y_html(y_arrow_3) << "\" x2=\"" << x_arrow_1 << "\" y2=\"" << y_html(y_arrow_1) << "\" style=\"stroke:" << str_color << "\" />\n";
			}
		}
	}

	for (auto & site : sites) {
		f << "<circle class=\"site_point_class\" cx=\""+ std::to_string(site.x)+ "\" cy=\""+ std::to_string(y_html(site.y))+ "\" r=\""+ std::to_string(SITE_POINT_RADIUS)+ "\" />\n";
	}

	f << "</svg>\n";
	f << "</body>\n</html>\n";
	f.close();
}


std::ostream & operator << (std::ostream & os, DCEL & d) {
	os << "DCEL ***************************************\n";
	os << "faces ============================\n";
	for (auto face : d._faces) {
		os << "----------------------\n";
		os << *face << "\n";
	}
	os << "edges ============================\n";
	for (auto he : d._half_edges) {
		os << *he << "\n";
	}
	os << "********************************************\n";

	return os;
}

