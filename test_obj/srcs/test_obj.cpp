#include "test_obj.h"


TestObj::TestObj() {

}


TestObj::TestObj(GLDrawManager * gl_draw_manager, ViewSystem * view_system) :
	_gl_draw_manager(gl_draw_manager), _view_system(view_system) 
{
	_obj_data = new ObjData("../data/test.obj");
	_obj_data->_use_ambient = false;
	_obj_data->_use_diffuse = true;
	_obj_data->_use_specular = false;
	_obj_data->_use_shininess = false;
	_obj_data->_use_opacity = false;
	_obj_data->update_data();

	update();
}


TestObj::~TestObj() {
	delete _obj_data;
}


void TestObj::anim() {
	//update();
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
	context->set_uniform("light_position", glm::value_ptr(LIGHT_POSITION));
	context->set_uniform("light_color", glm::value_ptr(LIGHT_COLOR));
	context->set_uniform("view_position", glm::value_ptr(glm::vec3(_view_system->_eye)));
	context->draw();
	context->deactivate();
}

