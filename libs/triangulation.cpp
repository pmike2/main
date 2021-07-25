// https://www.newcastle.edu.au/__data/assets/pdf_file/0019/22519/23_A-fast-algortithm-for-generating-constrained-Delaunay-triangulations.pdf


#include <cmath>
#include <iostream>
#include <algorithm>
#include <deque>
#include <fstream>
#include <sstream>

#include <glm/gtx/string_cast.hpp>

#include "triangulation.h"
#include "geom_2d.h"


using namespace std;



// --------------------------------------------------
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


bool Triangle::is_valid() {
	if ((_vertices[0]< 0) || (_vertices[1]< 0) || (_vertices[2]< 0)) {
		return false;
	}
	return true;
}


// --------------------------------------------------
PointBin::PointBin() {

}


PointBin::PointBin(glm::vec2 pt_init, glm::vec2 pt, int idx_init, int idx_bin) :
	_pt_init(pt_init), _pt(pt), _idx_init(idx_init), _idx_bin(idx_bin) {
	
}


PointBin::~PointBin() {
	
}


// --------------------------------------------------
bool sort_pts_by_idx_bin(PointBin * pt1, PointBin * pt2) {
	return pt1->_idx_bin< pt2->_idx_bin;
}


bool sort_pts_by_idx_init(PointBin * pt1, PointBin * pt2) {
	return pt1->_idx_init< pt2->_idx_init;
}


// ----------------------------------------------------
Opposition::Opposition() {

}


