// https://www.newcastle.edu.au/__data/assets/pdf_file/0019/22519/23_A-fast-algortithm-for-generating-constrained-Delaunay-triangulations.pdf


#include <cmath>
#include <iostream>
#include <algorithm>
#include <deque>
#include <fstream>
#include <sstream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "triangulation.h"
#include "geom_2d.h"


using namespace std;



// --------------------------------------------------
// un triangle stocke les indices des sommets et des pointeurs vers les triangles adjacents
// _vertices_init est utile pour retrouver les indices initiaux après tri par bin
Triangle::Triangle() {
	_vertices[0]= -1;
	_vertices[1]= -1;
	_vertices[2]= -1;
	_vertices_init[0]= -1;
	_vertices_init[1]= -1;
	_vertices_init[2]= -1;
	_adjacents[0]= 0;
	_adjacents[1]= 0;
	_adjacents[2]= 0;
}


Triangle::Triangle(int v0, int v1, int v2, Triangle * a0, Triangle * a1, Triangle * a2) {
	_vertices[0]= v0;
	_vertices[1]= v1;
	_vertices[2]= v2;
	_vertices_init[0]= -1;
	_vertices_init[1]= -1;
	_vertices_init[2]= -1;
	_adjacents[0]= a0;
	_adjacents[1]= a1;
	_adjacents[2]= a2;
}


Triangle::~Triangle() {
	
}

/*
bool Triangle::is_valid() {
	if ((_vertices[0]< 0) || (_vertices[1]< 0) || (_vertices[2]< 0)) {
		return false;
	}
	return true;
}
*/

// --------------------------------------------------
// PointBin représente un sommet du graphe ; pt_init est le point donné en argument, pt est normalisé, idx_init est l'indice dans la liste initiale et idx_bin l'indice du bin
// contenant le point ; _triangles est la liste des triangles qui ont ce point pour sommet
PointBin::PointBin() {

}


PointBin::PointBin(pt_2d pt_init, pt_2d pt, int idx_init, int idx_bin) :
	_pt_init(pt_init), _pt(pt), _idx_init(idx_init), _idx_bin(idx_bin) {
	
}


PointBin::~PointBin() {
	
}


// --------------------------------------------------
// fonctions de tri par bin et par classement initial
bool sort_pts_by_idx_bin(PointBin * pt1, PointBin * pt2) {
	return pt1->_idx_bin< pt2->_idx_bin;
}


bool sort_pts_by_idx_init(PointBin * pt1, PointBin * pt2) {
	return pt1->_idx_init< pt2->_idx_init;
}


// ----------------------------------------------------
// une Opposition représente 2 triangles adjacents et les indices de l'edge commun pour ces 2 triangles
Opposition::Opposition() {

}


Opposition::Opposition(Triangle * triangle_1, Triangle * triangle_2) :
	_triangle_1(triangle_1), _triangle_2(triangle_2), _is_valid(true), _edge_idx_1(-1), _edge_idx_2(-1) {
	
	if ((!_triangle_1) || (!_triangle_2)) {
		_is_valid= false;
		return;
	}

	for (unsigned int i=0; i<3; ++i) {
		if (_triangle_1->_adjacents[i]== _triangle_2) {
			_edge_idx_1= i;
			break;
		}
	}
	if (_edge_idx_1== -1) {
		_is_valid= false;
		return;
	}

	for (unsigned int i=0; i<3; ++i) {
		if (_triangle_2->_adjacents[i]== _triangle_1) {
			_edge_idx_2= i;
			break;
		}
	}
	if (_edge_idx_2== -1) {
		_is_valid= false;
		return;
	}
}


Opposition::~Opposition() {

}


// --------------------------------------------------
// un ConstrainedEdge est un edge qui doit être présent dans la triangulation finale
ConstrainedEdge::ConstrainedEdge() {

}


ConstrainedEdge::ConstrainedEdge(std::pair<unsigned int, unsigned int> idx_init, std::pair<unsigned int, unsigned int> idx) :
	_idx_init(idx_init), _idx(idx) 
{

}


ConstrainedEdge::~ConstrainedEdge() {

}


// --------------------------------------------------
Triangulation::Triangulation() {

}


Triangulation::Triangulation(const vector<pt_2d> & pts, const vector<pair<unsigned int, unsigned int> > & constrained_edges, 
	bool clean_in_constrained_polygon, bool sort_by_bin, bool verbose) :
	_sort_by_bin(sort_by_bin), _verbose(verbose)
{
	// on redirige cout vers un fichier qui est affiché dans le log du html
	streambuf * coutbuf;
	if (_verbose) {
		ofstream out_stream("../data/out.txt");
		coutbuf= cout.rdbuf();
		cout.rdbuf(out_stream.rdbuf());
	}

	// init de _pts et _constrained_edges
	init(pts, constrained_edges);

	// ajout d'un triangle englobant
	add_large_triangle();

	// ajout un par un des points
	for (unsigned int idx_pt=0; idx_pt<_pts.size()- 3; ++idx_pt) {
		add_pt(idx_pt);
	}

	// remplissage de l'attribut PointBin._triangles
	set_idx_triangles();

	// ajout un par un des edges de contrainte
	for (unsigned int idx_edge=0; idx_edge<_constrained_edges.size(); ++idx_edge) {
		add_constrained_edge(idx_edge);
	}

	// suppression triangle englobant
	remove_large_triangle();

	// suppression des triangles contenus dans les polygones définis par les edges de contrainte
	if (clean_in_constrained_polygon) {
		clean_in_constrained_poly();
	}

	// tri dans l'ordre initial
	finish();

	// cout redevient cout
	if (_verbose) {
		cout.rdbuf(coutbuf);
	}
}


