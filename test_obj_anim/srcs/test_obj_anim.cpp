
#include "test_obj_anim.h"



mat_4d parse_js_matrix(json js) {
	float mat_values[16];
	uint col = 0;
	uint lig = 0;
	for (auto & row : js) {
		col = 0;
		for (auto & x : row) {
			mat_values[4 * col + lig] = x;
			col++;
		}
		lig++;
	}
	return glm::make_mat4(mat_values);
}


// ------------------------------------------------
AnimatedObjBone::AnimatedObjBone() {

}


AnimatedObjBone::AnimatedObjBone(std::string name, mat_4d mat_local, std::string parent_name) :
	_name(name), _mat_local(mat_local), _parent(NULL), _parent_name(parent_name)
{

}


AnimatedObjBone::~AnimatedObjBone() {
	
}


std::ostream & operator << (std::ostream & os, AnimatedObjBone & bone) {
	os << "name = " << bone._name;
	os << " ; matrix_local = " << glm::to_string(bone._mat_local);
	os << " ; parent = " << bone._parent_name;
	return os;
}


// ------------------------------------------------
AnimatedObjTransform::AnimatedObjTransform() {

}


AnimatedObjTransform::AnimatedObjTransform(AnimatedObjBone * bone, mat_4d mat) :
	_bone(bone), _mat(mat)
{

}

AnimatedObjTransform::~AnimatedObjTransform() {

}


std::ostream & operator << (std::ostream & os, AnimatedObjTransform & transform) {
	os << "idx = " << transform._idx;
	os << " ; mat = " << glm::to_string(transform._mat);
	return os;
}


// ------------------------------------------------
AnimatedObjFrame::AnimatedObjFrame() {

}


AnimatedObjFrame::~AnimatedObjFrame() {
	delete _data;
}


AnimatedObjTransform * AnimatedObjFrame::get_transform(AnimatedObjBone * bone) {
	for (auto & tr : _transforms) {
		if (tr->_bone == bone) {
			return tr;
		}
	}
	return NULL;
}


std::ostream & operator << (std::ostream & os, AnimatedObjFrame & frame) {
	os << "transforms = ";
	for (auto & transform : frame._transforms) {
		os << *transform << "\n";
	}
	return os;
}


// ------------------------------------------------
AnimatedObjAction::AnimatedObjAction() {

}


AnimatedObjAction::AnimatedObjAction(std::string name) : _name(name) {

}


AnimatedObjAction::~AnimatedObjAction() {

}


std::ostream & operator << (std::ostream & os, AnimatedObjAction & action) {
	os << "name = " << action._name << "\n";
	os << "frames = ";
	for (auto & frame : action._frames) {
		os << *frame << "\n";
	}
	return os;
}


// ------------------------------------------------
AnimatedObjObject::AnimatedObjObject() {

}


AnimatedObjObject::AnimatedObjObject(ObjObject * static_object) : _static_object(static_object) {
	uint n_vertices = _static_object->_vertices.size();
	_bones = new AnimatedObjBone *[n_vertices * 4];
	_weights = new number[n_vertices * 4];
	for (uint i=0; i<n_vertices * 4; ++i) {
		_bones[i] = NULL;
		_weights[i] = -1.0;
	}
}


AnimatedObjObject::~AnimatedObjObject() {

}


// ------------------------------------------------
AnimatedObjModel::AnimatedObjModel() {

}


