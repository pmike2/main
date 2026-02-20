#include <iostream>
#include <cstdlib>
#include <sstream>
#include <fstream>

#include <SDL2/SDL_image.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "json.hpp"

#include "utile.h"

#include "gl_draw.h"


using json = nlohmann::json;


// -------------------------------------------------------------------------------------------
void _check_gl_error(const char * file, int line){
	GLenum err(glGetError());

	while (err!= GL_NO_ERROR){
		std::string error;
		switch(err){
			case GL_INVALID_OPERATION: error="INVALID_OPERATION"; break;
			case GL_INVALID_ENUM: error="INVALID_ENUM"; break;
			case GL_INVALID_VALUE: error="INVALID_VALUE"; break;
			case GL_OUT_OF_MEMORY: error="OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error="INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error.c_str() << " ; " << file << " ; " << line << "\n";
		
		err= glGetError();
	}
}


// Verif que le link des shaders de prog s'est bien déroulé
void check_gl_program(GLuint prog) {
	GLint status;

	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status== GL_FALSE) {
		int loglen;
		char logbuffer[1000];
		glGetProgramInfoLog(prog, sizeof(logbuffer), &loglen, logbuffer);
		std::cout << "OpenGL Program Linker Error " << loglen << " ; " << logbuffer << "\n";
	}
	else {
		int loglen;
		char logbuffer[1000];
		glGetProgramInfoLog(prog, sizeof(logbuffer), &loglen, logbuffer);
		if (loglen> 0) {
			std::cout << "OpenGL Program Linker OK " << loglen << " ; " << logbuffer << "\n";
		}
		glValidateProgram(prog);
		glGetProgramInfoLog(prog, sizeof(logbuffer), &loglen, logbuffer);
		if (loglen> 0) {
			std::cout << "OpenGL Program Validation results " << loglen << " ; " << logbuffer << "\n";
		}
	}
}


// Remplissage d'une chaine de caractères avec le contenu du fichier d'un shader
char * load_source(const char * filename) {
	char *src= NULL;   /* code source de notre shader */
	FILE *fp= NULL;	   /* fichier */
	long size;			/* taille du fichier */
	long i;				/* compteur */
	
	fp= fopen(filename, "r");

	/* on verifie si l'ouverture a echoue */
	if (fp== NULL) {
		std::cout << "impossible d'ouvrir le fichier " << filename << "\n";
		return NULL;
	}
	
	/* on recupere la longueur du fichier */
	fseek(fp, 0, SEEK_END);
	size= ftell(fp);
	
	/* on se replace au debut du fichier */
	rewind(fp);
	
	/* on alloue de la memoire pour y placer notre code source */
	src= (char *)malloc(size+ 1); /* +1 pour le caractere de fin de chaine '\0' */
	if (src== NULL) {
		fclose(fp);
		std::cout << "erreur d'allocation de memoire" << "\n";
		return NULL;
	}
	
	/* lecture du fichier */
	for (i=0; i<size; i++) {
		src[i]= fgetc(fp);
	}
	
	/* on place le dernier caractere a '\0' */
	src[size]= '\0';
	
	fclose(fp);
	
	return src;
}


// assignation, compilation et verification que la compilation s'est bien faite
GLuint load_shader(GLenum type, const char * filename) {
	GLuint shader= 0;
	GLsizei logsize= 0;
	GLint compile_status= GL_TRUE;
	char * log= NULL;
	char * src= NULL;
	
	/* creation d'un shader de sommet */
	shader= glCreateShader(type);
	if (shader== 0) {
		std::cout << "impossible de creer le shader" << "\n";
		return 0;
	}
	
	/* chargement du code source */
	src= load_source(filename);
	if (src== NULL) {
		std::cout << "Le fichier de shader n'existe pas" << "\n";
		glDeleteShader(shader);
		return 0;
	}
	
	/* assignation du code source */
	glShaderSource(shader, 1, (const GLchar**)&src, NULL);
	
	/* compilation du shader */
	glCompileShader(shader);
	
	/* liberation de la memoire du code source */
	free(src);
	src = NULL;
	
	/* verification du succes de la compilation */
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status!= GL_TRUE) {
		/* erreur a la compilation recuperation du log d'erreur */
		
		/* on recupere la taille du message d'erreur */
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);
		
		/* on alloue un espace memoire dans lequel OpenGL ecrira le message */
		log= (char *)malloc(logsize + 1);
		if (log == NULL) {
			std::cout <<	 "impossible d'allouer de la memoire" << "\n";
			return 0;
		}
		/* initialisation du contenu */
		memset(log, '\0', logsize + 1);
		
		glGetShaderInfoLog(shader, logsize, &logsize, log);
		std::cout << "impossible de compiler le shader" << filename << " : " << log << "\n";
		
		/* ne pas oublier de liberer la memoire et notre shader */
		free(log);
		glDeleteShader(shader);
		
		return 0;
	}
	
	return shader;
}


