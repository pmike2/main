
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


AnimatedObjTransform::~AnimatedObjTransform() {

}


std::ostream & operator << (std::ostream & os, AnimatedObjTransform & transform) {
	os << "idx = " << transform._idx;
	os << " ; mat = " << glm::to_string(transform._mat);
	return os;
}


// ------------------------------------------------
AnimatedObjFrame::AnimatedObjFrame() {
	//_transforms.set_empty_key(NULL);
}


AnimatedObjFrame::~AnimatedObjFrame() {

}


std::ostream & operator << (std::ostream & os, AnimatedObjFrame & frame) {
	os << "transforms = ";
	for (auto & transform : frame._transforms) {
		os << transform.first->_name << " -> " << *transform.second << "\n";
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
	/*_bones.set_empty_key("");
	_actions.set_empty_key("");
	_objects.set_empty_key("");*/

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
		_objects[object->_name] = new AnimatedObjObject(object);
	}

	std::ifstream ifs(json_path);
	json js = json::parse(ifs);
	ifs.close();

	for (json::iterator it = js["bones"].begin(); it != js["bones"].end(); ++it) {
		std::string bone_name = it.key();

		mat_4d mat = parse_js_matrix(it.value()["matrix_local"]);

		if (it.value()["parent"].is_null()) {
			_bones[bone_name] = new AnimatedObjBone(bone_name, mat);
		}
		else {
			std::string parent_name = it.value()["parent"];
			_bones[bone_name] = new AnimatedObjBone(bone_name, mat, parent_name);
		}

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
						if (_objects[obj_name]->_bones[idx] == NULL) {
							_objects[obj_name]->_bones[idx] = _bones[bone_name];
							_objects[obj_name]->_weights[idx] = weight;
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
		if (b.second->_parent_name != "") {
			b.second->_parent = _bones[b.second->_parent_name];
		}
	}

	if (_mode == ANIMATED_MODEL_RIGID) {
		for (auto & object : js["objects"]) {
			std::string object_name = object["name"];
			std::string bone_name = object["bone"];
			_objects[object_name]->_parent_bone = _bones[bone_name];
		}
	}

	for (json::iterator it_action = js["actions"].begin(); it_action != js["actions"].end(); ++it_action) {
		std::string action_name = it_action.key();
		_actions[action_name] = new AnimatedObjAction(action_name);
		for (auto & f : it_action.value()) {
			AnimatedObjFrame * frame = new AnimatedObjFrame();
			for (json::iterator it_f = f.begin(); it_f != f.end(); ++it_f) {
				std::string bone_name = it_f.key();

				mat_4d mat = parse_js_matrix(it_f.value()["matrix_basis"]);

				AnimatedObjTransform * transform = new AnimatedObjTransform();
				AnimatedObjBone * bone = _bones[bone_name];
				transform->_mat = mat;
				frame->_transforms[bone] = transform;
			}
			_actions[action_name]->_frames.push_back(frame);
		}
	}

	update_matrices();
}


AnimatedObjModel::~AnimatedObjModel() {
	for (auto & b : _bones) {
		delete b.second;
	}
	_bones.clear();
	for (auto & a : _actions) {
		delete a.second;
	}
	_actions.clear();
	delete _matrices;
	delete _obj_data;
}


void AnimatedObjModel::update_matrices() {
	_n_matrices = 0;
	for (auto & ac : _actions) {
		AnimatedObjAction * action = ac.second;
		for (auto & frame : action->_frames) {
			for (auto & tr : frame->_transforms) {
				_n_matrices++;
			}
		}
	}
	_matrices = new float[_n_matrices * 16];
	//std::cout << n_matrices << "\n";

	uint compt = 0;
	for (auto & ac : _actions) {
		AnimatedObjAction * action = ac.second;
		for (auto & frame : action->_frames) {
			for (auto & tr : frame->_transforms) {
				AnimatedObjBone * bone = tr.first;
				AnimatedObjTransform * transform = tr.second;

				mat_4d m = bone->_mat_local * transform->_mat * glm::inverse(bone->_mat_local);

				AnimatedObjBone * parent = bone->_parent; 
				while (parent != NULL) {
					m = parent->_mat_local *
						frame->_transforms[parent]->_mat *
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


std::ostream & operator << (std::ostream & os, AnimatedObjModel & model) {
	os << "bones =\n";
	for (auto & bone : model._bones) {
		std::cout << *bone.second << "\n";
	}
	os << "\nactions =\n";
	for (auto & action : model._actions) {
		std::cout << *action.second << "\n";
	}
	return os;
}


// ------------------------------------------------
AnimatedObjInstance::AnimatedObjInstance() {

}


AnimatedObjInstance::AnimatedObjInstance(AnimatedObjModel * model, pt_3d pos, time_point t, quat q) :
	InstancePosRot(pos, q, pt_3d(1.0)),
	_model(model), _last_anim_t(t), _current_action(model->_actions.begin()->first), _current_frame(0)
{

}


AnimatedObjInstance::~AnimatedObjInstance() {

}


void AnimatedObjInstance::anim(time_point t) {
	auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_anim_t).count();
	if (dt > 30) {
		_last_anim_t = t;
		_current_frame++;
		if (_current_frame >= _model->_actions[_current_action]->_frames.size()) {
			_current_frame = 0;
		}
	}
}


// ------------------------------------------------
TestObjAnim::TestObjAnim() {

}


TestObjAnim::TestObjAnim(GLDrawManager * gl_draw_manager, ViewSystem * view_system, time_point t) :
	_gl_draw_manager(gl_draw_manager), _view_system(view_system), _paused(false)
{
	//_models.set_empty_key("");

	std::vector<std::string> model_names {"test", "test2", "test3"};
	
	for (auto & model_name : model_names) {
		_models[model_name] = new AnimatedObjModel("../data/" + model_name + ".json");
		//std::cout << *_models[model_name] << "\n";
		_gl_draw_manager->add_texture_buffer(model_name, "anim_buffer", GL_R32F, 0);
		_gl_draw_manager->set_texture_buffer_data(model_name, "anim_buffer", _models[model_name]->_matrices, _models[model_name]->_n_matrices * 16 * sizeof(float));

		GLDrawContext * context = _gl_draw_manager->get_context(model_name);
		context->_n_pts = _models[model_name]->_obj_data->_n_pts;
		context->set_data(_models[model_name]->_obj_data->_data, 0);
	}
	
	/*_instances.push_back(new AnimatedObjInstance(_models["test"], pt_3d(0.0, 0.0, 0.0), t));
	_instances.push_back(new AnimatedObjInstance(_models["test"], pt_3d(5.0, 0.0, 0.0), t));
	_instances.push_back(new AnimatedObjInstance(_models["test2"], pt_3d(0.0, 0.0, 5.0), t));
	_instances.push_back(new AnimatedObjInstance(_models["test3"], pt_3d(-5.0, -5.0, 0.0), t));*/

	//_instances[0]->_current_action = "walk";


	for (uint i=0; i<200; ++i) {
		//int j = rand_int(0, model_names.size() - 1);
		//std::string model_name = model_names[j];
		std::string model_name = "test3";
		//std::string model_name = "test";
		_instances.push_back(new AnimatedObjInstance(_models[model_name], rand_pt_3d(pt_3d(-20.0), pt_3d(20.0)), t, rand_quat()));
	}

	for (auto & model : _models) {
		update(model.second);
	}
}


TestObjAnim::~TestObjAnim() {
	for (auto & instance : _instances) {
		delete instance;
	}
	_instances.clear();
	for (auto & model : _models) {
		delete model.second;
	}
	_models.clear();
}


void TestObjAnim::anim(time_point t) {
	if (_paused) {
		return;
	}

	for (auto & instance : _instances) {
		instance->anim(t);
	}

	for (auto & model : _models) {
		update(model.second);
	}
}


void TestObjAnim::update(AnimatedObjModel * model) {
	GLDrawContext * context = _gl_draw_manager->get_context(model->_name);
	context->_n_pts = 0;
	for (auto & instance : _instances) {
		if (instance->_model == model) {
			context->_n_pts += model->_obj_data->_n_pts;
		}
	}

	uint n_attrs_per_pts = context->_buffers[0]->_n_attrs_per_pts;

	float * data = new float[context->_n_pts * n_attrs_per_pts];

	float * ptr = data;
	
	for (auto & instance : _instances) {
		if (instance->_model != model) {
			continue;
		}

		AnimatedObjFrame * frame = model->_actions[instance->_current_action]->_frames[instance->_current_frame];

		// ------------------------------------------------------------------------------
		if (model->_mode == ANIMATED_MODEL_RIGID) {
			for (auto & o : model->_objects) {
			
				ObjObject * object = o.second->_static_object;
				AnimatedObjBone * bone = o.second->_parent_bone;
				AnimatedObjTransform * transform = frame->_transforms[bone];

				for (auto & face : object->_faces) {
					for (uint idx_pt=0; idx_pt<3; ++idx_pt) {
						pt_3d pt = object->_vertices[face->_vertices_idx[idx_pt]];
						pt_3d normal;
						if (face->_normal_active) {
							normal = object->_normals[face->_normals_idx[idx_pt]];
						}
						else {
							normal = object->compute_normal(face);
						}

						ptr[0] = float(pt.x);
						ptr[1] = float(pt.y);
						ptr[2] = float(pt.z);
						ptr[3] = float(normal.x);
						ptr[4] = float(normal.y);
						ptr[5] = float(normal.z);
						ptr += 6;

						if (model->_obj_data->_use_ambient) {
							ptr[0] = float(face->_material->_ambient.r);
							ptr[1] = float(face->_material->_ambient.g);
							ptr[2] = float(face->_material->_ambient.b);
							ptr += 3;
						}
						if (model->_obj_data->_use_diffuse) {
							ptr[0] = float(face->_material->_diffuse.r);
							ptr[1] = float(face->_material->_diffuse.g);
							ptr[2] = float(face->_material->_diffuse.b);
							ptr += 3;
						}
						if (model->_obj_data->_use_specular) {
							ptr[0] = float(face->_material->_specular.r);
							ptr[1] = float(face->_material->_specular.g);
							ptr[2] = float(face->_material->_specular.b);
							ptr += 3;
						}
						if (model->_obj_data->_use_shininess) {
							ptr[0] = float(face->_material->_shininess);
							ptr++;
						}
						if (model->_obj_data->_use_opacity) {
							ptr[0] = float(face->_material->_opacity);
							ptr++;
						}

						ptr[0] = float(transform->_idx);
						ptr++;

						const float * instance_mat = glm::value_ptr(glm::mat4(instance->_model2world));
						std::memcpy(ptr, instance_mat, 16 * sizeof(float));
						ptr += 16;
					}
				}
			}
		}

		// ------------------------------------------------------------
		else if (model->_mode == ANIMATED_MODEL_WEIGHT) {
			for (auto & o : model->_objects) {
				
				ObjObject * object = o.second->_static_object;
				
				for (auto & face : object->_faces) {
					for (uint idx_pt=0; idx_pt<3; ++idx_pt) {
						pt_3d pt = object->_vertices[face->_vertices_idx[idx_pt]];
						pt_3d normal;
						if (face->_normal_active) {
							normal = object->_normals[face->_normals_idx[idx_pt]];
						}
						else {
							normal = object->compute_normal(face);
						}

						ptr[0] = float(pt.x);
						ptr[1] = float(pt.y);
						ptr[2] = float(pt.z);
						ptr[3] = float(normal.x);
						ptr[4] = float(normal.y);
						ptr[5] = float(normal.z);
						ptr += 6;

						if (model->_obj_data->_use_ambient) {
							ptr[0] = float(face->_material->_ambient.r);
							ptr[1] = float(face->_material->_ambient.g);
							ptr[2] = float(face->_material->_ambient.b);
							ptr += 3;
						}
						if (model->_obj_data->_use_diffuse) {
							ptr[0] = float(face->_material->_diffuse.r);
							ptr[1] = float(face->_material->_diffuse.g);
							ptr[2] = float(face->_material->_diffuse.b);
							ptr += 3;
						}
						if (model->_obj_data->_use_specular) {
							ptr[0] = float(face->_material->_specular.r);
							ptr[1] = float(face->_material->_specular.g);
							ptr[2] = float(face->_material->_specular.b);
							ptr += 3;
						}
						if (model->_obj_data->_use_shininess) {
							ptr[0] = float(face->_material->_shininess);
							ptr++;
						}
						if (model->_obj_data->_use_opacity) {
							ptr[0] = float(face->_material->_opacity);
							ptr++;
						}

						for (int i=0; i<4; ++i) {
							AnimatedObjBone * bone = o.second->_bones[4 * face->_vertices_idx[idx_pt] + i];
							if (bone != NULL) {
								AnimatedObjTransform * transform = frame->_transforms[bone]; // performance hit
								ptr[0] = float(transform->_idx);
								//ptr[0] = 1.0f;
								ptr++;
							}
							else {
								ptr[0] = -1.0f;
								ptr++;
							}
						}
						
						for (int i=0; i<4; ++i) {
							ptr[0] = o.second->_weights[4 * face->_vertices_idx[idx_pt] + i];
							ptr++;
						}

						const float * instance_mat = glm::value_ptr(glm::mat4(instance->_model2world));
						std::memcpy(ptr, instance_mat, 16 * sizeof(float));
						ptr += 16;
					}
				}
			}
		}
	}

	context->set_data(data);
	delete[] data;
}


void TestObjAnim::draw() {
	for (auto & model : _models) {
		GLDrawContext * context = _gl_draw_manager->get_context(model.first);
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