Triangulation::~Triangulation() {
	for (auto & pt : _pts) {
		delete pt;
	}
	_pts.clear();

	for (auto & t : _triangles) {
		delete t;
	}
	_triangles.clear();

	for (auto & constrained_edge : _constrained_edges) {
		delete constrained_edge;
	}
	_constrained_edges.clear();

	for (auto & bin : _bins) {
		delete bin;
	}
	_bins.clear();

	delete _aabb;
}


// initialisation de la liste des points et des constrained edges
void Triangulation::init(const vector<pt_2d> & pts, const vector<pair<unsigned int, unsigned int> > & constrained_edges) {
	if (_verbose) {
		cout << "\ninit\n";
	}

	number xmin= 1e8;
	number ymin= 1e8;
	number xmax= -1e8;
	number ymax= -1e8;
	for (auto pt : pts) {
		if (pt.x< xmin) {
			xmin= pt.x;
		}
		if (pt.y< ymin) {
			ymin= pt.y;
		}
		if (pt.x> xmax) {
			xmax= pt.x;
		}
		if (pt.y> ymax) {
			ymax= pt.y;
		}
	}

	// découpage en bins (paquets) de la liste de points
	// censé accélerer le traitement car en insérant le point suivant on n'est pas loin du point précédent
	unsigned int subdiv= (unsigned int)(pow(pts.size(), 0.25));
	number step= 1.0/ subdiv;
	for (unsigned int idx_bin=0; idx_bin< subdiv* subdiv; ++idx_bin) {
		unsigned int row= idx_bin/ subdiv;
		unsigned int col= idx_bin% subdiv;
		if (row% 2== 1) {
			col= subdiv- 1- col;
		}
		AABB_2D * aabb= new AABB_2D(pt_2d(col* step, row* step), pt_2d(step, step));
		if (_verbose) {
			cout << "bin=" << *aabb << "\n";
		}
		_bins.push_back(aabb);
	}

	_aabb= new AABB_2D(pt_2d(xmin, ymin), pt_2d(xmax- xmin, ymax- ymin));
	if (_verbose) {
		cout << "aabb=" << *_aabb << "\n";
	}
	
	number m= max(_aabb->_size.x, _aabb->_size.y);
	for (unsigned int i=0; i<pts.size(); ++i) {
		pt_2d normalized_pt((pts[i].x- _aabb->_pos.x)/ m, (pts[i].y- _aabb->_pos.y)/ m);
		int idx_bin_ok= -1;
		for (unsigned int idx_bin=0; idx_bin<_bins.size(); ++idx_bin) {
			if (point_in_aabb(normalized_pt, _bins[idx_bin])) {
				idx_bin_ok= idx_bin;
				break;
			}
		}
		if (idx_bin_ok== -1) {
			cerr << "ERREUR recherche bin\n";
			continue;
		}
		_pts.push_back(new PointBin(pts[i], normalized_pt, i, idx_bin_ok));
	}

	if (_sort_by_bin) {
		sort(_pts.begin(), _pts.end(), sort_pts_by_idx_bin);
	}

	for (auto constrained_edge : constrained_edges) {
		pair<unsigned int, unsigned int> sort_constrained_edge;
		
		if (_sort_by_bin) {
			bool is_first_found= false;
			bool is_second_found= false;
			for (unsigned int idx_pt=0; idx_pt<_pts.size(); ++idx_pt) {
				if (_pts[idx_pt]->_idx_init== constrained_edge.first) {
					sort_constrained_edge.first= idx_pt;
					is_first_found= true;
				}
				if (_pts[idx_pt]->_idx_init== constrained_edge.second) {
					sort_constrained_edge.second= idx_pt;
					is_second_found= true;
				}
				if ((is_first_found) && (is_second_found)) {
					break;
				}
			}
		}
		else {
			sort_constrained_edge.first= constrained_edge.first;
			sort_constrained_edge.second= constrained_edge.second;
		}
		_constrained_edges.push_back(new ConstrainedEdge(constrained_edge, sort_constrained_edge));
	}
}


// ajout d'un triangle englobant
void Triangulation::add_large_triangle() {
	if (_verbose) {
		cout << "\nadd_large_triangle\n";
	}

	_pts.push_back(new PointBin(pt_2d(-100.0f, -100.0f), pt_2d(-100.0f, -100.0f), -1, -1));
	_pts.push_back(new PointBin(pt_2d(100.0f, -100.0f), pt_2d(100.0f, -100.0f), -1, -1));
	_pts.push_back(new PointBin(pt_2d(0.0f, 100.0f), pt_2d(0.0f, 100.0f), -1, -1));

	Triangle * t= new Triangle();
	t->_vertices[0]= _pts.size()- 3;
	t->_vertices[1]= _pts.size()- 2;
	t->_vertices[2]= _pts.size()- 1;
	_triangles.push_back(t);
}


