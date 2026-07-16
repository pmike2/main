#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "animated_obj.h"


// cf le script d'export de Blender, on a en entrée une liste de rows
// et on veut être en column-major comme glm et opengl par défaut
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
	_bone(bone), _mat_basis(mat)
{

}

AnimatedObjTransform::~AnimatedObjTransform() {

}


std::ostream & operator << (std::ostream & os, AnimatedObjTransform & transform) {
	os << "bone = " << transform._bone->_name;
	os << " ; mat_basis = " << glm::to_string(transform._mat_basis);
	os << " ; mat_final = " << glm::to_string(transform._mat_final);
	return os;
}


// ------------------------------------------------
AnimatedObjFrame::AnimatedObjFrame() {

}


AnimatedObjFrame::~AnimatedObjFrame() {
	
}


AnimatedObjTransform * AnimatedObjFrame::get_transform(AnimatedObjBone * bone) {
	for (auto & tr : _transforms) {
		if (tr->_bone == bone) {
			return tr;
		}
	}
	std::cerr << "AnimatedObjFrame::get_transform : bone " << bone->_name << " introuvable\n";
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

}


AnimatedObjObject::~AnimatedObjObject() {

}


void AnimatedObjObject::sort_per_weight() {
	for (auto & it : _weights_per_vertex) {
		if (it.second.size() > 4) {
			std::cerr << "objet " << _static_object->_name << " : trop de bones (" << it.second.size() << " > 4), on ne garde que les 4 plus influents\n";
			std::sort(it.second.begin(), it.second.end(), 
				[](std::pair<AnimatedObjBone *, number> a, std::pair<AnimatedObjBone *, number> b) {return a.second > b.second;});
			it.second.erase(it.second.begin() + 4, it.second.begin() + it.second.size());

			// on ajuste le dernier poids pour que la somme fasse 4; est-ce nécessaire ?
			it.second[3].second = 1.0 - (it.second[0].second + it.second[1].second + it.second[2].second);
		}
	}
}


std::ostream & operator << (std::ostream & os, AnimatedObjObject & obj) {
	os << "static_object = " << obj._static_object->_name;
	return os;
}


// ------------------------------------------------
AnimatedObjModel::AnimatedObjModel() {

}


AnimatedObjModel::AnimatedObjModel(std::string json_path) {
	std::filesystem::path js_path = json_path;
	std::string obj_filename = js_path.stem().string() + ".obj";
	std::filesystem::path obj_path = js_path.parent_path() / obj_filename;

	_name = js_path.stem().string();
	
	// set static data
	_obj_data = new ObjData(obj_path.string());
	_obj_data->_use_ambient = false;
	_obj_data->_use_diffuse = true;
	_obj_data->_use_specular = false;
	_obj_data->_use_shininess = false;
	_obj_data->_use_opacity = false;
	_obj_data->update_data();

	// ajouts des surcouches AnimatedObjObject
	for (auto & object : _obj_data->_objects) {
		_objects.push_back(new AnimatedObjObject(object));
	}

	// parcours json
	std::ifstream ifs(json_path);
	json js = json::parse(ifs);
	ifs.close();

	// fps
	if (js["fps"].is_null()) {
		_fps = 24.0; // valeur par défaut de Blender
	}
	else {
		_fps = js["fps"];
	}
	_n_ms_per_frame = uint(1000.0 / _fps);

	// armature ; vaut mat4(1.0) si l'origine de l'armature est en (0,0,0)
	_mat_armature = parse_js_matrix(js["armature"]);

	// bones
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

		// cas ANIMATED_MODEL_WEIGHT -------------------------------------------------
		if (!it.value()["weights"].is_null()) {
			_mode = ANIMATED_MODEL_WEIGHT;
			for (json::iterator it_weight = it.value()["weights"].begin(); it_weight != it.value()["weights"].end(); ++it_weight) {
				std::string obj_name = it_weight.key();
				AnimatedObjObject * object = get_animated_object(obj_name);

				for (json::iterator it_weight_2 = it_weight.value().begin(); it_weight_2 != it_weight.value().end(); ++it_weight_2) {
					uint vertex_idx = std::stoi(it_weight_2.key());
					number weight = it_weight_2.value();

					if (object->_weights_per_vertex.count(vertex_idx) == 0) {
						object->_weights_per_vertex[vertex_idx] = std::vector<std::pair<AnimatedObjBone *, number> >{};
					}
					object->_weights_per_vertex[vertex_idx].push_back(std::make_pair(bone, weight));
				}
			}
		}
		// cas ANIMATED_MODEL_RIGID -------------------------------------------------
		else {
			_mode = ANIMATED_MODEL_RIGID;
		}
	}

	// set des parents des Bones à partir de leurs noms
	for (auto & b : _bones) {
		if (b->_parent_name != "") {
			b->_parent = get_bone(b->_parent_name);
		}
	}

	// dans le cas ANIMATED_MODEL_RIGID on associe un unique os à chaque objet
	if (_mode == ANIMATED_MODEL_RIGID) {
		for (auto & o : js["objects"]) {
			std::string object_name = o["name"];
			std::string bone_name = o["bone"];
			AnimatedObjObject * object = get_animated_object(object_name);
			AnimatedObjBone * bone = get_bone(bone_name);
			object->_parent_bone = bone;
		}
	}

	// actions
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

	// tri des poids les plus influents (si > 4)
	for (auto & object : _objects) {
		object->sort_per_weight();
	}

	compute_transform_final_matrix();
	compute_buffer_texture_data();
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
	delete _obj_data;
}


