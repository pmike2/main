#include "bbox_2d.h"


using namespace std;


AABB_2D::AABB_2D() {

}


AABB_2D::AABB_2D(glm::vec2 pt_min, glm::vec2 pt_max) : _pt_min(pt_min), _pt_max(pt_max) {
    /*for (unsigned int i=0; i<4; ++i) {
		_contacts[i]= nullptr;
	}*/
}


AABB_2D::~AABB_2D() {
    
}


bool point_in_aabb(const glm::vec2 & pt, const AABB_2D * aabb) {
	return ((pt.x> aabb->_pt_min.x) && (pt.x< aabb->_pt_max.x) && (pt.y> aabb->_pt_min.y) && (pt.y< aabb->_pt_max.y));
}


bool aabb_intersects_aabb(const AABB_2D * aabb_1, const AABB_2D * aabb_2) {
	return ((aabb_1->_pt_min.x< aabb_2->_pt_max.x) && (aabb_1->_pt_max.x> aabb_2->_pt_min.x) && (aabb_1->_pt_min.y< aabb_2->_pt_max.y) && (aabb_1->_pt_max.y> aabb_2->_pt_min.y));
}


bool ray_intersects_aabb(const glm::vec2 & ray_origin, const glm::vec2 & ray_dir, const AABB_2D * aabb, glm::vec2 & contact_pt, glm::vec2 & contact_normal, float & t_hit_near) {
	contact_pt.x= 0.0f;
	contact_pt.y= 0.0f;
	contact_normal.x= 0.0f;
	contact_normal.y= 0.0f;
	t_hit_near= 0.0f;

	if (glm::length2(ray_dir)< 1e-6f) {
		return false;
	}

	glm::vec2 ray_dir_inv= 1.0f/ ray_dir;

	glm::vec2 k_near= (aabb->_pt_min- ray_origin)* ray_dir_inv;
	glm::vec2 k_far = (aabb->_pt_max- ray_origin)* ray_dir_inv;

	if (k_near.x> k_far.x) {
		swap(k_near.x, k_far.x);
	}
	if (k_near.y> k_far.y) {
		swap(k_near.y, k_far.y);
	}

	if ((k_near.x> k_far.y) || (k_near.y> k_far.x)) {
		return false;
	}

	t_hit_near= max(k_near.x, k_near.y);
	float t_hit_far= min(k_far.x, k_far.y);

	if (t_hit_far< 0.0f) {
		return false;
	}

	contact_pt= ray_origin+ t_hit_near* ray_dir;

	if (k_near.x> k_near.y) {
		if (ray_dir.x< 0.0f) {
			contact_normal.x= 1.0f;
			contact_normal.y= 0.0f;
		}
		else {
			contact_normal.x= -1.0f;
			contact_normal.y= 0.0f;
		}
	}
	else if (k_near.x< k_near.y) {
		if (ray_dir.y< 0.0f) {
			contact_normal.x= 0.0f;
			contact_normal.y= 1.0f;
		}
		else {
			contact_normal.x= 0.0f;
			contact_normal.y= -1.0f;
		}
	}

	return true;
}

/*
bool dynamic_aabb_intersects_aabb(const AABB_2D * dynamic_aabb, const AABB_2D * static_aabb, const float time_step, glm::vec2 & contact_pt, glm::vec2 & contact_normal, float & contact_time) {
	if (glm::length2(dynamic_aabb->_velocity)< 1e-6f) {
		return false;
	}

	AABB_2D expanded;
	expanded._pt_min= static_aabb->_pt_min- 0.5f* (dynamic_aabb->_pt_max- dynamic_aabb->_pt_min);
	expanded._pt_max= static_aabb->_pt_max+ 0.5f* (dynamic_aabb->_pt_max- dynamic_aabb->_pt_min);

	if (ray_intersects_aabb(0.5f* (dynamic_aabb->_pt_min+ dynamic_aabb->_pt_max), time_step* dynamic_aabb->_velocity, &expanded, contact_pt, contact_normal, contact_time)) {
		return ((contact_time> 0.0f) && (contact_time< 1.0f));
	}
	
	return false;
}
*/


/*
bool resolve_dynamic_aabb_intersects_aabb(AABB_2D * dynamic_aabb, const AABB_2D * static_aabb, const float time_step) {
	glm::vec2 contact_pt(0.0f);
	glm::vec2 contact_normal(0.0f);
	float contact_time= 0.0f;
	if (dynamic_aabb_intersects_aabb(dynamic_aabb, static_aabb, time_step, &contact_pt, &contact_normal, &contact_time)) {

		return true;
	}

	return false;
}
*/
