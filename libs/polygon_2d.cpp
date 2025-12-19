#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include "polygon_2d.h"
#include "geom_2d.h"
#include "utile.h"


Polygon2D::Polygon2D() : _area(0.0), _centroid(pt_2d(0.0)), _radius(0.0), _inertia(0.0) {
	_aabb= new AABB_2D();
}


Polygon2D::Polygon2D(const Polygon2D & polygon) {
	_aabb= new AABB_2D(*polygon._aabb);
	set_points(polygon._pts);

	_triangles_idx.clear();
	for (auto tri : polygon._triangles_idx) {
		_triangles_idx.push_back(tri);
	}
}


Polygon2D::Polygon2D(const std::vector<pt_2d> pts, bool convexhull) {
	_aabb= new AABB_2D();
	set_points(pts, convexhull);
}


Polygon2D::~Polygon2D() {
	clear();
	delete _aabb;
}


void Polygon2D::clear() {
	_pts.clear();
	_normals.clear();
	_area = 0.0;
	_centroid = pt_2d(0.0);
	_radius = 0.0;
	_aabb->_pos = pt_2d(0.0);
	_aabb->_size = pt_2d(0.0);
	_triangles_idx.clear();
	_inertia = 0.0;
}


void Polygon2D::set_points(const number * points, unsigned int n_points, bool convexhull) {
	_pts.clear();
	for (unsigned int i=0; i<n_points; ++i) {
		_pts.push_back(pt_2d(points[2* i], points[2* i+ 1]));
	}
	if (convexhull) {
		convex_hull_2d(_pts);
	}
}


void Polygon2D::set_points(const std::vector<pt_2d> pts, bool convexhull) {
	_pts.clear();
	for (auto & pt : pts) {
		_pts.push_back(pt_2d(pt));
	}
	if (convexhull) {
		convex_hull_2d(_pts);
	}
}


void Polygon2D::centroid2zero() {
	// on met le centre du polygon sur le centre de gravité
	for (unsigned int i=0; i<_pts.size(); ++i) {
		_pts[i]-= _centroid;
	}
	_centroid= pt_2d(0.0);
}


void Polygon2D::randomize(unsigned int n_points, number radius, pt_2d center, bool convexhull) {
	_pts.clear();
	for (unsigned int i=0; i<n_points; ++i) {
		number x= center.x+ rand_number(-radius, radius);
		number y= center.y+ rand_number(-radius, radius);
		_pts.push_back(pt_2d(x, y));
	}
	if (convexhull) {
		convex_hull_2d(_pts);
	}
}


void Polygon2D::set_rectangle(const pt_2d origin, const pt_2d size) {
	_pts.clear();
	_pts.push_back(origin);
	_pts.push_back(origin+ pt_2d(size.x, 0.0));
	_pts.push_back(origin+ size);
	_pts.push_back(origin+ pt_2d(0.0, size.y));
}


void Polygon2D::set_bbox(const BBox_2D & bbox) {
	_pts.clear();
	for (unsigned int i=0; i<4; ++i) {
		_pts.push_back(bbox._pts[i]);
	}
}


void Polygon2D::translate(pt_2d v) {
	for (unsigned int i=0; i<_pts.size(); ++i) {
		_pts[i]+= v;
	}
}


void Polygon2D::rotate(pt_2d center, number alpha) {
	for (unsigned int i=0; i<_pts.size(); ++i) {
		_pts[i]= center+ rot(_pts[i]- center, alpha);
	}
}


void Polygon2D::scale(pt_2d scale) {
	for (unsigned int i=0; i<_pts.size(); ++i) {
		_pts[i]*= scale;
	}
}


void Polygon2D::update_area() {
	// calcul aire
	_area= 0.0;
	for (unsigned int i=0; i<_pts.size(); ++i) {
		pt_2d pt1= _pts[i];
		pt_2d pt2= _pts[(i+ 1)% _pts.size()];
		_area+= 0.5f* (pt1.x* pt2.y- pt1.y* pt2.x);
	}

	// si clockwise -> anticlockwise
	if (_area< 0.0) {
		_area*= -1.0;
		reverse(_pts.begin(), _pts.end());
	}
}


void Polygon2D::update_centroid() {
	// calcul centre de gravité
	_centroid= pt_2d(0.0);
	for (unsigned int i=0; i<_pts.size(); ++i) {
		pt_2d pt1= _pts[i];
		pt_2d pt2= _pts[(i+ 1)% _pts.size()];
		_centroid+= (0.5f* THIRD/ _area)* (pt1.x* pt2.y- pt1.y* pt2.x)* (pt1+ pt2);
	}
}


