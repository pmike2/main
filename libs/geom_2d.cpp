#include <iostream>
#include <iomanip>
#include <cfloat> // FLT_MAX

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "geom_2d.h"
#include "utile.h"


using namespace std;


// renvoie la norme de la composante z du prod vectoriel
number cross2d(pt_type v1, pt_type v2) {
	return v1.x* v2.y- v1.y* v2.x;
}


// tri de points selon x ; utile au calcul de convex hull
bool cmp_points(pt_type & pt1, pt_type & pt2) {
	return pt1.x< pt2.x;
}


// est-ce que situé en pt_ref, regardant vers dir_ref, pt_test est à gauche
bool is_left(pt_type pt_ref, pt_type dir_ref, pt_type pt_test) {
	return cross2d(pt_type(pt_test- pt_ref), dir_ref)<= 0.0;
}


bool is_pt_inside_poly(pt_type & pt, Polygon2D * poly) {
	// ne fonctionne que pour les polygones convexes
	/*if (glm::distance(pt, poly->_centroid)> poly->_radius) {
		return false;
	}*/
	/*for (unsigned int i=0; i<poly->_pts.size(); ++i) {
		pt_type pt1= poly->_pts[i];
		pt_type pt2= poly->_pts[(i+ 1)% poly->_pts.size()];
		if (!is_left(pt1, pt2- pt1, pt)) {
			return false;
		}
	}
	return true;*/

	if (!point_in_aabb(pt, poly->_aabb)) {
		return false;
	}

	// raycast ; cf https://stackoverflow.com/questions/217578/how-can-i-determine-whether-a-2d-point-is-within-a-polygon
	int i, j, c= 0;
	for (i=0, j=poly->_pts.size()- 1; i<poly->_pts.size(); j=i++) {
		if ( 
			((poly->_pts[i].y> pt.y)!= (poly->_pts[j].y> pt.y)) &&
			(pt.x< (poly->_pts[j].x- poly->_pts[i].x)* (pt.y- poly->_pts[i].y)/ (poly->_pts[j].y- poly->_pts[i].y)+ poly->_pts[i].x) 
		) {
			c= !c;
		}
	}
	return c;
}


bool is_poly_inside_poly(Polygon2D * small_poly, Polygon2D * big_poly) {
	if (!aabb_contains_aabb(big_poly->_aabb, small_poly->_aabb)) {
		return false;
	}
	for (auto pt : small_poly->_pts) {
		if (!is_pt_inside_poly(pt, big_poly)) {
			return false;
		}
	}
	return true;
}


// cf https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
bool segment_intersects_segment(const pt_type & pt1_begin, const pt_type & pt1_end, const pt_type & pt2_begin, const pt_type & pt2_end, pt_type * result, bool exclude_seg1_extremities, bool exclude_seg2_extremities) {
	pt_type dir1= pt1_end- pt1_begin;
	pt_type dir2= pt2_end- pt2_begin;
	
	// parallèles
	number a= cross2d(dir1, dir2);
	//if (abs(a)<= EPSILON) {
	if (a== 0.0) {
		return false;
	}
	
	number t1= cross2d(pt2_begin- pt1_begin, dir2)/ a;

	// on fait ca ici meme si on renvoie false c'est utile dans les cas limites
	result->x= pt1_begin.x+ t1* dir1.x;
	result->y= pt1_begin.y+ t1* dir1.y;

	if (exclude_seg1_extremities) {
		if ((t1<= EPSILON) || (t1>= 1.0- EPSILON)) {
		//if ((t1<= 0.0) || (t1>= 1.0)) {
			return false;
		}
	}
	else {
		if ((t1< 0.0) || (t1> 1.0)) {
			return false;
		}
	}
	
	number t2= cross2d(pt2_begin- pt1_begin, dir1)/ a;
	if (exclude_seg2_extremities) {
		if ((t2<= EPSILON) || (t2>= 1.0- EPSILON)) {
		//if ((t2<= 0.0) || (t2>= 1.0)) {
			return false;
		}
	}
	else {
		if ((t2< 0.0) || (t2> 1.0)) {
			return false;
		}
	}

	return true;
}