// renvoie le triangle contenant un point
Triangle * Triangulation::get_containing_triangle(unsigned int idx_pt) {
	if (_verbose) {
		cout << "get_containing_triangle\n";
	}

	// on part du dernier triangle ajouté car le tri par bin implique que le point courant ne doit pas être loin du point précédent
	Triangle * last_triangle= _triangles[_triangles.size()- 1];
	bool search_triangle= true;
	unsigned int compt= 0;
	bool search_triangle_ok= true;

	while (search_triangle) {
		compt++;
		// le code n'est pas robuste à ce niveau, il arrive que l'on ne trouve pas le triangle ; cf segment_intersects_segment
		if (compt> _triangles.size()) {
			search_triangle_ok= false;
			cerr << "Erreur search_triangle\n";
			break;
		}
		
		search_triangle= false;
		pt_2d bary= (_pts[last_triangle->_vertices[0]]->_pt+ _pts[last_triangle->_vertices[1]]->_pt+ _pts[last_triangle->_vertices[2]]->_pt)/ 3.0;
		
		if (_verbose) {
			cout << "bary=" << glm::to_string(bary) << "\n";
		}
		
		// pour chaque arête du triangle courant, si le segment [barycentre, point_a _inserer] intersecte l'arête on passe au triangle adjacent
		for (unsigned int i=0; i<3; ++i) {
			pt_2d result;
			if (segment_intersects_segment(bary, _pts[idx_pt]->_pt, _pts[last_triangle->_vertices[i]]->_pt, _pts[last_triangle->_vertices[(i+ 1)% 3]]->_pt, &result, true, false)) {
				last_triangle= last_triangle->_adjacents[i];
				if (!last_triangle) {
					search_triangle_ok= false;
					cerr << "ERREUR last_triangle NULL\n";
					break;
				}

				if (_verbose) {
					cout << "bary intersects\n";
					print_triangle(last_triangle);
				}
				
				search_triangle= true;
				break;
			}
		}
	}
	
	if (search_triangle_ok) {
		return last_triangle;
	}
	else {
		return 0;
	}
}


// suppression d'un triangle
void Triangulation::delete_triangle(Triangle * triangle, bool update_point_bin) {
	if (_verbose) {
		cout << "delete_triangle\n";
		print_triangle(triangle);
	}

	// pour chaque sommet du triangle on supprime la référence au triangle
	if (update_point_bin) {
		for (unsigned int i=0; i<3; ++i) {
			vector<Triangle *>::iterator it= find(_pts[triangle->_vertices[i]]->_triangles.begin(), _pts[triangle->_vertices[i]]->_triangles.end(), triangle);
			if (it!= _pts[triangle->_vertices[i]]->_triangles.end()) {
				_pts[triangle->_vertices[i]]->_triangles.erase(it);
			}
		}
	}

	vector<Triangle *>::iterator it= find(_triangles.begin(), _triangles.end(), triangle);
	if (it!= _triangles.end()) {
		_triangles.erase(it);
	}
	delete triangle;
	triangle= 0;
}


// insertion d'un triangle
void Triangulation::insert_triangle(Triangle * triangle, bool update_point_bin) {
	if (_verbose) {
		cout << "insert_triangle\n";
		print_triangle(triangle);
	}

	// maj des listes de triangles passant par les sommets du triangle a insérer
	if (update_point_bin) {
		for (unsigned int i=0; i<3; ++i) {
			_pts[triangle->_vertices[i]]->_triangles.push_back(triangle);
		}
	}
	_triangles.insert(_triangles.end(), triangle);
}


// met à jour les attributs de new_triangle_1 et new_triangle_2 pour qu'ils correspondent à opposition mais avec l'autre diagonale
void Triangulation::swap_triangle(Opposition * opposition, Triangle * new_triangle_1, Triangle * new_triangle_2) {
	if (_verbose) {
		cout << "swap_triangle\n";
		print_triangle(opposition->_triangle_1);
		print_triangle(opposition->_triangle_2);
	}

	Triangle * new_triangle_1_adjs[2]= {
		opposition->_triangle_2->_adjacents[(opposition->_edge_idx_2+ 2)% 3],
		opposition->_triangle_1->_adjacents[(opposition->_edge_idx_1+ 1) % 3]
	};
	
	new_triangle_1->_vertices[0]= opposition->_triangle_1->_vertices[(opposition->_edge_idx_1+ 2)% 3];
	new_triangle_1->_vertices[1]= opposition->_triangle_2->_vertices[(opposition->_edge_idx_2+ 2)% 3];
	new_triangle_1->_vertices[2]= opposition->_triangle_2->_vertices[opposition->_edge_idx_2];
	new_triangle_1->_adjacents[0]= 0;
	new_triangle_1->_adjacents[1]= new_triangle_1_adjs[0];
	new_triangle_1->_adjacents[2]= new_triangle_1_adjs[1];

	Triangle * new_triangle_2_adjs[2]= {
		opposition->_triangle_1->_adjacents[(opposition->_edge_idx_1+ 2) % 3],
		opposition->_triangle_2->_adjacents[(opposition->_edge_idx_2+ 1) % 3]
	};
	
	new_triangle_2->_vertices[0]= opposition->_triangle_2->_vertices[(opposition->_edge_idx_2+ 2)% 3];
	new_triangle_2->_vertices[1]= opposition->_triangle_1->_vertices[(opposition->_edge_idx_1+ 2) % 3];
	new_triangle_2->_vertices[2]= opposition->_triangle_1->_vertices[opposition->_edge_idx_1];
	new_triangle_2->_adjacents[0]= 0;
	new_triangle_2->_adjacents[1]= new_triangle_2_adjs[0];
	new_triangle_2->_adjacents[2]= new_triangle_2_adjs[1];

	new_triangle_1->_adjacents[0]= new_triangle_2;
	new_triangle_2->_adjacents[0]= new_triangle_1;

	for (unsigned int i=0; i<2; ++i) {
		if (new_triangle_1_adjs[i]) {
			for (unsigned int j=0; j<3; ++j) {
				if ((new_triangle_1_adjs[i]->_adjacents[j]== opposition->_triangle_1) || (new_triangle_1_adjs[i]->_adjacents[j]== opposition->_triangle_2)) {
					new_triangle_1_adjs[i]->_adjacents[j]= new_triangle_1;
					break;
				}
			}
		}
	}

	for (unsigned int i=0; i<2; ++i) {
		if (new_triangle_2_adjs[i]) {
			for (unsigned int j=0; j<3; ++j) {
				if ((new_triangle_2_adjs[i]->_adjacents[j]== opposition->_triangle_1) || (new_triangle_2_adjs[i]->_adjacents[j]== opposition->_triangle_2)) {
					new_triangle_2_adjs[i]->_adjacents[j]= new_triangle_2;
					break;
				}
			}
		}
	}
}


