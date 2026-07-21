#include "test_obj_anim.h"


TestInstance::TestInstance() {

}


TestInstance::TestInstance(AnimatedObjModel * model, pt_3d pos, time_point t, number angle, std::string action_name) :
	AnimatedObjInstance(model, pos, t, quat(1.0, 0.0, 0.0, 0.0), action_name)
{
	set_angle(angle);
}


TestInstance::~TestInstance() {

}


void TestInstance::set_angle(number angle) {
	_angle = angle;
	_direction.x = cos(_angle);
	_direction.y = sin(_angle);
	// par défaut les modèles regardent vers -Y -> rotation de pi / 2
	set_rot(glm::angleAxis(_angle + M_PI * 0.5, pt_3d(0.0, 0.0, 1.0)));
}


void TestInstance::anim_test(time_point t) {
	if (rand_int(0, 100) == 0) {
		if (get_action() == "walk") {
			set_action("watch");
		}
		else {
			set_angle(rand_number(0.0, 2.0 * M_PI));
			set_action("walk");
		}
	}

	anim(t);

	if (_model->_actions[_idx_action]->_name == "walk") {
		set_pos(_position + 0.03 * pt_3d(_direction.x, _direction.y, 0.0));
	}
}


// ------------------------------------------------------------------

TestObjAnim::TestObjAnim() {

}


TestObjAnim::TestObjAnim(GLDrawManager * gl_draw_manager, ViewSystem * view_system, time_point t) :
	_gl_draw_manager(gl_draw_manager), _view_system(view_system), _paused(true)
{
	//_gl_draw_manager->set_verbose(true);
	
	fs root_anim_objs = "../data/anim_objs";

	for (auto & anim_obj_dir : std::filesystem::directory_iterator(root_anim_objs)) {
		if (!anim_obj_dir.is_directory()) {
			continue;
		}

		std::string anim_obj_name = anim_obj_dir.path().filename().string();

		if (anim_obj_name != "perso2") {
			continue;
		}

		std::string json_name = anim_obj_name + ".json";
		fs json_path = anim_obj_dir.path() / json_name;
		//std::cout << json_path << "\n";
		AnimatedObjModel * model = new AnimatedObjModel(json_path);
		//model->_n_ms_per_frame = 200;
		//std::cout << *model << "\n";
		std::cout << model->_obj_data->_n_pts << "\n";

		GLDrawContext * context = _gl_draw_manager->get_context(model->_name);

		_gl_draw_manager->add_texture_buffer(model->_name, "anim_buffer", GL_R32F, 0);
		_gl_draw_manager->set_texture_buffer_data(model->_name, "anim_buffer", model->_buffer_texture_data, model->_buffer_texture_data_size * sizeof(float));
		// on a plus besoin de ça
		delete model->_buffer_texture_data;
		
		_gl_draw_manager->add_texture(
			context->_name, "idx_texture", GL_TEXTURE_2D, 1,
				std::map<GLenum, int>{
				{GL_TEXTURE_MIN_FILTER, GL_NEAREST}, {GL_TEXTURE_MAG_FILTER, GL_NEAREST},
				{GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE}, {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE}
				},
			GL_RED, glm::uvec3(IDX_TEXTURE_DATA_SIZE, IDX_TEXTURE_DATA_SIZE, 0), GL_RED, GL_FLOAT
		);
		_gl_draw_manager->set_texture_data(context->_name, "idx_texture", model->_idx_texture_data);

		//_gl_draw_manager->_texture_pool->get_texture(context->_name, "idx_texture")->export2pgm("../data/test.pgm");
		//_gl_draw_manager->_texture_pool->get_texture(context->_name, "idx_texture")->print_data();

		std::vector<fs> diffuse_textures, normal_textures;
		for (auto & material : model->_obj_data->_materials) {
			if (material->_diffuse_tex_path != "") {
				diffuse_textures.push_back(material->_diffuse_tex_path);
			}
			else {
				std::cerr << "Matériau sans diffuse -> ca va être bizarre\n";
			}
			if (material->_normal_tex_path != "") {
				normal_textures.push_back(material->_normal_tex_path);
			}
			else {
				std::cerr << "Matériau sans normal -> ca va être bizarre\n";
			}
		}

		_gl_draw_manager->add_texture(
			context->_name, "diffuse_texture", GL_TEXTURE_2D_ARRAY, 2,
				std::map<GLenum, int>{
				{GL_TEXTURE_MIN_FILTER, GL_LINEAR}, {GL_TEXTURE_MAG_FILTER, GL_LINEAR},
				{GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE}, {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE}
				},
			GL_RGBA, glm::uvec3(512, 512, diffuse_textures.size()), GL_BGRA, GL_UNSIGNED_BYTE
		);
		_gl_draw_manager->set_texture_data(context->_name, "diffuse_texture", diffuse_textures);

		_gl_draw_manager->add_texture(
			context->_name, "normal_texture", GL_TEXTURE_2D_ARRAY, 3,
				std::map<GLenum, int>{
				{GL_TEXTURE_MIN_FILTER, GL_LINEAR}, {GL_TEXTURE_MAG_FILTER, GL_LINEAR},
				{GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE}, {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE}
				},
		GL_RGBA, glm::uvec3(512, 512, diffuse_textures.size()), GL_BGRA, GL_UNSIGNED_BYTE
		);
		_gl_draw_manager->set_texture_data(context->_name, "normal_texture", normal_textures);

		_models.push_back(model);

		//std::cout << *context << "\n";
	}

	//std::cout << *_gl_draw_manager << "\n";

	//_instances.push_back(new AnimatedObjInstance(get_model("test"), pt_3d(0.0, 0.0, 0.0), t));
	//_instances.push_back(new AnimatedObjInstance(get_model("test"), pt_3d(5.0, 0.0, 0.0), t));
	//_instances.push_back(new AnimatedObjInstance(get_model("test2"), pt_3d(0.0, 0.0, 5.0), t));
	//_instances.push_back(new AnimatedObjInstance(get_model("test3"), pt_3d(-5.0, -5.0, 0.0), t));

	//_instances[0]->_current_action = "walk";


	uint n_instances = 2000;
	for (uint i=0; i<n_instances; ++i) {
		//int j = rand_int(0, model_names.size() - 1);
		//std::string model_name = model_names[j];
		std::string model_name = "perso2";
		//std::string model_name = "perso";

		//pt_3d pos(0.0);
		//pt_3d pos = rand_pt_3d(pt_3d(-20.0), pt_3d(20.0));
		//pt_3d pos = pt_3d(float(i) * 1.0, float(i) * 1.0, 0.0);
		pt_3d pos = rand_pt_3d(-100.0, 100.0, -100.0, 100.0, 0.0, 0.0);

		number angle = rand_number(0.0, M_PI * 2.0);
		//number angle = 0.0;

		uint idx_action = rand_int(0, get_model(model_name)->_actions.size() - 1);
		//uint idx_action = 0;
		std::string action_name = get_model(model_name)->_actions[idx_action]->_name;
		//std::string action_name = "walk";

		_instances.push_back(new TestInstance(get_model(model_name), pos, t, angle, action_name));
	}

	for (auto & model : _models) {
		update_static_buffer(model);
		update_dynamic_buffer(model);
	}

	// à activer lors du debug, et à faire quand tout a été mis en place (textures notamment)
	//_gl_draw_manager->validate();
}


