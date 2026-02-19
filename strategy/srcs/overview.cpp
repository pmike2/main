#include "overview.h"


OverView::OverView() {

}


OverView::OverView(GLDrawManager * gl_draw_manager) : _gl_draw_manager(gl_draw_manager), _tex_width(OVERVIEW_TEXTURE_SIZE), _tex_height(OVERVIEW_TEXTURE_SIZE) {
	_gl_draw_manager->add_texture("overview_texture", GL_TEXTURE_2D, 0,
		std::map<GLenum, int>{
			{GL_TEXTURE_MIN_FILTER, GL_NEAREST}, {GL_TEXTURE_MAG_FILTER, GL_NEAREST},
			{GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE}, {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE}
		},
		GL_RGB, glm::uvec3(_tex_width, _tex_height, 0), GL_RGB, GL_UNSIGNED_BYTE);

	// ----------------------------------------
	// TODO : mettre tout ça dans gl_draw.h ?
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _tex_width, _tex_height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	
	glGenFramebuffers(1, &_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _gl_draw_manager->_texture_pool->get_texture("overview_texture")->_id, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
	
	GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "OverView glCheckFramebufferStatus échec\n";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ----------------------------------------
	_aabb = new AABB_2D(DEFAULT_OVERVIEW_POSITION, DEFAULT_OVERVIEW_SIZE);

	GLDrawContext * context= _gl_draw_manager->get_context("overview");
	context->_n_pts = 6;
	
	float * data = new float[context->data_size()];
	float * ptr = data;
	
	pt_4d pos[4] = {
		pt_4d(_aabb->_pos.x, _aabb->_pos.y, 0.0, 0.0),
		pt_4d(_aabb->_pos.x + _aabb->_size.x, _aabb->_pos.y, 1.0, 0.0),
		pt_4d(_aabb->_pos.x + _aabb->_size.x, _aabb->_pos.y + _aabb->_size.y, 1.0, 1.0),
		pt_4d(_aabb->_pos.x, _aabb->_pos.y + _aabb->_size.y, 0.0, 1.0)
	};
	const uint idxs[6] = {0, 1, 2, 0, 2, 3};

	for (uint i=0; i<6; ++i) {
		ptr[0] = pos[idxs[i]].x;
		ptr[1] = pos[idxs[i]].y;
		ptr[2] = pos[idxs[i]].z;
		ptr[3] = pos[idxs[i]].w;
		ptr += 4;
	}

	context->set_data(data);
	delete[] data;

	// -------------------------------------------
	// on fait un screen2gl carré ; les valeurs ont été calculées de façon empirique mais on doit pouvoir trouver une formule
	const float GL_WIDTH= 10.0f;
	const float GL_HEIGHT= 10.0f;
	const number RHO = 200.0;
	_screengl= new ScreenGL(_tex_width, _tex_height, GL_WIDTH, GL_HEIGHT);
	_view_system= new ViewSystem(_gl_draw_manager, _screengl);
	_view_system->set_2d(RHO);

}


OverView::~OverView() {
	delete _aabb;
}


void OverView::start_draw_in_texture() {
	glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
	glViewport(0, 0, _tex_width, _tex_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void OverView::end_draw_in_texture() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void OverView::draw(ViewSystem * view_system) {
	GLDrawContext * context= _gl_draw_manager->get_context("overview");
	context->activate();
	context->set_uniform("camera2clip_matrix", glm::value_ptr(glm::mat4(view_system->_camera2clip)));
	context->set_uniform("z", float(Z_OVERVIEW));
	context->set_uniform("alpha", float(DEFAULT_OVERVIEW_ALPHA));
	context->draw();
	context->deactivate();
}