AnimatedObjModel::AnimatedObjModel(std::string json_path) {
	std::filesystem::path js_path = json_path;
	std::string obj_filename = js_path.stem().string() + ".obj";
	std::filesystem::path obj_path = js_path.parent_path() / obj_filename;

	_name = js_path.stem().string();
	
	_obj_data = new ObjData(obj_path.string());
	_obj_data->_use_ambient = false;
	_obj_data->_use_diffuse = true;
	_obj_data->_use_specular = false;
	_obj_data->_use_shininess = false;
	_obj_data->_use_opacity = false;
	_obj_data->update_data();

	for (auto & object : _obj_data->_objects) {
		_objects.push_back(new AnimatedObjObject(object));
	}

	std::ifstream ifs(json_path);
	json js = json::parse(ifs);
	ifs.close();

	for (json::iterator it = js["bones"].begin(); it != js["bones"].end(); ++it) {
		std::string bone_name = it.key();

		mat_4d mat = parse_js_matrix(it.value()["matrix_local"]);

		AnimatedObjBone * bone;
		if (it.value()["parent"].is_null()) {
			bone = new AnimatedObjBone(bone_name, mat);
		}
		else {
			std::string parent_name = it.value()["parent"];
			bone = new AnimatedObjBone(bone_name, mat, parent_name);
		}
		_bones.push_back(bone);

		if (!it.value()["weights"].is_null()) {
			_mode = ANIMATED_MODEL_WEIGHT;
			for (json::iterator it_weight = it.value()["weights"].begin(); it_weight != it.value()["weights"].end(); ++it_weight) {
				std::string obj_name = it_weight.key();

				for (json::iterator it_weight_2 = it_weight.value().begin(); it_weight_2 != it_weight.value().end(); ++it_weight_2) {
					uint vertex_idx = std::stoi(it_weight_2.key());
					number weight = it_weight_2.value();

					bool too_many_bones = true;
					for (uint i=0; i<4; ++i) {
						uint idx = 4 * vertex_idx + i;
						AnimatedObjObject * object = get_animated_object(obj_name);
						
						if (object->_bones[idx] == NULL) {
							object->_bones[idx] = bone;
							object->_weights[idx] = weight;
							too_many_bones = false;
							break;
						}
					}
					if (too_many_bones) {
						std::cerr << "trop de bones\n";
					}
				}
			}
		}
		else {
			_mode = ANIMATED_MODEL_RIGID;
		}
	}

	for (auto & b : _bones) {
		if (b->_parent_name != "") {
			b->_parent = get_bone(b->_parent_name);
		}
	}

	if (_mode == ANIMATED_MODEL_RIGID) {
		for (auto & o : js["objects"]) {
			std::string object_name = o["name"];
			std::string bone_name = o["bone"];
			AnimatedObjObject * object = get_animated_object(object_name);
			AnimatedObjBone * bone = get_bone(bone_name);
			object->_parent_bone = bone;
		}
	}

	for (json::iterator it_action = js["actions"].begin(); it_action != js["actions"].end(); ++it_action) {
		std::string action_name = it_action.key();
		AnimatedObjAction * action = new AnimatedObjAction(action_name);
		for (auto & f : it_action.value()) {
			AnimatedObjFrame * frame = new AnimatedObjFrame();
			for (json::iterator it_f = f.begin(); it_f != f.end(); ++it_f) {
				std::string bone_name = it_f.key();
				mat_4d mat = parse_js_matrix(it_f.value()["matrix_basis"]);
				AnimatedObjBone * bone = get_bone(bone_name);
				frame->_transforms.push_back(new AnimatedObjTransform(bone, mat));
			}
			action->_frames.push_back(frame);
		}
		_actions.push_back(action);
	}

	update_matrices();
	update_frames();
}


AnimatedObjModel::~AnimatedObjModel() {
	for (auto & b : _bones) {
		delete b;
	}
	_bones.clear();
	for (auto & a : _actions) {
		delete a;
	}
	_actions.clear();
	delete _matrices;
	delete _obj_data;
}


void AnimatedObjModel::update_matrices() {
	_n_matrices = 0;
	for (auto & action : _actions) {
		for (auto & frame : action->_frames) {
			for (auto & tr : frame->_transforms) {
				_n_matrices++;
			}
		}
	}
	_matrices = new float[_n_matrices * 16];

	uint compt = 0;
	for (auto & action : _actions) {
		for (auto & frame : action->_frames) {
			for (auto & transform : frame->_transforms) {
				AnimatedObjBone * bone = transform->_bone;

				mat_4d m = bone->_mat_local * transform->_mat * glm::inverse(bone->_mat_local);

				AnimatedObjBone * parent = bone->_parent; 
				while (parent != NULL) {
					m = parent->_mat_local *
						frame->get_transform(parent)->_mat *
						glm::inverse(parent->_mat_local) *
						m;
					parent = parent->_parent;
				}

				const number * mat_data = (const number *) glm::value_ptr(m);
				for (uint i=0; i<16; ++i) {
					_matrices[16 * compt + i] = float(mat_data[i]);
				}
				transform->_idx = compt;
				compt++;
			}
		}
	}
}