TestObjAnim::~TestObjAnim() {
	for (auto & instance : _instances) {
		delete instance;
	}
	_instances.clear();
	for (auto & model : _models) {
		delete model;
	}
	_models.clear();
}


AnimatedObjModel * TestObjAnim::get_model(std::string model_name) {
	for (auto & model : _models) {
		if (model->_name == model_name) {
			return model;
		}
	}
	return NULL;
}


void TestObjAnim::anim(time_point t) {
	if (_paused) {
		return;
	}

	for (auto & instance : _instances) {
		instance->anim_test(t);
	}

	for (auto & model : _models) {
		update_dynamic_buffer(model);
	}
}


void TestObjAnim::update_static_buffer(AnimatedObjModel * model) {
	GLDrawContext * context = _gl_draw_manager->get_context(model->_name);
	
	if (model->_name == "perso2") {
		model->_obj_data->update_data(std::vector<OBJDATA_DATA_ITEM>{OBJDATA_VERTEX, OBJDATA_NORMAL, OBJDATA_TANGENT, OBJDATA_BITANGENT, OBJDATA_TEXTURE, OBJDATA_SHININESS});
	}
	else {
		model->_obj_data->update_data(std::vector<OBJDATA_DATA_ITEM>{OBJDATA_VERTEX, OBJDATA_NORMAL, OBJDATA_TEXTURE});
	}

	context->_n_pts = model->_obj_data->_n_pts;
	context->set_data(model->_obj_data->_data, 0);
	//context->show_data();
}


void TestObjAnim::update_dynamic_buffer(AnimatedObjModel * model) {
	GLDrawContext * context = _gl_draw_manager->get_context(model->_name);

	context->_n_instances = 0;
	for (auto & instance : _instances) {
		if (instance->_model == model) {
			context->_n_instances++;
		}
	}

	float * data = new float[context->_n_instances * (1 + 1 + 16)];
	float * ptr = data;

	for (auto & instance : _instances) {
		if (instance->_model != model) {
			continue;
		}

		ptr[0] = float(instance->_idx_action);
		ptr[1] = float(instance->_idx_frame);
		ptr += 2;

		const float * instance_mat = glm::value_ptr(glm::mat4(instance->_model2world));
		std::memcpy(ptr, instance_mat, 16 * sizeof(float));
		ptr += 16;
	}

	context->set_data(data, 1);
	delete[] data;
}


void TestObjAnim::draw() {
	for (auto & model : _models) {
		GLDrawContext * context = _gl_draw_manager->get_context(model->_name);

		context->activate();
		context->set_uniform("world2clip_matrix", glm::value_ptr(glm::mat4(_view_system->_world2clip)));
		context->set_uniform("light_position", glm::value_ptr(LIGHT_POSITION));
		context->set_uniform("light_color", glm::value_ptr(LIGHT_COLOR));
		context->set_uniform("view_position", glm::value_ptr(glm::vec3(_view_system->_eye)));
		context->draw();
		context->deactivate();
	}
}


bool TestObjAnim::key_down(InputState * input_state, SDL_Keycode key, time_point t) {
	if (key == SDLK_a) {
		for (auto & instance : _instances) {
			if (instance->get_action() == "move") {
				instance->set_action("move2");
			}
			else if (instance->get_action() == "move2") {
				instance->set_action("move");
			}
		}
		return true;
	}

	if (key == SDLK_SPACE) {
		_paused = !_paused;
	}
	return false;
}