bool ray_intersects_segment(const pt_type & origin, const pt_type & direction, const pt_type & pt_begin, const pt_type & pt_end, pt_type * result) {
	pt_type dir_segment= pt_end- pt_begin;
	number a= cross2d(direction, dir_segment);
	// parallèles
	if (abs(a)< EPSILON) {
		return false;
	}
	number t1= cross2d(pt_begin- origin, dir_segment)/ a;
	if (t1< 0.0) {
		return false;
	}
	number t2= cross2d(pt_begin- origin, direction)/ a;
	if ((t2< 0.0) || (t2> 1.0)) {
		return false;
	}
	result->x= origin.x+ t1* direction.x;
	result->y= origin.y+ t1* direction.y;
	return true;
}


bool ray_intersects_ray(const pt_type & origin1, const pt_type & direction1, const pt_type & origin2, const pt_type & direction2, pt_type * result) {
	number a= cross2d(direction1, direction2);
	// parallèles
	if (abs(a)< EPSILON) {
		return false;
	}
	number t1= cross2d(origin2- origin1, direction2)/ a;
	if (t1< 0.0) {
		return false;
	}
	number t2= cross2d(origin2- origin1, direction1)/ a;
	if (t2< 0.0) {
		return false;
	}
	result->x= origin1.x+ t1* direction1.x;
	result->y= origin1.y+ t1* direction1.y;
	return true;
}


// si existe intersection la + proche du pt de départ du segment avec le poly
bool segment_intersects_poly(const pt_type & pt_begin, const pt_type & pt_end, const Polygon2D * poly, pt_type * result) {
	if ((pt_begin.x< poly->_aabb->_pos.x) && (pt_end.x< poly->_aabb->_pos.x)) {
		return false;
	}
	if ((pt_begin.x> poly->_aabb->_pos.x+ poly->_aabb->_size.x) && (pt_end.x> poly->_aabb->_pos.x+ poly->_aabb->_size.x)) {
		return false;
	}
	if ((pt_begin.y< poly->_aabb->_pos.y) && (pt_end.y< poly->_aabb->_pos.y)) {
		return false;
	}
	if ((pt_begin.y> poly->_aabb->_pos.y+ poly->_aabb->_size.y) && (pt_end.y> poly->_aabb->_pos.y+ poly->_aabb->_size.y)) {
		return false;
	}

	number min_dist= FLT_MAX;
	bool is_inter= false;
	pt_type inter(0.0);
	for (unsigned int i=0; i<poly->_pts.size(); ++i) {
		pt_type pt1= poly->_pts[i];
		pt_type pt2= poly->_pts[(i+ 1)% poly->_pts.size()];
		
		if (segment_intersects_segment(pt1, pt2, pt_begin, pt_end, &inter)) {
			number dist2= glm::distance2(pt_begin, inter);
			if (dist2< min_dist) {
				is_inter= true;
				min_dist= dist2;
				if (result!= NULL) {
					result->x= inter.x;
					result->y= inter.y;
				}
			}
		}
	}
	return is_inter;
}