void AnimatedObjModel::compute_transform_final_matrix() {
	for (auto & action : _actions) {
		for (auto & frame : action->_frames) {
			for (auto & transform : frame->_transforms) {
				AnimatedObjBone * bone = transform->_bone;
				
				// https://blender.stackexchange.com/questions/44637/how-can-i-manually-calculate-bpy-types-posebone-matrix-using-blenders-python-ap/121495
				// il faut se mettre dans le repère de l'armature en faisant inverse(bone->_mat_local)
				// puis appliquer la transfo
				// puis revenir dans le repère du Bone avec bone->_mat_local
				transform->_mat_final = bone->_mat_local * transform->_mat_basis * glm::inverse(bone->_mat_local);

				// et composer avec toutes les transfos des parents
				AnimatedObjBone * parent = bone->_parent; 
				while (parent != NULL) {
					transform->_mat_final = 
						parent->_mat_local *
						frame->get_transform(parent)->_mat_basis *
						glm::inverse(parent->_mat_local) *
						transform->_mat_final
					;
					
						parent = parent->_parent;
				}

				// enfin on applique le changement de système lié à la matrice de l'armature
				// (qui vaut mat4(1.0) si origine de l'armature en (0,0,0))
				transform->_mat_final = _mat_armature * transform->_mat_final * glm::inverse(_mat_armature);
			}
		}
	}
}


