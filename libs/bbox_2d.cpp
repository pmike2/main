#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "bbox_2d.h"


using namespace std;


AABB_2D::AABB_2D() {

}


AABB_2D::AABB_2D(glm::vec2 pos, glm::vec2 size) : _pos(pos), _size(size) {
}


AABB_2D::~AABB_2D() {
		
}


bool point_in_aabb(const glm::vec2 & pt, const AABB_2D * aabb) {
	return ((pt.x> aabb->_pos.x) && (pt.x< aabb->_pos.x+ aabb->_size.x) && (pt.y> aabb->_pos.y) && (pt.y< aabb->_pos.y+ aabb->_size.y));
}


bool aabb_intersects_aabb(const AABB_2D * aabb_1, const AABB_2D * aabb_2) {
	return ((aabb_1->_pos.x< aabb_2->_pos.x+ aabb_2->_size.x) && (aabb_1->_pos.x+ aabb_1->_size.x> aabb_2->_pos.x) && (aabb_1->_pos.y< aabb_2->_pos.y+ aabb_2->_size.y) && (aabb_1->_pos.y+ aabb_1->_size.y> aabb_2->_pos.y));
}


bool ray_intersects_aabb(const glm::vec2 & ray_origin, const glm::vec2 & ray_dir, const AABB_2D * aabb, glm::vec2 & contact_pt, glm::vec2 & contact_normal, float & t_hit_near) {
	contact_pt.x= 0.0f;
	contact_pt.y= 0.0f;
	contact_normal.x= 0.0f;
	contact_normal.y= 0.0f;
	t_hit_near= 0.0f;

	// choisir ici une valeur suffisamment petite, dans le code original il fait un std::isnan
	//if (glm::length2(ray_dir)< 1e-9f) {
	if ((ray_dir.x== 0.0f) && (ray_dir.y== 0.0f)) {
		return false;
	}

	glm::vec2 ray_dir_inv= 1.0f/ ray_dir;
	//glm::dvec2 ray_dir_inv= glm::dvec2(1.0/ (double)(ray_dir.x), 1.0/ (double)(ray_dir.y));

	glm::vec2 k_near= (aabb->_pos- ray_origin)* ray_dir_inv;
	glm::vec2 k_far = (aabb->_pos+ aabb->_size- ray_origin)* ray_dir_inv;

	cout << "___\n";
	cout << "ray_dir= (" << ray_dir.x << " ; " << ray_dir.y << ")\n";
	cout << "ray_dir_inv= (" << ray_dir_inv.x << " ; " << ray_dir_inv.y << ")\n";
	cout << "k_near= (" << k_near.x << " ; " << k_near.y << ")\n";
	cout << "k_far= (" << k_far.x << " ; " << k_far.y << ")\n";
	cout << "___\n";

	//glm::dvec2 k_near= glm::dvec2((double)(aabb->_pos.x+ aabb->_size.x- ray_origin.x)* ray_dir_inv.x, (double)(aabb->_pos.y+ aabb->_size.y- ray_origin.y)* ray_dir_inv.y);
	//glm::dvec2 k_far= glm::dvec2((double)(aabb->_pos.x+ aabb->_size.x- ray_origin.x)* ray_dir_inv.x, (double)(aabb->_pos.y+ aabb->_size.y- ray_origin.y)* ray_dir_inv.y);

	if (k_near.x> k_far.x) {
		swap(k_near.x, k_far.x);
	}
	if (k_near.y> k_far.y) {
		swap(k_near.y, k_far.y);
	}

	if ((k_near.x> k_far.y) || (k_near.y> k_far.x)) {
		return false;
	}

	t_hit_near= (float)(max(k_near.x, k_near.y));
	float t_hit_far= (float)(min(k_far.x, k_far.y));

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