Opposition::Opposition(Triangle * triangle_1, Triangle * triangle_2) : _triangle_1(triangle_1), _triangle_2(triangle_2), _is_valid(true) {
	_edge_idx_1= -1;
	_edge_idx_2= -1;
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
Triangulation::Triangulation() {

}


Triangulation::Triangulation(const vector<glm::vec2> & pts, const vector<pair<unsigned int, unsigned int> > & constrained_edges, bool sort_by_bin, bool verbose) :
	_sort_by_bin(sort_by_bin), _verbose(verbose)
{
	streambuf * coutbuf;
	ofstream out_stream("../data/out.txt");
	if (_verbose) {
		coutbuf= cout.rdbuf();
		//cout.rdbuf(out_stream.rdbuf());
	}

	init_pts(pts);
	add_large_triangle();

	for (unsigned int idx_pt=0; idx_pt<_pts.size()- 3; ++idx_pt) {
		add_pt(idx_pt);
	}

	remove_large_triangle();
	finish_pts();

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

	for (auto & bin : _bins) {
		delete bin;
	}
	_bins.clear();

	delete _aabb;
}


void Triangulation::init_pts(const vector<glm::vec2> & pts) {
	float xmin= 1e8f;
	float ymin= 1e8f;
	float xmax= -1e8f;
	float ymax= -1e8f;
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

	unsigned int subdiv= (unsigned int)(pow(pts.size(), 0.25f));
	float step= 1.0f/ subdiv;
	for (unsigned int idx_bin=0; idx_bin< subdiv* subdiv; ++idx_bin) {
		unsigned int row= idx_bin/ subdiv;
		unsigned int col= idx_bin% subdiv;
		if (row% 2== 1) {
			col= subdiv- 1- col;
		}
		AABB_2D * aabb= new AABB_2D(glm::vec2(col* step, row* step), glm::vec2(step, step));
		if (_verbose) {
			cout << "bin=" << *aabb << "\n";
		}
		_bins.push_back(aabb);
	}

	_aabb= new AABB_2D(glm::vec2(xmin, ymin), glm::vec2(xmax- xmin, ymax- ymin));
	if (_verbose) {
		cout << "aabb=" << *_aabb << "\n";
	}
	
	float m= max(_aabb->_size.x, _aabb->_size.y);
	for (unsigned int i=0; i<pts.size(); ++i) {
		glm::vec2 normalized_pt((pts[i].x- _aabb->_pos.x)/ m, (pts[i].y- _aabb->_pos.y)/ m);
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
}


void Triangulation::add_large_triangle() {
	_pts.push_back(new PointBin(glm::vec2(0.0f), glm::vec2(-100.0f, -100.0f), -1, -1));
	_pts.push_back(new PointBin(glm::vec2(0.0f), glm::vec2(100.0f, -100.0f), -1, -1));
	_pts.push_back(new PointBin(glm::vec2(0.0f), glm::vec2(0.0f, 100.0f), -1, -1));

	Triangle * t= new Triangle();
	t->_vertices[0]= _pts.size()- 3;
	t->_vertices[1]= _pts.size()- 2;
	t->_vertices[2]= _pts.size()- 1;
	_triangles.push_back(t);
}


Triangle * Triangulation::get_containing_triangle(unsigned int idx_pt) {
	Triangle * last_triangle= _triangles[_triangles.size()- 1];
	bool search_triangle= true;
	unsigned int compt= 0;
	bool search_triangle_ok= true;

	while (search_triangle) {
		compt++;
		if (compt> _triangles.size()) {
			search_triangle_ok= false;
			cerr << "Erreur search_triangle\n";
			break;
		}
		search_triangle= false;
		glm::vec2 bary= (_pts[last_triangle->_vertices[0]]->_pt+ _pts[last_triangle->_vertices[1]]->_pt+ _pts[last_triangle->_vertices[2]]->_pt)/ 3.0f;
		if (_verbose) {
			cout << "bary=" << glm::to_string(bary) << "\n";
		}
		for (unsigned int i=0; i<3; ++i) {
			glm::vec2 result;
			if (segment_intersects_segment(bary, _pts[idx_pt]->_pt, _pts[last_triangle->_vertices[i]]->_pt, _pts[last_triangle->_vertices[(i+ 1)% 3]]->_pt, &result, true)) {
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


void Triangulation::delete_triangle(Triangle * triangle) {
	vector<Triangle *>::iterator it= find(_triangles.begin(), _triangles.end(), triangle);
	if (it!= _triangles.end()) {
		_triangles.erase(it);
	}
	delete triangle;
	triangle= 0;
}


void Triangulation::swap_triangle(Opposition * opposition, Triangle * new_triangle_1, Triangle * new_triangle_2) {
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


void Triangulation::add_pt(unsigned int idx_pt) {
	if (_verbose) {
		cout << "--------------------------------------------------\n";
		cout << "pt=" << glm::to_string(_pts[idx_pt]->_pt) << "\n";
		cout << "pt_init=" << glm::to_string(_pts[idx_pt]->_pt_init) << "\n";
	}

	Triangle * containing_triangle= get_containing_triangle(idx_pt);
	if (!containing_triangle) {
		return;
	}

	Triangle * t1= new Triangle(containing_triangle->_vertices[0], containing_triangle->_vertices[1], idx_pt, containing_triangle->_adjacents[0], NULL, NULL);
	Triangle * t2= new Triangle(idx_pt, containing_triangle->_vertices[1], containing_triangle->_vertices[2], NULL, containing_triangle->_adjacents[1], NULL);
	Triangle * t3= new Triangle(containing_triangle->_vertices[2], containing_triangle->_vertices[0], idx_pt, containing_triangle->_adjacents[2], NULL, NULL);

	t1->_adjacents[1]= t2;
	t1->_adjacents[2]= t3;
	t2->_adjacents[0]= t1;
	t2->_adjacents[2]= t3;
	t3->_adjacents[1]= t1;
	t3->_adjacents[2]= t2;

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
	
	if (_verbose) {
		cout << "erase\n";
		print_triangle(containing_triangle, true);
	}
	
	delete_triangle(containing_triangle);

	_triangles.insert(_triangles.end(), new_tris, new_tris+ 3);
	
	if (_verbose) {
		cout << "insert\n";
		print_triangle(t1, true);
		print_triangle(t2, true);
		print_triangle(t3, true);
	}

	while (!opposition_deque.empty()) {
		Opposition * opposition= opposition_deque.back();
		opposition_deque.pop_back();
		if (!opposition->_is_valid) {
			cerr << "ERREUR opposition non valide\n";
			continue;
		}
		if (_verbose) {
			cout << "deque\n";
			print_triangle(opposition->_triangle_1);
			print_triangle(opposition->_triangle_2);
		}
		
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

			if (_verbose) {
				cout << "erase2\n";
				print_triangle(opposition->_triangle_1);
				print_triangle(opposition->_triangle_2);
			}

			delete_triangle(opposition->_triangle_1);
			delete_triangle(opposition->_triangle_2);

			Triangle * new_tris_2[2]= {new_triangle_1, new_triangle_2};
			_triangles.insert(_triangles.end(), new_tris_2, new_tris_2+ 2);

			if (_verbose) {
				cout << "insert2\n";
				print_triangle(new_triangle_1);
				print_triangle(new_triangle_2);
			}
			
			if (_verbose) {
				cout << flush;
			}
		}
	}
}


void Triangulation::remove_large_triangle() {
	for (auto & t : _triangles) {
		for (unsigned int i=0; i<3; ++i) {
			if (t->_vertices[i]>= _pts.size()- 3) {
				//delete t;
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


void Triangulation::finish_pts() {
	for (auto triangle : _triangles) {
		for (unsigned int i=0; i<3; ++i) {
			triangle->_vertices_init[i]= _pts[triangle->_vertices[i]]->_idx_init;
		}
	}
	
	if (_sort_by_bin) {
		sort(_pts.begin(), _pts.end(), sort_pts_by_idx_init);
	}
}


int Triangulation::idx_triangle(Triangle * triangle) {
	for (unsigned int i=0; i<_triangles.size(); ++i) {
		if (_triangles[i]== triangle) {
			return i;
		}
	}
	return -1;
}


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


void Triangulation::draw(std::string svg_path, bool verbose) {
	unsigned int svg_width= 700;
	unsigned int svg_height= 700;
	float viewbox_width = 1.0f+ 2.0f* _svg_margin;
	float viewbox_height= 1.0f+ 2.0f* _svg_margin;
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
		glm::vec2 pt_svg= svg_coords(_pts[i]->_pt_init);
		if (verbose) {
			//f << "<text class=\"point_text_class\" x=\"" << pt_svg.x+ 0.02f << "\" y=\"" << pt_svg.y << "\" >" << _pts[i]->_idx_init << "</text>\n";
			f << "<text class=\"point_text_class\" x=\"" << pt_svg.x+ 0.02f << "\" y=\"" << pt_svg.y << "\" >" << _pts[i]->_idx_bin << "</text>\n";
		}
		f << "<circle class=\"point_class\" cx=\"" << pt_svg.x << "\" cy=\"" << pt_svg.y << "\" r=\"" << 0.003f << "\" />\n";
	}

	float m= max(_aabb->_size.x, _aabb->_size.y);
	for (unsigned int idx_tri=0; idx_tri<_triangles.size(); ++idx_tri) {
		f << "<g>\n";

		if (verbose) {
			glm::vec2 bary= (_pts[_triangles[idx_tri]->_vertices_init[0]]->_pt_init+ _pts[_triangles[idx_tri]->_vertices_init[1]]->_pt_init+ _pts[_triangles[idx_tri]->_vertices_init[2]]->_pt_init)/ 3.0f;
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
			glm::vec2 pt_svg_1= svg_coords(_pts[_triangles[idx_tri]->_vertices_init[i]]->_pt_init);
			f << pt_svg_1.x << "," << pt_svg_1.y << " ";
		}
		f << "\" />\n";
		
		if (verbose) {
			float radius;
			glm::vec2 center(0.0f);
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


glm::vec2 Triangulation::svg_coords(glm::vec2 & v) {
	float m= max(_aabb->_size.x, _aabb->_size.y);
	return glm::vec2(_svg_margin+ (v.x- _aabb->_pos.x)/ m, _svg_margin+ (_aabb->_pos.y+ _aabb->_size.y- v.y)/ m);
}