// le gros morceau
// on remplit un tableau qui sera mis dans un buffer texture avec toutes les matrices finales des transfos
void AnimatedObjModel::compute_buffer_texture_data() {
	_buffer_texture_data_size = 0;
	for (uint idx_action=0; idx_action<_actions.size(); ++idx_action) {
		AnimatedObjAction * action = _actions[idx_action];
		for (uint idx_frame=0; idx_frame<action->_frames.size(); ++idx_frame) {
			_buffer_texture_data_size += _obj_data->_n_pts * 16;
		}
	}

	_buffer_texture_data = new float[_buffer_texture_data_size];
	for (uint i=0; i<_buffer_texture_data_size; ++i) {
		_buffer_texture_data[i] = 0.0;
	}

	for (uint i=0; i<IDX_TEXTURE_DATA_SIZE*IDX_TEXTURE_DATA_SIZE; ++i) {
		_idx_texture_data[i] = 0;
	}

	// pour chaque action, pour chaque frame de l'action, pour chaque vertex, on stocke la matrice qui sera appliquée
	// au vertex dans le shader
	int idx_buffer_texture = 0;
	for (uint idx_action=0; idx_action<_actions.size(); ++idx_action) {
		AnimatedObjAction * action = _actions[idx_action];
		for (uint idx_frame=0; idx_frame<action->_frames.size(); ++idx_frame) {
			AnimatedObjFrame * frame = action->_frames[idx_frame];

			_idx_texture_data[idx_action * IDX_TEXTURE_DATA_SIZE + idx_frame] = float(idx_buffer_texture);
			
			// dans le cas ANIMATED_MODEL_RIGID tous les vertices d'un objet sont affectés par le même bone : o->_parent_bone
			if (_mode == ANIMATED_MODEL_RIGID) {
				for (auto & o : _objects) {
					ObjObject * object = o->_static_object;
					AnimatedObjBone * bone = o->_parent_bone;
					AnimatedObjTransform * transform = frame->get_transform(bone);
					const float * mat_data = glm::value_ptr(glm::mat4(transform->_mat_final));

					for (auto & face : object->_faces) {
						for (uint idx_pt=0; idx_pt<3; ++idx_pt) {
							float * ptr = _buffer_texture_data + idx_buffer_texture;
							std::memcpy(ptr, mat_data, 16 * sizeof(float));

							idx_buffer_texture += 16;
						}
					}
				}
			}

			// dans le cas ANIMATED_MODEL_WEIGHT on calcule la somme pondérée des matrices (max 4) associées au vertex
			else if (_mode == ANIMATED_MODEL_WEIGHT) {
				for (auto & o : _objects) {
					ObjObject * object = o->_static_object;
					
					for (auto & face : object->_faces) {
						for (uint idx_pt=0; idx_pt<3; ++idx_pt) {
							mat_4d m;
							uint n_bones = o->_weights_per_vertex[face->_vertices_idx[idx_pt]].size();
							if (n_bones > 0) {
								m = mat_4d(0.0);
								for (int i=0; i<4; ++i) {
									if (i > n_bones - 1) {
										break;
									}
									AnimatedObjBone * bone = o->_weights_per_vertex[face->_vertices_idx[idx_pt]][i].first;
									number weight = o->_weights_per_vertex[face->_vertices_idx[idx_pt]][i].second;
									AnimatedObjTransform * transform = frame->get_transform(bone);
									m += weight * transform->_mat_final;
								}
							}
							else {
								m = mat_4d(1.0);
							}

							const float * mat_data = glm::value_ptr(glm::mat4(m));
							float * ptr = _buffer_texture_data + idx_buffer_texture;
							std::memcpy(ptr, mat_data, 16 * sizeof(float));

							idx_buffer_texture += 16;
						}
					}
				}
			}
		}
	}

	/*for (uint i=0; i<IDX_TEXTURE_DATA_SIZE*IDX_TEXTURE_DATA_SIZE; ++i) {
		if (i % IDX_TEXTURE_DATA_SIZE == 0) {
			std::cout << "\n";
		}
		std::cout << _idx_texture_data[i] << " ; ";
	}
	std::cout << "\n";*/

	/*for (uint i=0; i<_buffer_texture_data_size; ++i) {
		if (i % 16 == 0) {
			std::cout << "\n";
		}
		std::cout << _buffer_texture_data[i] << " ; ";
	}
	std::cout << "\n";*/
}


AnimatedObjObject * AnimatedObjModel::get_animated_object(std::string obj_name) {
	for (auto & obj : _objects) {
		if (obj->_static_object->_name == obj_name) {
			return obj;
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
	os << "name = " << model._name;
	os << " ; buffer_texture_data_size = " << model._buffer_texture_data_size;
	if (model._mode == ANIMATED_MODEL_RIGID) {
		os << " ; mode = ANIMATED_MODEL_RIGID\n";
	}
	else if (model._mode == ANIMATED_MODEL_WEIGHT) {
		os << " ; mode = ANIMATED_MODEL_WEIGHT\n";
	}

	os << "bones =\n";
	for (auto & bone : model._bones) {
		std::cout << *bone << "\n";
	}
	
	/*os << "\nactions =\n";
	for (auto & action : model._actions) {
		std::cout << *action << "\n";
	}*/
	
	os << "\nobjects =\n";
	for (auto & obj : model._objects) {
		std::cout << *obj << "\n";
	}
	
	return os;
}


// ------------------------------------------------
AnimatedObjInstance::AnimatedObjInstance() {

}


AnimatedObjInstance::AnimatedObjInstance(AnimatedObjModel * model, pt_3d pos, time_point t, quat q, std::string action_name) :
	InstancePosRot(pos, q, pt_3d(1.0)),
	_model(model), _last_anim_t(t), _idx_frame(0), _idx_action(0)
{
	if (action_name != "") {
		set_action(action_name);
	}
}


AnimatedObjInstance::~AnimatedObjInstance() {

}


void AnimatedObjInstance::anim(time_point t) {
	auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_anim_t).count();
	if (dt > _model->_n_ms_per_frame) {
		_last_anim_t = t;
		_idx_frame++;
		if (_idx_frame >= _model->_actions[_idx_action]->_frames.size()) {
			_idx_frame = 0;
		}
	}
}


void AnimatedObjInstance::set_action(std::string action_name) {
	for (uint idx_action=0; idx_action<_model->_actions.size(); ++idx_action) {
		if (_model->_actions[idx_action]->_name == action_name) {
			_idx_action = idx_action;
			_idx_frame = 0;
			return;
		}
	}
	std::cerr << action_name << " : Action non trouvée\n";
}


std::string AnimatedObjInstance::get_action() {
	return _model->_actions[_idx_action]->_name;
}