// ajout d'un point
void Triangulation::add_pt(unsigned int idx_pt) {
	if (_verbose) {
		cout << "\nadd_pt\n";
		cout << "pt=" << glm::to_string(_pts[idx_pt]->_pt) << "\n";
		cout << "pt_init=" << glm::to_string(_pts[idx_pt]->_pt_init) << "\n";
	}

	// récup du triangle contenant le point
	Triangle * containing_triangle= get_containing_triangle(idx_pt);
	if (!containing_triangle) {
		return;
	}

	// création des 3 triangles subdivisant le triangle initial
	Triangle * t1= new Triangle(containing_triangle->_vertices[0], containing_triangle->_vertices[1], idx_pt, containing_triangle->_adjacents[0], NULL, NULL);
	Triangle * t2= new Triangle(idx_pt, containing_triangle->_vertices[1], containing_triangle->_vertices[2], NULL, containing_triangle->_adjacents[1], NULL);
	Triangle * t3= new Triangle(containing_triangle->_vertices[2], containing_triangle->_vertices[0], idx_pt, containing_triangle->_adjacents[2], NULL, NULL);

	t1->_adjacents[1]= t2;
	t1->_adjacents[2]= t3;
	t2->_adjacents[0]= t1;
	t2->_adjacents[2]= t3;
	t3->_adjacents[1]= t1;
	t3->_adjacents[2]= t2;

	// Pour chaque nouveau triangle, si il existe un triangle adjacent, on met à jour l'adjacent de l'adjacent et on ajoute une Opposition
	Triangle * new_tris[3]= {t1, t2, t3};
	deque<Opposition *> opposition_deque;
	for (unsigned int i=0; i<3; ++i) {
		if (containing_triangle->_adjacents[i]) {
			for (unsigned int j=0; j<3; ++j) {
				if (containing_triangle->_adjacents[i]->_adjacents[j]== containing_triangle) {
					containing_triangle->_adjacents[i]->_adjacents[j]= new_tris[i];
					opposition_deque.push_back(new Opposition(new_tris[i], containing_triangle->_adjacents[i]));
					break;
				}
			}
		}
	}
	
	// suppression du triangle initial
	delete_triangle(containing_triangle);

	// insertion des nouveaux triangles
	insert_triangle(t1);
	insert_triangle(t2);
	insert_triangle(t3);
	
	// traitement de la queue d'Oppositions
	while (!opposition_deque.empty()) {
		Opposition * opposition= opposition_deque.back();
		opposition_deque.pop_back();
		if (!opposition->_is_valid) {
			cerr << "ERREUR opposition non valide\n";
			continue;
		}
		
		// si le nouveau point est contenu dans le cercle circonscrit au triangle de Opposition (triangle_2 qui est celui n'ayant pas le point pour sommet)
		// on change de diagonale et on ajoute à la queue les éventuelles nouvelles oppositions
		if (point_in_circumcircle(_pts[opposition->_triangle_2->_vertices[0]]->_pt, _pts[opposition->_triangle_2->_vertices[1]]->_pt, _pts[opposition->_triangle_2->_vertices[2]]->_pt, _pts[idx_pt]->_pt)) {
			Triangle * new_triangle_1= new Triangle();
			Triangle * new_triangle_2= new Triangle();

			swap_triangle(opposition, new_triangle_1, new_triangle_2);

			if (opposition->_triangle_2->_adjacents[(opposition->_edge_idx_2+ 1)% 3]) {
				opposition_deque.push_back(new Opposition(new_triangle_2, opposition->_triangle_2->_adjacents[(opposition->_edge_idx_2+ 1)% 3]));
			}
			if (opposition->_triangle_2->_adjacents[(opposition->_edge_idx_2+ 2)% 3]) {
				opposition_deque.push_back(new Opposition(new_triangle_1, opposition->_triangle_2->_adjacents[(opposition->_edge_idx_2+ 2)% 3]));
			}

			delete_triangle(opposition->_triangle_1);
			delete_triangle(opposition->_triangle_2);

			insert_triangle(new_triangle_1);
			insert_triangle(new_triangle_2);
		}
	}
}


// maj de la liste des triangles passant par chaque sommet
void Triangulation::set_idx_triangles() {
	if (_verbose) {
		cout << "\nset_idx_triangles\n";
	}

	for (auto triangle : _triangles) {
		for (unsigned int i=0; i<3; ++i) {
			_pts[triangle->_vertices[i]]->_triangles.push_back(triangle);
		}
	}
}