void AnimatedObjModel::update_frames() {
	uint n_attrs_per_pts;
	if (_mode == ANIMATED_MODEL_RIGID) {
		n_attrs_per_pts = 1;
	}
	else if (_mode == ANIMATED_MODEL_WEIGHT) {
		n_attrs_per_pts = 4 + 4;
	}

	for (auto & action : _actions) {
		for (auto & frame : action->_frames) {
			frame->_data = new float[_obj_data->_n_pts * n_attrs_per_pts];
			float * ptr = frame->_data;

			if (_mode == ANIMATED_MODEL_RIGID) {
				for (auto & o : _objects) {
					ObjObject * object = o->_static_object;
					AnimatedObjBone * bone = o->_parent_bone;
					AnimatedObjTransform * transform = frame->get_transform(bone);

					for (auto & face : object->_faces) {
						for (uint idx_pt=0; idx_pt<3; ++idx_pt) {
							ptr[0] = float(transform->_idx);
							ptr++;
						}
					}
				}
			}
			else if (_mode == ANIMATED_MODEL_WEIGHT) {
				for (auto & o : _objects) {
					ObjObject * object = o->_static_object;
					
					for (auto & face : object->_faces) {
						for (uint idx_pt=0; idx_pt<3; ++idx_pt) {
							for (int i=0; i<4; ++i) {
								AnimatedObjBone * bone = o->_bones[4 * face->_vertices_idx[idx_pt] + i];
								if (bone != NULL) {
									AnimatedObjTransform * transform = frame->get_transform(bone);
									ptr[0] = float(transform->_idx);
									ptr++;
								}
								else {
									ptr[0] = -1.0f;
									ptr++;
								}
							}
							
							for (int i=0; i<4; ++i) {
								ptr[0] = o->_weights[4 * face->_vertices_idx[idx_pt] + i];
								ptr++;
							}
						}
					}
				}
			}
		}
	}
}


AnimatedObjObject * AnimatedObjModel::get_animated_object(std::string obj_name) {
	for (auto & obj : _objects) {
		if (obj->_static_object->_name == obj_name) {
			return obj;
		}
	}
	return NULL;
}


AnimatedObjAction * AnimatedObjModel::get_action(std::string action_name) {
	for (auto & action : _actions) {
		if (action->_name == action_name) {
			return action;
		}
	}
	return NULL;
}


AnimatedObjBone * AnimatedObjModel::get_bone(std::string bone_name) {
	for (auto & bone : _bones) {
		if (bone->_name == bone_name) {
			return bone;
		}
	}
	return NULL;
}


std::ostream & operator << (std::ostream & os, AnimatedObjModel & model) {
	os << "bones =\n";
	for (auto & bone : model._bones) {
		std::cout << *bone << "\n";
	}
	os << "\nactions =\n";
	for (auto & action : model._actions) {
		std::cout << *action << "\n";
	}
	return os;
}


// ------------------------------------------------
AnimatedObjInstance::AnimatedObjInstance() {

}


AnimatedObjInstance::AnimatedObjInstance(AnimatedObjModel * model, pt_3d pos, time_point t, quat q) :
	InstancePosRot(pos, q, pt_3d(1.0)),
	_model(model), _last_anim_t(t), _current_frame_idx(0)
{
	_current_action = model->_actions[0];
	_current_frame = _current_action->_frames[_current_frame_idx];
}


AnimatedObjInstance::~AnimatedObjInstance() {

}


void AnimatedObjInstance::anim(time_point t) {
	auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_anim_t).count();
	if (dt > 30) {
		_last_anim_t = t;
		_current_frame_idx++;
		if (_current_frame_idx >= _current_action->_frames.size()) {
			_current_frame_idx = 0;
		}
		_current_frame = _current_action->_frames[_current_frame_idx];
	}
}


// ------------------------------------------------
TestObjAnim::TestObjAnim() {

}


TestObjAnim::TestObjAnim(GLDrawManager * gl_draw_manager, ViewSystem * view_system, time_point t) :
	_gl_draw_manager(gl_draw_manager), _view_system(view_system), _paused(false)
{
	std::vector<std::string> model_names {"test", "test2", "test3"};
	
	for (auto & model_name : model_names) {
		AnimatedObjModel * model = new AnimatedObjModel("../data/" + model_name + ".json");
		_gl_draw_manager->add_texture_buffer(model_name, "anim_buffer", GL_R32F, 0);
		_gl_draw_manager->set_texture_buffer_data(model_name, "anim_buffer", model->_matrices, model->_n_matrices * 16 * sizeof(float));

		GLDrawContext * context = _gl_draw_manager->get_context(model_name);
		context->_n_pts = model->_obj_data->_n_pts;
		context->set_data(model->_obj_data->_data, 0);

		_models.push_back(model);
	}
	
	/*_instances.push_back(new AnimatedObjInstance(_models["test"], pt_3d(0.0, 0.0, 0.0), t));
	_instances.push_back(new AnimatedObjInstance(_models["test"], pt_3d(5.0, 0.0, 0.0), t));
	_instances.push_back(new AnimatedObjInstance(_models["test2"], pt_3d(0.0, 0.0, 5.0), t));
	_instances.push_back(new AnimatedObjInstance(_models["test3"], pt_3d(-5.0, -5.0, 0.0), t));*/

	//_instances[0]->_current_action = "walk";


	for (uint i=0; i<1000; ++i) {
		//int j = rand_int(0, model_names.size() - 1);
		//std::string model_name = model_names[j];
		std::string model_name = "test3";
		//std::string model_name = "test";
		_instances.push_back(new AnimatedObjInstance(get_model(model_name), rand_pt_3d(pt_3d(-20.0), pt_3d(20.0)), t, rand_quat()));
	}

	for (auto & model : _models) {
		update_n_pts(model);
		update_static_buffer(model);
		update_animation_buffer(model);
		update_model2world_buffer(model);
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
		//update_n_pts(model);
		//update_static_buffer(model);
		update_animation_buffer(model);
		update_model2world_buffer(model);
	}
}