void Polygon2D::update_normals() {
	// calcul normales (norme == 1)
	// on doit etre en anticlockwise a ce moment ; on fait une rotation de -90 ie (x,y)->(y,-x)
	// pour que la normale pointe vers l'extérieur du polygone
	_normals.clear();
	for (unsigned int i=0; i<_pts.size(); ++i) {
		pt_2d pt1= _pts[i];
		pt_2d pt2= _pts[(i+ 1)% _pts.size()];
		_normals.push_back(glm::normalize(pt_2d(pt2.y- pt1.y, pt1.x- pt2.x)));
	}
}


void Polygon2D::update_radius() {
	// calcul rayon cercle englobant ; n'est pertinent que si le poly est convexe...
	_radius= 0.0;
	for (auto it_pt : _pts) {
		number dist2= glm::distance2(it_pt, _centroid);
		if (dist2> _radius) {
			_radius= dist2;
		}
	}
	_radius= sqrt(_radius);
}


void Polygon2D::update_aabb() {
	// calcul AABB
	number xmin= 1e8;
	number ymin= 1e8;
	number xmax= -1e8;
	number ymax= -1e8;
	for (auto it_pt : _pts) {
		if (it_pt.x< xmin) xmin= it_pt.x;
		if (it_pt.y< ymin) ymin= it_pt.y;
		if (it_pt.x> xmax) xmax= it_pt.x;
		if (it_pt.y> ymax) ymax= it_pt.y;
	}
	_aabb->_pos.x= xmin;
	_aabb->_pos.y= ymin;
	_aabb->_size.x= xmax- xmin;
	_aabb->_size.y= ymax- ymin;
}


void Polygon2D::update_inertia() {
	// moment inertie
	// https://physics.stackexchange.com/questions/493736/moment-of-inertia-for-an-arbitrary-polygon
	_inertia= 0.0;
	if (_pts.size()> 2) {
		for (int idx_pt=0; idx_pt<_pts.size()- 1; ++idx_pt) {
			pt_2d pt1= _pts[idx_pt]- _centroid;
			pt_2d pt2= _pts[idx_pt+ 1]- _centroid;
			_inertia+= (pt1.x* pt2.y- pt2.x* pt1.y)* (
				pt1.x* pt1.x+ pt1.x* pt2.x+ pt2.x* pt2.x+
				pt1.y* pt1.y+ pt1.y* pt2.y+ pt2.y* pt2.y
			);

			/*_inertia+= (_pts[idx_pt].x* _pts[idx_pt+ 1].y- _pts[idx_pt+ 1].x* _pts[idx_pt].y)* (
				_pts[idx_pt].x* _pts[idx_pt].x+ _pts[idx_pt].x* _pts[idx_pt+ 1].x+ _pts[idx_pt+ 1].x* _pts[idx_pt+ 1].x+
				_pts[idx_pt].y* _pts[idx_pt].y+ _pts[idx_pt].y* _pts[idx_pt+ 1].y+ _pts[idx_pt+ 1].y* _pts[idx_pt+ 1].y
			);*/
		}
		_inertia/= 12.0;
	}
}


void Polygon2D::update_all() {
	update_area();
	update_centroid();
	update_normals();
	update_radius();
	update_aabb();
	update_inertia();
}


// pt du polygon le + à droite le long d'une direction
void Polygon2D::min_max_pt_along_dir(const pt_2d direction, unsigned int * idx_pt_min, number * dist_min, unsigned int * idx_pt_max, number * dist_max) const {
	*dist_min= 1e10;
	*dist_max= -1e10;
	*idx_pt_min= 0;
	*idx_pt_max= 0;
	for (unsigned int i=0; i<_pts.size(); ++i) {
		number dist= glm::dot(direction, _pts[i]);
		if (dist> *dist_max) {
			*dist_max= dist;
			*idx_pt_max= i;
		}
		if (dist< *dist_min) {
			*dist_min= dist;
			*idx_pt_min= i;
		}
	}
}


void Polygon2D::triangulate() {
	_triangles_idx.clear();
	Triangulation * tgl= new Triangulation(_pts);
	for (auto tri : tgl->_triangles) {
		std::vector<int> v{tri->_vertices[0], tri->_vertices[1], tri->_vertices[2]};
		_triangles_idx.push_back(v);
	}
	delete tgl;
}


// attention ne fonctionne que pour des polys convexes avec buffer_size > 0
Polygon2D * Polygon2D::buffered(number buffer_size) {
	Polygon2D * result = new Polygon2D(*this);
	for (auto & pt : result->_pts) {
		pt += buffer_size * glm::normalize(pt - _centroid);
	}
	return result;
}


std::ostream & operator << (std::ostream & os, const Polygon2D & polygon) {
	os << "area= " << polygon._area << " ; centroid= " << glm_to_string(polygon._centroid);
	os << " ; aabb_pos=" << glm_to_string(polygon._aabb->_pos) << " ; aabb_size=" << glm_to_string(polygon._aabb->_size);
	return os;
}
