// https://www.newcastle.edu.au/__data/assets/pdf_file/0019/22519/23_A-fast-algortithm-for-generating-constrained-Delaunay-triangulations.pdf


#include <cmath>
#include <iostream>
#include <algorithm>
#include <deque>
#include <fstream>

#include <glm/gtx/string_cast.hpp>

#include "triangulation.h"
#include "geom_2d.h"


using namespace std;



// --------------------------------------------------
Triangle::Triangle() {
	_vertices[0]= -1;
	_vertices[1]= -1;
	_vertices[2]= -1;
	_adjacents[0]= NULL;
	_adjacents[1]= NULL;
	_adjacents[2]= NULL;
}


Triangle::Triangle(int v0, int v1, int v2, Triangle * a0, Triangle * a1, Triangle * a2) {
	_vertices[0]= v0;
	_vertices[1]= v1;
	_vertices[2]= v2;
	_adjacents[0]= a0;
	_adjacents[1]= a1;
	_adjacents[2]= a2;
}


Triangle::~Triangle() {
	
}


// --------------------------------------------------
PointBin::PointBin() {

}


PointBin::PointBin(glm::vec2 pt, int idx_bin) : _pt(pt), _idx_bin(idx_bin) {
	
}


PointBin::~PointBin() {
	
}


bool sort_pts(PointBin * pt1, PointBin * pt2) {
	return pt1->_idx_bin< pt2->_idx_bin;
}


// --------------------------------------------------
Triangulation::Triangulation() {

}


