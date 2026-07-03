#include "json.hpp"

#include "test_obj_anim.h"


using json = nlohmann::json;


// ------------------------------------------------
AnimatedObjBone::AnimatedObjBone() {

}


AnimatedObjBone::AnimatedObjBone(std::string name) : _name(name) {

}


AnimatedObjBone::~AnimatedObjBone() {
	
}


// ------------------------------------------------
AnimatedObjTransform::AnimatedObjTransform() {

}


AnimatedObjTransform::~AnimatedObjTransform() {

}


// ------------------------------------------------
AnimatedObjFrame::AnimatedObjFrame() {

}


AnimatedObjFrame::~AnimatedObjFrame() {

}


// ------------------------------------------------
AnimatedObjAction::AnimatedObjAction() {

}


AnimatedObjAction::AnimatedObjAction(std::string name) : _name(name) {

}


AnimatedObjAction::~AnimatedObjAction() {

}


// ------------------------------------------------
AnimatedObj::AnimatedObj() {

}


AnimatedObj::AnimatedObj(std::string json_path) : _current_action(""), _current_frame(0) {
	std::filesystem::path js_path = json_path;
	std::string obj_filename = js_path.stem().string() + ".obj";
	std::filesystem::path obj_path = js_path.parent_path() / obj_filename;
	_obj_data = new ObjData(obj_path.string());

	std::ifstream ifs(json_path);
	json js = json::parse(ifs);
	ifs.close();

	for (json::iterator it = js["bones"].begin(); it != js["bones"].end(); ++it) {
		std::string bone_name = it.key();
		// = it.value();
		_bones[bone_name] = new AnimatedObjBone(bone_name);
	}

	for (auto & object : js["objects"]) {
		std::string object_name = object["name"];
		std::string bone_name = object["bone"];

		ObjObject * obj = _obj_data->get_object(object_name);
		_obj2bone[obj] = _bones[bone_name];
	}

	for (json::iterator it_action = js["actions"].begin(); it_action != js["actions"].end(); ++it_action) {
		std::string action_name = it_action.key();
		if (_current_action == "") {
			_current_action = action_name;
		}
		_actions[action_name] = new AnimatedObjAction(action_name);
		for (auto & f : it_action.value()) {
			AnimatedObjFrame * frame = new AnimatedObjFrame();
			for (json::iterator it_f = f.begin(); it_f != f.end(); ++it_f) {
				std::string bone_name = it_f.key();
				number mat_values[16];
				uint compt = 0;
				for (auto & f : it_f.value()) {
					number x = f;
					mat_values[compt++] = x;
				}

				AnimatedObjTransform * transform = new AnimatedObjTransform();
				transform->_mat = glm::make_mat4(mat_values);
				frame->_transforms[_bones[bone_name]] = transform;
			}
			_actions[action_name]->_frames.push_back(frame);
		}
	}
}


AnimatedObj::~AnimatedObj() {

}


void AnimatedObj::update_matrices() {
	uint compt = 0;
	for (auto & ac : _actions) {
		AnimatedObjAction * action = ac.second;
		for (auto & frame : action->_frames) {
			for (auto & tr : frame->_transforms) {
				AnimatedObjTransform * transform = tr.second;
				_matrices[compt] = transform->_mat;
				transform->_idx = compt;
				compt++;
				if (compt >= N_MAX_MATRICES) {
					std::cerr << "trop de matrices\n";
					return;
				}
			}
		}
	}
}


void AnimatedObj::update_data() {
	_obj_data->update_data();

	_n_attrs_per_pts = _obj_data->_n_attrs_per_pts + 1;
	_data = new float[_obj_data->_n_pts * _n_attrs_per_pts];

	float * ptr = _data;
	for (auto & object : _obj_data->_objects) {
		
		AnimatedObjBone * bone = _obj2bone[object];
		AnimatedObjTransform * transform = _actions[_current_action]->_frames[_current_frame]->_transforms[bone];
		
		std::cout << "ok\n";
		std::cout << object->_name << "\n";
		std::cout << transform->_idx << "\n";


		for (auto & face : object->_faces) {
			for (uint i=0; i<3; ++i) {
				pt_3d pt = object->_vertices[face->_vertices_idx[i]];
				pt_3d normal;
				if (face->_normal_active) {
					normal = object->_normals[face->_normals_idx[i]];
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
				ptr[6] = float(transform->_idx);
				
				ptr += 7;

				if (_obj_data->_use_ambient) {
					ptr[0] = float(face->_material->_ambient.r);
					ptr[1] = float(face->_material->_ambient.g);
					ptr[2] = float(face->_material->_ambient.b);
					ptr += 3;
				}
				if (_obj_data->_use_diffuse) {
					ptr[0] = float(face->_material->_diffuse.r);
					ptr[1] = float(face->_material->_diffuse.g);
					ptr[2] = float(face->_material->_diffuse.b);
					ptr += 3;
				}
				if (_obj_data->_use_specular) {
					ptr[0] = float(face->_material->_specular.r);
					ptr[1] = float(face->_material->_specular.g);
					ptr[2] = float(face->_material->_specular.b);
					ptr += 3;
				}
				if (_obj_data->_use_shininess) {
					ptr[0] = float(face->_material->_shininess);
					ptr++;
				}
				if (_obj_data->_use_opacity) {
					ptr[0] = float(face->_material->_opacity);
					ptr++;
				}
			}
		}
	}
}


void AnimatedObj::anim() {
	_current_frame++;
	if (_current_frame >= _actions[_current_action]->_frames.size()) {
		_current_frame = 0;
	}
}


// ------------------------------------------------
TestObjAnim::TestObjAnim() {

}


TestObjAnim::TestObjAnim(GLDrawManager * gl_draw_manager, ViewSystem * view_system) :
	_gl_draw_manager(gl_draw_manager), _view_system(view_system) 
{
	_animated_obj = new AnimatedObj("../data/test.json");

	_animated_obj->_obj_data = new ObjData("../data/test.obj");
	_animated_obj->_obj_data->_use_ambient = false;
	_animated_obj->_obj_data->_use_diffuse = true;
	_animated_obj->_obj_data->_use_specular = false;
	_animated_obj->_obj_data->_use_shininess = false;
	_animated_obj->_obj_data->_use_opacity = false;
	//_animated_obj->_obj_data->update_data();

	_animated_obj->update_data();
	_animated_obj->update_matrices();

	//std::cout << *_obj_data << "\n";

	update();
}


TestObjAnim::~TestObjAnim() {
	delete _animated_obj;
}


void TestObjAnim::anim() {
	//update();
	_animated_obj->anim();
	_animated_obj->update_data();
}


void TestObjAnim::update() {
	GLDrawContext * context= _gl_draw_manager->get_context("obj");
	context->_n_pts = _animated_obj->_obj_data->_n_pts;
	context->set_data(_animated_obj->_data);
	//context->show_data();
}


void TestObjAnim::draw() {
	GLDrawContext * context= _gl_draw_manager->get_context("obj");
	context->activate();
	context->set_uniform("world2clip_matrix", glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	context->set_uniform("light_position", glm::value_ptr(LIGHT_POSITION));
	context->set_uniform("light_color", glm::value_ptr(LIGHT_COLOR));
	context->set_uniform("view_position", glm::value_ptr(glm::vec3(_view_system->_eye)));
	context->set_uniform("anim_matrices", glm::value_ptr(_animated_obj->_matrices[0]), N_MAX_MATRICES);
	context->draw();
	context->deactivate();
}

