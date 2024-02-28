#include <cmath>
#include <iostream>
#include <cstdlib>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>

#include "bbox.h"

using namespace std;


AABB::AABB() {
	set_vmin_vmax(glm::vec3(0.0f), glm::vec3(0.0f));
}


AABB::AABB(const glm::vec3 & vmin, const glm::vec3 & vmax) {
	set_vmin_vmax(vmin, vmax);
}


AABB::~AABB() {

}


void AABB::set_vmin_vmax(const glm::vec3 & vmin, const glm::vec3 & vmax) {
	_vmin= vmin;
	_vmax= vmax;
	_radius= abs(_vmin.x);
	if (abs(_vmax.x)> _radius) _radius= abs(_vmax.x);
	if (abs(_vmin.y)> _radius) _radius= abs(_vmin.y);
	if (abs(_vmax.y)> _radius) _radius= abs(_vmax.y);
	if (abs(_vmin.z)> _radius) _radius= abs(_vmin.z);
	if (abs(_vmax.z)> _radius) _radius= abs(_vmax.z);

	_pts[0]= glm::vec3(_vmin.x, _vmin.y, _vmin.z);
	_pts[1]= glm::vec3(_vmax.x, _vmin.y, _vmin.z);
	_pts[2]= glm::vec3(_vmin.x, _vmax.y, _vmin.z);
	_pts[3]= glm::vec3(_vmax.x, _vmax.y, _vmin.z);
	_pts[4]= glm::vec3(_vmin.x, _vmin.y, _vmax.z);
	_pts[5]= glm::vec3(_vmax.x, _vmin.y, _vmax.z);
	_pts[6]= glm::vec3(_vmin.x, _vmax.y, _vmax.z);
	_pts[7]= glm::vec3(_vmax.x, _vmax.y, _vmax.z);
}


vector<vector<unsigned int> > AABB::triangles_idxs() {
	vector<vector<unsigned int> > idx= {
		{0, 4, 2}, {2, 4, 6}, // x-
		{1, 3, 7}, {1, 7, 5}, // x+
		{0, 1, 5}, {0, 5, 4}, // y-
		{3, 2, 6}, {3, 6, 7}, // y+
		{0, 2, 3}, {0, 3, 1}, // z-
		{5, 7, 6}, {5, 6, 4} // z+
	};
	return idx;
}


void AABB::translate(glm::vec3 v) {
	set_vmin_vmax(_vmin+ v, _vmax+ v);
}


void AABB::scale(float x) {
	set_vmin_vmax(x* _vmin, x* _vmax);
}


ostream & operator << (ostream & os, const AABB & aabb) {
	os << "vmin=" << glm::to_string(aabb._vmin) << " ; vmax=" << glm::to_string(aabb._vmax);
	return os;
}


// ------------------------------------------------------------------------------------------------------
BBox::BBox() : _vmin(glm::vec3(0.0f)), _vmax(glm::vec3(0.0f)), _radius(0.0f) {
	
}


BBox::BBox(const glm::vec3 & vmin, const glm::vec3 & vmax, const glm::mat4 & model2world) : _vmin(vmin), _vmax(vmax), _model2world(model2world) {
	_radius= abs(_vmin.x);
	if (abs(_vmax.x)> _radius) _radius= abs(_vmax.x);
	if (abs(_vmin.y)> _radius) _radius= abs(_vmin.y);
	if (abs(_vmax.y)> _radius) _radius= abs(_vmax.y);
	if (abs(_vmin.z)> _radius) _radius= abs(_vmin.z);
	if (abs(_vmax.z)> _radius) _radius= abs(_vmax.z);
	_aabb= new AABB();

	set_model2world(model2world);
}


BBox::~BBox() {
	delete _aabb;
}


