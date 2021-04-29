#include <iostream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>


#include "light.h"

using namespace std;


LightDraw::LightDraw() {
	
}


LightDraw::LightDraw(GLuint prog_draw, glm::vec3 position_world, glm::vec3 spot_cone_direction_world, glm::vec3 color) {
	_prog_draw= prog_draw;
	
	_data[0]=  position_world.x;
	_data[1]=  position_world.y;
	_data[2]=  position_world.z;
	_data[3]=  color.x;
	_data[4]=  color.y;
	_data[5]=  color.z;
	_data[6]=  position_world.x+ spot_cone_direction_world.x* LIGHT_DRAW_MULT_FACTOR;
	_data[7]=  position_world.y+ spot_cone_direction_world.y* LIGHT_DRAW_MULT_FACTOR;
	_data[8]=  position_world.z+ spot_cone_direction_world.z* LIGHT_DRAW_MULT_FACTOR;
	_data[9]=  color.x* 0.5f;
	_data[10]= color.y* 0.5f;
	_data[11]= color.z* 0.5f;

	glGenBuffers(1, &_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_data), _data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	_position_loc     = glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_world2clip_loc   = glGetUniformLocation(_prog_draw, "world2clip_matrix");
}


void LightDraw::draw(glm::mat4 & world2clip) {
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);

	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(world2clip));
	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void *)0);
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void *)(3* sizeof(float)));

	glDrawArrays(GL_LINES, 0, 2);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


// ---------------------------------------------------------------------------------------
Light::Light() {
	
}


Light::Light(const LightParams &lp, GLuint prog_draw, glm::vec3 position_world, glm::vec3 spot_cone_direction_world) :
	_lp(lp), _is_active(true), _position_world(position_world), _spot_cone_direction_world(spot_cone_direction_world)
{
	_color= glm::vec3(1.0f, 1.0f, 1.0f); // lumière blanche ; a faire évoluer ???
	
	_light_draw= LightDraw(prog_draw, position_world, spot_cone_direction_world, _color);
}


void Light::anim(glm::mat4 & world2camera) {
	if (!_is_active)
		return;

	_position_camera= glm::vec3(world2camera* glm::vec4(_position_world, 1.0f));

	_spot_cone_direction_camera= glm::mat3(world2camera)* _spot_cone_direction_world;
	_spot_cone_direction_camera= glm::normalize(_spot_cone_direction_camera);
}


void Light::print() {
	cout << "Light -----------------------------" << endl;
	cout << "position_camera=" << glm::to_string(_position_camera) << endl;
	cout << "spot_cone_direction_camera=" << glm::to_string(_spot_cone_direction_camera) << endl;
	cout << "is_spotlight=" << _lp._is_spotlight << endl;
	cout << "color=" << glm::to_string(_color) << endl;
	cout << "position_world=" << glm::to_string(_position_world) << endl;
	cout << "spot_cone_direction_world=" << glm::to_string(_spot_cone_direction_world) << endl;
	cout << "is_active=" << _is_active << endl;
	cout << "-----------------------------" << endl;
}


// déplacement de la lumière
void Light::move(glm::vec3 destination) {
	_position_world= destination;
}


// ---------------------------------------------------------------------------------------
LightsUBO::LightsUBO() {

}