GLuint create_prog(std::string vs_path, std::string fs_path, std::string gs_path, bool check_program) {
	GLuint vs= load_shader(GL_VERTEX_SHADER  , vs_path.c_str());
	GLuint fs= load_shader(GL_FRAGMENT_SHADER, fs_path.c_str());
	GLuint prog= glCreateProgram();
	glAttachShader(prog, fs);
	glAttachShader(prog, vs);
	if (gs_path!= "") {
		GLuint gs= load_shader(GL_GEOMETRY_SHADER, gs_path.c_str());
		glAttachShader(prog, gs);
	}
	glLinkProgram(prog);

	if (check_program) {
		check_gl_program(prog);
	}

	return prog;
}


std::vector<GLDrawContextUniform *> active_uniforms(GLuint prog) {
	std::vector<GLDrawContextUniform *> result;

	GLint nUniforms, size, location, maxLen;
	GLchar * name;
	GLsizei written;
	GLenum type;
	glGetProgramiv(prog, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
	glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &nUniforms);
	name= (GLchar *) malloc(maxLen);
	for( int i=0; i<nUniforms; ++i ) {
		glGetActiveUniform(prog, i, maxLen, &written, &size, &type, name);
		location = glGetUniformLocation(prog, name);
		GLDrawContextUniform * uniform = new GLDrawContextUniform(std::string(name), location, type, size);
		result.push_back(uniform);
	}
	free(name);

	return result;
}


std::vector<GLDrawContextAttrib *> active_attribs(GLuint prog) {
	std::vector<GLDrawContextAttrib *> result;

	GLint written, size, location, max_length, n_attribs;
	GLenum type;
	GLchar * name;
	glGetProgramiv(prog, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_length);
	glGetProgramiv(prog, GL_ACTIVE_ATTRIBUTES, &n_attribs);
	name= (GLchar *) malloc(max_length);
	
	for (int i=0; i<n_attribs; ++i) {
		glGetActiveAttrib(prog, i, max_length, &written, &size, &type, name);
		location = glGetAttribLocation(prog, name);
		GLDrawContextAttrib * attrib = new GLDrawContextAttrib(std::string(name), location, size, type);
		result.push_back(attrib);
	}

	free(name);

	return result;
}


// --------------------------------------------------------
GLDrawTexture::GLDrawTexture() {

}


GLDrawTexture::GLDrawTexture(std::string name, GLenum target, uint offset, std::map<GLenum, int> params, int internal_format, glm::uvec3 size, GLenum format, GLenum type) :
	_name(name), _target(target), _offset(offset), _params(params), _internal_format(internal_format), _size(size), _format(format), _type(type)
{
	glGenTextures(1, &_id);
	glActiveTexture(GL_TEXTURE0 + _offset);
	glBindTexture(_target, _id);
	glActiveTexture(0);

	for (auto & param : _params) {
		glTexParameteri(_target, param.first, param.second);
	}

	if (_target == GL_TEXTURE_2D_ARRAY) {
		glTexImage3D(_target, 0, _internal_format, _size[0], _size[1], _size[2], 0, _format, _type, NULL);
	}
	else if (_target == GL_TEXTURE_2D) {
		glTexImage2D(_target, 0, _internal_format, _size[0], _size[1], 0, _format, _type, NULL);
	}
	else {
		std::cerr << "GLDrawTexture::GLDrawTexture : _target = " << _target << " inconnu\n";
	}

	glBindTexture(_target, 0);
}


GLDrawTexture::~GLDrawTexture() {

}