// renvoie l'opposition des 2 triangles définis par un edge commun
Opposition * Triangulation::opposition_from_edge(std::pair<unsigned int, unsigned int> edge) {
	unsigned int compt= 0;
	Triangle * triangle_1= 0;
	Triangle * triangle_2= 0;
	for (auto triangle : _pts[edge.first]->_triangles) {
		for (unsigned int i=0; i<3; ++i) {
			if (triangle->_vertices[i]== edge.second) {
				if (compt== 0) {
					triangle_1= triangle;
				}
				else {
					triangle_2= triangle;
				}
				compt++;
				break;
			}
		}
		if (compt> 1) {
			break;
		}
	}
	return new Opposition(triangle_1, triangle_2);
}


// ajout d'un edge de contraintes, ie on veut qu'il soit présent dans la triangulation finale
void Triangulation::add_constrained_edge(unsigned int idx_edge) {
	if (_verbose) {
		cout << "\nadd_constrained_edge\n";
		cout << _constrained_edges[idx_edge]->_idx_init.first << " ; " << _constrained_edges[idx_edge]->_idx_init.second << "\n";
	}

	std::pair<unsigned int, unsigned int> constrained_edge= _constrained_edges[idx_edge]->_idx;

	if (constrained_edge.first== constrained_edge.second) {
		return;
	}

	// si on trouve un triangle qui passe par les 2 points du edge c'est qu'il est déjà présent dans la triangluation
	for (auto t0 : _pts[constrained_edge.first]->_triangles) {
		for (auto t1 : _pts[constrained_edge.second]->_triangles) {
			if (t0== t1) {
				return;
			}
		}
	}

	if (_verbose) {
		cout << "get intersecting triangles\n";
	}

	// liste des edges intersectant le constrained_edge
	// on commence par chercher le triangle passant par le 1er point de constrained_edge qui a un edge intersectant
	deque<pair<unsigned int, unsigned int> > intersecting_edges;
	Triangle * intersecting_triangle;
	unsigned int intersecting_edge= 0;
	pt_2d pt1_begin= _pts[constrained_edge.first]->_pt;
	pt_2d pt1_end  = _pts[constrained_edge.second]->_pt;

	for (auto triangle : _pts[constrained_edge.first]->_triangles) {
		unsigned int idx_pt= 0;
		for (unsigned int i=0; i<3; ++i) {
			if (triangle->_vertices[i]== constrained_edge.first) {
				idx_pt= i;
				break;
			}
		}
		pt_2d pt2_begin= _pts[triangle->_vertices[(idx_pt+ 1)% 3]]->_pt;
		pt_2d pt2_end  = _pts[triangle->_vertices[(idx_pt+ 2)% 3]]->_pt;
		pt_2d result;

		if (segment_intersects_segment(pt1_begin, pt1_end, pt2_begin, pt2_end, &result)) {
			intersecting_triangle= triangle;
			intersecting_edge= (idx_pt+ 1)% 3;
			intersecting_edges.push_back(make_pair(triangle->_vertices[intersecting_edge], triangle->_vertices[(intersecting_edge+ 1)% 3]));
			break;
		}
	}

	// puis on va de triangle adjacent en triangle adjacent jusqu'a tomber sur constrained_edge.second
	while (true) {
		intersecting_triangle= intersecting_triangle->_adjacents[intersecting_edge];
		unsigned int idx_pt= 0;
		for (unsigned int i=0; i<3; ++i) {
			if (intersecting_triangle->_vertices[i]== intersecting_edges.back().first) {
				idx_pt= i;
				break;
			}
		}
		if (intersecting_triangle->_vertices[(idx_pt+ 1)% 3]== constrained_edge.second) {
			break;
		}
		
		pt_2d pt2_begin= _pts[intersecting_triangle->_vertices[idx_pt]]->_pt;
		pt_2d pt2_end  = _pts[intersecting_triangle->_vertices[(idx_pt+ 1)% 3]]->_pt;
		pt_2d result;

		if (segment_intersects_segment(pt1_begin, pt1_end, pt2_begin, pt2_end, &result)) {
			intersecting_edge= idx_pt;
		}
		else {
			intersecting_edge= (idx_pt+ 1)% 3;
		}
		
		intersecting_edges.push_back(make_pair(intersecting_triangle->_vertices[intersecting_edge], intersecting_triangle->_vertices[(intersecting_edge+ 1)% 3]));
	}

	if (_verbose) {
		cout << "get new edges\n";
	}

	// pour chaque edge intersectant si les 2 triangles ayant cet edge en commun forment un quadrilatère convexe
	// on swape la diagonale ; si cette nouvelle diagonale intersecte tjrs le constrained_edge on la remet dans la liste
	deque<pair<unsigned int, unsigned int> > new_edges;
	while (!intersecting_edges.empty()) {
		pair<unsigned int, unsigned int> intersecting_edge= intersecting_edges.front();
		intersecting_edges.pop_front();
		if (_verbose) {
			cout << "intersecting edge= " << intersecting_edge.first << " ; " << intersecting_edge.second << "\n";
		}

		Opposition * opposition= opposition_from_edge(intersecting_edge);

		pt_2d quad[4]= {
			_pts[opposition->_triangle_1->_vertices[(opposition->_edge_idx_1+ 2)% 3]]->_pt,
			_pts[opposition->_triangle_1->_vertices[opposition->_edge_idx_1]]->_pt,
			_pts[opposition->_triangle_2->_vertices[(opposition->_edge_idx_2+ 2)% 3]]->_pt,
			_pts[opposition->_triangle_2->_vertices[opposition->_edge_idx_2]]->_pt
		};

		if (!is_quad_convex(quad)) {
			if (_verbose) {
				cout << "non convex\n";
			}
			intersecting_edges.push_back(intersecting_edge);
			continue;
		}

		Triangle * new_triangle_1= new Triangle();
		Triangle * new_triangle_2= new Triangle();
		swap_triangle(opposition, new_triangle_1, new_triangle_2);
		unsigned int idx_pt_1= opposition->_triangle_1->_vertices[(opposition->_edge_idx_1+ 2) % 3];
		unsigned int idx_pt_2= opposition->_triangle_2->_vertices[(opposition->_edge_idx_2+ 2) % 3];
		delete_triangle(opposition->_triangle_1, true);
		delete_triangle(opposition->_triangle_2, true);
		insert_triangle(new_triangle_1, true);
		insert_triangle(new_triangle_2, true);

		pt_2d pt2_begin= _pts[idx_pt_1]->_pt;
		pt_2d pt2_end  = _pts[idx_pt_2]->_pt;
		pt_2d result;

		if (segment_intersects_segment(pt1_begin, pt1_end, pt2_begin, pt2_end, &result, true)) {
			intersecting_edges.push_back(make_pair(idx_pt_1, idx_pt_2));
			if (_verbose) {
				cout << "still intersects\n";
			}
		}
		else {
			new_edges.push_back(make_pair(idx_pt_1, idx_pt_2));
			if (_verbose) {
				cout << "add to new edges\n";
			}
		}
	}

	if (_verbose) {
		cout << "delaunay\n";
	}

	// pour chaque new_edge on cherche à rétablir l'aspect Delaunay, sauf s'il s'agit du constrained_edge
	while (!new_edges.empty()) {
		pair<unsigned int, unsigned int> new_edge= new_edges.front();
		new_edges.pop_front();
		
		if (_verbose) {
			cout << "new_edge= " << new_edge.first << " ; " << new_edge.second << "\n";
		}

		if (((new_edge.first== constrained_edge.first) && (new_edge.second== constrained_edge.second)) || ((new_edge.first== constrained_edge.second) && (new_edge.second== constrained_edge.first))) {
			continue;
		}

		Opposition * opposition= opposition_from_edge(new_edge);
		if (   (point_in_circumcircle(_pts[opposition->_triangle_1->_vertices[0]]->_pt, _pts[opposition->_triangle_1->_vertices[1]]->_pt, _pts[opposition->_triangle_1->_vertices[2]]->_pt, _pts[opposition->_triangle_2->_vertices[(opposition->_edge_idx_2+ 2)% 3]]->_pt))
			|| (point_in_circumcircle(_pts[opposition->_triangle_2->_vertices[0]]->_pt, _pts[opposition->_triangle_2->_vertices[1]]->_pt, _pts[opposition->_triangle_2->_vertices[2]]->_pt, _pts[opposition->_triangle_1->_vertices[(opposition->_edge_idx_1+ 2)% 3]]->_pt))) {
			Triangle * new_triangle_1= new Triangle();
			Triangle * new_triangle_2= new Triangle();
			swap_triangle(opposition, new_triangle_1, new_triangle_2);
			unsigned int idx_pt_1= opposition->_triangle_1->_vertices[(opposition->_edge_idx_1+ 2) % 3];
			unsigned int idx_pt_2= opposition->_triangle_2->_vertices[(opposition->_edge_idx_2+ 2) % 3];
			delete_triangle(opposition->_triangle_1, true);
			delete_triangle(opposition->_triangle_2, true);
			insert_triangle(new_triangle_1, true);
			insert_triangle(new_triangle_2, true);
			new_edges.push_back(make_pair(idx_pt_1, idx_pt_2));
		}
	}
}