// version multi où on récupère toutes les intersections
bool segment_intersects_poly_multi(const pt_type & pt_begin, const pt_type & pt_end, const Polygon2D * poly, vector<pt_type> * result) {
	if ((pt_begin.x< poly->_aabb->_pos.x) && (pt_end.x< poly->_aabb->_pos.x)) {
		return false;
	}
	if ((pt_begin.x> poly->_aabb->_pos.x+ poly->_aabb->_size.x) && (pt_end.x> poly->_aabb->_pos.x+ poly->_aabb->_size.x)) {
		return false;
	}
	if ((pt_begin.y< poly->_aabb->_pos.y) && (pt_end.y< poly->_aabb->_pos.y)) {
		return false;
	}
	if ((pt_begin.y> poly->_aabb->_pos.y+ poly->_aabb->_size.y) && (pt_end.y> poly->_aabb->_pos.y+ poly->_aabb->_size.y)) {
		return false;
	}

	number min_dist= FLT_MAX;
	bool is_inter= false;
	pt_type inter(0.0);
	for (unsigned int i=0; i<poly->_pts.size(); ++i) {
		pt_type pt1= poly->_pts[i];
		pt_type pt2= poly->_pts[(i+ 1)% poly->_pts.size()];
		
		if (segment_intersects_segment(pt1, pt2, pt_begin, pt_end, &inter)) {
			number dist2= glm::distance2(pt_begin, inter);
			if (dist2< min_dist) {
				is_inter= true;
				min_dist= dist2;
				if (result!= NULL) {
					result->push_back(inter);
				}
			}
		}
	}
	return is_inter;
}

// d(pt, [seg1, seg2])
bool distance_segment_pt(pt_type & seg1, pt_type & seg2, pt_type & pt, number * dist, pt_type * proj) {
	number seg_norm2= glm::distance2(seg1, seg2);
	bool proj_in_segment= true;
	
	if (seg_norm2< EPSILON) {
		return glm::distance(seg1, pt);
	}
	
	number t= glm::dot(pt- seg1, seg2- seg1)/ seg_norm2;
	if ((t< 0.0) || (t> 1.0)) {
		t= max((number)(0.0), min((number)(1.0), t));
		proj_in_segment= false;
	}
	
	proj->x= seg1.x+ t* (seg2.x- seg1.x);
	proj->y= seg1.y+ t* (seg2.y- seg1.y);
	*dist= glm::distance(pt, *proj);
	return proj_in_segment;
}


number distance_poly_pt(Polygon2D * poly, pt_type & pt, pt_type * proj) {
	number dist_min= FLT_MAX;
	number dist;
	pt_type proj2;

	if (is_pt_inside_poly(pt, poly)) {
		if (proj!= NULL) {
			proj->x= pt.x;
			proj->y= pt.y;
		}
		return 0.0;
	}

	for (unsigned int i=0; i<poly->_pts.size(); ++i) {
		pt_type pt1= poly->_pts[i];
		pt_type pt2= poly->_pts[(i+ 1)% poly->_pts.size()];

		bool x= distance_segment_pt(pt1, pt2, pt, &dist, &proj2);
		if (dist< dist_min) {
			dist_min= dist;
			if (proj!= NULL) {
				proj->x= proj2.x;
				proj->y= proj2.y;
			}
		}
	}
	return dist_min;
}


number distance_poly_segment(Polygon2D * poly, pt_type & seg1, pt_type & seg2, pt_type * proj) {
	number dist_min= FLT_MAX;
	number dist;
	pt_type proj2;

	if (is_pt_inside_poly(seg1, poly)) {
		// proj ?
		return 0.0;
	}

	if (segment_intersects_poly(seg1, seg2, poly, &proj2)) {
		if (proj!= NULL) {
			proj->x= proj2.x;
			proj->y= proj2.y;
		}
		return 0.0;
	}

	for (unsigned int i=0; i<poly->_pts.size(); ++i) {
		pt_type pt1= poly->_pts[i];
		pt_type pt2= poly->_pts[(i+ 1)% poly->_pts.size()];

		bool x1= distance_segment_pt(pt1, pt2, seg1, &dist, &proj2);
		if (dist< dist_min) {
			dist_min= dist;
			if (proj!= NULL) {
				proj->x= proj2.x;
				proj->y= proj2.y;
			}
		}

		bool x2= distance_segment_pt(pt1, pt2, seg2, &dist, &proj2);
		if (dist< dist_min) {
			dist_min= dist;
			if (proj!= NULL) {
				proj->x= proj2.x;
				proj->y= proj2.y;
			}
		}
	}

	for (unsigned int i=0; i<poly->_pts.size(); ++i) {
		bool x1= distance_segment_pt(seg1, seg2, poly->_pts[i], &dist, &proj2);
		if (dist< dist_min) {
			dist_min= dist;
			if (proj!= NULL) {
				proj->x= proj2.x;
				proj->y= proj2.y;
			}
		}
	}

	return dist_min;
}