void BBox::set_model2world(const glm::mat4 & model2world) {
	_model2world= model2world;

	_pts[0]= glm::vec3(_model2world* glm::vec4(_vmin.x, _vmin.y, _vmin.z, 1.0f));
	_pts[1]= glm::vec3(_model2world* glm::vec4(_vmax.x, _vmin.y, _vmin.z, 1.0f));
	_pts[2]= glm::vec3(_model2world* glm::vec4(_vmin.x, _vmax.y, _vmin.z, 1.0f));
	_pts[3]= glm::vec3(_model2world* glm::vec4(_vmax.x, _vmax.y, _vmin.z, 1.0f));
	_pts[4]= glm::vec3(_model2world* glm::vec4(_vmin.x, _vmin.y, _vmax.z, 1.0f));
	_pts[5]= glm::vec3(_model2world* glm::vec4(_vmax.x, _vmin.y, _vmax.z, 1.0f));
	_pts[6]= glm::vec3(_model2world* glm::vec4(_vmin.x, _vmax.y, _vmax.z, 1.0f));
	_pts[7]= glm::vec3(_model2world* glm::vec4(_vmax.x, _vmax.y, _vmax.z, 1.0f));

	glm::vec3 vmin(_pts[0]);
	glm::vec3 vmax(_pts[0]);
	for (unsigned int i=1; i<8; ++i) {
		if (_pts[i].x< vmin.x) {
			vmin.x= _pts[i].x;
		}
		if (_pts[i].y< vmin.y) {
			vmin.y= _pts[i].y;
		}
		if (_pts[i].z< vmin.z) {
			vmin.z= _pts[i].z;
		}
		if (_pts[i].x> vmax.x) {
			vmax.x= _pts[i].x;
		}
		if (_pts[i].y> vmax.y) {
			vmax.y= _pts[i].y;
		}
		if (_pts[i].z> vmax.z) {
			vmax.z= _pts[i].z;
		}
	}
	_aabb->set_vmin_vmax(vmin, vmax);
}


vector<vector<unsigned int> > BBox::triangles_idxs() {
	vector<vector<unsigned int> > idx= {
		{0, 4, 2}, {2, 4, 6}, // x-
		{1, 3, 7}, {1, 7, 5}, // x+
		{0, 1, 5}, {0, 5, 4}, // y-
		{3, 2, 6}, {3, 6, 7}, // y+
		{0, 2, 3}, {0, 3, 1}, // z-
		{5, 7, 6}, {5, 6, 4} // z+
	};
	return idx;
}


// ---------------------------------------------------------------------------------------------------------------------
InstancePosRot::InstancePosRot() {

}


InstancePosRot::InstancePosRot(const glm::vec3 & position, const glm::quat & rotation, const glm::vec3 & scale) :
	_position(position), _rotation(rotation), _scale(scale), _active(false), _dist2(0.0f), _selected(false)
{
	_model2world= glm::translate(_position)* mat4_cast(_rotation)* glm::scale(_scale);
	_bbox= new BBox();
	_emprise= new AABB_2D();
}


InstancePosRot::InstancePosRot(const glm::vec3 & position, const glm::quat & rotation, const glm::vec3 & scale, AABB * aabb) : 
	_position(position), _rotation(rotation), _scale(scale), _active(false), _dist2(0.0f), _selected(false)
{
	_model2world= glm::translate(_position)* mat4_cast(_rotation)* glm::scale(_scale);
	_bbox= new BBox(aabb->_vmin, aabb->_vmax, _model2world);
	_emprise= new AABB_2D(glm::vec2(_bbox->_aabb->_vmin), glm::vec2(_bbox->_aabb->_vmax- _bbox->_aabb->_vmin));
}


InstancePosRot::~InstancePosRot() {
	delete _bbox;
	delete _emprise;
}


void InstancePosRot::set_pos_rot_scale(const glm::vec3 & position, const glm::quat & rotation, const glm::vec3 & scale) {
	_position= position;
	_rotation= rotation;
	_scale= scale;
	_model2world= glm::translate(_position)* mat4_cast(_rotation)* glm::scale(_scale);
	_bbox->set_model2world(_model2world);
	_emprise->_pos= glm::vec2(_bbox->_aabb->_vmin);
	_emprise->_size= glm::vec2(_bbox->_aabb->_vmax- _bbox->_aabb->_vmin);
}


// lent, mieux vaut utiliser l'autre
void InstancePosRot::set_pos_rot_scale(const glm::mat4 & mat) {
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(mat, _scale, _rotation, _position, skew, perspective);
	_model2world= glm::translate(_position)* mat4_cast(_rotation)* glm::scale(_scale);
	_bbox->set_model2world(_model2world);
}


