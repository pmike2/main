#include <iostream>
#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "utile.h"

#include "geom.h"


// Intersection de 2 triangles en 3D
// http://web.stanford.edu/class/cs277/resources/papers/Moller1997b.pdf
// ATTENTION : reste à gérer le cas ou les triangles sont coplanaires !
bool triangle_intersects_triangle(pt_3d v[3], pt_3d w[3]) {
	// -------
	pt_3d nv= glm::cross(v[1]- v[0], v[2]- v[0]);
	number dv= -1.0f* glm::dot(nv, v[0]);
	pt_3d dvw;
	dvw[0]= glm::dot(nv, w[0])+ dv;
	dvw[1]= glm::dot(nv, w[1])+ dv;
	dvw[2]= glm::dot(nv, w[2])+ dv;
	if ((dvw[0]* dvw[1]> 0.0f) && (dvw[0]* dvw[2]> 0.0f)) {
		return false;
	}

	// -------
	pt_3d nw= glm::cross(w[1]- w[0], w[2]- w[0]);
	number dw= -1.0f* glm::dot(nw, w[0]);
	pt_3d dwv;
	dwv[0]= glm::dot(nw, v[0])+ dw;
	dwv[1]= glm::dot(nw, v[1])+ dw;
	dwv[2]= glm::dot(nw, v[2])+ dw;
	if ((dwv[0]* dwv[1]> 0.0f) && (dwv[0]* dwv[2]> 0.0f)) {
		return false;
	}

	// -------
	//pt_3d d= glm::normalize(glm::cross(nv, nw));
	pt_3d d= glm::cross(nv, nw);

	// -------
	uint v_idx_alone, v_idx_couple_1, v_idx_couple_2;
	if (dwv[0]* dwv[1]< 0.0f) {
		if (dwv[0]* dwv[2]< 0.0f) {
			v_idx_alone= 0;
			v_idx_couple_1= 1;
			v_idx_couple_2= 2;
		}
		else {
			v_idx_alone= 1;
			v_idx_couple_1= 0;
			v_idx_couple_2= 2;
		}
	}
	else {
		v_idx_alone= 2;
		v_idx_couple_1= 0;
		v_idx_couple_2= 1;
	}

	pt_3d v_dot_d;
	v_dot_d[0]= glm::dot(d, v[0]);
	v_dot_d[1]= glm::dot(d, v[1]);
	v_dot_d[2]= glm::dot(d, v[2]);

	number tv1= v_dot_d[v_idx_couple_1]+ (v_dot_d[v_idx_alone]- v_dot_d[v_idx_couple_1])* dwv[v_idx_couple_1]/ (dwv[v_idx_couple_1]- dwv[v_idx_alone]);
	number tv2= v_dot_d[v_idx_couple_2]+ (v_dot_d[v_idx_alone]- v_dot_d[v_idx_couple_2])* dwv[v_idx_couple_2]/ (dwv[v_idx_couple_2]- dwv[v_idx_alone]);

	// -------
	uint w_idx_alone, w_idx_couple_1, w_idx_couple_2;
	if (dvw[0]* dvw[1]< 0.0f) {
		if (dvw[0]* dvw[2]< 0.0f) {
			w_idx_alone= 0;
			w_idx_couple_1= 1;
			w_idx_couple_2= 2;
		}
		else {
			w_idx_alone= 1;
			w_idx_couple_1= 0;
			w_idx_couple_2= 2;
		}
	}
	else {
		w_idx_alone= 2;
		w_idx_couple_1= 0;
		w_idx_couple_2= 1;
	}

	pt_3d w_dot_d;
	w_dot_d[0]= glm::dot(d, w[0]);
	w_dot_d[1]= glm::dot(d, w[1]);
	w_dot_d[2]= glm::dot(d, w[2]);

	number tw1= w_dot_d[w_idx_couple_1]+ (w_dot_d[w_idx_alone]- w_dot_d[w_idx_couple_1])* dvw[w_idx_couple_1]/ (dvw[w_idx_couple_1]- dvw[w_idx_alone]);
	number tw2= w_dot_d[w_idx_couple_2]+ (w_dot_d[w_idx_alone]- w_dot_d[w_idx_couple_2])* dvw[w_idx_couple_2]/ (dvw[w_idx_couple_2]- dvw[w_idx_alone]);

	// -------
	/*cout << "v0=" << glm::to_string(v[0]) << " ; v1=" << glm::to_string(v[1]) << "v2=" << glm::to_string(v[2]) << "\n";
	cout << "w0=" << glm::to_string(w[0]) << " ; w1=" << glm::to_string(w[1]) << "w2=" << glm::to_string(w[2]) << "\n";
	cout << "nv=" << glm::to_string(nv) << "\n";
	cout << "nw=" << glm::to_string(nw) << "\n";
	cout << "d=" << glm::to_string(d) << "\n";
	cout << "v_idx_alone=" << v_idx_alone << " ; v_idx_couple_1=" << v_idx_couple_1 << " ; v_idx_couple_2=" << v_idx_couple_2 << "\n";
	cout << "w_idx_alone=" << w_idx_alone << " ; w_idx_couple_1=" << w_idx_couple_1 << " ; w_idx_couple_2=" << w_idx_couple_2 << "\n";
	cout << "v_dot_d[v_idx_alone]=" << v_dot_d[v_idx_alone] << " ; v_dot_d[v_idx_couple_1]=" << v_dot_d[v_idx_couple_1] << " ; v_dot_d[v_idx_couple_2]=" << v_dot_d[v_idx_couple_2] << "\n";
	cout << "w_dot_d[w_idx_alone]=" << w_dot_d[w_idx_alone] << " ; w_dot_d[w_idx_couple_1]=" << w_dot_d[w_idx_couple_1] << " ; w_dot_d[w_idx_couple_2]=" << w_dot_d[w_idx_couple_2] << "\n";
	cout << "tv1=" << tv1 << " ; tv2=" << tv2 << " ; tw1=" << tw1 << " ; tw2=" << tw2 << "\n";*/

	if ((std::max(tv1, tv2)< std::min(tw1, tw2)) || (std::min(tv1, tv2)> std::max(tw1, tw2))) {
		return false;
	}

	return true;
}


