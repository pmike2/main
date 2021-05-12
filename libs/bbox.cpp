
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
	_emprise= new AABB_2D(glm::vec2(_position+ _bbox->_vmin), glm::vec2(_bbox->_vmax- _bbox->_vmin));
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
	_emprise->_pos= glm::vec2(_position+ _bbox->_vmin);
	//cout << glm::to_string(_position) << " ; " << *_emprise << glm::to_string(_bbox->_vmin) << "\n";
}


// lent, mieux vaut utiliser l'autre
/*void InstancePosRot::set_pos_rot_scale(const glm::mat4 & mat) {
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(mat, _scale, _rotation, _position, skew, perspective);
	_model2world= glm::translate(_position)* mat4_cast(_rotation)* glm::scale(_scale);
	_bbox->set_model2world(_model2world);
}*/


void InstancePosRot::update_dist2(glm::vec3 view_eye) {
	// glm::distance2 est lent !
	//_dist2= glm::distance2(_position, view_eye);
	_dist2= (_position.x- view_eye.x)* (_position.x- view_eye.x)+ (_position.y- view_eye.y)* (_position.y- view_eye.y)+ (_position.z- view_eye.z)* (_position.z- view_eye.z);
}


// -----------------------------------------------------------------------------------------------------
bool aabb_intersects(AABB * aabb_1, AABB * aabb_2) {
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

// a revoir

// cf https://www.geometrictools.com/Documentation/DynamicCollisionDetection.pdf
/*bool bbox_intersects(BBox * bbox1, glm::mat3 rotation1, glm::vec3 translation1, BBox * bbox2, glm::mat3 rotation2, glm::vec3 translation2) {
	bool verbose= false;

	glm::vec3 A0= rotation1* glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 A1= rotation1* glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 A2= rotation1* glm::vec3(0.0f, 0.0f, 1.0f);
	glm::mat3 A= rotation1;
	
	// on recentre la bbox si xmin != -xmax, etc
	float a0= (bbox1->_vmax.x- bbox1->_vmin.x)* 0.5f;
	float a1= (bbox1->_vmax.y- bbox1->_vmin.y)* 0.5f;
	float a2= (bbox1->_vmax.z- bbox1->_vmin.z)* 0.5f;
	
	glm::vec3 B0= rotation2* glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 B1= rotation2* glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 B2= rotation2* glm::vec3(0.0f, 0.0f, 1.0f);
	glm::mat3 B= rotation2;
	
	float b0= (bbox2->_vmax.x- bbox2->_vmin.x)* 0.5f;
	float b1= (bbox2->_vmax.y- bbox2->_vmin.y)* 0.5f;
	float b2= (bbox2->_vmax.z- bbox2->_vmin.z)* 0.5f;
	
	glm::vec3 D= 
		(translation2+ glm::vec3((bbox2->_vmax.x+ bbox2->_vmin.x)* 0.5f, (bbox2->_vmax.y+ bbox2->_vmin.y)* 0.5f, (bbox2->_vmax.z+ bbox2->_vmin.z)* 0.5f))-
		(translation1+ glm::vec3((bbox1->_vmax.x+ bbox1->_vmin.x)* 0.5f, (bbox1->_vmax.y+ bbox1->_vmin.y)* 0.5f, (bbox1->_vmax.z+ bbox1->_vmin.z)* 0.5f));
	
	glm::mat3 C= glm::transpose(A)* B;
	
	if (verbose) {
		cout << "A0=(" << A0[0] << ";" << A0[1] << ";" << A0[2] << ")" << endl;
		cout << "A1=(" << A1[0] << ";" << A1[1] << ";" << A1[2] << ")" << endl;
		cout << "A2=(" << A2[0] << ";" << A2[1] << ";" << A2[2] << ")" << endl;
		cout << "a0=" << a0 << "; a1=" << a1 << "; a2=" << a2 << endl;

		cout << "B0=(" << B0[0] << ";" << B0[1] << ";" << B0[2] << ")" << endl;
		cout << "B1=(" << B1[0] << ";" << B1[1] << ";" << B1[2] << ")" << endl;
		cout << "B2=(" << B2[0] << ";" << B2[1] << ";" << B2[2] << ")" << endl;
		cout << "b0=" << b0 << "; b1=" << b1 << "; b2=" << b2 << endl;
	
		cout << "D=(" << D[0] << ";" << D[1] << ";" << D[2] << ")" << endl;
	
		cout << "C=(" << endl;
		cout << C[0][0] << ";" << C[0][1] << ";" << C[0][2] << endl;
		cout << C[1][0] << ";" << C[1][1] << ";" << C[1][2] << endl;
		cout << C[2][0] << ";" << C[2][1] << ";" << C[2][2] << endl;
		cout << ")" << endl;
	}
	
	if (a0+ b0* abs(C[0][0])+ b1* abs(C[0][1])+ b2* abs(C[0][2])< glm::dot(A0, D)) {
		if (verbose) cout << "A0" << endl;
		return false;
	}
	if (a1+ b0* abs(C[1][0])+ b1* abs(C[1][1])+ b2* abs(C[1][2])< glm::dot(A1, D)) {
		if (verbose) cout << "A1" << endl;
		return false;
	}
	if (a2+ b0* abs(C[2][0])+ b1* abs(C[2][1])+ b2* abs(C[2][2])< glm::dot(A2, D)) {
		if (verbose) cout << "A2" << endl;
		return false;
	}

	if (b0+ a0* abs(C[0][0])+ a1* abs(C[1][0])+ a2* abs(C[2][0])< glm::dot(B0, D)) {
		if (verbose) cout << "B0" << endl;
		return false;
	}
	if (b1+ a0* abs(C[0][1])+ a1* abs(C[1][1])+ a2* abs(C[2][1])< glm::dot(B1, D)) {
		if (verbose) cout << "B1" << endl;
		return false;
	}
	if (b2+ a0* abs(C[0][2])+ a1* abs(C[1][2])+ a2* abs(C[2][2])< glm::dot(B2, D)) {
		if (verbose) cout << "B2" << endl;
		return false;
	}

	if (a1* abs(C[2][0])+ a2* abs(C[1][0])+ b1* abs(C[0][2])+ b2* abs(C[0][1])< abs(C[1][0]* glm::dot(A2, D)- C[2][0]* glm::dot(A1, D))) {
		if (verbose) cout << "A0 x B0" << endl;
		return false;
	}
	if (a1* abs(C[2][1])+ a2* abs(C[1][1])+ b0* abs(C[0][2])+ b2* abs(C[0][0])< abs(C[1][1]* glm::dot(A2, D)- C[2][1]* glm::dot(A1, D))) {
		if (verbose) cout << "A0 x B1" << endl;
		return false;
	}
	if (a1* abs(C[2][2])+ a2* abs(C[1][2])+ b0* abs(C[0][1])+ b1* abs(C[0][0])< abs(C[1][2]* glm::dot(A2, D)- C[2][2]* glm::dot(A1, D))) {
		if (verbose) cout << "A0 x B2" << endl;
		return false;
	}
	
	if (a0* abs(C[2][0])+ a2* abs(C[0][0])+ b1* abs(C[1][2])+ b2* abs(C[1][1])< abs(C[2][0]* glm::dot(A0, D)- C[0][0]* glm::dot(A2, D))) {
		if (verbose) cout << "A1 x B0" << endl;
		return false;
	}
	if (a0* abs(C[2][1])+ a2* abs(C[0][1])+ b0* abs(C[1][2])+ b2* abs(C[1][0])< abs(C[2][1]* glm::dot(A0, D)- C[0][1]* glm::dot(A2, D))) {
		if (verbose) cout << "A1 x B1" << endl;
		return false;
	}
	if (a0* abs(C[2][2])+ a2* abs(C[0][2])+ b0* abs(C[1][1])+ b1* abs(C[1][0])< abs(C[2][2]* glm::dot(A0, D)- C[0][2]* glm::dot(A2, D))) {
		if (verbose) cout << "A1 x B2" << endl;
		return false;
	}
	
	if (a0* abs(C[1][0])+ a1* abs(C[0][0])+ b1* abs(C[2][2])+ b2* abs(C[2][1])< abs(C[0][0]* glm::dot(A1, D)- C[1][0]* glm::dot(A0, D))) {
		if (verbose) cout << "A2 x B0" << endl;
		return false;
	}
	if (a0* abs(C[1][1])+ a1* abs(C[0][1])+ b0* abs(C[2][2])+ b2* abs(C[2][0])< abs(C[0][1]* glm::dot(A1, D)- C[1][1]* glm::dot(A0, D))) {
		if (verbose) cout << "A2 x B1" << endl;
		return false;
	}
	if (a0* abs(C[1][2])+ a1* abs(C[0][2])+ b0* abs(C[2][1])+ b1* abs(C[2][0])< abs(C[0][2]* glm::dot(A1, D)- C[1][2]* glm::dot(A0, D))) {
		if (verbose) cout << "A2 x B2" << endl;
		return false;
	}

	return true;
}
*/