void InstancePosRot::update_dist2(glm::vec3 view_eye) {
	// glm::distance2 est lent !
	//_dist2= glm::distance2(_position, view_eye);
	_dist2= (_position.x- view_eye.x)* (_position.x- view_eye.x)+ (_position.y- view_eye.y)* (_position.y- view_eye.y)+ (_position.z- view_eye.z)* (_position.z- view_eye.z);
}


// -----------------------------------------------------------------------------------------------------
bool aabb_intersects_aabb(AABB * aabb_1, AABB * aabb_2) {
	if ((aabb_1->_vmin.x> aabb_2->_vmax.x) || (aabb_1->_vmax.x< aabb_2->_vmin.x) ||
		(aabb_1->_vmin.y> aabb_2->_vmax.y) || (aabb_1->_vmax.y< aabb_2->_vmin.y) ||
		(aabb_1->_vmin.z> aabb_2->_vmax.z) || (aabb_1->_vmax.z< aabb_2->_vmin.z)) {
		return false;
	}
	return true;
}


bool aabb_intersects_bbox(AABB * aabb, BBox * bbox) {
	for (unsigned int i=0; i<8; ++i) {
		if ((bbox->_pts[i].x> aabb->_vmin.x) && (bbox->_pts[i].x< aabb->_vmax.x) &&
			(bbox->_pts[i].y> aabb->_vmin.y) && (bbox->_pts[i].y< aabb->_vmax.y) &&
			(bbox->_pts[i].z> aabb->_vmin.z) && (bbox->_pts[i].z< aabb->_vmax.z)) {
			return true;
		}
	}
	return false;
}


// https://www.jkh.me/files/tutorials/Separating%20Axis%20Theorem%20for%20Oriented%20Bounding%20Boxes.pdf
bool bbox_intersects_bbox(BBox * bbox_1, BBox * bbox_2) {
	glm::vec3 center_1= 0.5f* (bbox_1->_pts[7]+ bbox_1->_pts[0]);
	glm::vec3 x_1= glm::normalize(bbox_1->_pts[1]- bbox_1->_pts[0]);
	glm::vec3 y_1= glm::normalize(bbox_1->_pts[2]- bbox_1->_pts[0]);
	glm::vec3 z_1= glm::normalize(bbox_1->_pts[4]- bbox_1->_pts[0]);
	float half_width_1= 0.5f* (bbox_1->_vmax[0]- bbox_1->_vmin[0]);
	float half_height_1= 0.5f* (bbox_1->_vmax[1]- bbox_1->_vmin[1]);
	float half_depth_1= 0.5f* (bbox_1->_vmax[2]- bbox_1->_vmin[2]);

	glm::vec3 center_2= 0.5f* (bbox_2->_pts[7]+ bbox_2->_pts[0]);
	glm::vec3 x_2= glm::normalize(bbox_2->_pts[1]- bbox_2->_pts[0]);
	glm::vec3 y_2= glm::normalize(bbox_2->_pts[2]- bbox_2->_pts[0]);
	glm::vec3 z_2= glm::normalize(bbox_2->_pts[4]- bbox_2->_pts[0]);
	float half_width_2= 0.5f* (bbox_2->_vmax[0]- bbox_2->_vmin[0]);
	float half_height_2= 0.5f* (bbox_2->_vmax[1]- bbox_2->_vmin[1]);
	float half_depth_2= 0.5f* (bbox_2->_vmax[2]- bbox_2->_vmin[2]);

	glm::vec3 axes[15]= {
		x_1, y_1, z_1, x_2, y_2, z_2, 
		glm::cross(x_1, x_2), glm::cross(x_1, y_2), glm::cross(x_1, z_2), 
		glm::cross(y_1, x_2), glm::cross(y_1, y_2), glm::cross(y_1, z_2), 
		glm::cross(z_1, x_2), glm::cross(z_1, y_2), glm::cross(z_1, z_2)
	};

	for (unsigned i=0; i<15; ++i) {
		float a= abs(glm::dot(axes[i], center_2- center_1));
		float b= abs(half_width_1* glm::dot(axes[i], x_1))+ abs(half_height_1* glm::dot(axes[i], y_1))+ abs(half_depth_1* glm::dot(axes[i], z_1))+
				 abs(half_width_2* glm::dot(axes[i], x_2))+ abs(half_height_2* glm::dot(axes[i], y_2))+ abs(half_depth_2* glm::dot(axes[i], z_2));
		if (a> b) {
			return false;
		}
	}
	return true;
}