bool aabb_intersects_aabb(AABB * aabb_1, AABB * aabb_2) {
	if ((aabb_1->_vmin.x> aabb_2->_vmax.x) || (aabb_1->_vmax.x< aabb_2->_vmin.x) ||
		(aabb_1->_vmin.y> aabb_2->_vmax.y) || (aabb_1->_vmax.y< aabb_2->_vmin.y) ||
		(aabb_1->_vmin.z> aabb_2->_vmax.z) || (aabb_1->_vmax.z< aabb_2->_vmin.z)) {
		return false;
	}
	return true;
}


bool aabb_intersects_bbox(AABB * aabb, BBox * bbox) {
	for (uint i=0; i<8; ++i) {
		if ((bbox->_pts[i].x> aabb->_vmin.x) && (bbox->_pts[i].x< aabb->_vmax.x) &&
			(bbox->_pts[i].y> aabb->_vmin.y) && (bbox->_pts[i].y< aabb->_vmax.y) &&
			(bbox->_pts[i].z> aabb->_vmin.z) && (bbox->_pts[i].z< aabb->_vmax.z)) {
			return true;
		}
	}
	return false;
}


// https://www.jkh.me/files/tutorials/Separating%20Axis%20Theorem%20or%20Oriented%20Bounding%20Boxes.pdf
bool bbox_intersects_bbox(BBox * bbox_1, BBox * bbox_2) {
	pt_3d center_1= 0.5* (bbox_1->_pts[7]+ bbox_1->_pts[0]);
	pt_3d x_1= glm::normalize(bbox_1->_pts[1]- bbox_1->_pts[0]);
	pt_3d y_1= glm::normalize(bbox_1->_pts[2]- bbox_1->_pts[0]);
	pt_3d z_1= glm::normalize(bbox_1->_pts[4]- bbox_1->_pts[0]);
	number half_width_1= 0.5* (bbox_1->_vmax[0]- bbox_1->_vmin[0]);
	number half_height_1= 0.5* (bbox_1->_vmax[1]- bbox_1->_vmin[1]);
	number half_depth_1= 0.5* (bbox_1->_vmax[2]- bbox_1->_vmin[2]);

	pt_3d center_2= 0.5* (bbox_2->_pts[7]+ bbox_2->_pts[0]);
	pt_3d x_2= glm::normalize(bbox_2->_pts[1]- bbox_2->_pts[0]);
	pt_3d y_2= glm::normalize(bbox_2->_pts[2]- bbox_2->_pts[0]);
	pt_3d z_2= glm::normalize(bbox_2->_pts[4]- bbox_2->_pts[0]);
	number half_width_2= 0.5* (bbox_2->_vmax[0]- bbox_2->_vmin[0]);
	number half_height_2= 0.5* (bbox_2->_vmax[1]- bbox_2->_vmin[1]);
	number half_depth_2= 0.5* (bbox_2->_vmax[2]- bbox_2->_vmin[2]);

	pt_3d axes[15]= {
		x_1, y_1, z_1, x_2, y_2, z_2, 
		glm::cross(x_1, x_2), glm::cross(x_1, y_2), glm::cross(x_1, z_2), 
		glm::cross(y_1, x_2), glm::cross(y_1, y_2), glm::cross(y_1, z_2), 
		glm::cross(z_1, x_2), glm::cross(z_1, y_2), glm::cross(z_1, z_2)
	};

	for (unsigned i=0; i<15; ++i) {
		number a= abs(glm::dot(axes[i], center_2- center_1));
		number b= abs(half_width_1* glm::dot(axes[i], x_1))+ abs(half_height_1* glm::dot(axes[i], y_1))+ abs(half_depth_1* glm::dot(axes[i], z_1))+
				 abs(half_width_2* glm::dot(axes[i], x_2))+ abs(half_height_2* glm::dot(axes[i], y_2))+ abs(half_depth_2* glm::dot(axes[i], z_2));
		if (a> b) {
			return false;
		}
	}
	return true;
}


