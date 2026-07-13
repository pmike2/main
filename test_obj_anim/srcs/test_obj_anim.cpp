#include "test_obj_anim.h"


TestObjAnim::TestObjAnim() {

}


TestObjAnim::TestObjAnim(GLDrawManager * gl_draw_manager, ViewSystem * view_system, time_point t) :
	_gl_draw_manager(gl_draw_manager), _view_system(view_system), _paused(false)
{
	std::vector<std::string> model_names {"test", "test2", "test3"};
	
	for (auto & model_name : model_names) {
		AnimatedObjModel * model = new AnimatedObjModel("../data/" + model_name + ".json");
		_gl_draw_manager->add_texture_buffer(model_name, "anim_buffer", GL_R32F, 0);
		_gl_draw_manager->set_texture_buffer_data(model_name, "anim_buffer", model->_buffer_texture_data, model->_buffer_texture_data_size * sizeof(float));
		// on a plus besoin de ça
		delete model->_buffer_texture_data;
		_models.push_back(model);

		//GLDrawContext * context = _gl_draw_manager->get_context(model->_name);
		//std::cout << *context << "\n";
	}
	
	//_instances.push_back(new AnimatedObjInstance(get_model("test"), pt_3d(0.0, 0.0, 0.0), t));
	//_instances.push_back(new AnimatedObjInstance(get_model("test"), pt_3d(5.0, 0.0, 0.0), t));
	//_instances.push_back(new AnimatedObjInstance(get_model("test2"), pt_3d(0.0, 0.0, 5.0), t));
	//_instances.push_back(new AnimatedObjInstance(get_model("test3"), pt_3d(-5.0, -5.0, 0.0), t));

	//_instances[0]->_current_action = "walk";


	for (uint i=0; i<30000; ++i) {
		//int j = rand_int(0, model_names.size() - 1);
		//std::string model_name = model_names[j];
		//std::string model_name = "test3";
		std::string model_name = "test";

		uint idx_action = rand_int(0, get_model(model_name)->_actions.size() - 1);

		_instances.push_back(new AnimatedObjInstance(get_model(model_name), rand_pt_3d(pt_3d(-20.0), pt_3d(20.0)), t, rand_quat(), get_model(model_name)->_actions[idx_action]->_name));
	}

	for (auto & model : _models) {
		update_static_buffer(model);
		update_dynamic_buffer(model);
	}
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
		instance->anim(t);
	}

	for (auto & model : _models) {
		update_dynamic_buffer(model);
	}
}


void TestObjAnim::update_static_buffer(AnimatedObjModel * model) {
	GLDrawContext * context = _gl_draw_manager->get_context(model->_name);
	context->_n_pts = model->_obj_data->_n_pts;
	context->set_data(model->_obj_data->_data, 0);
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

		GLDrawContext * context = _gl_draw_manager->get_context("test3");
		context->show_data();
	}
	return false;
}