float aabb_distance_pt_2(AABB * aabb, const glm::vec3 & pt) {
	float dx, dy, dz;
	
	if (pt.x> aabb->_vmax.x) {
		dx= pt.x- aabb->_vmax.x;
	}
	else if (pt.x< aabb->_vmin.x) {
		dx= aabb->_vmin.x- pt.x;
	}
	else {
		dx= 0.0f;
	}

	if (pt.y> aabb->_vmax.y) {
		dy= pt.y- aabb->_vmax.y;
	}
	else if (pt.y< aabb->_vmin.y) {
		dy= aabb->_vmin.y- pt.y;
	}
	else {
		dy= 0.0f;
	}

	if (pt.z> aabb->_vmax.z) {
		dz= pt.z- aabb->_vmax.z;
	}
	else if (pt.z< aabb->_vmin.z) {
		dz= aabb->_vmin.z- pt.z;
	}
	else {
		dz= 0.0f;
	}

	return dx* dx+ dy* dy+ dz* dz;
}


float aabb_distance_pt(AABB * aabb, const glm::vec3 & pt) {
	return sqrt(aabb_distance_pt_2(aabb, pt));
}


// cf https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
bool ray_intersects_aabb(glm::vec3 origin, glm::vec3 direction, AABB * aabb, float & t_hit) {
	direction= glm::normalize(direction);
	if (direction.x== 0.0f) {
		direction.x= 1e-7;
	}
	if (direction.y== 0.0f) {
		direction.y= 1e-7;
	}
	if (direction.z== 0.0f) {
		direction.z= 1e-7;
	}
	glm::vec3 dirfrac(1.0f/ direction.x, 1.0f/ direction.y, 1.0f/ direction.z);
	float t1= (aabb->_vmin.x- origin.x)* dirfrac.x;
	float t2= (aabb->_vmax.x- origin.x)* dirfrac.x;
	float t3= (aabb->_vmin.y- origin.y)* dirfrac.y;
	float t4= (aabb->_vmax.y- origin.y)* dirfrac.y;
	float t5= (aabb->_vmin.z- origin.z)* dirfrac.z;
	float t6= (aabb->_vmax.z- origin.z)* dirfrac.z;

	float tmin= max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
	float tmax= min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

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


bool segment_intersects_aabb(const glm::vec3 & pt1, const glm::vec3 & pt2, AABB * aabb) {
	float t_hit;
	bool ray_inter= ray_intersects_aabb(pt1, pt2- pt1, aabb, t_hit);
	//cout << ray_inter << " ; " << t_hit << " ; " << glm::length(pt2- pt1) << "\n";
	if ((!ray_inter) || (t_hit> glm::length(pt2- pt1))) {
		return false;
	}
	return true;
}


// ------------------------------------------------------------------------------------------------------
/*BBoxDraw::BBoxDraw() {

}


BBoxDraw::BBoxDraw(GLuint prog_draw, const glm::vec3 & vmin, const glm::vec3 & vmax, const glm::vec3 & color) : _prog_draw(prog_draw), _vmin(vmin), _vmax(vmax), _model2world(glm::mat4(1.0f)) {
	_color= glm::vec4(color.x, color.y, color.z, 1.0f);
	init();
}


BBoxDraw::BBoxDraw(GLuint prog_draw, AABB * aabb, const glm::vec3 & color) : _prog_draw(prog_draw), _vmin(aabb->_vmin), _vmax(aabb->_vmax), _model2world(glm::mat4(1.0f)) {
	_color= glm::vec4(color.x, color.y, color.z, 1.0f);
	init();
}


BBoxDraw::BBoxDraw(GLuint prog_draw, BBox * bbox, const glm::vec3 & color) : _prog_draw(prog_draw), _vmin(bbox->_vmin), _vmax(bbox->_vmax), _model2world(bbox->_model2world) {
	_color= glm::vec4(color.x, color.y, color.z, 1.0f);
	init();
}


BBoxDraw::~BBoxDraw() {
}


void BBoxDraw::init() {
	_data[0]= _vmax.x; _data[1]= _vmin.y; _data[2]= _vmin.z;
	_data[3]= _vmax.x; _data[4]= _vmax.y; _data[5]= _vmin.z;
	
	_data[6]= _vmax.x; _data[7]= _vmax.y; _data[8]= _vmin.z;
	_data[9]= _vmax.x; _data[10]= _vmax.y; _data[11]= _vmax.z;
	
	_data[12]= _vmax.x; _data[13]= _vmax.y; _data[14]= _vmax.z;
	_data[15]= _vmax.x; _data[16]= _vmin.y; _data[17]= _vmax.z;
	
	_data[18]= _vmax.x; _data[19]= _vmin.y; _data[20]= _vmax.z;
	_data[21]= _vmax.x; _data[22]= _vmin.y; _data[23]= _vmin.z;
	
	// ---
	_data[24]= _vmin.x; _data[25]= _vmin.y; _data[26]= _vmin.z;
	_data[27]= _vmin.x; _data[28]= _vmax.y; _data[29]= _vmin.z;
	
	_data[30]= _vmin.x; _data[31]= _vmax.y; _data[32]= _vmin.z;
	_data[33]= _vmin.x; _data[34]= _vmax.y; _data[35]= _vmax.z;
	
	_data[36]= _vmin.x; _data[37]= _vmax.y; _data[38]= _vmax.z;
	_data[39]= _vmin.x; _data[40]= _vmin.y; _data[41]= _vmax.z;
	
	_data[42]= _vmin.x; _data[43]= _vmin.y; _data[44]= _vmax.z;
	_data[45]= _vmin.x; _data[46]= _vmin.y; _data[47]= _vmin.z;
	
	// ---
	_data[48]= _vmax.x; _data[49]= _vmin.y; _data[50]= _vmin.z;
	_data[51]= _vmin.x; _data[52]= _vmin.y; _data[53]= _vmin.z;
	
	_data[54]= _vmax.x; _data[55]= _vmax.y; _data[56]= _vmin.z;
	_data[57]= _vmin.x; _data[58]= _vmax.y; _data[59]= _vmin.z;
	
	_data[60]= _vmax.x; _data[61]= _vmax.y; _data[62]= _vmax.z;
	_data[63]= _vmin.x; _data[64]= _vmax.y; _data[65]= _vmax.z;
	
	_data[66]= _vmax.x; _data[67]= _vmin.y; _data[68]= _vmax.z;
	_data[69]= _vmin.x; _data[70]= _vmin.y; _data[71]= _vmax.z;
	
	_world2clip= glm::mat4(1.0f);

	glGenBuffers(1, &_buffer);
	
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_data), _data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(_prog_draw);

	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_color_loc= glGetUniformLocation(_prog_draw, "color");
	_model2world_loc= glGetUniformLocation(_prog_draw, "model2world_matrix");
	_world2clip_loc= glGetUniformLocation(_prog_draw, "world2clip_matrix");

	glUseProgram(0);
}


void BBoxDraw::draw() {
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
		
	glEnableVertexAttribArray(_position_loc);

	glUniformMatrix4fv(_model2world_loc, 1, GL_FALSE, glm::value_ptr(_model2world));
	glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(_world2clip));
	glUniform4fv(_color_loc, 1, glm::value_ptr(_color));
	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 3* sizeof(float), (void*)0);

	glDrawArrays(GL_LINES, 0, 24);

	glDisableVertexAttribArray(_position_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void BBoxDraw::anim(const glm::mat4 & world2clip) {
	_world2clip= world2clip;
}


void BBoxDraw::set_model2world(const glm::mat4 & model2world) {
	_model2world= model2world;
}*/

// ---------------------------------------------------------------------------------------