Triangulation::Triangulation(vector<glm::vec2> & pts) {
	float xmin= 1e8f;
	float ymin= 1e8f;
	float xmax= -1e8f;
	float ymax= -1e8f;
	for (auto pt : pts) {
		_pts_init.push_back(glm::vec2(pt));
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

	unsigned int subdiv= (unsigned int)(pow(_pts_init.size(), 0.25f));
	float step= 1.0f/ subdiv;
	for (unsigned int idx_bin=0; idx_bin< subdiv* subdiv; ++idx_bin) {
		unsigned int row= idx_bin/ subdiv;
		unsigned int col= idx_bin% subdiv;
		if (row% 2== 1) {
			col= subdiv- col;
		}
		cout << "row=" << row << " ; col=" << col << "\n";
		AABB_2D * aabb= new AABB_2D(glm::vec2(col* step, row* step), glm::vec2((col+ 1)* step, (row+ 1)* step));
		_bins.push_back(aabb);
	}

	_aabb= new AABB_2D(glm::vec2(xmin, ymin), glm::vec2(xmax- xmin, ymax- ymin));
	cout << "aabb=" << *_aabb << "\n";
	for (auto pt : pts) {
		unsigned int idx_bin_ok= 0;
		for (unsigned int idx_bin=0; idx_bin<_bins.size(); ++idx_bin) {
			if (point_in_aabb(pt, _bins[idx_bin])) {
				idx_bin_ok= idx_bin;
				break;
			}
		}
		_pts.push_back(new PointBin(glm::vec2((pt.x- _aabb->_pos.x)/ _aabb->_size.x, (pt.y- _aabb->_pos.y)/ _aabb->_size.y), idx_bin_ok));
	}

	sort(_pts.begin(), _pts.end(), sort_pts);

	_pts.push_back(new PointBin(glm::vec2(-100.0f, -100.0f), -1));
	_pts.push_back(new PointBin(glm::vec2(100.0f, -100.0f), -1));
	_pts.push_back(new PointBin(glm::vec2(0.0f, 100.0f), -1));

	for (auto pt : _pts) {
		cout << glm::to_string(pt->_pt) << "\n";
	}

	Triangle * t= new Triangle();
	t->_vertices[0]= _pts.size()- 3;
	t->_vertices[1]= _pts.size()- 2;
	t->_vertices[2]= _pts.size()- 1;
	_triangles.push_back(t);

	Triangle * last_triangle= NULL;
	for (unsigned int idx_pt=0; idx_pt<_pts.size()- 3; ++idx_pt) {
		cout << "-------------------------\n";
		cout << "pt=" << glm::to_string(_pts[idx_pt]->_pt) << "\n";

		last_triangle= _triangles[_triangles.size()- 1];
		bool search_triangle= true;
		while (search_triangle) {
			search_triangle= false;
			glm::vec2 bary= (_pts[last_triangle->_vertices[0]]->_pt+ _pts[last_triangle->_vertices[1]]->_pt+ _pts[last_triangle->_vertices[2]]->_pt)/ 3.0f;
			cout << "bary=" << glm::to_string(bary) << "\n";
			for (unsigned int i=0; i<3; ++i) {
				glm::vec2 result;
				if (segment_intersects_segment(bary, _pts[idx_pt]->_pt, _pts[last_triangle->_vertices[i]]->_pt, _pts[last_triangle->_vertices[(i+ 1)% 3]]->_pt, &result)) {
					if (result!= _pts[idx_pt]->_pt) {
						last_triangle= last_triangle->_adjacents[i];
						cout << "bary intersects\n";
						print_triangle(last_triangle);
						search_triangle= true;
						break;
					}
				}
			}
		}
		cout << "last_triangle\n";
		print_triangle(last_triangle);
		
		Triangle * t1= new Triangle(last_triangle->_vertices[0], last_triangle->_vertices[1], idx_pt, last_triangle->_adjacents[0], NULL, NULL);
		Triangle * t2= new Triangle(idx_pt, last_triangle->_vertices[1], last_triangle->_vertices[2], NULL, last_triangle->_adjacents[1], NULL);
		Triangle * t3= new Triangle(last_triangle->_vertices[2], last_triangle->_vertices[0], idx_pt, last_triangle->_adjacents[2], NULL, NULL);
		
		t1->_adjacents[1]= t2;
		t1->_adjacents[2]= t3;
		t2->_adjacents[0]= t1;
		t2->_adjacents[2]= t3;
		t3->_adjacents[1]= t1;
		t3->_adjacents[2]= t2;

		Triangle * new_tris[3]= {t1, t2, t3};

		deque<Opposition *> opposition_deque;

		for (unsigned int i=0; i<3; ++i) {
			if (last_triangle->_adjacents[i]) {
				for (unsigned int j=0; j<3; ++j) {
					if (last_triangle->_adjacents[i]->_adjacents[j]== last_triangle) {
						last_triangle->_adjacents[i]->_adjacents[j]= new_tris[i];
						opposition_deque.push_back(new Opposition(new_tris[i], last_triangle->_adjacents[i]));
						break;
					}
				}
			}
		}

		cout << "erase\n";
		print_triangle(last_triangle, true);

		_triangles.erase(remove(_triangles.begin(), _triangles.end(), last_triangle), _triangles.end());
		_triangles.insert(_triangles.end(), new_tris, new_tris+ 3);

		cout << "insert\n";
		print_triangle(t1, true);
		print_triangle(t2, true);
		print_triangle(t3, true);

		while (!opposition_deque.empty()) {
			Opposition * opposition= opposition_deque.back();
			opposition_deque.pop_back();
			cout << "deque\n";
			print_triangle(opposition->_new_triangle);
			print_triangle(opposition->_opposite_triangle);

			if (point_in_circumcircle(_pts[opposition->_opposite_triangle->_vertices[0]]->_pt, _pts[opposition->_opposite_triangle->_vertices[1]]->_pt, _pts[opposition->_opposite_triangle->_vertices[2]]->_pt, _pts[idx_pt]->_pt)) {
				Triangle * t4_adjs[2]= {
					opposition->_opposite_triangle->_adjacents[(opposition->_opposite_edge_idx+ 2)% 3],
					opposition->_new_triangle->_adjacents[(opposition->_new_edge_idx+ 1) % 3]
				};
				Triangle * t4= new Triangle(
					opposition->_new_triangle->_vertices[(opposition->_new_edge_idx+ 2)% 3],
					opposition->_opposite_triangle->_vertices[(opposition->_opposite_edge_idx+ 2)% 3],
					opposition->_opposite_triangle->_vertices[opposition->_opposite_edge_idx],
					NULL, t4_adjs[0], t4_adjs[1]);

				Triangle * t5_adjs[2]= {
					opposition->_new_triangle->_adjacents[(opposition->_new_edge_idx+ 2) % 3],
					opposition->_opposite_triangle->_adjacents[(opposition->_opposite_edge_idx+ 1) % 3]
				};
				Triangle * t5= new Triangle(
					opposition->_opposite_triangle->_vertices[(opposition->_opposite_edge_idx+ 2)% 3],
					opposition->_new_triangle->_vertices[(opposition->_new_edge_idx+ 2) % 3],
					opposition->_new_triangle->_vertices[opposition->_new_edge_idx],
					NULL, t5_adjs[0], t5_adjs[1]);
				
				t4->_adjacents[0]= t5;
				t5->_adjacents[0]= t4;

				Triangle * new_tris_2[2]= {t4, t5};

				for (unsigned int i=0; i<2; ++i) {
					if (t4_adjs[i]) {
						for (unsigned int j=0; j<3; ++j) {
							if ((t4_adjs[i]->_adjacents[j]== opposition->_new_triangle) || (t4_adjs[i]->_adjacents[j]== opposition->_opposite_triangle)) {
								t4_adjs[i]->_adjacents[j]= t4;
								break;
							}
						}
					}
				}

				for (unsigned int i=0; i<2; ++i) {
					if (t5_adjs[i]) {
						for (unsigned int j=0; j<3; ++j) {
							if ((t5_adjs[i]->_adjacents[j]== opposition->_new_triangle) || (t5_adjs[i]->_adjacents[j]== opposition->_opposite_triangle)) {
								t5_adjs[i]->_adjacents[j]= t5;
								break;
							}
						}
					}
				}

				if (opposition->_opposite_triangle->_adjacents[(opposition->_opposite_edge_idx+ 1)% 3]) {
					opposition_deque.push_back(new Opposition(t5, opposition->_opposite_triangle->_adjacents[(opposition->_opposite_edge_idx+ 1)% 3]));
				}
				if (opposition->_opposite_triangle->_adjacents[(opposition->_opposite_edge_idx+ 2)% 3]) {
					opposition_deque.push_back(new Opposition(t4, opposition->_opposite_triangle->_adjacents[(opposition->_opposite_edge_idx+ 2)% 3]));
				}

				cout << "erase2\n";
				print_triangle(opposition->_new_triangle);
				print_triangle(opposition->_opposite_triangle);

				_triangles.erase(remove(_triangles.begin(), _triangles.end(), opposition->_new_triangle), _triangles.end());
				_triangles.erase(remove(_triangles.begin(), _triangles.end(), opposition->_opposite_triangle), _triangles.end());
				_triangles.insert(_triangles.end(), new_tris_2, new_tris_2+ 2);

				cout << "insert2\n";
				print_triangle(t4);
				print_triangle(t5);
			}
		}
	}

	_triangles.erase(remove_if(_triangles.begin(), _triangles.end(), [this](Triangle * t) { 
		for (unsigned int i=0; i<3; ++i) {
			if (t->_vertices[i]>= _pts.size()- 3) {
				return true;
			}
		}
		return false;
	}), _triangles.end());

}


Triangulation::~Triangulation() {
	// TODO
}


void Triangulation::print_triangle(Triangle * triangle, bool verbose) {
	cout << glm::to_string(_pts[triangle->_vertices[0]]->_pt) << " ; " << glm::to_string(_pts[triangle->_vertices[1]]->_pt) << " ; " << glm::to_string(_pts[triangle->_vertices[2]]->_pt) << "\n";
	if (verbose) {
		for (unsigned int i=0; i<3; ++i) {
			Triangle * adj= triangle->_adjacents[i];
			if (adj) {
				cout << "\tadj" << i << "=\n";
				cout << "\t" << glm::to_string(_pts[adj->_vertices[0]]->_pt) << " ; " << glm::to_string(_pts[adj->_vertices[1]]->_pt) << " ; " << glm::to_string(_pts[adj->_vertices[2]]->_pt) << "\n";
			}
		}
	}
}


void Triangulation::draw(std::string svg_path) {
	ofstream f;
	f.open(svg_path);
	f << "<!DOCTYPE html>\n<html>\n<body>\n";
	f << "<svg width=\"1000\" height=\"1000\" viewbox=\"" << 0 << " " << 0 << " " << 1+ 2* _svg_margin << " " << 1+ 2* _svg_margin << "\">\n";

	for (auto pt : _pts_init) {
		string color= "black";
		float radius= 0.01f;
		glm::vec2 pt_svg= svg_coords(pt);
		f << "<circle cx=\"" << pt_svg.x << "\" cy=\"" << pt_svg.y << "\" r=\"" << radius << "\" fill=\"" << color << "\" />\n";
	}
	for (auto t : _triangles) {
		for (unsigned int i=0; i<3; ++i) {
			glm::vec2 pt_svg_1= svg_coords(_pts_init[t->_vertices[i]]);
			glm::vec2 pt_svg_2= svg_coords(_pts_init[t->_vertices[(i+ 1)% 3]]);
			f << "<line x1=\"" << pt_svg_1.x << "\" y1=\"" << pt_svg_1.y << "\" x2=\"" << pt_svg_2.x << "\" y2=\"" << pt_svg_2.y << "\" stroke=\"red\" stroke-width=\"0.001\" />\n";
		}
	}

	f << "</svg>\n</body>\n</html>\n";
	f.close();

}


glm::vec2 Triangulation::svg_coords(glm::vec2 & v) {
	return glm::vec2(_svg_margin+ (v.x- _aabb->_pos.x)/ _aabb->_size.x, _svg_margin+ (_aabb->_pos.y+ _aabb->_size.y- v.y)/ _aabb->_size.y);
}


// ----------------------------------------------------
Opposition::Opposition() {

}


Opposition::Opposition(Triangle * new_triangle, Triangle * opposite_triangle) : _new_triangle(new_triangle), _opposite_triangle(opposite_triangle) {
	_new_edge_idx= -1;
	_opposite_edge_idx= -1;
	for (unsigned int i=0; i<3; ++i) {
		if (_new_triangle->_adjacents[i]== _opposite_triangle) {
			_new_edge_idx= i;
			break;
		}
	}
	for (unsigned int i=0; i<3; ++i) {
		if (_opposite_triangle->_adjacents[i]== _new_triangle) {
			_opposite_edge_idx= i;
			break;
		}
	}
}


Opposition::~Opposition() {

}