void GLDrawTexture::set_data(void * data, uint depth, int width, int height) {
	if (width < 0) {
		width = _size[0];
	}
	if (height < 0) {
		height = _size[1];
	}
	
	glActiveTexture(GL_TEXTURE0 + _offset);
	glBindTexture(_target, _id);
	glActiveTexture(0);

	if (_target == GL_TEXTURE_2D_ARRAY) {
		glTexSubImage3D(_target, 0, 0, 0, depth, width, height, 1, _format, _type, data);
	}
	else if (_target == GL_TEXTURE_2D) {
		// TODO
		std::cerr << "GLDrawTexture::GLDrawTexture : TODO !! _target = GL_TEXTURE_2D\n";
	}
	else {
		std::cerr << "GLDrawTexture::GLDrawTexture : _target = " << _target << " inconnu\n";
	}

	glBindTexture(_target, 0);
}


void GLDrawTexture::set_data(std::vector<std::string> pngs) {
	glActiveTexture(GL_TEXTURE0 + _offset);
	glBindTexture(_target, _id);
	glActiveTexture(0);

	for (uint idx_png=0; idx_png<pngs.size(); ++idx_png) {
		// parfois on veut sauter des indices, auquel cas on spécifie NO_PNG
		if (pngs[idx_png]== NO_PNG) {
			continue;
		}

		if (!file_exists(pngs[idx_png])) {
			std::cerr << "png=" << pngs[idx_png] << " n'existe pas\n";
			return;
		}
		SDL_Surface * surface = IMG_Load(pngs[idx_png].c_str());
		if (!surface) {
			std::cerr << "IMG_Load error :" << IMG_GetError() << "\n";
			return;
		}

		glTexSubImage3D(_target, 0, 0, 0, idx_png, _size[0], _size[1], 1, _format, _type, surface->pixels);

		SDL_FreeSurface(surface);
	}

	glBindTexture(_target, 0);
}


