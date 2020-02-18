#include "gl_utils.h"

using namespace std;


void _check_gl_error(const char * file, int line){
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


// Verif que le link des shaders de prog s'est bien déroulé
void check_gl_program(GLuint prog) {
	GLint status;

	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status== GL_FALSE) {
		int loglen;
		char logbuffer[1000];
		glGetProgramInfoLog(prog, sizeof(logbuffer), &loglen, logbuffer);
		cout << "OpenGL Program Linker Error " << loglen << " ; " << logbuffer << endl;
	}
	else {
		int loglen;
		char logbuffer[1000];
		glGetProgramInfoLog(prog, sizeof(logbuffer), &loglen, logbuffer);
		if (loglen> 0) {
			cout << "OpenGL Program Linker OK " << loglen << " ; " << logbuffer << endl;
		}
		glValidateProgram(prog);
		glGetProgramInfoLog(prog, sizeof(logbuffer), &loglen, logbuffer);
		if (loglen> 0) {
			cout << "OpenGL Program Validation results " << loglen << " ; " << logbuffer << endl;
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


// Remplissage d'une chaine de caractères avec le contenu du fichier d'un shader
char * load_source(const char * filename) {
	char *src= NULL;   /* code source de notre shader */
	FILE *fp= NULL;	   /* fichier */
	long size;			/* taille du fichier */
	long i;				/* compteur */
	
	char filename_complet[512];

	absolute_path(filename, filename_complet);
	
	/* on ouvre le fichier */
	fp= fopen(filename_complet, "r");
	/* on verifie si l'ouverture a echoue */
	if (fp== NULL) {
		cout << "impossible d'ouvrir le fichier " << filename << endl;
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
		cout << "erreur d'allocation de memoire" << endl;
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
		cout << "impossible de creer le shader" << endl;
		return 0;
	}
	
	/* chargement du code source */
	src= load_source(filename);
	if (src== NULL) {
		cout << "Le fichier de shader n'existe pas" << endl;
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
			cout <<	 "impossible d'allouer de la memoire" << endl;
			return 0;
		}
		/* initialisation du contenu */
		memset(log, '\0', logsize + 1);
		
		glGetShaderInfoLog(shader, logsize, &logsize, log);
		cout << "impossible de compiler le shader" << filename << " : " << log << endl;
		
		/* ne pas oublier de liberer la memoire et notre shader */
		free(log);
		glDeleteShader(shader);
		
		return 0;
	}
	
	return shader;
}


unsigned int load_cube_map(vector<string> faces) {
	unsigned int texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
	
	for (unsigned int i=0; i<faces.size(); ++i) {
    	SDL_Surface *surface = IMG_Load(faces[i].c_str());
		if (!surface) {
			cout << "IMG_Load error :" << IMG_GetError() << endl;
			continue;
		}
    	// sais pas pourquoi mais GL_BGRA fonctionne mieux que GL_RGBA
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, surface->w, surface->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, surface->pixels);
		SDL_FreeSurface(surface);
	}
    
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	return texture_id;
}


GLuint create_prog(string vs_path, string fs_path) {
	GLuint vs= load_shader(GL_VERTEX_SHADER  , vs_path.c_str());
	GLuint fs= load_shader(GL_FRAGMENT_SHADER, fs_path.c_str());
	GLuint prog= glCreateProgram();
	glAttachShader(prog, fs);
	glAttachShader(prog, vs);
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
