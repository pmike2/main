// https://www.newcastle.edu.au/__data/assets/pdf_file/0019/22519/23_A-fast-algortithm-for-generating-constrained-Delaunay-triangulations.pdf


#include <cmath>
#include <iostream>
#include <algorithm>
#include <deque>

#include <glm/gtx/string_cast.hpp>

#include "triangulation.h"
#include "geom_2d.h"


using namespace std;


bool sort_pts(PointBin & pt1, PointBin & pt2) {
	return pt1._idx_bin< pt2._idx_bin;
}


// --------------------------------------------------
Triangle::Triangle() {
	_vertices[0]= -1;
	_vertices[1]= -1;
	_vertices[2]= -1;
	_adjacents[0]= -1;
	_adjacents[1]= -1;
	_adjacents[2]= -1;
}


Triangle(int v0, int v1, int v2, int a0, int a1, int a2) {
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
	for (auto pt : pts) {
		unsigned int idx_bin_ok= 0;
		for (unsigned int idx_bin=0; idx_bin<_bins.size(); ++idx_bin) {
			if (point_in_aabb(pt, _bins[idx_bin])) {
				idx_bin_ok= idx_bin;
				break;
			}
		}
		_pts.push_back(new PointBin(glm::vec2((pt.x- _aabb->_xmin)/ _aabb->_size.x, (pt.y- _aabb->_ymin)/ _aabb->_size.y), idx_bin_ok));
	}

	sort(_pts.begin(), _pts.end(), sort_pts);

	_pts.push_back(new PointBin(glm::vec2(-100.0f, -100.0f), -1));
	_pts.push_back(new PointBin(glm::vec2(100.0f, -100.0f), -1));
	_pts.push_back(new PointBin(glm::vec2(0.0f, 100.0f), -1));

	Triangle * t= new Triangle();
	t->_vertices[0]= _pts.size()- 3;
	t->_vertices[1]= _pts.size()- 2;
	t->_vertices[2]= _pts.size()- 1;
	_triangles.push_back(t);

	unsigned int last_triangle_idx= 0;
	for (unsigned int idx_pt=0; idx_pt<_pts.size(); ++idx_pt) {
		glm::vec2 bary= (_pts[_triangles[last_triangle_idx]->_vertices[0]]+ _pts[_triangles[last_triangle_idx]->_vertices[1]]+ _pts[_triangles[last_triangle_idx]->_vertices[2]])/ 3.0f;
		bool search_triangle= true;
		while (search_triangle) {
			search_triangle= false;
			for (unsigned int i=0; i<3; ++i) {
				glm::vec2 result;
				if (segment_intersects_segment(bary, _pts[idx_pt], _pts[_triangles[last_triangle_idx]->_vertices[i], _pts[_triangles[last_triangle_idx]->_vertices[(i+ 1)% 3], &result)) {
					last_triangle_idx= _triangles[last_triangle_idx]->_adjacents[i];
					search_triangle= true;
					break;
				}
			}
		}
		
		// cf Fig 4 PDF
		Triangle * t1= new Triangle(_triangles[last_triangle_idx]->_vertices[0], _triangles[last_triangle_idx]->_vertices[1], idx_pt, 
			_triangles[last_triangle_idx]->_adjacents[0], last_triangle_idx+ 1, last_triangle_idx+ 2);

		Triangle * t2= new Triangle(idx_pt, _triangles[last_triangle_idx]->_vertices[1], _triangles[last_triangle_idx]->_vertices[2],
			last_triangle_idx, _triangles[last_triangle_idx]->_adjacents[1], last_triangle_idx+ 2);

		Triangle * t3= new Triangle(_triangles[last_triangle_idx]->_vertices[2], _triangles[last_triangle_idx]->_vertices[0], idx_pt, 
			_triangles[last_triangle_idx]->_adjacents[2], last_triangle_idx, last_triangle_idx+ 1);

		deque<unsigned int> tris_deque;
		for (unsigned int i=0; i<3; ++i) {
			if (_triangles[last_triangle_idx]->_adjacents[i]>= 0) {
				tris_deque.push_back(_triangles[last_triangle_idx]->_adjacents[i]);
			}
		}

		_triangles.erase(_triangles.begin()+ last_triangle_idx);
		Triangle * new_tris[3]= {t1, t2, t3};
		_triangles.insert(_triangles.begin()+ last_triangle_idx, new_tris, new_tris+ 3);

		while (!tris_deque.empty()) {
			unsigned int opposite_triangle_idx= tris_deque.back();
			tris_deque.pop_back();
			
			for (unsigned int i=0; i<3; ++i) {
				for (unsigned int j=0; j<3; ++j) {
					if (triangles[last_triangle_idx]->_vertices[i]== triangles[opposite_triangle_idx]->_vertices[j]) {
						common_1.push_back(i);
						common_2.push_back(j);
					}
				}
			}
			if (common_1[0]> common_1[1]) {
				int swap= common_1[0];
				common_1[0]= common_1[1]
				common_1[1]= swap;
			}
			if (common_2[0]> common_2[1]) {
				int swap= common_2[0];
				common_2[0]= common_2[1]
				common_2[1]= swap;
			}
			for (unsigned int i=0; i<3; ++i) {
				if (find(common_1.begin(), common_1.end(), i)== common_1.end()) {
					alone_1= i;
					break;
				}
			}
			for (unsigned int i=0; i<3; ++i) {
				if (find(common_2.begin(), common_2.end(), i)== common_2.end()) {
					alone_2= i;
					break;
				}
			}

			if (point_in_circumcircle(_pts[_triangles[opposite_triangle_idx]->_vertices[0]], _pts[_triangles[opposite_triangle_idx]->_vertices[1]], _pts[_triangles[opposite_triangle_idx]->_vertices[2]], _pts[idx_pt])) {
				Triangle * t4= new Triangle(triangles[last_triangle_idx]->_vertices[common_1[0]], triangles[last_triangle_idx]->_vertices[common_1[1]], triangles[opposite_triangle_idx]->_vertices[alone_2],
					triangles[last_triangle_idx]->_adjacents[common_1[0]], _triangles[opposite_triangle_idx]->_adjacents[], );

			}
		}
	}
}


Triangulation::~Triangulation() {
	// TODO
}