// TODO : gérer les cas non GL_RED - GL_UNSIGNED_BYTE
void GLDrawTexture::export2pgm(std::string pgm_path) {
	glActiveTexture(GL_TEXTURE0 + _offset);
	glBindTexture(_target, _id);
	glActiveTexture(0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	if (_target == GL_TEXTURE_2D) {
		unsigned char * pixels= new unsigned char[_size[0] * _size[1]];
		glGetTexImage(_target, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
		FILE *f;
		f= fopen(pgm_path.c_str(), "wb");
		fprintf(f, "P5\n%d %d\n%d\n", _size[0], _size[1], 255);
		for (uint i=0; i<_size[1]; ++i) {
			fwrite(pixels+ i* _size[0], 1, _size[0], f);
		}
		fclose(f);
		delete[] pixels;
	}
	
	else if (_target == GL_TEXTURE_2D_ARRAY) {
		unsigned char * pixels= new unsigned char[_size[0] * _size[1] * _size[2]];
		glGetTexImage(_target, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);

		for (uint d=0; d<_size[2]; ++d) {
			std::string pgm_path_ = pgm_path+ "/tex_array_"+ std::to_string(d)+ ".pgm";
			FILE *f;
			f= fopen(pgm_path_.c_str(), "wb");
			fprintf(f, "P5\n%d %d\n%d\n", _size[0], _size[1], 255);
			for (uint i=0; i<_size[1]; ++i) {
				fwrite(pixels + d * _size[0] * _size[1] + i * _size[0], 1, _size[0], f);
			}
			fclose(f);
		}
		delete[] pixels;
	}

	glBindTexture(_target, 0);
}


std::ostream & operator << (std::ostream & os, const GLDrawTexture & tex) {
	os << "name = " << tex._name << " ; target = " << tex._target;
	os << " ; id = " << tex._id << " ; offset = " << tex._offset;
	os << " ; internal_format = " << tex._internal_format << " ; size = " << glm::to_string(tex._size);
	os << " ; format = " << tex._format << " ; type = " << tex._type;
	return os;
}


// --------------------------------------------------------
GLDrawTexturePool::GLDrawTexturePool() {

}


GLDrawTexturePool::~GLDrawTexturePool() {
	for (auto & texture : _textures) {
		delete texture;
	}
	_textures.clear();
}


GLDrawTexture * GLDrawTexturePool::get_texture(std::string name) {
	for (auto & texture : _textures) {
		if (texture->_name == name) {
			return texture;
		}
	}
	std::cerr << "GLDrawTexturePool::get_texture : " << name << " inexistante\n";
	return NULL;
}


GLDrawTexture * GLDrawTexturePool::add_texture(std::string name, GLenum target, uint offset, std::map<GLenum, int> params, int internal_format, glm::uvec3 size, GLenum format, GLenum type) {
	for (auto & texture : _textures) {
		if (texture->_name == name) {
			std::cerr << "GLDrawTexturePool::add_texture : " << name << " existe déjà\n";
			return NULL;
		}
	}

	GLDrawTexture * texture = new GLDrawTexture(name, target, offset, params, internal_format, size, format, type);
	_textures.push_back(texture);
	return texture;
}


std::ostream & operator << (std::ostream & os, const GLDrawTexturePool & texpool) {
	for (auto & texture : texpool._textures) {
		os << *texture << "\n";
	}
	return os;
}


// --------------------------------------------------------
GLDrawContextAttrib::GLDrawContextAttrib() {

}


GLDrawContextAttrib::GLDrawContextAttrib(std::string name, GLint loc, uint size, GLenum type) : _name(name), _loc(loc), _offset(0), _in_default_buffer(true) {
	// attention size ici est la dimension du tableau de types donc 1 si ce n'est pas un tableau
	if (type == GL_FLOAT) {
		_size = 1;
	}
	else if (type == GL_FLOAT_VEC2) {
		_size = 2;
	}
	else if (type == GL_FLOAT_VEC3) {
		_size = 3;
	}
	else if (type == GL_FLOAT_VEC4) {
		_size = 4;
	}
	else if (type == GL_FLOAT_MAT3) {
		_size = 9;
	}
	else if (type == GL_FLOAT_MAT4) {
		_size = 16;
	}
	else if (type == GL_INT) {
		_size = 1;
	}
	else {
		std::cerr << "GLDrawContextAttrib : type " << type << "non reconnu. compléter avec : https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetActiveAttrib.xhtml\n";
		return;
	}
	_size *= size;
}


GLDrawContextAttrib::~GLDrawContextAttrib() {

}


std::ostream & operator << (std::ostream & os, const GLDrawContextAttrib & dca) {
	os << "name = " << dca._name;
	os << " ; loc = " << dca._loc << " ; size = " << dca._size << " ; offset = " << dca._offset;
	os << " ; in_default_buffer = " << dca._in_default_buffer;

	return os;
}


// --------------------------------------------------------
GLDrawContextUniform::GLDrawContextUniform() {

}


GLDrawContextUniform::GLDrawContextUniform(std::string name, GLint loc, GLenum type, GLint size) :
	_name(name), _loc(loc), _type(type), _size(size)
{

}


GLDrawContextUniform::~GLDrawContextUniform() {

}


std::ostream & operator << (std::ostream & os, const GLDrawContextUniform & dcu) {
	os << "name = " << dcu._name << " ; " << "loc = " << dcu._loc << " ; ";
	os << "type = " << dcu._type << " ; " << "size = " << dcu._size;
	return os;
}


// --------------------------------------------------------
GLDrawContextBuffer::GLDrawContextBuffer() {

}


GLDrawContextBuffer::GLDrawContextBuffer(bool is_instanced, GLenum usage) : _is_instanced(is_instanced), _usage(usage), _n_attrs_per_pts(0) {

}


GLDrawContextBuffer::~GLDrawContextBuffer() {
	for (auto & attrib : _attribs) {
		delete attrib;
	}
	_attribs.clear();
}


std::ostream & operator << (std::ostream & os, const GLDrawContextBuffer & dcb) {
	os << "id = " << dcb._id << " ; n_attrs_per_pts = " << dcb._n_attrs_per_pts << " ; usage = " << dcb._usage;
	os  << " ; is_instanced = " << dcb._is_instanced;
	os << "\n\t";
	os << "attribs :\n";
	for (auto attrib : dcb._attribs) {
		os << "\t\t" << *attrib << "\n";
	}
	return os;
}


// --------------------------------------------------------
GLDrawContext::GLDrawContext() {

}


GLDrawContext::GLDrawContext(std::string name, GLuint prog, GLenum draw_mode, std::vector<GLDrawContextBuffer *> buffers, bool active) :
	_name(name), _prog(prog), _n_pts(0), _active(active), _n_instances(0), _draw_mode(draw_mode), _verbose(false)
{

	_uniforms = active_uniforms(_prog);

	for (auto & buffer : buffers) {
		glGenBuffers(1, &buffer->_id);

		// on trie les attributs par leurs location afin de correctement calculer l'offset
		std::sort(buffer->_attribs.begin(), buffer->_attribs.end(), [](const GLDrawContextAttrib * a, const GLDrawContextAttrib * b) { return a->_loc < b->_loc; });
		uint offset = 0;
		for (auto attrib : buffer->_attribs) {
			attrib->_offset = offset;
			offset += attrib->_size;
			buffer->_n_attrs_per_pts += attrib->_size;
		}
		_buffers.push_back(buffer);
	}

	glGenVertexArrays(1, &_vao);

	glBindVertexArray(_vao);
	
	for (auto & buffer : _buffers) {
		glBindBuffer(GL_ARRAY_BUFFER, buffer->_id);
		for (auto attr : buffer->_attribs) {
			if (buffer->_is_instanced) {
				if (attr->_size == 16) {
					for (uint i=0; i<4; ++i) {
						glEnableVertexAttribArray(attr->_loc + i);
						glVertexAttribPointer(attr->_loc + i, 4, GL_FLOAT, GL_FALSE, buffer->_n_attrs_per_pts * sizeof(float), (void*)((attr->_offset + i * 4) * sizeof(float)));
						glVertexAttribDivisor(attr->_loc + i, 1);
					}
				}
				else {
					glEnableVertexAttribArray(attr->_loc);
					glVertexAttribPointer(attr->_loc, attr->_size, GL_FLOAT, GL_FALSE, buffer->_n_attrs_per_pts * sizeof(float), (void*)((attr->_offset) * sizeof(float)));
					glVertexAttribDivisor(attr->_loc, 1);
				}
			}
			else {
				glEnableVertexAttribArray(attr->_loc);
				glVertexAttribPointer(attr->_loc, attr->_size, GL_FLOAT, GL_FALSE, buffer->_n_attrs_per_pts * sizeof(float), (void*)(attr->_offset * sizeof(float)));
			}
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glBindVertexArray(0);
}


GLDrawContext::~GLDrawContext() {
	for (auto & buffer : _buffers) {
		delete buffer;
	}
	_buffers.clear();
	for (auto & uniform : _uniforms) {
		delete uniform;
	}
	_uniforms.clear();
}


void GLDrawContext::set_data(float * data, uint idx_buffer) {
	if (_verbose) {
		std::cout << "GLDrawContext " << _name << " : set_data for buffer " << idx_buffer << "\n";
	}
	GLDrawContextBuffer * buffer = _buffers[idx_buffer];
	glBindBuffer(GL_ARRAY_BUFFER, buffer->_id);
	if (buffer->_is_instanced) {
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * _n_instances * buffer->_n_attrs_per_pts, data, buffer->_usage);
	}
	else {
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * _n_pts * buffer->_n_attrs_per_pts, data, buffer->_usage);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void GLDrawContext::clear_data(uint idx_buffer) {
	if (_verbose) {
		std::cout << "GLDrawContext " << _name << " : clear_data for buffer " << idx_buffer << "\n";
	}
	GLDrawContextBuffer * buffer = _buffers[idx_buffer];
	glBindBuffer(GL_ARRAY_BUFFER, buffer->_id);
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, buffer->_usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void GLDrawContext::activate() {
	if (!_active) {
		return;
	}

	if (_verbose) {
		std::cout << "GLDrawContext " << _name << " : activate\n";
	}

	glUseProgram(_prog);
	glBindVertexArray(_vao);

	for (auto & texture : _textures) {
		glActiveTexture(GL_TEXTURE0 + texture->_offset);
		glBindTexture(texture->_target, texture->_id);
		glActiveTexture(0);
		GLDrawContextUniform * uniform = get_uniform(texture->_name);
		glUniform1i(uniform->_loc, texture->_offset);
	}
}


void GLDrawContext::deactivate() {
	if (!_active) {
		return;
	}

	if (_verbose) {
		std::cout << "GLDrawContext " << _name << " : deactivate\n";
	}

	glBindVertexArray(0);
	glUseProgram(0);

	for (auto & texture : _textures) {
		glActiveTexture(GL_TEXTURE0 + texture->_offset);
		glBindTexture(texture->_target, 0);
		glActiveTexture(0);
	}
}


void GLDrawContext::draw() {
	if (!_active) {
		return;
	}

	if (_verbose) {
		std::cout << "GLDrawContext " << _name << " : draw\n";
	}

	bool is_instanced = false;
	for (auto & buffer : _buffers) {
		if (buffer->_is_instanced) {
			is_instanced = true;
			break;
		}
	}
	if (is_instanced) {
		glDrawArraysInstanced(_draw_mode, 0, _n_pts, _n_instances);
	}
	else {
		glDrawArrays(_draw_mode, 0, _n_pts);
	}
}


GLDrawContextUniform * GLDrawContext::get_uniform(std::string uniform_name) {
	for (auto & uniform : _uniforms) {
		if (uniform->_name == uniform_name) {
			return uniform;
		}
	}
	std::cerr << "GLDrawContext " << _name << " get_uniform : " << uniform_name << " non trouvé\n";
	return NULL;
}


void GLDrawContext::set_uniform(std::string uniform_name, float data) {
	if (!_active) {
		return;
	}

	GLDrawContextUniform * uniform = get_uniform(uniform_name);
	if (uniform->_type == GL_FLOAT) {
		glUniform1f(uniform->_loc, data);
	}
	else {
		std::cerr << "GLDrawContext::set_uniform : " << uniform_name << " bad type = " << uniform->_type << "\n";
	}
}


void GLDrawContext::set_uniform(std::string uniform_name, const float * data) {
	if (!_active) {
		return;
	}

	GLDrawContextUniform * uniform = get_uniform(uniform_name);
	if (uniform->_type == GL_FLOAT_VEC2) {
		glUniform2fv(uniform->_loc, 1, data);
	}
	else if (uniform->_type == GL_FLOAT_VEC3) {
		glUniform3fv(uniform->_loc, 1, data);
	}
	else if (uniform->_type == GL_FLOAT_VEC4) {
		glUniform4fv(uniform->_loc, 1, data);
	}
	else if (uniform->_type == GL_FLOAT_MAT3) {
		glUniformMatrix3fv(uniform->_loc, 1, false, data);
	}
	else if (uniform->_type == GL_FLOAT_MAT4) {
		glUniformMatrix4fv(uniform->_loc, 1, false, data);
	}
	else {
		std::cerr << "GLDrawContext::set_uniform : " << uniform_name << " bad type = " << uniform->_type << "\n";
	}
}


void GLDrawContext::set_uniform(std::string uniform_name, int data) {
	if (!_active) {
		return;
	}

	GLDrawContextUniform * uniform = get_uniform(uniform_name);
	if (uniform->_type == GL_INT) {
		glUniform1i(uniform->_loc, data);
	}
	else {
		std::cerr << "GLDrawContext::set_uniform : " << uniform_name << " bad type = " << uniform->_type << "\n";
	}
}


void GLDrawContext::set_uniform(std::string uniform_name, const int * data) {
	if (!_active) {
		return;
	}

	GLDrawContextUniform * uniform = get_uniform(uniform_name);
	if (uniform->_type == GL_INT_VEC2) {
		glUniform2iv(uniform->_loc, 1, data);
	}
	else if (uniform->_type == GL_INT_VEC3) {
		glUniform3iv(uniform->_loc, 1, data);
	}
	else if (uniform->_type == GL_INT_VEC4) {
		glUniform4iv(uniform->_loc, 1, data);
	}
	else {
		std::cerr << "GLDrawContext::set_uniform : " << uniform_name << " bad type = " << uniform->_type << "\n";
	}
}


void GLDrawContext::set_uniform(std::string uniform_name, uint data) {
	if (!_active) {
		return;
	}

	GLDrawContextUniform * uniform = get_uniform(uniform_name);
	if (uniform->_type == GL_UNSIGNED_INT) {
		glUniform1ui(uniform->_loc, data);
	}
	else {
		std::cerr << "GLDrawContext::set_uniform : " << uniform_name << " bad type = " << uniform->_type << "\n";
	}
}


void GLDrawContext::set_uniform(std::string uniform_name, const uint * data) {
	if (!_active) {
		return;
	}

	GLDrawContextUniform * uniform = get_uniform(uniform_name);
	if (uniform->_type == GL_UNSIGNED_INT_VEC2) {
		glUniform2uiv(uniform->_loc, 1, data);
	}
	else if (uniform->_type == GL_UNSIGNED_INT_VEC3) {
		glUniform3uiv(uniform->_loc, 1, data);
	}
	else if (uniform->_type == GL_UNSIGNED_INT_VEC4) {
		glUniform4uiv(uniform->_loc, 1, data);
	}
	else {
		std::cerr << "GLDrawContext::set_uniform : " << uniform_name << " bad type = " << uniform->_type << "\n";
	}
}


uint GLDrawContext::data_size(uint idx_buffer) {
	GLDrawContextBuffer * buffer = _buffers[idx_buffer];
	if (buffer->_is_instanced) {
		return _buffers[idx_buffer]->_n_attrs_per_pts * _n_instances;
	}
	else {
		return _buffers[idx_buffer]->_n_attrs_per_pts * _n_pts;
	}
}


void GLDrawContext::show_data(uint idx_buffer) {
	GLDrawContextBuffer * buffer = _buffers[idx_buffer];
	uint ds = data_size(idx_buffer);
	float * data = new float[ds];

	glBindBuffer(GL_ARRAY_BUFFER, buffer->_id);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, ds * sizeof(float), data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	for (uint i=0; i<ds; ++i) {
		if (i % _buffers[idx_buffer]->_n_attrs_per_pts == 0) {
			std::cout << "\n";
		}
		std::cout << data[i] << " ; ";
	}
	std::cout << "\n";

	delete[] data;
}


std::ostream & operator << (std::ostream & os, const GLDrawContext & dc) {
	os << "name = " << dc._name;
	os << " ; prog = " << dc._prog << " ; vao = " << dc._vao;
	os << " ; n_pts = " << dc._n_pts << " ; active = " << dc._active;
	os << "\n";

	os << "locs_uniform :\n";
	for (auto uniform : dc._uniforms) {
		os << "\t" << *uniform << "\n";
	}

	os << "buffers\n";
	for (auto & buffer : dc._buffers) {
		os << "\t" << *buffer << "\n";
	}

	return os;
}

// -------------------------------------------------------------------------------------------
GLDrawManager::GLDrawManager() {

}


GLDrawManager::GLDrawManager(std::string json_path) : _verbose(false) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	std::vector<std::string> shaders;
	if (js["add_dir_shaders"] != nullptr) {
		for (auto & dir_shader : js["add_dir_shaders"]) {
			std::vector<std::string> l = list_files(dir_shader, "vert");
			for (auto & shader_path : l) {
				std::pair<std::string, std::string> p = splitext(shader_path);
				shaders.push_back(p.first);
			}
		}
	}
	if (js["add_shaders"] != nullptr) {
		for (auto & shader : js["add_shaders"]) {
			shaders.push_back(shader);
		}
	}

	// obligé de faire ce vao temporaire pour valider les programmes
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	std::map<std::string, GLuint> progs;
	for (auto & shader : shaders) {
		std::string vert = shader + ".vert";
		std::string frag = shader + ".frag";
		std::string geom = shader + ".geom";
		if (!file_exists(vert) || !file_exists(frag)) {
			std::cerr << "GLDrawManager : " << vert << " et/ou " << frag << " n'existe pas.\n";
			return;
		}

		if (file_exists(geom)) {
			progs[basename(shader)]= create_prog(vert, frag, geom);
		}
		else {
			progs[basename(shader)]= create_prog(vert, frag);
		}
	}

	check_gl_error();

	for (json::iterator it = js["contexts"].begin(); it != js["contexts"].end(); ++it) {
		auto & context_name= it.key();
		auto & context_dic= it.value();

		std::string shader_name = context_dic["shader"];

		if (progs.count(shader_name) == 0) {
			std::cerr << "GLDrawManager : " << shader_name << " n'est pas un nom de shader existant.\n";
			return;
		}

		GLenum usage = GL_STATIC_DRAW;
		if (context_dic["usage"] != nullptr) {
			if (context_dic["usage"] == "GL_STATIC_DRAW") {
				usage = GL_STATIC_DRAW;
			}
			else if (context_dic["usage"] == "GL_DYNAMIC_DRAW") {
				usage = GL_DYNAMIC_DRAW;
			}
			else if (context_dic["usage"] == "GL_STREAM_DRAW") {
				usage = GL_STREAM_DRAW;
			}
			else {
				std::cerr << "GLDrawManager : usage = " << context_dic["usage"] << " non reconnu.\n";
				return;
			}
		}

		GLenum draw_mode;
		if (context_dic["mode"] == "GL_LINES") {
			draw_mode = GL_LINES;
		}
		else if (context_dic["mode"] == "GL_TRIANGLES") {
			draw_mode = GL_TRIANGLES;
		}
		else {
			std::cerr << "GLDrawManager : mode = " << context_dic["mode"] << " non reconnu.\n";
			return;
		}
		
		std::vector<GLDrawContextAttrib *> attribs = active_attribs(progs[shader_name]);
		
		std::vector<GLDrawContextBuffer *> buffers;

		GLDrawContextBuffer * default_buffer = new GLDrawContextBuffer(false, usage);
		// TODO : faire un usage différent pour chaque buffer ?
		buffers.push_back(default_buffer);

		if (context_dic["buffers"] != nullptr) {
			for (auto & buff : context_dic["buffers"]) {
				bool is_instanced = buff["instanced"];
				std::vector<std::string> attrs;
				for (auto & attr : buff["attrs"]) {
					attrs.push_back(attr);
				}
				
				GLDrawContextBuffer * buffer = new GLDrawContextBuffer(is_instanced, usage);
				for (auto & attr_name : attrs) {
					for (auto & attrib : attribs) {
						if (attrib->_name == attr_name) {
							attrib->_in_default_buffer = false;
							buffer->_attribs.push_back(attrib);
							break;
						}
					}
				}
				buffers.push_back(buffer);
			}
		}

		// tous les attributs qui n'ont pas été utilisé par un buffer sont attribués au buffer par défaut
		for (auto & attrib : attribs) {
			if (attrib->_in_default_buffer) {
				default_buffer->_attribs.push_back(attrib);
			}
		}
		
		bool active = true;
		if (context_dic["active"] != nullptr) {
			active = context_dic["active"];
		}

		_contexts.push_back(new GLDrawContext(context_name, progs[shader_name], draw_mode, buffers, active));
	}

	_texture_pool = new GLDrawTexturePool();
}


GLDrawManager::~GLDrawManager() {
	for (auto & context : _contexts) {
		delete context;
	}
	_contexts.clear();
	delete _texture_pool;
}


GLDrawContext * GLDrawManager::get_context(std::string context_name) {
	for (auto & context : _contexts) {
		if (context->_name == context_name) {
			return context;
		}
	}
	std::cerr << "GLDrawManager::get_context : " << context_name << " inconnu.\n";
	return NULL;
}


void GLDrawManager::set_data(std::string context_name, uint n_pts, float * data) {
	GLDrawContext * context = get_context(context_name);
	if (context == NULL) {
		return;
	}

	if (_verbose) {
		std::cout << "GLDrawManager : set_data for " << context_name << "\n";
	}

	context->_n_pts = n_pts;
	context->set_data(data);
}


void GLDrawManager::set_active(std::string context_name){
	GLDrawContext * context = get_context(context_name);
	if (context == NULL) {
		return;
	}

	if (_verbose) {
		std::cout << "GLDrawManager : set_active for " << context_name << "\n";
	}

	context->_active = true;
}


void GLDrawManager::set_inactive(std::string context_name){
	GLDrawContext * context = get_context(context_name);
	if (context == NULL) {
		return;
	}

	if (_verbose) {
		std::cout << "GLDrawManager : set_inactive for " << context_name << "\n";
	}

	context->_active = false;
}


void GLDrawManager::switch_active(std::string context_name) {
	GLDrawContext * context = get_context(context_name);
	if (context == NULL) {
		return;
	}

	if (_verbose) {
		std::cout << "GLDrawManager : switch_active for " << context_name << "\n";
	}

	if (context->_active) {
		context->_active = false;
	}
	else {
		context->_active = true;
	}
}


void GLDrawManager::add_texture(std::string name, GLenum target, uint offset, std::map<GLenum, int> params, int internal_format, glm::uvec3 size, GLenum format, GLenum type) {
	GLDrawTexture * texture = _texture_pool->add_texture(name, target, offset, params, internal_format, size, format, type);

	for (auto & context : _contexts) {
		for (auto & uniform : context->_uniforms) {
			if (uniform->_name == name) {
				context->_textures.push_back(texture);
				break;
			}
		}
	}
}


void GLDrawManager::set_texture_data(std::string name, void * data, uint depth, int width, int height) {
	GLDrawTexture * texture = _texture_pool->get_texture(name);
	texture->set_data(data, depth, width, height);
}


void GLDrawManager::set_texture_data(std::string name, std::vector<std::string> pngs) {
	GLDrawTexture * texture = _texture_pool->get_texture(name);
	texture->set_data(pngs);
}


void GLDrawManager::set_verbose(bool verbose) {
	_verbose = verbose;
	for (auto & context : _contexts) {
		context->_verbose = verbose;
	}
}


std::ostream & operator << (std::ostream & os, const GLDrawManager & gdm) {
	os << "contexts :\n";
	for (auto & context : gdm._contexts) {
		std::cout << *context << "\n";
	}
	os << "texture_pool:\n";
	os << *gdm._texture_pool;
	return os;
}
