#include <iostream>
#include <cstdlib>
#include <sstream>
#include <fstream>

#include <SDL2/SDL_image.h>

#include "json.hpp"

#include "utile.h"
#include "gl_utils.h"


using json = nlohmann::json;


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


// --------------------------------------------------------
std::ostream & operator << (std::ostream & os, const DrawContextAttrib & dca) {
	os << "name = " << dca._name;
	os << " ; loc = " << dca._loc << " ; size = " << dca._size << " ; offset = " << dca._offset;

	return os;
}


// --------------------------------------------------------
std::ostream & operator << (std::ostream & os, const DrawContextBuffer & dcb) {
	os << "id = " << dcb._id << " ; n_attrs_per_pts = " << dcb._n_attrs_per_pts << " ; usage = " << dcb._usage;
	os  << " ; is_instanced = " << dcb._is_instanced;
	os << "\n\t";
	os << "attribs :\n";
	for (auto attrib : dcb._attribs) {
		os << "\t\t" << attrib << "\n";
	}
	return os;
}


// --------------------------------------------------------
DrawContext::DrawContext() {

}


DrawContext::DrawContext(GLuint prog, std::vector<std::string> locs_attrib, std::vector<std::string> locs_uniform, GLenum usage, bool active) :
	_prog(prog), _n_pts(0), _active(active), _n_instances(0)
{
	for (auto loc : locs_uniform) {
		_locs_uniform[loc]= glGetUniformLocation(_prog, loc.c_str());
	}

	DrawContextBuffer buffer;
	buffer._n_attrs_per_pts = 0;
	buffer._usage = usage; // TODO : faire un usage différent pour chaque buffer ?
	buffer._is_instanced = false;

	uint offset = 0;
	
	for (auto loc : locs_attrib) {
		DrawContextAttrib attrib;
		std::stringstream ss(loc);
		std::string s_size, s_option;
		std::getline(ss, attrib._name, ':');
		std::getline(ss, s_size, ':');
		attrib._loc = glGetAttribLocation(_prog, attrib._name.c_str());
		attrib._size = std::stoul(s_size);
		std::getline(ss, s_option, ':');

		if (s_option == "new_buffer") {
			_buffers.push_back(buffer);
			buffer._attribs.clear();
			buffer._n_attrs_per_pts = 0;
			buffer._is_instanced = false;
			offset = 0;
		}
		
		else if (s_option == "new_instanced_buffer") {
			_buffers.push_back(buffer);
			buffer._attribs.clear();
			buffer._n_attrs_per_pts = 0;
			buffer._is_instanced = true;
			offset = 0;
		}

		attrib._offset = offset;
		offset += attrib._size;
		buffer._n_attrs_per_pts += attrib._size;
		
		buffer._attribs.push_back(attrib);
	}
	_buffers.push_back(buffer);

	for (auto & buffer : _buffers) {
		glGenBuffers(1, &buffer._id);
	}
	glGenVertexArrays(1, &_vao);

	glBindVertexArray(_vao);
	
	for (auto & buffer : _buffers) {
		glBindBuffer(GL_ARRAY_BUFFER, buffer._id);
		for (auto attr : buffer._attribs) {
			if (buffer._is_instanced) {
				if (attr._size == 16) {
					for (uint i=0; i<4; ++i) {
						glEnableVertexAttribArray(attr._loc + i);
						glVertexAttribPointer(attr._loc + i, 4, GL_FLOAT, GL_FALSE, buffer._n_attrs_per_pts * sizeof(float), (void*)((attr._offset + i * 4) * sizeof(float)));
						glVertexAttribDivisor(attr._loc + i, 1);
					}
				}
				else {
					glEnableVertexAttribArray(attr._loc);
					glVertexAttribPointer(attr._loc, attr._size, GL_FLOAT, GL_FALSE, buffer._n_attrs_per_pts * sizeof(float), (void*)((attr._offset) * sizeof(float)));
					glVertexAttribDivisor(attr._loc, 1);
				}
			}
			else {
				glEnableVertexAttribArray(attr._loc);
				glVertexAttribPointer(attr._loc, attr._size, GL_FLOAT, GL_FALSE, buffer._n_attrs_per_pts * sizeof(float), (void*)(attr._offset * sizeof(float)));
			}
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glBindVertexArray(0);
}


DrawContext::~DrawContext() {

}


void DrawContext::set_data(float * data, uint idx_buffer) {
	DrawContextBuffer buffer = _buffers[idx_buffer];
	glBindBuffer(GL_ARRAY_BUFFER, buffer._id);
	if (buffer._is_instanced) {
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * _n_instances * buffer._n_attrs_per_pts, data, buffer._usage);
	}
	else {
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * _n_pts * buffer._n_attrs_per_pts, data, buffer._usage);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void DrawContext::clear_data(uint idx_buffer) {
	DrawContextBuffer buffer = _buffers[idx_buffer];
	glBindBuffer(GL_ARRAY_BUFFER, buffer._id);
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, buffer._usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void DrawContext::activate() {
	glUseProgram(_prog);
	glBindVertexArray(_vao);
}


void DrawContext::deactivate() {
	glBindVertexArray(0);
	glUseProgram(0);
}


uint DrawContext::data_size(uint idx_buffer) {
	return _buffers[idx_buffer]._n_attrs_per_pts * _n_pts;
}


void DrawContext::show_data(uint idx_buffer) {
	uint ds = data_size(idx_buffer);
	float * data = new float[ds];
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, ds * sizeof(float), data);
	for (uint i=0; i<ds; ++i) {
		if (i % _buffers[idx_buffer]._n_attrs_per_pts == 0) {
			std::cout << "\n";
		}
		std::cout << data[i] << " ; ";
	}
	std::cout << "\n";
	delete[] data;
}


std::ostream & operator << (std::ostream & os, const DrawContext & dc) {
	os << "prog = " << dc._prog << " ; vao = " << dc._vao;
	os << " ; n_pts = " << dc._n_pts << " ; active = " << dc._active;
	os << "\n";

	os << "locs_uniform :\n";
	for (auto unif : dc._locs_uniform) {
		os << "\t" << unif.first << " -> " << unif.second << "\n";
	}

	os << "buffers\n";
	for (auto & buffer : dc._buffers) {
		os << "\t" << buffer << "\n";
	}

	return os;
}

// -------------------------------------------------------------------------------------------
GLDrawManager::GLDrawManager() {

}


GLDrawManager::GLDrawManager(std::string json_path) {
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

		bool found_shader = false;
		for (json::iterator it2 = js["shaders"].begin(); it2 != js["shaders"].end(); ++it2) {
			auto & shader_name= it2.key();
			auto & shader_dic= it2.value();

			if (progs.count(shader_name) == 0) {
				std::cerr << "GLDrawManager : " << shader_name << " n'est pas un nom de shader existant.\n";
				return;
			}

			if (shader_name == context_dic["shader"]) {
				found_shader = true;

				std::vector<std::string> locs_attrib;
				for (auto & loc : shader_dic["locs"]) {
					locs_attrib.push_back(loc);
				}
				
				std::vector<std::string> locs_uniform;
				for (auto & loc : shader_dic["uniforms"]) {
					locs_uniform.push_back(loc);
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

				bool active = true;
				if (context_dic["active"] != nullptr) {
					active = context_dic["active"];
				}

				_contexts[context_name] = new DrawContext(progs[shader_name], locs_attrib, locs_uniform, usage, active);

				break;
			}
		}
		if (!found_shader) {
			std::cerr << "GLDrawManager : " << context_name << " : shader " << context_dic["shader"] << " non trouvé.\n";
			return;
		}
	}
}


GLDrawManager::~GLDrawManager() {
	for (auto & context : _contexts) {
		delete context.second;
	}
	_contexts.clear();
}


DrawContext * GLDrawManager::get_context(std::string context_name) {
	if (_contexts.count(context_name) == 0) {
		std::cerr << "GLDrawManager::get_context : " << context_name << " inconnu.\n";
		return NULL;
	}
	return _contexts[context_name];
}


void GLDrawManager::set_data(std::string context_name, uint n_pts, float * data) {
	DrawContext * context = get_context(context_name);
	if (context == NULL) {
		return;
	}

	context->_n_pts = n_pts;
	context->set_data(data);
}


void GLDrawManager::set_active(std::string context_name){
	DrawContext * context = get_context(context_name);
	if (context == NULL) {
		return;
	}

	context->_active = true;
}


void GLDrawManager::set_inactive(std::string context_name){
	DrawContext * context = get_context(context_name);
	if (context == NULL) {
		return;
	}

	context->_active = false;
}


void GLDrawManager::switch_active(std::string context_name) {
	DrawContext * context = get_context(context_name);
	if (context == NULL) {
		return;
	}

	if (context->_active) {
		context->_active = false;
	}
	else {
		context->_active = true;
	}
}


std::ostream & operator << (std::ostream & os, const GLDrawManager & gdm) {
	for (auto & context : gdm._contexts) {
		std::cout << context.first << " : " << *context.second << "\n";
	}
	return os;
}


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
		std::cout << location << " ; " << name << "\n";
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
		std::cout << location << " ; " << name << "\n";
	}
	free(name);
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


// gestion multi-fenetre
void set_subwindow(const float bkgnd_color[4], int x, int y, int w, int h) {
	glClearColor(bkgnd_color[0], bkgnd_color[1], bkgnd_color[2], bkgnd_color[3]);
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, y, w, h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
	glViewport(x, y, w, h);
}


// export dans fichier .pgm d'une texture en niveaux de gris
void export_texture2pgm(std::string pgm_path, uint width, uint height) {
	unsigned char * pixels= new unsigned char[width* height];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
	FILE *f;
	f= fopen(pgm_path.c_str(), "wb");
	fprintf(f, "P5\n%d %d\n%d\n", width, height, 255);
	for (int i=0; i<height; ++i) {
		fwrite(pixels+ i* width, 1, width, f);
	}
	fclose(f);
	delete[] pixels;
}


void export_texture_array2pgm(std::string pgm_dir_path, uint width, uint height, uint depth) {
	unsigned char * pixels= new unsigned char[width* height* depth];
	glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
	for (uint d=0; d<depth; ++d) {
		std::string pgm_path= pgm_dir_path+ "/tex_array_"+ std::to_string(d)+ ".pgm";
		FILE *f;
		f= fopen(pgm_path.c_str(), "wb");
		fprintf(f, "P5\n%d %d\n%d\n", width, height, 255);
		for (int i=0; i<height; ++i) {
			fwrite(pixels+ d* width* height+ i* width, 1, width, f);
		}
		fclose(f);
	}
	delete[] pixels;
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


void fill_texture_array(uint texture_offset, uint texture_idx, uint texture_size, std::vector<std::string> pngs) {
	glActiveTexture(GL_TEXTURE0 + texture_offset);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture_idx);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, texture_size, texture_size, pngs.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	for (uint idx_png=0; idx_png<pngs.size(); ++idx_png) {
		// parfois on veut sauter des indices, auquel cas on spécifie NO_PNG
		if (pngs[idx_png]== NO_PNG) {
			continue;
		}

		if (!file_exists(pngs[idx_png])) {
			std::cout << "png=" << pngs[idx_png] << " n'existe pas\n";
			return;
		}
		SDL_Surface * surface = IMG_Load(pngs[idx_png].c_str());
		if (!surface) {
			std::cout << "IMG_Load error :" << IMG_GetError() << "\n";
			return;
		}

		// sais pas pourquoi mais GL_BGRA fonctionne mieux que GL_RGBA
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
						0,                             // mipmap number
						0, 0, idx_png,   // xoffset, yoffset, zoffset
						texture_size, texture_size, 1, // width, height, depth
						GL_BGRA,                       // format
						GL_UNSIGNED_BYTE,              // type
						surface->pixels);              // pointer to data

		SDL_FreeSurface(surface);
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);

	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

