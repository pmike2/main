#include <iostream>
#include <iomanip>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "geom_2d.h"
#include "utile.h"


pt_2d rot(pt_2d v, number alpha) {
	return pt_2d(v.x* cos(alpha)- v.y* sin(alpha), v.x* sin(alpha)+ v.y* cos(alpha));
}


number norm(pt_2d v) {
	return sqrt(v.x * v.x + v.y * v.y);
}


number norm2(pt_2d v) {
	return v.x * v.x + v.y * v.y;
}


pt_2d normalized(pt_2d v) {
	return v/ norm(v);
}


number scal(pt_2d u, pt_2d v) {
	return u.x* v.x+ u.y* v.y;
}


number determinant(pt_2d u, pt_2d v) {
	return u.x* v.y- u.y* v.x;
}


pt_2d proj(pt_2d v2proj, pt_2d v_proj_on) {
	pt_2d v_unit= normalized(v_proj_on);
	number s= scal(v2proj, v_unit);
	return s* v_unit;
}


number angle(pt_2d u, pt_2d v) {
	return atan2(determinant(u, v), scal(u, v));
}

// angle -> matrice de rotation
void rotation_float2mat(float rot, mat_2d & mat) {
	// glm est en column-major order par défaut -> mat[col][row]
	mat[0][0]= cos(rot);
	mat[0][1]= sin(rot);
	mat[1][0]= -sin(rot);
	mat[1][1]= cos(rot);
}


// renvoie la norme de la composante z du prod vectoriel
number cross2d(const pt_2d & v1, const pt_2d & v2) {
	return v1.x* v2.y- v1.y* v2.x;
}


// est-ce que situé en pt_ref, regardant vers dir_ref, pt_test est à gauche
bool is_left(const pt_2d & pt_ref, const pt_2d & dir_ref, const pt_2d & pt_test) {
	return cross2d(pt_2d(pt_test- pt_ref), dir_ref)<= 0.0;
}


bool is_ccw(const pt_2d & pt1, const pt_2d & pt2, const pt_2d & pt3) {
	return (pt2.x- pt1.x)* (pt3.y- pt1.y)- (pt3.x- pt1.x)* (pt2.y- pt1.y)> 0.0;
}


