

#include "light.h"

using namespace std;


LightDraw::LightDraw() {
	
}


LightDraw::LightDraw(GLuint prog_draw, float* position_world, float* spot_cone_direction_world, float* color) {
	_prog_draw= prog_draw;
	
	_data[0]=  position_world[0];
	_data[1]=  position_world[1];
	_data[2]=  position_world[2];
	_data[3]=  color[0];
	_data[4]=  color[1];
	_data[5]=  color[2];
	_data[6]=  position_world[0]+ spot_cone_direction_world[0]* LIGHT_DRAW_MULT_FACTOR;
	_data[7]=  position_world[1]+ spot_cone_direction_world[1]* LIGHT_DRAW_MULT_FACTOR;
	_data[8]=  position_world[2]+ spot_cone_direction_world[2]* LIGHT_DRAW_MULT_FACTOR;
	_data[9]=  color[0]* 0.5f;
	_data[10]= color[1]* 0.5f;
	_data[11]= color[2]* 0.5f;

	glGenBuffers(1, &_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_data), _data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	_position_loc     = glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_world2clip_loc   = glGetUniformLocation(_prog_draw, "world2clip_matrix");
}


void LightDraw::draw(float * world2clip) {
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);

	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, world2clip);
	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_LINES, 0, 2);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


// ---------------------------------------------------------------------------------------
Light::Light() {
	
}


Light::Light(const LightParams &lp, GLuint prog_draw, float* position_world, float* spot_cone_direction_world) :
	_lp(lp), _is_active(true)
{
	unsigned int i;
	
	for (i=0; i<3; i++)
		_color[i]= 1.0f; // lumière blanche
	for (i=0; i<4; i++)
		_position_world[i]= position_world[i];
	for (i=0; i<3; i++)
		_spot_cone_direction_world[i]= spot_cone_direction_world[i];
	
	_light_draw= LightDraw(prog_draw, position_world, spot_cone_direction_world, _color);
}


void Light::anim(float * world2camera) {
	if (!_is_active)
		return;

	glm::mat4 glm_world2camera= glm::make_mat4(world2camera);

	glm::vec4 glm_position_world= glm::make_vec4(_position_world);
	glm::vec4 glm_position_camera= glm_world2camera* glm_position_world;
	_position_camera[0]= glm_position_camera[0];
	_position_camera[1]= glm_position_camera[1];
	_position_camera[2]= glm_position_camera[2];

	glm::vec3 glm_cone_world= glm::make_vec3(_spot_cone_direction_world);
	glm::mat3 glm_w2c_upperleft= glm::mat3(glm_world2camera);
	glm::vec3 glm_cone_camera= glm_w2c_upperleft* glm_cone_world;
	if ( fabs(glm_cone_camera[0])+ fabs(glm_cone_camera[1])+ fabs(glm_cone_camera[2])< 0.00001f ) {
		cout << "cone nul\n";
		_spot_cone_direction_camera[0]= 0.0f;
		_spot_cone_direction_camera[1]= 0.0f;
		_spot_cone_direction_camera[2]= 0.0f;
	}
	else {
		glm::vec3 glm_cone_camera_norm= glm::normalize(glm_cone_camera);
		_spot_cone_direction_camera[0]= glm_cone_camera_norm[0];
		_spot_cone_direction_camera[1]= glm_cone_camera_norm[1];
		_spot_cone_direction_camera[2]= glm_cone_camera_norm[2];
	}
}


void Light::print() {
	cout << "Light -----------------------------" << endl;
	cout << "position_camera=(" << _position_camera[0] << "," << _position_camera[1] << "," << _position_camera[2] << ")" << endl;
	cout << "spot_cone_direction_camera=(" << _spot_cone_direction_camera[0] << "," << _spot_cone_direction_camera[1] << "," << _spot_cone_direction_camera[2] << ")" << endl;
	cout << "is_spotlight=" << _lp._is_spotlight << endl;
	cout << "color=(" << _color[0] << "," << _color[1] << "," << _color[2] << ")" << endl;
	cout << "position_world=(" << _position_world[0] << "," << _position_world[1] << "," << _position_world[2] << "," << _position_world[3] << ")" << endl;
	cout << "spot_cone_direction_world=(" << _spot_cone_direction_world[0] << "," << _spot_cone_direction_world[1] << "," << _spot_cone_direction_world[2] << ")" << endl;
	cout << "is_active=" << _is_active << endl;
	cout << "-----------------------------" << endl;
}