void TestObjAnim::update_n_pts(AnimatedObjModel * model) {
	GLDrawContext * context = _gl_draw_manager->get_context(model->_name);
	context->_n_pts = 0;
	for (auto & instance : _instances) {
		if (instance->_model == model) {
			context->_n_pts += model->_obj_data->_n_pts;
		}
	}
}


void TestObjAnim::update_static_buffer(AnimatedObjModel * model) {
	GLDrawContext * context = _gl_draw_manager->get_context(model->_name);

	uint n_attrs_per_pts = model->_obj_data->_n_attrs_per_pts;
	uint n_floats_per_instance = model->_obj_data->_n_pts * n_attrs_per_pts;

	float * data = new float[context->_n_pts * n_attrs_per_pts];

	float * ptr = data;
	
	for (auto & instance : _instances) {
		if (instance->_model != model) {
			continue;
		}

		std::memcpy(ptr, model->_obj_data->_data, n_floats_per_instance * sizeof(float));
		ptr += n_floats_per_instance;
	}

	context->set_data(data, 0);
	delete[] data;
}


void TestObjAnim::update_animation_buffer(AnimatedObjModel * model) {
	GLDrawContext * context = _gl_draw_manager->get_context(model->_name);

	uint n_attrs_per_pts;
	if (model->_mode == ANIMATED_MODEL_RIGID) {
		n_attrs_per_pts = 1;
	}
	else if (model->_mode == ANIMATED_MODEL_WEIGHT) {
		n_attrs_per_pts = 4 + 4;
	}
	uint n_floats_per_instance = model->_obj_data->_n_pts * n_attrs_per_pts;

	float * data = new float[context->_n_pts * n_attrs_per_pts];

	float * ptr = data;
	
	for (auto & instance : _instances) {
		if (instance->_model != model) {
			continue;
		}
		
		AnimatedObjFrame * frame = instance->_current_frame;

		std::memcpy(ptr, frame->_data, n_floats_per_instance * sizeof(float));
		ptr += n_floats_per_instance;
	}

	context->set_data(data, 1);
	delete[] data;
}


void TestObjAnim::update_model2world_buffer(AnimatedObjModel * model) {
	GLDrawContext * context = _gl_draw_manager->get_context(model->_name);

	uint n_attrs_per_pts = 16;

	float * data = new float[context->_n_pts * n_attrs_per_pts];

	float * ptr = data;
	
	for (auto & instance : _instances) {
		if (instance->_model != model) {
			continue;
		}

		for (uint i=0; i<model->_obj_data->_n_pts; ++i) {
			const float * instance_mat = glm::value_ptr(glm::mat4(instance->_model2world));
			std::memcpy(ptr, instance_mat, 16 * sizeof(float));
			ptr += 16;
		}
	}

	context->set_data(data, 2);
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
		//context->set_uniform("anim_matrices[0]", &model.second->_matrices[0], N_MAX_MATRICES);
		
		context->draw();
		context->deactivate();
	}
}


bool TestObjAnim::key_down(InputState * input_state, SDL_Keycode key, time_point t) {
	/*if (key == SDLK_a) {
		if (_animated_obj->_current_action == "walk") {
			_animated_obj->_current_action = "shake";
			_animated_obj->_current_frame = 0;
		}
		else if (_animated_obj->_current_action == "shake") {
			_animated_obj->_current_action = "walk";
			_animated_obj->_current_frame = 0;
		}
		return true;
	}*/

	if (key == SDLK_SPACE) {
		_paused = !_paused;

		GLDrawContext * context = _gl_draw_manager->get_context("test3");
		context->show_data();
	}
	return false;
}
