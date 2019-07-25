#ifndef GLERROR_H
#define GLERROR_H
 
#include <cstdlib>
#include <string>

#include <OpenGL/gl3.h>

#include "utile.h"
#include "constantes.h"

void _check_gl_error(const char *file, int line);
void gl_versions();
void active_uniforms(GLuint prog);
void active_attribs(GLuint prog);
 
#define check_gl_error() _check_gl_error(__FILE__,__LINE__)
 
#endif // GLERROR_H