// suppression des triangles inclus dans les polygones définis par les constrained_edge
// on détecte un changement de polygone lorsque le 1er sommet du edge suivant ne correspond pas au 2 sommet du edge courant
void Triangulation::clean_in_constrained_poly() {
	if (_verbose) {
		cout << "\nclean_in_constrained_poly\n";
	}

	vector<number> pts;
	for (unsigned int idx_edge=0; idx_edge<_constrained_edges.size(); ++idx_edge) {
		pts.push_back(_pts[_constrained_edges[idx_edge]->_idx.first]->_pt.x);
		pts.push_back(_pts[_constrained_edges[idx_edge]->_idx.first]->_pt.y);
		
		// fin polygone courant
		if ((idx_edge== _constrained_edges.size()- 1) || (_constrained_edges[idx_edge]->_idx.second!= _constrained_edges[idx_edge+ 1]->_idx.first)) {
			Polygon2D * constrained_poly= new Polygon2D();
			constrained_poly->set_points(pts.data(), pts.size()/ 2);
			//std::cout << *constrained_poly << "\n";
			pts.clear();
			for (auto & t : _triangles) {
				pt_2d bary= (_pts[t->_vertices[0]]->_pt+ _pts[t->_vertices[1]]->_pt+ _pts[t->_vertices[2]]->_pt)/ 3.0;
				if (is_pt_inside_poly(bary, constrained_poly)) {
					if (_verbose) {
						cout << "remove :";
						print_triangle(t);
					}
					delete t;
					t= 0;
				}
			}

			delete constrained_poly;

			_triangles.erase(remove_if(_triangles.begin(), _triangles.end(), [this](Triangle * t) { 
				if (!t) {
					return true;
				}
				return false;
			}), _triangles.end());
		}
	}
}