LightsUBO::LightsUBO(GLuint prog) : _prog(prog) {
	_n_attrs= sizeof(LIGHT_NAMES)/ sizeof(GLchar *);
	_indices= new GLuint[_n_attrs];
	_offsets= new GLint[_n_attrs];

	glUseProgram(_prog);
	_lights_loc= glGetUniformBlockIndex(_prog, "lights_uni");
	glGetActiveUniformBlockiv(_prog, _lights_loc, GL_UNIFORM_BLOCK_DATA_SIZE, &_ubo_size);
	glGetUniformIndices(_prog, _n_attrs, (const GLchar **)LIGHT_NAMES, _indices);
	glGetActiveUniformsiv(_prog, _n_attrs, _indices, GL_UNIFORM_OFFSET, _offsets);
	glUseProgram(0);
	
	_light_size= _ubo_size/ N_MAX_LIGHTS;
	_lights_buff= new char[_ubo_size];
	
	reset_buff();

	glGenBuffers(1, &_lights_buff_idx);

	glBindBuffer(GL_UNIFORM_BUFFER, _lights_buff_idx);
	glBufferData(GL_UNIFORM_BUFFER, _ubo_size, _lights_buff, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	// si on utilise plusieurs UBOs il faudra adapter light_binding_point
	_light_binding_point= 0;
	glBindBufferBase(GL_UNIFORM_BUFFER, _light_binding_point, _lights_buff_idx);
	glUniformBlockBinding(_prog, _lights_loc, _light_binding_point);

}


LightsUBO::~LightsUBO() {
	for (auto light : _lights) {
		delete light;
	}
	_lights.clear();
	delete[] _lights_buff;
	delete[] _indices;
	delete[] _offsets;
	glDeleteBuffers(1, &_lights_buff_idx);
}


void LightsUBO::reset_buff() {
	float f0= 0.0f;
	float t0[3]= {0.0f, 0.0f, 0.0f};
	bool b0= false;
	for (unsigned int i=0; i<N_MAX_LIGHTS; i++)	{
		memcpy(_lights_buff+ i* _light_size+ _offsets[0], t0, sizeof(float)* 3);
		memcpy(_lights_buff+ i* _light_size+ _offsets[1], t0, sizeof(float)* 3);
		memcpy(_lights_buff+ i* _light_size+ _offsets[2], t0, sizeof(float)* 3);
		memcpy(_lights_buff+ i* _light_size+ _offsets[3], &f0, sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[4], &f0, sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[5], &f0, sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[6], &f0, sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[7], &f0, sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[8], &f0, sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[9], &b0, sizeof(bool));
		memcpy(_lights_buff+ i* _light_size+ _offsets[10], &b0, sizeof(bool));
	}
}

	
void LightsUBO::update() {
	reset_buff();

	unsigned int i= 0;
	for (auto light : _lights) {
		memcpy(_lights_buff+ i* _light_size+ _offsets[0], glm::value_ptr(light->_color), sizeof(float)* 3);
		memcpy(_lights_buff+ i* _light_size+ _offsets[1], glm::value_ptr(light->_position_camera), sizeof(float)* 3);
		memcpy(_lights_buff+ i* _light_size+ _offsets[2], glm::value_ptr(light->_spot_cone_direction_camera), sizeof(float)* 3);
		memcpy(_lights_buff+ i* _light_size+ _offsets[3],  &(light->_lp._strength),              sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[4],  &(light->_lp._constant_attenuation),  sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[5],  &(light->_lp._linear_attenuation),    sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[6],  &(light->_lp._quadratic_attenuation), sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[7],  &(light->_lp._spot_cos_cutoff),       sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[8],  &(light->_lp._spot_exponent),         sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[9],  &(light->_is_active),                 sizeof(bool));
		memcpy(_lights_buff+ i* _light_size+ _offsets[10], &(light->_lp._is_spotlight),          sizeof(bool));
		i++;
	}
	glBindBuffer(GL_UNIFORM_BUFFER, _lights_buff_idx);
	glBufferData(GL_UNIFORM_BUFFER, _ubo_size, _lights_buff, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void LightsUBO::add_light(const LightParams & lp, GLuint prog_draw, glm::vec3 position_world, glm::vec3 spot_cone_direction_world) {
	Light * light= new Light(lp, prog_draw, position_world, spot_cone_direction_world);
	_lights.push_back(light);
	
	update();
}


void LightsUBO::anim(glm::mat4 & world2camera) {
	for (auto light : _lights) {
		// simu jour /nuit
		/*glm::mat4 m= glm::rotate(0.005f, glm::vec3(1.0f, 0.0f, 0.0f));
		light->_position_world= glm::vec3(m* glm::vec4(light->_position_world, 1.0f));*/
		light->anim(world2camera);
	}

	update();
}


void LightsUBO::draw(glm::mat4 & world2clip) {
	for (auto light : _lights) {
		light->_light_draw.draw(world2clip);
	}
}


void LightsUBO::print() {
	cout << " LightsUBO -----------------------------" << endl;
	cout << "lights_loc=" << _lights_loc << endl;
	cout << "lights_buff_idx=" << _lights_buff_idx << endl;
	cout << "ubo_size=" << _ubo_size << endl;
	cout << "light_binding_point=" << _light_binding_point << endl;
	cout << "light_size=" << _light_size << endl;
	cout << "prog=" << _prog << endl;
	cout << "-----------------------------" << endl;
}

