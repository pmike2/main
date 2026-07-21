#include "test_obj.h"



TestObj::TestObj() {

}


TestObj::TestObj(GLDrawManager * gl_draw_manager, ViewSystem * view_system) :
	_gl_draw_manager(gl_draw_manager), _view_system(view_system), _model2world(1.0), _angle(0.0), _current_context_name("obj")
{
	_obj_data = new ObjData("../data/test2.obj");

	std::vector<fs> diffuse_textures, normal_textures;
	for (auto & material : _obj_data->_materials) {
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

	// obj
	_gl_draw_manager->add_texture(
		"obj", "diffuse_texture", GL_TEXTURE_2D_ARRAY, 0,
			std::map<GLenum, int>{
			{GL_TEXTURE_MIN_FILTER, GL_LINEAR}, {GL_TEXTURE_MAG_FILTER, GL_LINEAR},
			{GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE}, {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE}
			},
		GL_RGBA, glm::uvec3(512, 512, diffuse_textures.size()), GL_BGRA, GL_UNSIGNED_BYTE
	);
	_gl_draw_manager->set_texture_data("obj", "diffuse_texture", diffuse_textures);

	// obj_normal
	_gl_draw_manager->add_texture(
		"obj_normal", "diffuse_texture", GL_TEXTURE_2D_ARRAY, 0,
			std::map<GLenum, int>{
			{GL_TEXTURE_MIN_FILTER, GL_LINEAR}, {GL_TEXTURE_MAG_FILTER, GL_LINEAR},
			{GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE}, {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE}
			},
		GL_RGBA, glm::uvec3(512, 512, diffuse_textures.size()), GL_BGRA, GL_UNSIGNED_BYTE
	);
	_gl_draw_manager->set_texture_data("obj_normal", "diffuse_texture", diffuse_textures);

	_gl_draw_manager->add_texture(
		"obj_normal", "normal_texture", GL_TEXTURE_2D_ARRAY, 1,
			std::map<GLenum, int>{
			{GL_TEXTURE_MIN_FILTER, GL_LINEAR}, {GL_TEXTURE_MAG_FILTER, GL_LINEAR},
			{GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE}, {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE}
			},
		GL_RGBA, glm::uvec3(512, 512, diffuse_textures.size()), GL_BGRA, GL_UNSIGNED_BYTE
	);
	_gl_draw_manager->set_texture_data("obj_normal", "normal_texture", normal_textures);

	update();
}


TestObj::~TestObj() {
	delete _obj_data;
}


void TestObj::anim() {
	_angle += 0.01;
	if (_angle > 2.0 * M_PI) {
		_angle -= 2.0 * M_PI;
	}
	_model2world = mat4_cast(glm::angleAxis(_angle + M_PI * 0.5, pt_3d(0.0, 1.0, 0.0)));
}


void TestObj::update() {
	GLDrawContext * context = _gl_draw_manager->get_context("obj");
	context->_n_pts = _obj_data->_n_pts;

	_obj_data->update_data(std::vector<OBJDATA_DATA_ITEM>{OBJDATA_VERTEX, OBJDATA_NORMAL, OBJDATA_TEXTURE});

	context->set_data(_obj_data->_data);
	//context->show_data();

	context = _gl_draw_manager->get_context("obj_normal");
	context->_n_pts = _obj_data->_n_pts;

	_obj_data->update_data(std::vector<OBJDATA_DATA_ITEM>{OBJDATA_VERTEX, OBJDATA_NORMAL, OBJDATA_TANGENT, OBJDATA_BITANGENT, OBJDATA_TEXTURE, OBJDATA_SHININESS});

	context->set_data(_obj_data->_data);
	//context->show_data();
}


void TestObj::draw() {
	GLDrawContext * context = _gl_draw_manager->get_context(_current_context_name);
	context->activate();
	context->set_uniform("world2clip_matrix", glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	context->set_uniform("model2world_matrix", glm::value_ptr(glm::mat4(_model2world)));
	context->set_uniform("light_position", glm::value_ptr(LIGHT_POSITION));
	context->set_uniform("light_color", glm::value_ptr(LIGHT_COLOR));
	context->set_uniform("view_position", glm::value_ptr(glm::vec3(_view_system->_eye)));
	context->draw();
	context->deactivate();
}


bool TestObj::key_down(InputState * input_state, SDL_Keycode key, time_point t) {
	if (key == SDLK_SPACE) {
		if (_current_context_name == "obj") {
			_current_context_name = "obj_normal";
		}
		else if (_current_context_name == "obj_normal") {
			_current_context_name = "obj";
		}
	}
	return false;
}
