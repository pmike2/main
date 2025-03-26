#include <iostream>
#include <cstdlib>

#include <SDL2/SDL_image.h>

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


pt_type ScreenGL::screen2gl(int i, int j) {
	number x, y;
	screen2gl(i, j, x, y);
	return pt_type(x, y);
}


void ScreenGL::gl2screen(number x, number y, int & i, int & j) {
	i= (int)((number)(_screen_width)* (x/ _gl_width+ 0.5));
	//j= (int)((number)(_screen_width)* (0.5- y/ _gl_height));
	j= (int)((number)(_screen_width)* (y/ _gl_height+ 0.5));
}


// --------------------------------------------------------
DrawContext::DrawContext() {

}


DrawContext::DrawContext(GLuint prog, GLuint buffer, std::vector<std::string> locs_attrib, std::vector<std::string> locs_uniform) :
	_prog(prog), _buffer(buffer), _n_pts(0), _n_attrs_per_pts(0)
{
	for (auto loc : locs_attrib) {
		_locs_attrib[loc]= glGetAttribLocation(_prog, loc.c_str());
	}
	for (auto loc : locs_uniform) {
		_locs_uniform[loc]= glGetUniformLocation(_prog, loc.c_str());
	}
}


DrawContext::~DrawContext() {

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
	
	//char filename_complet[512];
	//absolute_path(filename, filename_complet);
	//fp= fopen(filename_complet, "r");

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


GLuint create_prog(std::string vs_path, std::string fs_path, std::string gs_path) {
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
	check_gl_program(prog);

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
void export_texture2pgm(std::string pgm_path, unsigned int width, unsigned int height) {
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


void export_texture_array2pgm(std::string pgm_dir_path, unsigned int width, unsigned int height, unsigned int depth) {
	unsigned char * pixels= new unsigned char[width* height* depth];
	glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
	for (unsigned int d=0; d<depth; ++d) {
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


void export_screen_to_ppm(std::string ppm_path, unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
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


float * draw_cross(float * data, pt_type center, float size, glm::vec4 color) {
	data[0]= float(center.x)- size;
	data[1]= float(center.y)- size;
	data[6]= float(center.x)+ size;
	data[7]= float(center.y)+ size;
	data[12]= float(center.x)+ size;
	data[13]= float(center.y)- size;
	data[18]= float(center.x)- size;
	data[19]= float(center.y)+ size;
	
	for (unsigned int i=0; i<4; ++i) {
		for (unsigned int j=0; j<4; ++j) {
			data[i* 6+ 2+ j]= color[j];
		}
	}
	return data+ 24;
}


float * draw_arrow(float * data, pt_type start, pt_type end, float tip_size, float angle, glm::vec4 color) {
	bool start_is_end= false;
	if ((abs(start.x- end.x)< 1e-5) && (abs(start.y- end.y)< 1e-5)) {
		start_is_end= true;
	}
	
	if (start_is_end) {
		for (unsigned int i=0; i<6; ++i) {
			data[i* 6+ 0]= float(start.x);
			data[i* 6+ 1]= float(start.y);
		}
	}
	else {
		pt_type norm= glm::normalize(start- end);
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

	for (unsigned int i=0; i<6; ++i) {
		for (unsigned int j=0; j<4; ++j) {
			data[i* 6+ 2+ j]= color[j];
		}
	}
	return data+ 36;
}


float * draw_polygon(float * data, std::vector<pt_type> pts, glm::vec4 color) {
	for (unsigned int idx_pt=0; idx_pt<pts.size(); ++idx_pt) {
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

	for (unsigned int i=0; i<pts.size()* 2; ++i) {
		for (unsigned int j=0; j<4; ++j) {
			data[i* 6+ 2+ j]= color[j];
		}
	}

	return data+ 6* 2* pts.size();
}


float * draw_nothing(float * data, unsigned int n_attrs_per_pts, unsigned int n_pts) {
	unsigned int compt= 0;
	for (unsigned int idx_pt=0; idx_pt<n_pts; ++idx_pt) {
		for (unsigned int idx_attr=0; idx_attr<n_attrs_per_pts; ++idx_attr) {
			data[compt++]= 0.0;
		}
	}
	return data+ n_attrs_per_pts* n_pts;
}


void fill_texture_array(unsigned int texture_offset, unsigned int texture_idx, unsigned int texture_size, std::vector<std::string> pngs) {
	glActiveTexture(GL_TEXTURE0+ texture_offset);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture_idx);
	
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, texture_size, texture_size, pngs.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	for (unsigned int idx_png=0; idx_png<pngs.size(); ++idx_png) {
		// parfois on veut sauter des indices, auquel cas on spécifie NO_PNG
		if (pngs[idx_png]== NO_PNG) {
			continue;
		}

		if (!file_exists(pngs[idx_png])) {
			std::cout << "png=" << pngs[idx_png] << " n'existe pas\n";
			return;
		}
		SDL_Surface * surface= IMG_Load(pngs[idx_png].c_str());
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

