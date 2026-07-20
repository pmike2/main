#include "test_obj.h"



TestObj::TestObj() {

}


TestObj::TestObj(GLDrawManager * gl_draw_manager, ViewSystem * view_system) :
	_gl_draw_manager(gl_draw_manager), _view_system(view_system), _model2world(1.0), _angle(0.0)
{
	_obj_data = new ObjData("../data/test2.obj");
	_obj_data->_use_ambient = false;
	_obj_data->_use_diffuse = true;
	_obj_data->_use_specular = false;
	_obj_data->_use_shininess = false;
	_obj_data->_use_opacity = false;
	_obj_data->_use_diffuse_texture = true;
	_obj_data->update_data();

	GLDrawContext * context = _gl_draw_manager->get_context("obj");
	std::vector<fs> diffuse_textures;
	for (auto & material : _obj_data->_materials) {
		if (material->_diffuse_tex_path != "") {
			diffuse_textures.push_back(material->_diffuse_tex_path);
		}
	}
	_gl_draw_manager->add_texture(
		context->_name, "diffuse_texture", GL_TEXTURE_2D_ARRAY, 0,
			std::map<GLenum, int>{
			{GL_TEXTURE_MIN_FILTER, GL_LINEAR}, {GL_TEXTURE_MAG_FILTER, GL_LINEAR},
			{GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE}, {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE}
			},
		GL_RGBA, glm::uvec3(512, 512, diffuse_textures.size()), GL_BGRA, GL_UNSIGNED_BYTE
	);
	_gl_draw_manager->set_texture_data(context->_name, "diffuse_texture", diffuse_textures);

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
	GLDrawContext * context= _gl_draw_manager->get_context("obj");
	context->_n_pts = _obj_data->_n_pts;
	context->set_data(_obj_data->_data);
	//context->show_data();
}


void TestObj::draw() {
	GLDrawContext * context= _gl_draw_manager->get_context("obj");
	context->activate();
	context->set_uniform("world2clip_matrix", glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	context->set_uniform("model2world_matrix", glm::value_ptr(glm::mat4(_model2world)));
	context->set_uniform("light_position", glm::value_ptr(LIGHT_POSITION));
	context->set_uniform("light_color", glm::value_ptr(LIGHT_COLOR));
	context->set_uniform("view_position", glm::value_ptr(glm::vec3(_view_system->_eye)));
	context->draw();
	context->deactivate();
}