// calcul convex hull 2D bouquin computational geom
void convex_hull_2d(std::vector<pt_type> & pts) {
	sort(pts.begin(), pts.end(), cmp_points);

	vector<pt_type> pts_upper;
	pts_upper.push_back(pts[0]);
	pts_upper.push_back(pts[1]);
	for (unsigned int i=2; i<pts.size(); ++i) {
		pts_upper.push_back(pts[i]);
		while ((pts_upper.size()> 2) && (is_left(pts_upper[pts_upper.size()- 3], pts_upper[pts_upper.size()- 2]- pts_upper[pts_upper.size()- 3], pts_upper[pts_upper.size()- 1]))) {
			pts_upper.erase(pts_upper.end()- 2);
		}
	}

	vector<pt_type> pts_lower;
	pts_lower.push_back(pts[pts.size()- 1]);
	pts_lower.push_back(pts[pts.size()- 2]);
	for (int i=pts.size()- 3; i>=0; --i) {
		pts_lower.push_back(pts[i]);
		while ((pts_lower.size()> 2) && (is_left(pts_lower[pts_lower.size()- 3], pts_lower[pts_lower.size()- 2]- pts_lower[pts_lower.size()- 3], pts_lower[pts_lower.size()- 1]))) {
			pts_lower.erase(pts_lower.end()- 2);
		}
	}

	pts_lower.erase(pts_lower.begin());
	pts_lower.erase(pts_lower.end()- 1);

	pts.clear();
	pts.insert(pts.begin(), pts_upper.begin(), pts_upper.end());
	pts.insert(pts.end()  , pts_lower.begin(), pts_lower.end());
}


bool is_ccw(const pt_type & pt1, const pt_type & pt2, const pt_type & pt3) {
	return (pt2.x- pt1.x)* (pt3.y- pt1.y)- (pt3.x- pt1.x)* (pt2.y- pt1.y)> 0.0;
}


// cf https://en.wikipedia.org/wiki/Curve_orientation
bool is_ccw(const vector<pt_type> & pts) {
	number xmin= 1e6;
	number ymin= 1e6;
	int idx_min= 0;
	for (int i=0; i<pts.size(); ++i) {
		if (pts[i].x< xmin) {
			xmin= pts[i].x;
			ymin= pts[i].y;
			idx_min= i;
		}
		else if (pts[i].x== xmin) {
			ymin= pts[i].y;
			idx_min= i;
		}
	}
	// pts[idx_min] appartient forcément à l'enveloppe convexe
	int idx_before= idx_min- 1;
	if (idx_before== -1) {
		idx_before= pts.size()- 1;
	}
	int idx_after= idx_min+ 1;
	if (idx_after== pts.size()) {
		idx_after= 0;
	}
	return is_ccw(pts[idx_before], pts[idx_min], pts[idx_after]);
}


std::pair<pt_type, number> circumcircle(pt_type & circle_pt1, pt_type & circle_pt2, pt_type & circle_pt3) {
	pt_type pt1, pt2, pt3;
	if (is_ccw(circle_pt1, circle_pt2, circle_pt3)) {
		pt1= circle_pt1;
		pt2= circle_pt2;
		pt3= circle_pt3;
	}
	else {
		pt1= circle_pt1;
		pt2= circle_pt3;
		pt3= circle_pt2;
	}
	pt_type d21= pt2- pt1;
	pt_type d31= pt3- pt1;
	pt_type d32= pt3- pt2;

	// points alignés
	if (d32.y* d21.x- d21.y* d32.x== 0.0) {
		return std::make_pair(pt_type(0.0), 0.0); // bof
	}
	number lambda= 0.5f* (d31.x* d32.x+ d32.y* d31.y)/ (d32.y* d21.x- d21.y* d32.x);
	pt_type center= pt_type(0.5f* (pt1.x+ pt2.x)- lambda* d21.y, 0.5f* (pt1.y+ pt2.y)+ lambda* d21.x);
	number radius= sqrt((center.x- pt1.x)* (center.x- pt1.x)+ (center.y- pt1.y)* (center.y- pt1.y));
	return std::make_pair(center, radius);
}


