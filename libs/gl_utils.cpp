#include <iostream>
#include <cstdlib>
#include <sstream>
#include <fstream>

#include "utile.h"

#include "gl_utils.h"



// -------------------------------------------------------------------------------------------
ScreenGL::ScreenGL() {

}


ScreenGL::ScreenGL(int screen_width, int screen_height, number gl_width, number gl_height) : 
	_screen_width(screen_width), _screen_height(screen_height), _gl_width(gl_width), _gl_height(gl_height)
{

}


ScreenGL::~ScreenGL() {

}


void ScreenGL::screen2gl(int i, int j, number & x, number & y) {
	x= ((number)(i)/ (number)(_screen_width)- 0.5f)* _gl_width;
	y= (0.5f- (number)(j)/ (number)(_screen_height))* _gl_height;
}


pt_2d ScreenGL::screen2gl(int i, int j) {
	number x, y;
	screen2gl(i, j, x, y);
	return pt_2d(x, y);
}


void ScreenGL::gl2screen(number x, number y, int & i, int & j) {
	i= (int)((number)(_screen_width)* (x/ _gl_width+ 0.5));
	j= (int)((number)(_screen_width)* (y/ _gl_height+ 0.5));
}


std::ostream & operator << (std::ostream & os, const ScreenGL & screengl) {
	os << "screen_width = " << screengl._screen_width << " ; screen_height = " << screengl._screen_height;
	os << " ; gl_width = " << screengl._gl_width << " ; gl_height = " << screengl._gl_height;
	return os;
}


// -------------------------------------------------------------------------------------------------
void gl_versions() {
	const GLubyte * renderer= glGetString(GL_RENDERER);
	const GLubyte * vendor= glGetString(GL_VENDOR);
	const GLubyte * version= glGetString(GL_VERSION);
	const GLubyte * glslversion= glGetString(GL_SHADING_LANGUAGE_VERSION);
	GLint maj_v, min_v;
	glGetIntegerv(GL_MAJOR_VERSION, &maj_v);
	glGetIntegerv(GL_MINOR_VERSION, &min_v);

	std::cout << "renderer=" << renderer << " ; vendor=" << vendor << " ; version=" << version << " ; glslversion=" << glslversion << " ; maj_v=" << maj_v << " ; min_v=" << min_v << "\n";
}


// gestion multi-fenetre
void set_subwindow(const float bkgnd_color[4], int x, int y, int w, int h) {
	glClearColor(bkgnd_color[0], bkgnd_color[1], bkgnd_color[2], bkgnd_color[3]);
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, y, w, h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
	glViewport(x, y, w, h);
}


void export_screen_to_ppm(std::string ppm_path, uint x, uint y, uint width, uint height) {
	unsigned char * pixels= new unsigned char[width* height* 3];
	// il faut spécifier 1 pour l'alignement sinon plantages divers lorsque width ou height ne sont
	// pas des multiples de 4
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	FILE *f;
	f= fopen(ppm_path.c_str(), "wb");
	fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);
	for (int i=height- 1; i>=0; --i) {
		for (int j=0; j<width; ++j) {
			fprintf(f, "%d %d %d\n", pixels[3* (width* i+ j)+ 0], pixels[3* (width* i+ j)+ 1], pixels[3* (width* i+ j)+ 2]);
		}
	}
	fclose(f);
	delete[] pixels;
}


float * draw_cross(float * data, pt_2d center, float size, glm::vec4 color) {
	data[0]= float(center.x)- size;
	data[1]= float(center.y)- size;
	data[6]= float(center.x)+ size;
	data[7]= float(center.y)+ size;
	data[12]= float(center.x)+ size;
	data[13]= float(center.y)- size;
	data[18]= float(center.x)- size;
	data[19]= float(center.y)+ size;
	
	for (uint i=0; i<4; ++i) {
		for (uint j=0; j<4; ++j) {
			data[i* 6+ 2+ j]= color[j];
		}
	}
	return data+ 24;
}


float * draw_arrow(float * data, pt_2d start, pt_2d end, float tip_size, float angle, glm::vec4 color) {
	bool start_is_end= false;
	if ((abs(start.x- end.x)< 1e-5) && (abs(start.y- end.y)< 1e-5)) {
		start_is_end= true;
	}
	
	if (start_is_end) {
		for (uint i=0; i<6; ++i) {
			data[i* 6+ 0]= float(start.x);
			data[i* 6+ 1]= float(start.y);
		}
	}
	else {
		pt_2d norm= glm::normalize(start- end);
		data[0]= float(start.x);
		data[1]= float(start.y);
		data[6]= float(end.x);
		data[7]= float(end.y);
		
		data[12]= float(end.x);
		data[13]= float(end.y);
		data[18]= float(end.x)+ tip_size* (cos(angle)* float(norm.x)- sin(angle)* float(norm.y));
		data[19]= float(end.y)+ tip_size* (sin(angle)* float(norm.x)+ cos(angle)* float(norm.y));

		data[24]= float(end.x);
		data[25]= float(end.y);
		data[30]= float(end.x)+ tip_size* (cos(angle)* float(norm.x)+ sin(angle)* float(norm.y));
		data[31]= float(end.y)+ tip_size* (-sin(angle)* float(norm.x)+ cos(angle)* float(norm.y));
	}

	for (uint i=0; i<6; ++i) {
		for (uint j=0; j<4; ++j) {
			data[i* 6+ 2+ j]= color[j];
		}
	}
	return data+ 36;
}


float * draw_polygon(float * data, std::vector<pt_2d> pts, glm::vec4 color) {
	for (uint idx_pt=0; idx_pt<pts.size(); ++idx_pt) {
		data[idx_pt* 6* 2+ 0]= float(pts[idx_pt].x);
		data[idx_pt* 6* 2+ 1]= float(pts[idx_pt].y);

		if (idx_pt< pts.size()- 1) {
			data[idx_pt* 6* 2+ 6]= float(pts[idx_pt+ 1].x);
			data[idx_pt* 6* 2+ 7]= float(pts[idx_pt+ 1].y);
		}
		else {
			data[idx_pt* 6* 2+ 6]= float(pts[0].x);
			data[idx_pt* 6* 2+ 7]= float(pts[0].y);
		}
	}

	for (uint i=0; i<pts.size()* 2; ++i) {
		for (uint j=0; j<4; ++j) {
			data[i* 6+ 2+ j]= color[j];
		}
	}

	return data+ 6* 2* pts.size();
}


float * draw_nothing(float * data, uint n_attrs_per_pts, uint n_pts) {
	uint compt= 0;
	for (uint idx_pt=0; idx_pt<n_pts; ++idx_pt) {
		for (uint idx_attr=0; idx_attr<n_attrs_per_pts; ++idx_attr) {
			data[compt++]= 0.0;
		}
	}
	return data+ n_attrs_per_pts* n_pts;
}

