#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "bbox_2d.h"


using namespace std;


AABB_2D::AABB_2D() : _pos(pt_type(0.0)), _size(pt_type(0.0)) {

}


AABB_2D::AABB_2D(pt_type pos, pt_type size) : _pos(pos), _size(size) {

}


AABB_2D::AABB_2D(const AABB_2D & aabb) {
	_pos= pt_type(aabb._pos);
	_size= pt_type(aabb._size);
}


AABB_2D::~AABB_2D() {

}


pt_type AABB_2D::center() {
	return _pos+ (number)(0.5)* _size;
}


AABB_2D * AABB_2D::buffered(number size) {
	return new AABB_2D(_pos- pt_type(size, size), _size+ (number)(2.0)* pt_type(size, size));
}


ostream & operator << (ostream & os, const AABB_2D & aabb) {
	os << "pos=" << glm::to_string(aabb._pos) << " ; size=" << glm::to_string(aabb._size);
	return os;
}


bool point_in_aabb(const pt_type & pt, const AABB_2D * aabb) {
	return ((pt.x>= aabb->_pos.x) && (pt.x<= aabb->_pos.x+ aabb->_size.x) && (pt.y>= aabb->_pos.y) && (pt.y<= aabb->_pos.y+ aabb->_size.y));
}


bool aabb_intersects_aabb(const AABB_2D * aabb_1, const AABB_2D * aabb_2) {
	return ((aabb_1->_pos.x<= aabb_2->_pos.x+ aabb_2->_size.x) && (aabb_1->_pos.x+ aabb_1->_size.x>= aabb_2->_pos.x) && (aabb_1->_pos.y<= aabb_2->_pos.y+ aabb_2->_size.y) && (aabb_1->_pos.y+ aabb_1->_size.y>= aabb_2->_pos.y));
}

bool aabb_contains_aabb(const AABB_2D * big_aabb, const AABB_2D * small_aabb) {
	return (big_aabb->_pos.x<= small_aabb->_pos.x && big_aabb->_pos.y<= small_aabb->_pos.y && big_aabb->_size.x>= small_aabb->_size.x && big_aabb->_size.y>= small_aabb->_size.y);
}


bool ray_intersects_aabb(const pt_type & ray_origin, const pt_type & ray_dir, const AABB_2D * aabb, pt_type & contact_pt, pt_type & contact_normal, number & t_hit_near) {
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

	pt_type ray_dir_inv= (number)(1.0)/ ray_dir;

	pt_type k_near= (aabb->_pos- ray_origin)* ray_dir_inv;
	pt_type k_far = (aabb->_pos+ aabb->_size- ray_origin)* ray_dir_inv;

	/*
	cout << "___\n";
	cout << "ray_dir= (" << ray_dir.x << " ; " << ray_dir.y << ")\n";
	cout << "ray_dir_inv= (" << ray_dir_inv.x << " ; " << ray_dir_inv.y << ")\n";
	cout << "k_near= (" << k_near.x << " ; " << k_near.y << ")\n";
	cout << "k_far= (" << k_far.x << " ; " << k_far.y << ")\n";
	cout << "___\n";
	*/

	if (k_near.x> k_far.x) {
		swap(k_near.x, k_far.x);
	}
	if (k_near.y> k_far.y) {
		swap(k_near.y, k_far.y);
	}

	if ((k_near.x> k_far.y) || (k_near.y> k_far.x)) {
		return false;
	}

	t_hit_near= (number)(max(k_near.x, k_near.y));
	number t_hit_far= (number)(min(k_far.x, k_far.y));

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