// suppression du triangle englobant et de ses sommets
void Triangulation::remove_large_triangle() {
	if (_verbose) {
		cout << "\nremove_large_triangle\n";
	}

	for (auto & t : _triangles) {
		for (unsigned int i=0; i<3; ++i) {
			if (t->_vertices[i]>= _pts.size()- 3) {
				if (_verbose) {
					cout << "remove :";
					print_triangle(t);
				}
				delete t;
				t= 0;
				break;
			}
		}
	}

	_triangles.erase(remove_if(_triangles.begin(), _triangles.end(), [this](Triangle * t) { 
		if (!t) {
			return true;
		}
		return false;
	}), _triangles.end());

	_pts.erase(_pts.end()- 3, _pts.end());
}


// on renseigne _vertices_init qui ne sert que pour debug ; et on rétablit l'ordre initial des points
void Triangulation::finish() {
	if (_verbose) {
		cout << "\nfinish\n";
	}

	for (auto triangle : _triangles) {
		for (unsigned int i=0; i<3; ++i) {
			triangle->_vertices_init[i]= _pts[triangle->_vertices[i]]->_idx_init;
		}
	}
	
	if (_sort_by_bin) {
		sort(_pts.begin(), _pts.end(), sort_pts_by_idx_init);
	}
}


// fonctions de debug ------------------------------------------------------------------------------------------------------------------------------

// renvoie l'indice d'un triangle
int Triangulation::idx_triangle(Triangle * triangle) {
	for (unsigned int i=0; i<_triangles.size(); ++i) {
		if (_triangles[i]== triangle) {
			return i;
		}
	}
	return -1;
}


// print
void Triangulation::print_triangle(Triangle * triangle, bool verbose, bool is_pt_init) {
	if (is_pt_init) {
		cout << idx_triangle(triangle) << " ; " << triangle->_vertices[0] << " ; " << glm::to_string(_pts[triangle->_vertices[0]]->_pt_init) << " ; " << triangle->_vertices[1] << " ; " << glm::to_string(_pts[triangle->_vertices[1]]->_pt_init) << " ; " << triangle->_vertices[2] << " ; " << glm::to_string(_pts[triangle->_vertices[2]]->_pt_init) << "\n";
	}
	else {
		cout << idx_triangle(triangle) << " ; " << triangle->_vertices[0] << " ; " << glm::to_string(_pts[triangle->_vertices[0]]->_pt) << " ; " << triangle->_vertices[1] << " ; " << glm::to_string(_pts[triangle->_vertices[1]]->_pt) << " ; " << triangle->_vertices[2] << " ; " << glm::to_string(_pts[triangle->_vertices[2]]->_pt) << "\n";
	}
	if (verbose) {
		for (unsigned int i=0; i<3; ++i) {
			Triangle * adj= triangle->_adjacents[i];
			if (adj) {
				cout << "\tadj" << i << "=\n";
				if (is_pt_init) {
					cout << "\t" << idx_triangle(triangle) << " ; " << adj->_vertices[0] << " ; " << glm::to_string(_pts[adj->_vertices[0]]->_pt_init) << " ; " << adj->_vertices[1] << " ; " << glm::to_string(_pts[adj->_vertices[1]]->_pt_init) << " ; " << adj->_vertices[2] << " ; " << glm::to_string(_pts[adj->_vertices[2]]->_pt_init) << "\n";
				}
				else {
					cout << "\t" << idx_triangle(triangle) << " ; " << adj->_vertices[0] << " ; " << glm::to_string(_pts[adj->_vertices[0]]->_pt) << " ; " << adj->_vertices[1] << " ; " << glm::to_string(_pts[adj->_vertices[1]]->_pt) << " ; " << adj->_vertices[2] << " ; " << glm::to_string(_pts[adj->_vertices[2]]->_pt) << "\n";
				}
			}
		}
	}
}