bool point_in_circumcircle(pt_type & circle_pt1, pt_type & circle_pt2, pt_type & circle_pt3, pt_type & pt) {
	std::pair<pt_type, number> circle= circumcircle(circle_pt1, circle_pt2, circle_pt3);
	pt_type center= circle.first;
	number radius= circle.second;
	number dist2center= sqrt((center.x- pt.x)* (center.x- pt.x)+ (center.y- pt.y)* (center.y- pt.y));
	return dist2center< radius;
}


// utilisé pour debug test_triangulation
void get_circle_center(pt_type & circle_pt1, pt_type & circle_pt2, pt_type & circle_pt3, pt_type & center, number * radius) {
	pt_type pt1, pt2, pt3;
	if (is_ccw(circle_pt1, circle_pt2, circle_pt3)) {
		pt1= circle_pt1;
		pt2= circle_pt2;
		pt3= circle_pt3;
	}
	else {
		pt1= circle_pt1;
		pt2= circle_pt3;
		pt3= circle_pt2;
	}

	pt_type d21= pt2- pt1;
	pt_type d31= pt3- pt1;
	pt_type d32= pt3- pt2;

	number lambda= 0.5f* (d31.x* d32.x+ d32.y* d31.y)/ (d32.y* d21.x- d21.y* d32.x);
	center= pt_type(0.5f* (pt1.x+ pt2.x)- lambda* d21.y, 0.5f* (pt1.y+ pt2.y)+ lambda* d21.x);

	*radius= sqrt((center.x- pt1.x)* (center.x- pt1.x)+ (center.y- pt1.y)* (center.y- pt1.y));
}


bool is_quad_convex(pt_type * pts) {
	bool is_positive= false;
	for (unsigned int i=0; i<4; ++i) {
		pt_type v1= pts[(i+ 1) % 4]- pts[i];
		pt_type v2= pts[(i+ 2) % 4]- pts[(i+ 1) % 4];
		number crossprod= cross2d(v1, v2);
		/*number dx1= pts[(i+ 1) % 4].x- pts[i].x;
		number dy1= pts[(i+ 1) % 4].y- pts[i].y;
		number dx2= pts[(i+ 2) % 4].x- pts[(i+ 1) % 4].x;
		number dy2= pts[(i+ 2) % 4].y- pts[(i+ 1) % 4].y;
		number crossprod= dx1* dy2- dy1* dx2;*/
		if (i== 0) {
			if (crossprod> 0.0) {
				is_positive= true;
			}
		}
		else {
			if (((is_positive) && (crossprod< 0.0)) || ((!is_positive) && (crossprod> 0.0))) {
				return false;
			}
		}
	}
	return true;
}


// ---------------------------------------------------------------------------------------------------
Polygon2D::Polygon2D() : _area(0.0), _centroid(pt_type(0.0)), _radius(0.0) {
	_aabb= new AABB_2D();
}


Polygon2D::Polygon2D(const Polygon2D & polygon) {
	_aabb= new AABB_2D(*polygon._aabb);
	set_points(polygon._pts);
}


Polygon2D::~Polygon2D() {
	_pts.clear();
	_normals.clear();
	delete _aabb;
}


void Polygon2D::set_points(number * points, unsigned int n_points, bool convexhull) {
	_pts.clear();
	for (unsigned int i=0; i<n_points; ++i) {
		_pts.push_back(pt_type(points[2* i], points[2* i+ 1]));
	}
	if (convexhull) {
		convex_hull_2d(_pts);
	}
	update_attributes();
}