// déplacement de la lumière
void Light::move(glm::vec3 destination) {
	_position_world[0]= destination[0];
	_position_world[1]= destination[1];
	_position_world[2]= destination[2];
}


// ---------------------------------------------------------------------------------------
LightsUBO::LightsUBO() {
	_n_attrs= sizeof(LIGHT_NAMES)/ sizeof(GLchar *);
	_indices= (GLuint *)malloc(_n_attrs* sizeof(GLuint));
	_offsets= (GLint *)malloc(_n_attrs* sizeof(GLint));
}


void LightsUBO::set_prog(GLuint prog) {
	_prog= prog;
}


// a faire APRES set_prog() !
void LightsUBO::init() {
	glUseProgram(_prog);
	_lights_loc= glGetUniformBlockIndex(_prog, "lights_uni");
	glGetActiveUniformBlockiv(_prog, _lights_loc, GL_UNIFORM_BLOCK_DATA_SIZE, &_ubo_size);
	glGetUniformIndices(_prog, _n_attrs, (const GLchar **)LIGHT_NAMES, _indices);
	glGetActiveUniformsiv(_prog, _n_attrs, _indices, GL_UNIFORM_OFFSET, _offsets);
	glUseProgram(0);

	_light_size= _ubo_size/ N_MAX_LIGHTS;
	_lights_buff= (char * )malloc(_ubo_size);
	memset(_lights_buff, 0, _ubo_size);
	
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


void LightsUBO::reset_buff() {
	float f0= 0.0f;
	float t0[3]= {0.0f, 0.0f, 0.0f};
	bool b0= false;
	for (unsigned int i=0; i<N_MAX_LIGHTS; i++)
	{
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

	
void LightsUBO::update(std::vector<Light> lights) {
	reset_buff();
	for (unsigned int i=0; i< lights.size(); i++) {
		memcpy(_lights_buff+ i* _light_size+ _offsets[0],  lights[i]._color,                      sizeof(float)* 3);
		memcpy(_lights_buff+ i* _light_size+ _offsets[1],  lights[i]._position_camera,            sizeof(float)* 3);
		memcpy(_lights_buff+ i* _light_size+ _offsets[2],  lights[i]._spot_cone_direction_camera, sizeof(float)* 3);
		memcpy(_lights_buff+ i* _light_size+ _offsets[3],  &(lights[i]._lp._strength),                sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[4],  &(lights[i]._lp._constant_attenuation),    sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[5],  &(lights[i]._lp._linear_attenuation),      sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[6],  &(lights[i]._lp._quadratic_attenuation),   sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[7],  &(lights[i]._lp._spot_cos_cutoff),         sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[8],  &(lights[i]._lp._spot_exponent),           sizeof(float));
		memcpy(_lights_buff+ i* _light_size+ _offsets[9],  &(lights[i]._is_active),               sizeof(bool));
		memcpy(_lights_buff+ i* _light_size+ _offsets[10], &(lights[i]._lp._is_spotlight),            sizeof(bool));
	}
	glBindBuffer(GL_UNIFORM_BUFFER, _lights_buff_idx);
	glBufferData(GL_UNIFORM_BUFFER, _ubo_size, _lights_buff, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	// si trop lent tester un memcpy partiel de juste ce qui a changé, accompagné des glBufferSubData juste nécessaires; ex :
	/*for (unsigned int i=0; i<lights.size(); i++)	{
		glBufferSubData(GL_UNIFORM_BUFFER, lights_ubo.light_size* i+ lights_ubo.offsets[0], sizeof(float)* 3, &(lights[i].color));
		glBufferSubData(GL_UNIFORM_BUFFER, lights_ubo.light_size* i+ lights_ubo.offsets[1], sizeof(float)* 3, &(lights[i].position_camera));
		glBufferSubData(GL_UNIFORM_BUFFER, lights_ubo.light_size* i+ lights_ubo.offsets[2], sizeof(float)* 3, &(lights[i].spot_cone_direction_camera));
		glBufferSubData(GL_UNIFORM_BUFFER, lights_ubo.light_size* i+ lights_ubo.offsets[9], sizeof(bool), &(lights[i].is_active));
	}*/
}


void LightsUBO::release() {
	free(_lights_buff);
	free(_indices);
	free(_offsets);
	glDeleteBuffers(1, &_lights_buff_idx);
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