// dessin d'un SVG pour debug
void Triangulation::draw(std::string svg_path, bool verbose) {
	unsigned int svg_width= 700;
	unsigned int svg_height= 700;
	number viewbox_width = 1.0f+ 2.0f* _svg_margin;
	number viewbox_height= 1.0f+ 2.0f* _svg_margin;
	ofstream f;
	f.open(svg_path);
	f << "<!DOCTYPE html>\n<html>\n<head>\n";
	f << "<style>\n";
	f << ".triangle_class {fill: transparent; stroke: black; stroke-width: 0.001; stroke-opacity: 0.3;}\n";
	f << ".circle_class {fill: none; stroke: blue; stroke-width: 0.001; stroke-opacity: 0.1;}\n";
	f << ".triangle_hover_class {fill: red; opacity: 0.2; stroke-opacity: 0.8;}\n";
	f << ".circle_hover_class {stroke-opacity: 1.0;}\n";
	f << ".circle_text_class {fill: gray; font-size: 0.04px; opacity: 0.4;}\n";
	f << ".point_class {fill: black;}\n";
	f << ".point_text_class {fill: red; font-size: 0.03px;}\n";
	f << "</style>\n</head>\n<body>\n";
	f << "<svg width=\"" << svg_width << "\" height=\"" << svg_height << "\" viewbox=\"" << 0 << " " << 0 << " " << viewbox_width << " " << viewbox_height << "\">\n";

	for (unsigned int i=0; i<_pts.size(); ++i) {
		pt_2d pt_svg= svg_coords(_pts[i]->_pt_init);
		if (verbose) {
			f << "<text class=\"point_text_class\" x=\"" << pt_svg.x+ 0.02f << "\" y=\"" << pt_svg.y << "\" >" << _pts[i]->_idx_init << "</text>\n";
			//f << "<text class=\"point_text_class\" x=\"" << pt_svg.x+ 0.02f << "\" y=\"" << pt_svg.y << "\" >" << _pts[i]->_idx_bin << "</text>\n";
		}
		f << "<circle class=\"point_class\" cx=\"" << pt_svg.x << "\" cy=\"" << pt_svg.y << "\" r=\"" << 0.003f << "\" />\n";
	}

	number m= max(_aabb->_size.x, _aabb->_size.y);
	for (unsigned int idx_tri=0; idx_tri<_triangles.size(); ++idx_tri) {
		f << "<g>\n";

		if (verbose) {
			pt_2d bary= (_pts[_triangles[idx_tri]->_vertices_init[0]]->_pt_init+ _pts[_triangles[idx_tri]->_vertices_init[1]]->_pt_init+ _pts[_triangles[idx_tri]->_vertices_init[2]]->_pt_init)/ 3.0;
			bary= svg_coords(bary);
			f << "<text class=\"circle_text_class\" x=\"" << bary.x << "\" y=\"" << bary.y << "\">" << idx_tri << "</text>\n";
		}

		f << "<polygon class=\"triangle_class\" id=\"poly_" << idx_tri << "\" data=\"vertices=";
		for (unsigned int i=0; i<3; ++i) {
			f << _triangles[idx_tri]->_vertices_init[i] << " ; ";
		}
		f <<  "adjacents=";
		for (unsigned int i=0; i<3; ++i) {
			if (_triangles[idx_tri]->_adjacents[i]) {
				f << "(";
				for (unsigned int j=0; j<3; ++j) {
					f << _triangles[idx_tri]->_adjacents[i]->_vertices_init[j] << " ; ";
				}
				f << ")";
			}
		}
		f << "\" points=\"";
		for (unsigned int i=0; i<3; ++i) {
			pt_2d pt_svg_1= svg_coords(_pts[_triangles[idx_tri]->_vertices_init[i]]->_pt_init);
			f << pt_svg_1.x << "," << pt_svg_1.y << " ";
		}
		f << "\" />\n";
		
		if (verbose) {
			number radius;
			pt_2d center(0.0f);
			get_circle_center(_pts[_triangles[idx_tri]->_vertices_init[0]]->_pt_init, _pts[_triangles[idx_tri]->_vertices_init[1]]->_pt_init, _pts[_triangles[idx_tri]->_vertices_init[2]]->_pt_init, center, &radius);
			center= svg_coords(center);
			f << "<circle class=\"circle_class\" cx=\"" << center.x << "\" cy=\"" << center.y << "\" r=\"" << radius/ m << "\" />\n";
		}

		f << "</g>\n";
	}
	f << "</svg>\n";

	if (verbose) {
		f << "<div id=\"coords\"></div>\n";
		f << "<div id=\"info\"></div>\n";

		f << "<script>\n";
		f << "var tris = document.getElementsByClassName('triangle_class');\n";
		f << "for (var i = 0; i < tris.length; i++) {\n";
		f << "tris[i].addEventListener('mouseover', mouseOverEffect);\n";
		f << "tris[i].addEventListener('mouseout', mouseOutEffect);}\n";
		f << "function mouseOverEffect() {\n";
		f << "this.classList.add(\"triangle_hover_class\"); this.parentNode.getElementsByTagName(\"circle\")[0].classList.add(\"circle_hover_class\");\n";
		f << "document.getElementById(\"info\").innerHTML='id='+ this.id+ ' '+ this.getAttribute(\"data\");\n";
		f << "}\n";
		f << "function mouseOutEffect() {\n";
		f << "this.classList.remove(\"triangle_hover_class\"); this.parentNode.getElementsByTagName(\"circle\")[0].classList.remove(\"circle_hover_class\");\n";
		f << "document.getElementById(\"info\").innerHTML=\"_\"\n";
		f << "}\n";
		f << "document.addEventListener('mousemove', (e)=> {document.getElementById(\"coords\").innerHTML=svg_inv_coords(e.clientX, e.clientY); })\n";
		f << "function svg_inv_coords(x_html, y_html) {\n";
		f << "var m= " << max(_aabb->_size.x, _aabb->_size.y) << ";\n";
		f << "var x= x_html*" << viewbox_width/ svg_width << ";\n";
		f << "var y= y_html*" << viewbox_height/ svg_height << ";\n";
		f << "var xx= (x- " << _svg_margin << ")* m+ " << _aabb->_pos.x << ";\n";
		f << "var yy= -(y- " << _svg_margin << ")* m+ " << _aabb->_pos.y+ _aabb->_size.y << ";\n";
		f << "return xx+ ' '+ yy;}\n";

		ifstream log_stream("../data/out.txt");
		stringstream log_buffer;
		log_buffer << log_stream.rdbuf();

		f << "console.log(`" << log_buffer.str() << "`)\n";
		f << "</script>\n";
	}

	f << "</body>\n</html>\n";
	f.close();
}


pt_2d Triangulation::svg_coords(pt_2d & v) {
	number m= max(_aabb->_size.x, _aabb->_size.y);
	return pt_2d(_svg_margin+ (v.x- _aabb->_pos.x)/ m, _svg_margin+ (_aabb->_pos.y+ _aabb->_size.y- v.y)/ m);
}