void Polygon2D::set_points(std::vector<pt_type> pts, bool convexhull) {
	_pts.clear();
	for (auto & pt : pts) {
		_pts.push_back(pt_type(pt));
	}
	if (convexhull) {
		convex_hull_2d(_pts);
	}
	update_attributes();
}


void Polygon2D::randomize(unsigned int n_points, number radius, pt_type center, bool convexhull) {
	_pts.clear();
	for (unsigned int i=0; i<n_points; ++i) {
		number x= center.x+ rand_number(-radius, radius);
		number y= center.y+ rand_number(-radius, radius);
		_pts.push_back(pt_type(x, y));
	}
	if (convexhull) {
		convex_hull_2d(_pts);
	}
	update_attributes();
}


void Polygon2D::set_rectangle(pt_type origin, pt_type size) {
	_pts.clear();
	_pts.push_back(origin);
	_pts.push_back(origin+ pt_type(size.x, 0.0));
	_pts.push_back(origin+ size);
	_pts.push_back(origin+ pt_type(0.0, size.y));
	update_attributes();
}


void Polygon2D::update_attributes() {
	// calcul aire
	_area= 0.0;
	for (unsigned int i=0; i<_pts.size(); ++i) {
		pt_type pt1= _pts[i];
		pt_type pt2= _pts[(i+ 1)% _pts.size()];
		_area+= 0.5f* (pt1.x* pt2.y- pt1.y* pt2.x);
	}

	// calcul centre de gravité
	_centroid= pt_type(0.0);
	for (unsigned int i=0; i<_pts.size(); ++i) {
		pt_type pt1= _pts[i];
		pt_type pt2= _pts[(i+ 1)% _pts.size()];
		_centroid+= (0.5f* THIRD/ _area)* (pt1.x* pt2.y- pt1.y* pt2.x)* (pt1+ pt2);
	}

	// a mettre en option ?
	// on met le centre du polygon sur le centre de gravité
	/*for (unsigned int i=0; i<_pts.size(); ++i) {
		_pts[i]-= _centroid;
	}
	_centroid= pt_type(0.0);*/

	// si clockwise -> anticlockwise
	if (_area< 0.0) {
		_area*= -1.0;
		reverse(_pts.begin(), _pts.end());
	}

	// calcul normales (norme == 1)
	// on doit etre en anticlockwise a ce moment ; on fait une rotation de -90 ie (x,y)->(y,-x)
	// pour que la normale pointe vers l'extérieur du polygone
	_normals.clear();
	for (unsigned int i=0; i<_pts.size(); ++i) {
		pt_type pt1= _pts[i];
		pt_type pt2= _pts[(i+ 1)% _pts.size()];
		_normals.push_back(glm::normalize(pt_type(pt2.y- pt1.y, pt1.x- pt2.x)));
	}

	// calcul rayon cercle englobant ; n'est pertinent que si le poly est convexe...
	_radius= 0.0;
	for (auto it_pt : _pts) {
		number dist2= glm::distance2(it_pt, _centroid);
		if (dist2> _radius) {
			_radius= dist2;
		}
	}
	_radius= sqrt(_radius);

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


// pt du polygon le + éloigné le long d'une direction
pt_type Polygon2D::farthest_pt_along_dir(pt_type direction) {
	number dist_max= -FLT_MAX;
	pt_type farthest_pt;
	for (unsigned int idx_pt=0; idx_pt<_pts.size(); ++idx_pt) {
		number dist= glm::dot(direction, _pts[idx_pt]);
		if (dist> dist_max) {
			dist_max= dist;
			farthest_pt= _pts[idx_pt];
		}
	}

	return farthest_pt;
}


void Polygon2D::print() {
	cout << "area= " << _area << " ; centroid= " << glm::to_string(_centroid);
	cout << " ; aabb_pos=" << glm::to_string(_aabb->_pos) << " ; aabb_size=" << glm::to_string(_aabb->_size);
	cout << "\n";
}