// cf https://en.wikipedia.org/wiki/Curve_orientation
bool is_ccw(const std::vector<pt_2d> & pts) {
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


bool point_in_aabb2d(const pt_2d & pt, const AABB_2D * aabb) {
	return ((pt.x>= aabb->_pos.x) && (pt.x<= aabb->_pos.x+ aabb->_size.x) && (pt.y>= aabb->_pos.y) && (pt.y<= aabb->_pos.y+ aabb->_size.y));
}


bool aabb2d_intersects_aabb2d(const AABB_2D * aabb_1, const AABB_2D * aabb_2) {
	return ((aabb_1->_pos.x<= aabb_2->_pos.x+ aabb_2->_size.x) && (aabb_1->_pos.x+ aabb_1->_size.x>= aabb_2->_pos.x) && (aabb_1->_pos.y<= aabb_2->_pos.y+ aabb_2->_size.y) && (aabb_1->_pos.y+ aabb_1->_size.y>= aabb_2->_pos.y));
}

bool aabb2d_contains_aabb2d(const AABB_2D * big_aabb, const AABB_2D * small_aabb) {
	return (big_aabb->_pos.x<= small_aabb->_pos.x && big_aabb->_pos.y<= small_aabb->_pos.y && big_aabb->_size.x>= small_aabb->_size.x && big_aabb->_size.y>= small_aabb->_size.y);
}


bool ray_intersects_aabb2d(const pt_2d & ray_origin, const pt_2d & ray_dir, const AABB_2D * aabb, pt_2d & contact_pt, pt_2d & contact_normal, number & t_hit_near) {
	contact_pt.x= 0.0;
	contact_pt.y= 0.0;
	contact_normal.x= 0.0;
	contact_normal.y= 0.0;
	t_hit_near= 0.0;

	// choisir ici une valeur suffisamment petite, dans le code original il fait un std::isnan
	//if (glm::length2(ray_dir)< 1e-9) {
	if ((ray_dir.x== 0.0) && (ray_dir.y== 0.0)) {
		return false;
	}

	pt_2d ray_dir_inv= (number)(1.0)/ ray_dir;

	pt_2d k_near= (aabb->_pos- ray_origin)* ray_dir_inv;
	pt_2d k_far = (aabb->_pos+ aabb->_size- ray_origin)* ray_dir_inv;

	/*
	cout << "___\n";
	cout << "ray_dir= (" << ray_dir.x << " ; " << ray_dir.y << ")\n";
	cout << "ray_dir_inv= (" << ray_dir_inv.x << " ; " << ray_dir_inv.y << ")\n";
	cout << "k_near= (" << k_near.x << " ; " << k_near.y << ")\n";
	cout << "k_far= (" << k_far.x << " ; " << k_far.y << ")\n";
	cout << "___\n";
	*/

	if (k_near.x> k_far.x) {
		std::swap(k_near.x, k_far.x);
	}
	if (k_near.y> k_far.y) {
		std::swap(k_near.y, k_far.y);
	}

	if ((k_near.x> k_far.y) || (k_near.y> k_far.x)) {
		return false;
	}

	t_hit_near= (number)(std::max(k_near.x, k_near.y));
	number t_hit_far= (number)(std::min(k_far.x, k_far.y));

	if (t_hit_far< 0.0) {
		return false;
	}

	contact_pt= ray_origin+ t_hit_near* ray_dir;

	if (k_near.x> k_near.y) {
		if (ray_dir.x< 0.0) {
			contact_normal.x= 1.0;
			contact_normal.y= 0.0;
		}
		else {
			contact_normal.x= -1.0;
			contact_normal.y= 0.0;
		}
	}
	else if (k_near.x< k_near.y) {
		if (ray_dir.y< 0.0) {
			contact_normal.x= 0.0;
			contact_normal.y= 1.0;
		}
		else {
			contact_normal.x= 0.0;
			contact_normal.y= -1.0;
		}
	}

	return true;
}


// TODO : ne pas utiliser Polygon2D pour ce test
bool bbox2d_intersects_bbox2d(const BBox_2D * bbox1, const BBox_2D * bbox2, pt_2d * axis, number * overlap, uint * idx_pt, bool * is_pt_in_poly1) {
	Polygon2D * poly1= new Polygon2D();
	poly1->set_bbox(*bbox1);
	Polygon2D * poly2= new Polygon2D();
	poly2->set_bbox(*bbox2);
	bool result= poly_intersects_poly(poly1, poly2, axis, overlap, idx_pt, is_pt_in_poly1);
	delete poly1;
	delete poly2;
	return result;
}


bool pt_in_bbox2d(const pt_2d & pt, const BBox_2D * bbox) {
	for (uint i=0; i<4; ++i) {
		uint j = i + 1;
		if (j >= 4) {
			j = 0;
		}
		if (!is_left(bbox->_pts[i], bbox->_pts[j] - bbox->_pts[i], pt)) {
			return false;
		}
	}
	return true;
}


std::pair<BBOX_SIDE, BBOX_CORNER> bbox2d_side_corner(const BBox_2D * bbox, const pt_2d & pt) {
	//const number THRESHOLD= 0.01;

	number bottom, right, top, left;
	pt_2d proj;
	distance_segment_pt(bbox->_pts[0], bbox->_pts[1], pt, &bottom, &proj);
	distance_segment_pt(bbox->_pts[1], bbox->_pts[2], pt, &right, &proj);
	distance_segment_pt(bbox->_pts[2], bbox->_pts[3], pt, &top, &proj);
	distance_segment_pt(bbox->_pts[3], bbox->_pts[0], pt, &left, &proj);

	number bottomleft= (bbox->_pts[0].x- pt.x)* (bbox->_pts[0].x- pt.x)+ (bbox->_pts[0].y- pt.y)* (bbox->_pts[0].y- pt.y);
	number bottomright= (bbox->_pts[1].x- pt.x)* (bbox->_pts[1].x- pt.x)+ (bbox->_pts[1].y- pt.y)* (bbox->_pts[1].y- pt.y);
	number topright= (bbox->_pts[2].x- pt.x)* (bbox->_pts[2].x- pt.x)+ (bbox->_pts[2].y- pt.y)* (bbox->_pts[2].y- pt.y);
	number topleft= (bbox->_pts[3].x- pt.x)* (bbox->_pts[3].x- pt.x)+ (bbox->_pts[3].y- pt.y)* (bbox->_pts[3].y- pt.y);

	BBOX_SIDE side;
	if (bottom<= right && bottom<= top && bottom<= left) {
		side= BOTTOM_SIDE;
	}
	else if (right<= bottom && right<= top && right<= left) {
		side= RIGHT_SIDE;
	}
	else if (top<= bottom && top<= right && top<= left) {
		side= TOP_SIDE;
	}
	else if (left<= bottom && left<= top && left<= right) {
		side= LEFT_SIDE;
	}
	else {
		side= NO_SIDE;
		std::cerr << "bottom=" << bottom << " ; right=" << right << " ; top=" << top << " ; left=" << left << "\n";
	}

	BBOX_CORNER corner;
	if (bottomleft<= bottomright && bottomleft<= topright && bottomleft<= topleft) {
		corner= BOTTOMLEFT_CORNER;
		//if (bottomleft< THRESHOLD) {side= NO_SIDE;}
	}
	else if (bottomright<= bottomleft && bottomright<= topright && bottomright<= topleft) {
		corner= BOTTOMRIGHT_CORNER;
		//if (bottomright< THRESHOLD) {side= NO_SIDE;}
	}
	else if (topright<= bottomleft && topright<= bottomright && topright<= topleft) {
		corner= TOPRIGHT_CORNER;
		//if (topright< THRESHOLD) {side= NO_SIDE;}
	}
	else if (topleft<= bottomleft && topleft<= topright && topleft<= bottomright) {
		corner= TOPLEFT_CORNER;
		//if (topleft< THRESHOLD) {side= NO_SIDE;}
	}
	else {
		corner= NO_CORNER;
		std::cerr << "bottomleft=" << bottomleft << " ; bottomright=" << bottomright << " ; topright=" << topright << " ; topleft=" << topleft << "\n";
	}

	return std::make_pair(side, corner);
}


// cf https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
bool segment_intersects_segment(const pt_2d & pt1_begin, const pt_2d & pt1_end, const pt_2d & pt2_begin, const pt_2d & pt2_end, pt_2d * result, bool exclude_seg1_extremities, bool exclude_seg2_extremities) {
	number xmin1, xmax1, ymin1, ymax1;
	number xmin2, xmax2, ymin2, ymax2;
	if (pt1_begin.x < pt1_end.x) {
		xmin1 = pt1_begin.x;
		xmax1 = pt1_end.x;
	}
	else {
		xmin1 = pt1_end.x;
		xmax1 = pt1_begin.x;
	}
	if (pt1_begin.y < pt1_end.y) {
		ymin1 = pt1_begin.y;
		ymax1 = pt1_end.y;
	}
	else {
		ymin1 = pt1_end.y;
		ymax1 = pt1_begin.y;
	}
	if (pt2_begin.x < pt2_end.x) {
		xmin2 = pt2_begin.x;
		xmax2 = pt2_end.x;
	}
	else {
		xmin2 = pt2_end.x;
		xmax2 = pt2_begin.x;
	}
	if (pt2_begin.y < pt2_end.y) {
		ymin2 = pt2_begin.y;
		ymax2 = pt2_end.y;
	}
	else {
		ymin2 = pt2_end.y;
		ymax2 = pt2_begin.y;
	}

	if (xmax1 < xmin2 || xmax2 < xmin1 || ymax1 < ymin2 || ymax2 < ymin1) {
		return false;
	}

	pt_2d dir1= pt1_end- pt1_begin;
	pt_2d dir2= pt2_end- pt2_begin;
	
	// parallèles
	number a= cross2d(dir1, dir2);
	//if (abs(a)<= EPSILON) {
	if (a== 0.0) {
		return false;
	}
	
	number t1= cross2d(pt2_begin- pt1_begin, dir2)/ a;

	// on fait ca ici meme si on renvoie false c'est utile dans les cas limites
	if (result!= NULL) {
		result->x= pt1_begin.x+ t1* dir1.x;
		result->y= pt1_begin.y+ t1* dir1.y;
	}

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


bool ray_intersects_segment(const pt_2d & origin, const pt_2d & direction, const pt_2d & pt_begin, const pt_2d & pt_end, pt_2d * result) {
	pt_2d dir_segment= pt_end- pt_begin;
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


bool ray_intersects_ray(const pt_2d & origin1, const pt_2d & direction1, const pt_2d & origin2, const pt_2d & direction2, pt_2d * result) {
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


bool is_pt_inside_poly(const pt_2d & pt, const Polygon2D * poly) {
	// ne fonctionne que pour les polygones convexes
	/*if (glm::distance(pt, poly->_centroid)> poly->_radius) {
		return false;
	}*/
	/*for (uint i=0; i<poly->_pts.size(); ++i) {
		pt_2d pt1= poly->_pts[i];
		pt_2d pt2= poly->_pts[(i+ 1)% poly->_pts.size()];
		if (!is_left(pt1, pt2- pt1, pt)) {
			return false;
		}
	}
	return true;*/

	if (!point_in_aabb2d(pt, poly->_aabb)) {
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


bool is_poly_inside_poly(const Polygon2D * small_poly, const Polygon2D * big_poly) {
	if (!aabb2d_contains_aabb2d(big_poly->_aabb, small_poly->_aabb)) {
		return false;
	}
	for (auto pt : small_poly->_pts) {
		if (!is_pt_inside_poly(pt, big_poly)) {
			return false;
		}
	}
	return true;
}

// Separating Axis Theorem (SAT); ne fonctionne que pour poly convexe; il faudrait sinon décomposer les polys en polys convexes
// https://dyn4j.org/2010/01/sat/
bool poly_intersects_poly(const Polygon2D * poly1, const Polygon2D * poly2, pt_2d * axis, number * overlap, uint * idx_pt, bool * is_pt_in_poly1) {

	std::vector<pt_2d> axis2check;
	axis2check.insert(axis2check.end(), poly1->_normals.begin(), poly1->_normals.end());
	axis2check.insert(axis2check.end(), poly2->_normals.begin(), poly2->_normals.end());

	*overlap= 1e10;
	*idx_pt= 0;
	*is_pt_in_poly1= false;
	
	uint idx_pt_normal_min= 0;
	uint idx_pt_normal_max= 0;
	uint idx_pt_other_min= 0;
	uint idx_pt_other_max= 0;
	number proj_normal_min, proj_normal_max;
	number proj_other_min, proj_other_max;
	number current_overlap;
	uint current_idx_pt;
	bool current_pt_in_poly1;

	for (uint idx_ax=0; idx_ax<axis2check.size(); ++idx_ax) {
		pt_2d ax= axis2check[idx_ax];
		
		// poly1 a la normale sur laquelle on projette
		if (idx_ax< poly1->_normals.size()) {
			current_pt_in_poly1= false;
			poly1->min_max_pt_along_dir(ax, &idx_pt_normal_min, &proj_normal_min, &idx_pt_normal_max, &proj_normal_max);
			poly2->min_max_pt_along_dir(ax, &idx_pt_other_min, &proj_other_min, &idx_pt_other_max, &proj_other_max);
		}
		// poly2 a la normale sur laquelle on projette
		else {
			current_pt_in_poly1= true;
			poly2->min_max_pt_along_dir(ax, &idx_pt_normal_min, &proj_normal_min, &idx_pt_normal_max, &proj_normal_max);
			poly1->min_max_pt_along_dir(ax, &idx_pt_other_min, &proj_other_min, &idx_pt_other_max, &proj_other_max);
		}
		
		// nmin < nmax < omin < omax -> pas d'intersection
		if (proj_other_min> proj_normal_max) {
			return false;
		}
		// omin < omax < nmin < nmax -> pas d'intersection
		else if (proj_normal_min> proj_other_max) {
			return false;
		}

		// tous ces cas mènent au même code (à vérifier ?) mais je laisse une trace du raisonnement
		else if (proj_other_min> proj_normal_min) {
			// nmin < omin < nmax < omax
			if (proj_other_max> proj_normal_max) {
				current_overlap= proj_normal_max- proj_other_min;
				current_idx_pt= idx_pt_other_min;
			}
			// nmin < omin < omax < nmax
			else {
				current_overlap= proj_normal_max- proj_other_min;
				current_idx_pt= idx_pt_other_min;
			}
		}
		else {
			// omin < nmin < nmax < omax
			if (proj_other_max> proj_normal_max) {
				current_overlap= proj_normal_max- proj_other_min;
				current_idx_pt= idx_pt_other_min;
			}
			// omin < nmin < omax < nmax
			else {
				current_overlap= proj_normal_max- proj_other_min;
				current_idx_pt= idx_pt_other_min;
			}
		}

		if (current_overlap< *overlap) {
			*overlap= current_overlap;
			*is_pt_in_poly1= current_pt_in_poly1;
			*idx_pt= current_idx_pt;
			axis->x= ax.x;
			axis->y= ax.y;

			/*std::cout << "overlap=" << *overlap << " ; idx_ax=" << idx_ax << " ; ax=(" << ax.x << " , " << ax.y << ")";
			std::cout << " ; is_pt_in_poly1=" << *is_pt_in_poly1 << " ; idx_pt=" << *idx_pt;
			std::cout << " ; proj_normal_min=" << proj_normal_min << " ; proj_normal_max=" << proj_normal_max;
			std::cout << " ; proj_other_min=" << proj_other_min << " ; proj_other_max=" << proj_other_max;
			std::cout << "\n";*/
		}
	}
	return true;
}


// si existe intersection la + proche du pt de départ du segment avec le poly
bool segment_intersects_poly(const pt_2d & pt_begin, const pt_2d & pt_end, const Polygon2D * poly, pt_2d * result) {
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

	number min_dist= 1e10;
	bool is_inter= false;
	pt_2d inter(0.0);
	for (uint i=0; i<poly->_pts.size(); ++i) {
		pt_2d pt1= poly->_pts[i];
		pt_2d pt2= poly->_pts[(i+ 1)% poly->_pts.size()];
		
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
bool segment_intersects_poly_multi(const pt_2d & pt_begin, const pt_2d & pt_end, const Polygon2D * poly, std::vector<pt_2d> * result) {
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

	number min_dist= 1e10;
	bool is_inter= false;
	pt_2d inter(0.0);
	for (uint i=0; i<poly->_pts.size(); ++i) {
		pt_2d pt1= poly->_pts[i];
		pt_2d pt2= poly->_pts[(i+ 1)% poly->_pts.size()];
		
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
bool distance_segment_pt(const pt_2d & seg1, const pt_2d & seg2, const pt_2d & pt, number * dist, pt_2d * proj) {
	number seg_norm2= glm::distance2(seg1, seg2);
	
	if (seg_norm2< EPSILON) {
		proj->x= seg1.x;
		proj->y= seg1.y;
		*dist= glm::distance(seg1, pt);
		return true;
	}
	
	number t= glm::dot(pt- seg1, seg2- seg1)/ seg_norm2;
	bool proj_in_segment= true;
	if ((t< 0.0) || (t> 1.0)) {
		t= std::max((number)(0.0), std::min((number)(1.0), t));
		proj_in_segment= false;
	}
	
	proj->x= seg1.x+ t* (seg2.x- seg1.x);
	proj->y= seg1.y+ t* (seg2.y- seg1.y);
	*dist= glm::distance(pt, *proj);
	return proj_in_segment;
}


number distance_poly_pt(const Polygon2D * poly, const pt_2d & pt, pt_2d * proj) {
	number dist_min= 1e10;
	number dist;
	pt_2d proj2;

	if (is_pt_inside_poly(pt, poly)) {
		if (proj!= NULL) {
			proj->x= pt.x;
			proj->y= pt.y;
		}
		return 0.0;
	}

	for (uint i=0; i<poly->_pts.size(); ++i) {
		pt_2d pt1= poly->_pts[i];
		pt_2d pt2= poly->_pts[(i+ 1)% poly->_pts.size()];

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


number distance_poly_segment(const Polygon2D * poly, const pt_2d & seg1, const pt_2d & seg2, pt_2d * proj) {
	number dist_min= 1e10;
	number dist;
	pt_2d proj2;

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

	for (uint i=0; i<poly->_pts.size(); ++i) {
		pt_2d pt1= poly->_pts[i];
		pt_2d pt2= poly->_pts[(i+ 1)% poly->_pts.size()];

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

	for (uint i=0; i<poly->_pts.size(); ++i) {
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


std::pair<pt_2d, number> circumcircle(const pt_2d & circle_pt1, const pt_2d & circle_pt2, const pt_2d & circle_pt3) {
	pt_2d pt1, pt2, pt3;
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
	pt_2d d21= pt2- pt1;
	pt_2d d31= pt3- pt1;
	pt_2d d32= pt3- pt2;

	// points alignés
	if (d32.y* d21.x- d21.y* d32.x== 0.0) {
		return std::make_pair(pt_2d(0.0), 0.0); // bof
	}
	number lambda= 0.5f* (d31.x* d32.x+ d32.y* d31.y)/ (d32.y* d21.x- d21.y* d32.x);
	pt_2d center= pt_2d(0.5f* (pt1.x+ pt2.x)- lambda* d21.y, 0.5f* (pt1.y+ pt2.y)+ lambda* d21.x);
	number radius= sqrt((center.x- pt1.x)* (center.x- pt1.x)+ (center.y- pt1.y)* (center.y- pt1.y));
	return std::make_pair(center, radius);
}


bool point_in_circumcircle(const pt_2d & circle_pt1, const pt_2d & circle_pt2, const pt_2d & circle_pt3, const pt_2d & pt) {
	std::pair<pt_2d, number> circle= circumcircle(circle_pt1, circle_pt2, circle_pt3);
	pt_2d center= circle.first;
	number radius= circle.second;
	return point_in_circle(center, radius, pt);
}


bool point_in_circle(const pt_2d & center, number radius, const pt_2d & pt) {
	number dist2center= sqrt((center.x- pt.x)* (center.x- pt.x)+ (center.y- pt.y)* (center.y- pt.y));
	return dist2center< radius;
}

// utilisé pour debug test_triangulation
void get_circle_center(const pt_2d & circle_pt1, const pt_2d & circle_pt2, const pt_2d & circle_pt3, pt_2d & center, number * radius) {
	pt_2d pt1, pt2, pt3;
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

	pt_2d d21= pt2- pt1;
	pt_2d d31= pt3- pt1;
	pt_2d d32= pt3- pt2;

	number lambda= 0.5f* (d31.x* d32.x+ d32.y* d31.y)/ (d32.y* d21.x- d21.y* d32.x);
	center= pt_2d(0.5f* (pt1.x+ pt2.x)- lambda* d21.y, 0.5f* (pt1.y+ pt2.y)+ lambda* d21.x);

	*radius= sqrt((center.x- pt1.x)* (center.x- pt1.x)+ (center.y- pt1.y)* (center.y- pt1.y));
}


bool is_quad_convex(const pt_2d * pts) {
	bool is_positive= false;
	for (uint i=0; i<4; ++i) {
		pt_2d v1= pts[(i+ 1) % 4]- pts[i];
		pt_2d v2= pts[(i+ 2) % 4]- pts[(i+ 1) % 4];
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


// calcul convex hull 2D bouquin computational geom
void convex_hull_2d(std::vector<pt_2d> & pts) {
	std::sort(pts.begin(), pts.end(), [](pt_2d & pt1, pt_2d & pt2){ return pt1.x < pt2.x;});

	std::vector<pt_2d> pts_upper;
	pts_upper.push_back(pts[0]);
	pts_upper.push_back(pts[1]);
	for (uint i=2; i<pts.size(); ++i) {
		pts_upper.push_back(pts[i]);
		while ((pts_upper.size()> 2) && (is_left(pts_upper[pts_upper.size()- 3], pts_upper[pts_upper.size()- 2]- pts_upper[pts_upper.size()- 3], pts_upper[pts_upper.size()- 1]))) {
			pts_upper.erase(pts_upper.end()- 2);
		}
	}

	std::vector<pt_2d> pts_lower;
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


