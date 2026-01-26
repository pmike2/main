#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "utile.h"

#include "bbox_2d.h"



AABB_2D::AABB_2D() : _pos(pt_2d(0.0)), _size(pt_2d(0.0)) {

}


AABB_2D::AABB_2D(pt_2d pos, pt_2d size) : _pos(pos), _size(size) {

}


AABB_2D::AABB_2D(const AABB_2D & aabb) {
	_pos= pt_2d(aabb._pos);
	_size= pt_2d(aabb._size);
}


AABB_2D::AABB_2D(const std::vector<pt_2d> & pts) {
	pt_2d pos_min = pt_2d(1e9);
	pt_2d pos_max = pt_2d(-1e9);

	for (auto & pt : pts) {
		if (pt.x < pos_min.x) {
			pos_min.x = pt.x;
		}
		if (pt.y < pos_min.y) {
			pos_min.y = pt.y;
		}
		if (pt.x > pos_max.x) {
			pos_max.x = pt.x;
		}
		if (pt.y > pos_max.y) {
			pos_max.y = pt.y;
		}
	}

	_pos = pos_min;
	_size = pos_max - pos_min;
}


AABB_2D::~AABB_2D() {

}


pt_2d AABB_2D::center() {
	return _pos+ (number)(0.5)* _size;
}


AABB_2D * AABB_2D::buffered(number size) {
	return new AABB_2D(_pos- pt_2d(size, size), _size+ (number)(2.0)* pt_2d(size, size));
}


void AABB_2D::buffer(number size) {
	_pos-= pt_2d(size, size);
	_size+= 2.0* pt_2d(size, size);
}


std::ostream & operator << (std::ostream & os, const AABB_2D & aabb) {
	os << "pos=" << glm::to_string(aabb._pos) << " ; size=" << glm::to_string(aabb._size);
	return os;
}


// BBox_2D ---------------------------------------------------------------------------------------------
BBox_2D::BBox_2D() : _center(pt_2d(0.0, 0.0)), _half_size(pt_2d(0.0, 0.0)), _alpha(0.0) {
	_aabb= new AABB_2D();
	update();
}


BBox_2D::BBox_2D(pt_2d center, pt_2d half_size, number alpha) : _center(center), _half_size(half_size), _alpha(alpha) {
	_aabb= new AABB_2D();
	update();
}


BBox_2D::BBox_2D(const BBox_2D & bbox) {
	_center = bbox._center;
	_half_size = bbox._half_size;
	_alpha = bbox._alpha;
	_aabb= new AABB_2D(*bbox._aabb);
	update();
}


void BBox_2D::set(pt_2d center, pt_2d half_size, number alpha) {
	_center = center;
	_half_size = half_size;
	_alpha = alpha;
	update();
}


BBox_2D::BBox_2D(number width, pt_2d pt1, pt_2d pt2) {
	_aabb= new AABB_2D();
	set(width, pt1, pt2);
}


BBox_2D::~BBox_2D() {
	delete _aabb;
}


void BBox_2D::set(number width, pt_2d pt1, pt_2d pt2) {
	_center = (pt1 + pt2) * 0.5;
	_half_size.x = 0.5 * glm::length(pt2 - pt1);
	_half_size.y = 0.5 * width;
	_alpha = atan2(pt2.y - pt1.y, pt2.x - pt1.x);

	update();
}


void BBox_2D::set_aabb(AABB_2D & aabb) {
	_center= aabb.center();
	_half_size= 0.5* aabb._size;
	_alpha= 0.0;
	
	update();
}


void BBox_2D::update() {
	number cos_alpha= cos(_alpha);
	number sin_alpha= sin(_alpha);

	_pts[0].x= _center.x- _half_size.x* cos_alpha+ _half_size.y* sin_alpha;
	_pts[0].y= _center.y- _half_size.x* sin_alpha- _half_size.y* cos_alpha;

	_pts[1].x= _center.x+ _half_size.x* cos_alpha+ _half_size.y* sin_alpha;
	_pts[1].y= _center.y+ _half_size.x* sin_alpha- _half_size.y* cos_alpha;

	_pts[2].x= _center.x+ _half_size.x* cos_alpha- _half_size.y* sin_alpha;
	_pts[2].y= _center.y+ _half_size.x* sin_alpha+ _half_size.y* cos_alpha;

	_pts[3].x= _center.x- _half_size.x* cos_alpha- _half_size.y* sin_alpha;
	_pts[3].y= _center.y- _half_size.x* sin_alpha+ _half_size.y* cos_alpha;

	number xmin, ymin, xmax, ymax;
	xmin= ymin= 1e10;
	xmax= ymax= -1e10;
	for (uint i=0; i<4; ++i) {
		if (_pts[i].x< xmin) {
			xmin= _pts[i].x;
		}
		if (_pts[i].y< ymin) {
			ymin= _pts[i].y;
		}
		if (_pts[i].x> xmax) {
			xmax= _pts[i].x;
		}
		if (_pts[i].y> ymax) {
			ymax= _pts[i].y;
		}
	}
	_aabb->_pos= pt_2d(xmin, ymin);
	_aabb->_size= pt_2d(xmax- xmin, ymax- ymin);
}


BBox_2D * BBox_2D::buffered(number size) {
	return new BBox_2D(_center, _half_size + pt_2d(size, size), _alpha);
}


std::vector<pt_2d> BBox_2D::segments() {
	return std::vector<pt_2d> {
		_pts[0], _pts[1], _pts[1], _pts[2], _pts[2], _pts[3], _pts[3], _pts[0]
	};
}


std::ostream & operator << (std::ostream & os, const BBox_2D & bbox) {
	os << "center=" << glm_to_string(bbox._center) << " ; half-size=" << glm_to_string(bbox._half_size) << " ; alpha=" << bbox._alpha;
	os << " ; pts =";
	for (auto &pt : bbox._pts) {
		os << glm_to_string(pt) << " ; ";
	}
	return os;
}