number aabb_distance_pt_2(AABB * aabb, const pt_3d & pt) {
	number dx, dy, dz;
	
	if (pt.x> aabb->_vmax.x) {
		dx= pt.x- aabb->_vmax.x;
	}
	else if (pt.x< aabb->_vmin.x) {
		dx= aabb->_vmin.x- pt.x;
	}
	else {
		dx= 0.0;
	}

	if (pt.y> aabb->_vmax.y) {
		dy= pt.y- aabb->_vmax.y;
	}
	else if (pt.y< aabb->_vmin.y) {
		dy= aabb->_vmin.y- pt.y;
	}
	else {
		dy= 0.0;
	}

	if (pt.z> aabb->_vmax.z) {
		dz= pt.z- aabb->_vmax.z;
	}
	else if (pt.z< aabb->_vmin.z) {
		dz= aabb->_vmin.z- pt.z;
	}
	else {
		dz= 0.0;
	}

	return dx* dx+ dy* dy+ dz* dz;
}


number aabb_distance_pt(AABB * aabb, const pt_3d & pt) {
	return sqrt(aabb_distance_pt_2(aabb, pt));
}


// cf https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
bool ray_intersects_aabb(pt_3d origin, pt_3d direction, AABB * aabb, number & t_hit) {
	direction= glm::normalize(direction);
	if (direction.x== 0.0) {
		direction.x= 1e-7;
	}
	if (direction.y== 0.0) {
		direction.y= 1e-7;
	}
	if (direction.z== 0.0) {
		direction.z= 1e-7;
	}
	pt_3d dirfrac(1.0/ direction.x, 1.0/ direction.y, 1.0/ direction.z);
	number t1= (aabb->_vmin.x- origin.x)* dirfrac.x;
	number t2= (aabb->_vmax.x- origin.x)* dirfrac.x;
	number t3= (aabb->_vmin.y- origin.y)* dirfrac.y;
	number t4= (aabb->_vmax.y- origin.y)* dirfrac.y;
	number t5= (aabb->_vmin.z- origin.z)* dirfrac.z;
	number t6= (aabb->_vmax.z- origin.z)* dirfrac.z;

	number tmin= std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
	number tmax= std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

	// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
	if (tmax < 0) {
		t_hit= tmax;
		return false;
	}

	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax) {
		t_hit= tmax;
		return false;
	}

	t_hit= tmin;
	return true;
}


bool segment_intersects_aabb(const pt_3d & pt1, const pt_3d & pt2, AABB * aabb) {
	number t_hit;
	bool ray_inter= ray_intersects_aabb(pt1, pt2- pt1, aabb, t_hit);
	//cout << ray_inter << " ; " << t_hit << " ; " << glm::length(pt2- pt1) << "\n";
	if ((!ray_inter) || (t_hit> glm::length(pt2- pt1))) {
		return false;
	}
	return true;
}

