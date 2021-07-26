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
float cross2d(glm::vec2 v1, glm::vec2 v2) {
    return v1.x* v2.y- v1.y* v2.x;
}


// tri de points selon x ; utile au calcul de convex hull
bool cmp_points(glm::vec2 & pt1, glm::vec2 & pt2) {
    return pt1.x< pt2.x;
}


// est-ce que situé en pt_ref, regardant vers dir_ref, pt_test est à gauche
bool is_left(glm::vec2 pt_ref, glm::vec2 dir_ref, glm::vec2 pt_test) {
    return cross2d(glm::vec2(pt_test- pt_ref), dir_ref)<= 0.0f;
}


bool is_pt_inside_poly(glm::vec2 & pt, Polygon2D * poly) {
	// ne fonctionne que pour les polygones convexes
    /*if (glm::distance(pt, poly->_centroid)> poly->_radius) {
        return false;
    }*/
    /*for (unsigned int i=0; i<poly->_pts.size(); ++i) {
        glm::vec2 pt1= poly->_pts[i];
        glm::vec2 pt2= poly->_pts[(i+ 1)% poly->_pts.size()];
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


// cf https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
bool segment_intersects_segment(glm::vec2 & pt1_begin, glm::vec2 & pt1_end, glm::vec2 & pt2_begin, glm::vec2 & pt2_end, glm::vec2 * result, bool exclude_extremities) {
    glm::vec2 dir1= pt1_end- pt1_begin;
    glm::vec2 dir2= pt2_end- pt2_begin;
    
    // parallèles
    float a= cross2d(dir1, dir2);
    //if (abs(a)<= EPSILON) {
	if (a== 0.0f) {
        return false;
    }
    
	float t1= cross2d(pt2_begin- pt1_begin, dir2)/ a;
	if (exclude_extremities) {
		//if ((t1<= EPSILON) || (t1>= 1.0f- EPSILON)) {
		if ((t1<= 0.0f) || (t1>= 1.0f)) {
			return false;
		}
	}
	else {
		if ((t1< 0.0f) || (t1> 1.0f)) {
        	return false;
    	}
	}
    
	float t2= cross2d(pt2_begin- pt1_begin, dir1)/ a;
	if (exclude_extremities) {
		//if ((t2<= EPSILON) || (t2>= 1.0f- EPSILON)) {
		if ((t2<= 0.0f) || (t2>= 1.0f)) {
			return false;
		}
	}
	else {
		if ((t2< 0.0f) || (t2> 1.0f)) {
			return false;
		}
	}

	result->x= pt1_begin.x+ t1* dir1.x;
    result->y= pt1_begin.y+ t1* dir1.y;
    return true;
}


bool ray_intersects_segment(glm::vec2 & origin, glm::vec2 & direction, glm::vec2 & pt_begin, glm::vec2 & pt_end, glm::vec2 * result) {
	glm::vec2 dir_segment= pt_end- pt_begin;
    // parallèles
    float a= cross2d(direction, dir_segment);
    if (abs(a)< EPSILON) {
        return false;
    }
    float t1= cross2d(pt_begin- origin, dir_segment)/ a;
    if (t1< 0.0f) {
        return false;
    }
    float t2= cross2d(pt_begin- origin, direction)/ a;
    if ((t2< 0.0f) || (t2> 1.0f)) {
        return false;
    }
    result->x= origin.x+ t1* direction.x;
    result->y= origin.y+ t1* direction.y;
    return true;
}


// si existe intersection la + proche du pt de départ du segment avec le poly
bool segment_intersects_poly(glm::vec2 & pt_begin, glm::vec2 & pt_end, Polygon2D * poly, glm::vec2 * result) {
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

    float min_dist= FLT_MAX;
    bool is_inter= false;
    glm::vec2 inter(0.0f);
    for (unsigned int i=0; i<poly->_pts.size(); ++i) {
        glm::vec2 pt1= poly->_pts[i];
        glm::vec2 pt2= poly->_pts[(i+ 1)% poly->_pts.size()];
        
        if (segment_intersects_segment(pt1, pt2, pt_begin, pt_end, &inter)) {
            float dist2= glm::distance2(pt_begin, inter);
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


// d(pt, [seg1, seg2])
bool distance_segment_pt(glm::vec2 & seg1, glm::vec2 & seg2, glm::vec2 & pt, float * dist, glm::vec2 * proj) {
    float seg_norm2= glm::distance2(seg1, seg2);
    bool proj_in_segment= true;
    
    if (seg_norm2< EPSILON) {
        return glm::distance(seg1, pt);
    }
    
    float t= glm::dot(pt- seg1, seg2- seg1)/ seg_norm2;
    if ((t< 0.0f) || (t> 1.0f)) {
        t= max(0.0f, min(1.0f, t));
        proj_in_segment= false;
    }
    
    proj->x= seg1.x+ t* (seg2.x- seg1.x);
    proj->y= seg1.y+ t* (seg2.y- seg1.y);
    *dist= glm::distance(pt, *proj);
    return proj_in_segment;
}


float distance_poly_pt(Polygon2D * poly, glm::vec2 & pt, glm::vec2 * proj) {
    float dist_min= FLT_MAX;
    float dist;
    glm::vec2 proj2;

    if (is_pt_inside_poly(pt, poly)) {
        if (proj!= NULL) {
            proj->x= pt.x;
            proj->y= pt.y;
        }
        return 0.0f;
    }

    for (unsigned int i=0; i<poly->_pts.size(); ++i) {
        glm::vec2 pt1= poly->_pts[i];
        glm::vec2 pt2= poly->_pts[(i+ 1)% poly->_pts.size()];

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


float distance_poly_segment(Polygon2D * poly, glm::vec2 & seg1, glm::vec2 & seg2, glm::vec2 * proj) {
    float dist_min= FLT_MAX;
    float dist;
    glm::vec2 proj2;

    if (is_pt_inside_poly(seg1, poly)) {
        // proj ?
        return 0.0f;
    }

    if (segment_intersects_poly(seg1, seg2, poly, &proj2)) {
        if (proj!= NULL) {
            proj->x= proj2.x;
            proj->y= proj2.y;
        }
        return 0.0f;
    }

    for (unsigned int i=0; i<poly->_pts.size(); ++i) {
        glm::vec2 pt1= poly->_pts[i];
        glm::vec2 pt2= poly->_pts[(i+ 1)% poly->_pts.size()];

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
void convex_hull_2d(std::vector<glm::vec2> & pts) {
    sort(pts.begin(), pts.end(), cmp_points);

    vector<glm::vec2> pts_upper;
    pts_upper.push_back(pts[0]);
    pts_upper.push_back(pts[1]);
    for (unsigned int i=2; i<pts.size(); ++i) {
        pts_upper.push_back(pts[i]);
		while ((pts_upper.size()> 2) && (is_left(pts_upper[pts_upper.size()- 3], pts_upper[pts_upper.size()- 2]- pts_upper[pts_upper.size()- 3], pts_upper[pts_upper.size()- 1]))) {
            pts_upper.erase(pts_upper.end()- 2);
        }
    }

    vector<glm::vec2> pts_lower;
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


bool is_ccw(glm::vec2 & pt1, glm::vec2 & pt2, glm::vec2 & pt3) {
	return (pt2.x- pt1.x)* (pt3.y- pt1.y)- (pt3.x- pt1.x)* (pt2.y- pt1.y)> 0.0f;
}


bool point_in_circumcircle(glm::vec2 & circle_pt1, glm::vec2 & circle_pt2, glm::vec2 & circle_pt3, glm::vec2 & pt) {
	glm::vec2 pt1, pt2, pt3;
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
	glm::vec2 d21= pt2- pt1;
	glm::vec2 d31= pt3- pt1;
	glm::vec2 d32= pt3- pt2;

	// points alignés
	if (d32.y* d21.x- d21.y* d32.x== 0.0f) {
		return false; // true ?
	}
	float lambda= 0.5f* (d31.x* d32.x+ d32.y* d31.y)/ (d32.y* d21.x- d21.y* d32.x);
	glm::vec2 center= glm::vec2(0.5f* (pt1.x+ pt2.x)- lambda* d21.y, 0.5f* (pt1.y+ pt2.y)+ lambda* d21.x);
	return (center.x- pt.x)* (center.x- pt.x)+ (center.y- pt.y)* (center.y- pt.y)< (center.x- pt1.x)* (center.x- pt1.x)+ (center.y- pt1.y)* (center.y- pt1.y);
}


// utilisé pour debug test_triangulation
void get_circle_center(glm::vec2 & circle_pt1, glm::vec2 & circle_pt2, glm::vec2 & circle_pt3, glm::vec2 & center, float * radius) {
	glm::vec2 pt1, pt2, pt3;
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

	glm::vec2 d21= pt2- pt1;
	glm::vec2 d31= pt3- pt1;
	glm::vec2 d32= pt3- pt2;

	float lambda= 0.5f* (d31.x* d32.x+ d32.y* d31.y)/ (d32.y* d21.x- d21.y* d32.x);
	center= glm::vec2(0.5f* (pt1.x+ pt2.x)- lambda* d21.y, 0.5f* (pt1.y+ pt2.y)+ lambda* d21.x);

	*radius= sqrt((center.x- pt1.x)* (center.x- pt1.x)+ (center.y- pt1.y)* (center.y- pt1.y));
}


bool is_quad_convex(glm::vec2 * pts) {
	bool is_positive= false;
	for (unsigned int i=0; i<4; ++i) {
		float dx1= pts[i+ 1].x- pts[i].x;
		float dy1= pts[i+ 1].y- pts[i].y;
		float dx2= pts[i+ 2].x- pts[i+ 1].x;
		float dy2= pts[i+ 2].y- pts[i+ 1].y;
		float crossprod= dx1* dy2- dy1* dx2;
		if (i== 0) {
			if (crossprod> 0.0f) {
				is_positive= true;
			}
		}
		else {
			if (((is_positive) && (crossprod< 0.0f)) || ((!is_positive) && (crossprod> 0.0f))) {
				return false;
			}
		}
	}
	return true;
}


// ---------------------------------------------------------------------------------------------------
Polygon2D::Polygon2D() : _area(0.0f), _centroid(glm::vec2(0.0f)), _radius(0.0f) {
	_aabb= new AABB_2D();
}


Polygon2D::~Polygon2D() {
    _pts.clear();
    _normals.clear();
	delete _aabb;
}


void Polygon2D::set_points(float * points, unsigned int n_points, bool convexhull) {
    _pts.clear();
    for (unsigned int i=0; i<n_points; ++i) {
        _pts.push_back(glm::vec2(points[2* i], points[2* i+ 1]));
    }
    if (convexhull) {
        convex_hull_2d(_pts);
    }
    update_attributes();
}


void Polygon2D::randomize(unsigned int n_points, float radius, glm::vec2 center, bool convexhull) {
    _pts.clear();
    for (unsigned int i=0; i<n_points; ++i) {
        float x= center.x+ rand_float(-radius, radius);
        float y= center.y+ rand_float(-radius, radius);
        _pts.push_back(glm::vec2(x, y));
    }
    if (convexhull) {
        convex_hull_2d(_pts);
    }
    update_attributes();
}


void Polygon2D::set_rectangle(glm::vec2 origin, glm::vec2 size) {
    _pts.clear();
    _pts.push_back(origin);
    _pts.push_back(origin+ glm::vec2(size.x, 0.0f));
    _pts.push_back(origin+ size);
    _pts.push_back(origin+ glm::vec2(0.0f, size.y));
    update_attributes();
}


void Polygon2D::update_attributes() {
    // calcul aire
    _area= 0.0f;
    for (unsigned int i=0; i<_pts.size(); ++i) {
        glm::vec2 pt1= _pts[i];
        glm::vec2 pt2= _pts[(i+ 1)% _pts.size()];
        _area+= 0.5f* (pt1.x* pt2.y- pt1.y* pt2.x);
    }

    // calcul centre de gravité
    _centroid= glm::vec2(0.0f);
    for (unsigned int i=0; i<_pts.size(); ++i) {
        glm::vec2 pt1= _pts[i];
        glm::vec2 pt2= _pts[(i+ 1)% _pts.size()];
        _centroid+= (0.5f* THIRD/ _area)* (pt1.x* pt2.y- pt1.y* pt2.x)* (pt1+ pt2);
    }

	// a mettre en option ?
    // on met le centre du polygon sur le centre de gravité
    /*for (unsigned int i=0; i<_pts.size(); ++i) {
        _pts[i]-= _centroid;
    }
    _centroid= glm::vec2(0.0f);*/

    // si clockwise -> anticlockwise
    if (_area< 0.0f) {
        _area*= -1.0f;
        reverse(_pts.begin(), _pts.end());
    }

    // calcul normales (norme == 1)
    // on doit etre en anticlockwise a ce moment ; on fait une rotation de -90 ie (x,y)->(y,-x)
    // pour que la normale pointe vers l'extérieur du polygone
    _normals.clear();
    for (unsigned int i=0; i<_pts.size(); ++i) {
        glm::vec2 pt1= _pts[i];
        glm::vec2 pt2= _pts[(i+ 1)% _pts.size()];
        _normals.push_back(glm::normalize(glm::vec2(pt2.y- pt1.y, pt1.x- pt2.x)));
    }

    // calcul rayon cercle englobant ; n'est pertinent que si le poly est convexe...
    _radius= 0.0f;
    for (auto it_pt : _pts) {
        float dist2= glm::distance2(it_pt, _centroid);
        if (dist2> _radius) {
            _radius= dist2;
        }
    }
    _radius= sqrt(_radius);

	// calcul AABB
	float xmin= 1e8;
	float ymin= 1e8;
	float xmax= -1e8;
	float ymax= -1e8;
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
glm::vec2 Polygon2D::farthest_pt_along_dir(glm::vec2 direction) {
    float dist_max= -FLT_MAX;
    glm::vec2 farthest_pt;
    for (unsigned int idx_pt=0; idx_pt<_pts.size(); ++idx_pt) {
        float dist= glm::dot(direction, _pts[idx_pt]);
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
