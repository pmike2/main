
#include "level.h"

using namespace std;


// ---------------------------------------------------------------------------------------
SkyBox::SkyBox() {

}


SkyBox::SkyBox(GLuint prog) : _prog(prog) {
	
	vector<string> faces;
	faces.push_back("modeles/skybox/right.jpg");
	faces.push_back("modeles/skybox/left.jpg");
	faces.push_back("modeles/skybox/top.jpg");
	faces.push_back("modeles/skybox/bottom.jpg");
	faces.push_back("modeles/skybox/front.jpg");
	faces.push_back("modeles/skybox/back.jpg");

	_cubemap_texture= load_cube_map(faces);
	
	// triangles des 6 faces du cube
	float data[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	glGenBuffers(1, &_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(_prog);
	_position_loc= glGetAttribLocation(_prog, "position_in");
	_world2camera_loc= glGetUniformLocation(_prog, "world2camera_matrix");
	_camera2clip_loc= glGetUniformLocation(_prog, "camera2clip_matrix");
	glUseProgram(0);
}


void SkyBox::draw() {
	glDepthMask(GL_FALSE);
	glUseProgram(_prog);
	glUniformMatrix4fv(_world2camera_loc, 1, GL_FALSE, _world2camera);
	glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _cubemap_texture);
	glEnableVertexAttribArray(_position_loc);
	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDisableVertexAttribArray(_position_loc);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);	
	glDepthMask(GL_TRUE);
}


void SkyBox::anim(float * world2camera, float * camera2clip) {
	// suppression des translations
	glm::mat4 glm_world2camera= glm::mat4(glm::mat3(glm::make_mat4(world2camera)));
	// on ajuste la rotation
	glm_world2camera= glm::rotate(glm_world2camera, glm::pi<float>()* 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));
	memcpy(_world2camera, glm::value_ptr(glm_world2camera), sizeof(float) * 16);
	memcpy(_camera2clip, camera2clip, sizeof(float) * 16);
}


// ---------------------------------------------------------------------------------------
Cloud::Cloud() {

}


Cloud::Cloud(GLuint prog_draw, GLuint prog_draw_basic, std::string model_path, std::string material_path, glm::vec3 position, glm::mat3 rotation_matrix, float size_factor, float speed) :
	_position(position), _rotation_matrix(rotation_matrix), _speed(speed)
{
	_model= ModelObj(prog_draw, prog_draw_basic);
	_model.load(model_path, material_path, size_factor);
	_model._ambient[0]= CLOUD_COLOR[0];
	_model._ambient[1]= CLOUD_COLOR[1];
	_model._ambient[2]= CLOUD_COLOR[2];
	
	_model._alpha= CLOUD_ALPHA;
}


Cloud::~Cloud() {
	_model.release();
}


void Cloud::draw() {
	_model.draw();
}


void Cloud::anim(float * world2camera, float * camera2clip) {
	glm::vec3 vec_advance= _rotation_matrix* glm::vec3(0.0f, 1.0f, 0.0f);
	
	_position+= _speed* vec_advance;
	if (_position.y> 2.0f* WORLD_SIZE)
		_position.y= -2.0f* WORLD_SIZE;
	if (_position.y< -2.0f* WORLD_SIZE)
		_position.y= 2.0f* WORLD_SIZE;
	
	glm::mat4 rotation= glm::mat4(_rotation_matrix);
	glm::mat4 translation= glm::translate(glm::mat4(1.0f), _position);
	glm::mat4 glm_model2world= translation* rotation;
	memcpy(_model._model2world, glm::value_ptr(glm_model2world), sizeof(float) * 16);
	
	_model.anim(world2camera, camera2clip);
}


// ---------------------------------------------------------------------------------------
LevelMap::LevelMap() {

}


LevelMap::LevelMap(GLuint prog_draw, unsigned int n_ships) : _prog_draw(prog_draw), _n_ships(n_ships) {
	_data= new float[_n_ships* 3* (2+ 3)];
	
	glGenBuffers(1, &_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, _n_ships* 3* (2+ 3)* sizeof(float), _data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_camera2clip_loc= glGetUniformLocation(_prog_draw, "camera2clip_matrix");
	
	// on veut X, Y entre -1 et 1; Z n'existe pas
	memset(_camera2clip, 0, sizeof(float) * 16);
	glm::mat4 glm_ortho= glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	memcpy(_camera2clip, glm::value_ptr(glm_ortho), sizeof(float) * 16);

}


LevelMap::~LevelMap() {
	delete _data;
}


void LevelMap::draw() {
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);

	glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _n_ships* 3);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

}


void LevelMap::anim(std::vector<Ship *> & ships) {
	
	for (unsigned int i=0; i<ships.size(); ++i) {
		glm::vec2 center= glm::vec2(ships[i]->_rigid_body._position/ WORLD_SIZE); // entre -1 et 1
		glm::vec2 fwd= glm::normalize(glm::vec2(ships[i]->_rigid_body._rotation_matrix* glm::vec3(0.0f, 1.0f, 0.0f)));
		
		glm::vec2 left = glm::vec2(-fwd.y, fwd.x);
		
		_data[i* 15+ 0]= center.x+ 3.0f* MAP_SHIP_SIZE* fwd.x;
		_data[i* 15+ 1]= center.y+ 3.0f* MAP_SHIP_SIZE* fwd.y;

		_data[i* 15+ 5]= center.x+ MAP_SHIP_SIZE* left.x;
		_data[i* 15+ 6]= center.y+ MAP_SHIP_SIZE* left.y;

		_data[i* 15+ 10]= center.x- MAP_SHIP_SIZE* left.x;
		_data[i* 15+ 11]= center.y- MAP_SHIP_SIZE* left.y;

		
		for (unsigned j=0; j<3; ++j) {
			_data[i* 15+ j* 5+ 2]= ships[i]->_color.x;
			_data[i* 15+ j* 5+ 3]= ships[i]->_color.y;
			_data[i* 15+ j* 5+ 4]= ships[i]->_color.z;
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, _n_ships* 3* (2+ 3)* sizeof(float), _data, GL_STATIC_DRAW);
}

