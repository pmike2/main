

#include "light.h"

using namespace std;


Light::Light(){
}


Light::Light(bool is_spotlight_, GLuint prog_draw_, struct timeval t_){

	// a faire varier dans config.cpp ?
	strength= 100.0;
	constant_attenuation= 0.4;
	linear_attenuation= 0.04;
	quadratic_attenuation= 0.01;
	spot_cos_cutoff= 0.99; // + la valeur est proche de 1.0, plus le spot est concentre; je prenais 0.99 à la base !
	spot_exponent= 3.0;
	
	is_spotlight= is_spotlight_;
	
	glGenBuffers(1, &draw_light_buffer);
	prog_draw= prog_draw_;
	world2clip_loc= glGetUniformLocation(prog_draw, "world2clip_matrix");

	is_active= true;
	t= t_;
}


void Light::draw(float * world2clip){
	if (!is_active)
		return;

	glUseProgram(prog_draw);
	glUniformMatrix4fv(world2clip_loc, 1, GL_FALSE, world2clip);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, draw_light_buffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));
	glDrawArrays(GL_LINES, 0, 6);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glUseProgram(0);
}


void Light::anim(float * world2camera){
	if (!is_active)
		return;

	unsigned int i;
	unsigned int diff_ms= diff_time_ms_from_now(&t);
	float morph= (float)(diff_ms)/ (float)(config->lifetime); // entre 0 (initial) et 1 (final)
	if (morph> 1.0) {
		is_active= false;
		//cout << "Fin Light" << endl;
		return;
	}

	// modif de morph en fonction des morph_pos
	for (i=1; i<config->morph_pos.size(); i++)
		if (config->morph_pos[i].x> morph)
			break;
	morph= config->morph_pos[i- 1].y+ (morph- config->morph_pos[i- 1].x)* 
		(config->morph_pos[i].y- config->morph_pos[i- 1].y)/ (config->morph_pos[i].x- config->morph_pos[i- 1].x);

	for (i=0; i<3; i++)
		current_values.color[i]= (1.0- morph)* config->init_values.color[i]+ morph* config->final_values.color[i];
	for (i=0; i<4; i++)
		current_values.position_world[i]= (1.0- morph)* config->init_values.position_world[i]+ morph* config->final_values.position_world[i];
	for (i=0; i<3; i++)
		current_values.spot_cone_direction_world[i]= (1.0- morph)* config->init_values.spot_cone_direction_world[i]+ morph* config->final_values.spot_cone_direction_world[i];
	
	glm::mat4 glm_world2camera= glm::make_mat4(world2camera);

	glm::vec4 glm_position_world= glm::make_vec4(current_values.position_world);
	glm::vec4 glm_position_camera= glm_world2camera* glm_position_world;
	position_camera[0]= glm_position_camera[0];
	position_camera[1]= glm_position_camera[1];
	position_camera[2]= glm_position_camera[2];

	glm::vec3 glm_cone_world= glm::make_vec3(current_values.spot_cone_direction_world);
	glm::mat3 glm_w2c_upperleft= glm::mat3(glm_world2camera);
	glm::vec3 glm_cone_camera= glm_w2c_upperleft* glm_cone_world;
	if ( fabs(glm_cone_camera[0])+ fabs(glm_cone_camera[1])+ fabs(glm_cone_camera[2])< 0.00001 )
	{
		spot_cone_direction_camera[0]= 0.;
		spot_cone_direction_camera[1]= 0.;
		spot_cone_direction_camera[2]= 0.;
	}
	else
	{
		glm::vec3 glm_cone_camera_norm= glm::normalize(glm_cone_camera);
		spot_cone_direction_camera[0]= glm_cone_camera_norm[0];
		spot_cone_direction_camera[1]= glm_cone_camera_norm[1];
		spot_cone_direction_camera[2]= glm_cone_camera_norm[2];
	}

	draw_light_data[0]= current_values.position_world[0];
	draw_light_data[1]= current_values.position_world[1];
	draw_light_data[2]= current_values.position_world[2];
	draw_light_data[3]= current_values.color[0];
	draw_light_data[4]= current_values.color[1];
	draw_light_data[5]= current_values.color[2];
	draw_light_data[6]= current_values.position_world[0]+ current_values.spot_cone_direction_world[0];
	draw_light_data[7]= current_values.position_world[1]+ current_values.spot_cone_direction_world[1];
	draw_light_data[8]= current_values.position_world[2]+ current_values.spot_cone_direction_world[2];
	draw_light_data[9]= current_values.color[0]* 0.5;
	draw_light_data[10]= current_values.color[1]* 0.5;
	draw_light_data[11]= current_values.color[2]* 0.5;

	glBindBuffer(GL_ARRAY_BUFFER, draw_light_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(draw_light_data), draw_light_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Light::set_config(LightConfig * config_) {
	config= config_;
}


void Light::print() {
	cout << "Light -----------------------------" << endl;
	cout << "position_camera=(" << position_camera[0] << "," << position_camera[1] << "," << position_camera[2] << ")" << endl;
	cout << "spot_cone_direction_camera=(" << spot_cone_direction_camera[0] << "," << spot_cone_direction_camera[1] << "," << spot_cone_direction_camera[2] << ")" << endl;
	cout << "is_spotlight=" << is_spotlight << endl;
	cout << "current_values.color=(" << current_values.color[0] << "," << current_values.color[1] << "," << current_values.color[2] << ")" << endl;
	cout << "current_values.position_world=(" << current_values.position_world[0] << "," << current_values.position_world[1] << "," << current_values.position_world[2] << "," << current_values.position_world[3] << ")" << endl;
	cout << "current_values.spot_cone_direction_world=(" << current_values.spot_cone_direction_world[0] << "," << current_values.spot_cone_direction_world[1] << "," << current_values.spot_cone_direction_world[2] << ")" << endl;
	cout << "is_active=" << is_active << endl;
	cout << "config->lifetime=" << config->lifetime << endl;
	cout << "-----------------------------" << endl;
}

// ----------------------------------------------------------------------------------------------------------------


LightsUBO::LightsUBO(){
	n_attrs= sizeof(LIGHT_NAMES)/ sizeof(GLchar *);
	indices= (GLuint *)malloc(n_attrs* sizeof(GLuint));
	offsets= (GLint *)malloc(n_attrs* sizeof(GLint));
}


void LightsUBO::set_prog(GLuint prog_){
	prog= prog_;
}


// a faire APRES set_prog() !
void LightsUBO::init(){
	glUseProgram(prog);
	lights_loc= glGetUniformBlockIndex(prog, "lights_uni");
	glGetActiveUniformBlockiv(prog, lights_loc, GL_UNIFORM_BLOCK_DATA_SIZE, &ubo_size);
	glGetUniformIndices(prog, n_attrs, (const GLchar **)LIGHT_NAMES, indices);
	glGetActiveUniformsiv(prog, n_attrs, indices, GL_UNIFORM_OFFSET, offsets);
	glUseProgram(0);

	light_size= ubo_size/ N_MAX_LIGHTS;
	lights_buff= (char * )malloc(ubo_size);
	memset(lights_buff, 0, ubo_size);
	
	reset_buff();

	glGenBuffers(1, &lights_buff_idx);

	glBindBuffer(GL_UNIFORM_BUFFER, lights_buff_idx);
	glBufferData(GL_UNIFORM_BUFFER, ubo_size, lights_buff, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	// si on utilise plusieurs UBOs il faudra adapter light_binding_point
	light_binding_point= 0;
	glBindBufferBase(GL_UNIFORM_BUFFER, light_binding_point, lights_buff_idx);
	glUniformBlockBinding(prog, lights_loc, light_binding_point);
}


void LightsUBO::reset_buff() {
	float f0= 0.0;
	float t0[3]= {0.0, 0.0, 0.0};
	bool b0= false;
	for (unsigned int i=0; i< N_MAX_LIGHTS; i++)
	{
		memcpy(lights_buff+ i* light_size+ offsets[0], t0, sizeof(float)* 3);
		memcpy(lights_buff+ i* light_size+ offsets[1], t0, sizeof(float)* 3);
		memcpy(lights_buff+ i* light_size+ offsets[2], t0, sizeof(float)* 3);
		memcpy(lights_buff+ i* light_size+ offsets[3], &f0, sizeof(float));
		memcpy(lights_buff+ i* light_size+ offsets[4], &f0, sizeof(float));
		memcpy(lights_buff+ i* light_size+ offsets[5], &f0, sizeof(float));
		memcpy(lights_buff+ i* light_size+ offsets[6], &f0, sizeof(float));
		memcpy(lights_buff+ i* light_size+ offsets[7], &f0, sizeof(float));
		memcpy(lights_buff+ i* light_size+ offsets[8], &f0, sizeof(float));
		memcpy(lights_buff+ i* light_size+ offsets[9], &b0, sizeof(bool));
		memcpy(lights_buff+ i* light_size+ offsets[10], &b0, sizeof(bool));
	}
}

	
void LightsUBO::update(std::vector<Light> lights) {
	reset_buff();
	for (unsigned int i=0; i< lights.size(); i++)
	{
		memcpy(lights_buff+ i* light_size+ offsets[0], lights[i].current_values.color, sizeof(float)* 3);
		memcpy(lights_buff+ i* light_size+ offsets[1], lights[i].position_camera, sizeof(float)* 3);
		memcpy(lights_buff+ i* light_size+ offsets[2], lights[i].spot_cone_direction_camera, sizeof(float)* 3);
		memcpy(lights_buff+ i* light_size+ offsets[3], &(lights[i].strength), sizeof(float));
		memcpy(lights_buff+ i* light_size+ offsets[4], &(lights[i].constant_attenuation), sizeof(float));
		memcpy(lights_buff+ i* light_size+ offsets[5], &(lights[i].linear_attenuation), sizeof(float));
		memcpy(lights_buff+ i* light_size+ offsets[6], &(lights[i].quadratic_attenuation), sizeof(float));
		memcpy(lights_buff+ i* light_size+ offsets[7], &(lights[i].spot_cos_cutoff), sizeof(float));
		memcpy(lights_buff+ i* light_size+ offsets[8], &(lights[i].spot_exponent), sizeof(float));
		memcpy(lights_buff+ i* light_size+ offsets[9], &(lights[i].is_active), sizeof(bool));
		memcpy(lights_buff+ i* light_size+ offsets[10], &(lights[i].is_spotlight), sizeof(bool));
	}
	glBindBuffer(GL_UNIFORM_BUFFER, lights_buff_idx);
	glBufferData(GL_UNIFORM_BUFFER, ubo_size, lights_buff, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	// si trop lent tester un memcpy partiel de juste ce qui a changé, accompagné des glBufferSubData juste nécessaires; ex :
	/*for (unsigned int i=0; i<lights.size(); i++)	{
		glBufferSubData(GL_UNIFORM_BUFFER, lights_ubo.light_size* i+ lights_ubo.offsets[0], sizeof(float)* 3, &(lights[i].current_values.color));
		glBufferSubData(GL_UNIFORM_BUFFER, lights_ubo.light_size* i+ lights_ubo.offsets[1], sizeof(float)* 3, &(lights[i].position_camera));
		glBufferSubData(GL_UNIFORM_BUFFER, lights_ubo.light_size* i+ lights_ubo.offsets[2], sizeof(float)* 3, &(lights[i].spot_cone_direction_camera));
		glBufferSubData(GL_UNIFORM_BUFFER, lights_ubo.light_size* i+ lights_ubo.offsets[9], sizeof(bool), &(lights[i].is_active));
	}*/
}


void LightsUBO::release(){
	free(lights_buff);
	glDeleteBuffers(1, &lights_buff_idx);
}


void LightsUBO::print() {
	cout << " LightsUBO -----------------------------" << endl;
	cout << "lights_loc=" << lights_loc << endl;
	cout << "lights_buff_idx=" << lights_buff_idx << endl;
	cout << "ubo_size=" << ubo_size << endl;
	cout << "light_binding_point=" << light_binding_point << endl;
	cout << "light_size=" << light_size << endl;
	cout << "prog=" << prog << endl;
	cout << "-----------------------------" << endl;
}

