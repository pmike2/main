#include "gl_error.h"

 
using namespace std;



void _check_gl_error(const char *file, int line){

	GLenum err(glGetError());

	while (err!= GL_NO_ERROR){
		string error;
		switch(err){
			case GL_INVALID_OPERATION: error="INVALID_OPERATION"; break;
			case GL_INVALID_ENUM: error="INVALID_ENUM"; break;
			case GL_INVALID_VALUE: error="INVALID_VALUE"; break;
			case GL_OUT_OF_MEMORY: error="OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error="INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		cout << error.c_str() << " ; " << file << " ; " << line << endl;
		
		err= glGetError();
	}
}


void gl_versions(){
	const GLubyte * renderer= glGetString(GL_RENDERER);
	const GLubyte * vendor= glGetString(GL_VENDOR);
	const GLubyte * version= glGetString(GL_VERSION);
	const GLubyte * glslversion= glGetString(GL_SHADING_LANGUAGE_VERSION);
	GLint maj_v, min_v;
	glGetIntegerv(GL_MAJOR_VERSION, &maj_v);
	glGetIntegerv(GL_MINOR_VERSION, &min_v);

	cout << "renderer=" << renderer << " ; vendor=" << vendor << " ; version=" << version << " ; glslversion=" << glslversion << " ; maj_v=" << maj_v << " ; min_v=" << min_v << endl;
}


void active_uniforms(GLuint prog) {
	GLint nUniforms, size, location, maxLen;
	GLchar * name;
	GLsizei written;
	GLenum type;
	glGetProgramiv(prog, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
	glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &nUniforms);
	name= (GLchar *) malloc(maxLen);
	for( int i=0; i<nUniforms; ++i ) {
		glGetActiveUniform(prog, i, maxLen, &written, &size, &type, name);
		location= glGetUniformLocation(prog, name);
		cout << location << " ; " << name << endl;
	}
	free(name);
}


void active_attribs(GLuint prog) {
	GLint written, size, location, maxLength, nAttribs;
	GLenum type;
	GLchar * name;
	glGetProgramiv(prog, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);
	glGetProgramiv(prog, GL_ACTIVE_ATTRIBUTES, &nAttribs);
	name= (GLchar *) malloc(maxLength);
	for( int i = 0; i < nAttribs; i++ ) {
		glGetActiveAttrib( prog, i, maxLength, &written, &size, &type, name );
		location = glGetAttribLocation(prog, name);
		cout << location << " ; " << name << endl;
	}
	free(name);
}
