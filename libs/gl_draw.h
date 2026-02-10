#ifndef GL_DRAW_H
#define GL_DRAW_H

#include <string>
#include <vector>
#include <map>

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>

#include "typedefs.h"


// utilisé dans fill_texture_array lorsque l'on veut sauter des indices
const std::string NO_PNG= "NO_PNG";


struct GLDrawContextUniform;
struct GLDrawContextAttrib;


void _check_gl_error(const char * file, int line);
#define check_gl_error() _check_gl_error(__FILE__,__LINE__)
void check_gl_program(GLuint prog);
char * load_source(const char * filename);
GLuint load_shader(GLenum type, const char * filename);
GLuint create_prog(std::string vs_path, std::string fs_path, std::string gs_path="", bool check_program=true);
std::vector<GLDrawContextUniform *> active_uniforms(GLuint prog);
std::vector<GLDrawContextAttrib *> active_attribs(GLuint prog);


struct GLDrawTexture {
	GLDrawTexture();
	GLDrawTexture(std::string name, GLenum target, uint offset, std::map<GLenum, int> params, int internal_format, glm::uvec3 size, GLenum format, GLenum type);
	~GLDrawTexture();
	void set_data(void * data, uint depth, int width = -1, int height = -1);
	void set_data(std::vector<std::string> pngs);
	void export2pgm(std::string pgm_path);
	friend std::ostream & operator << (std::ostream & os, const GLDrawTexture & tex);


	std::string _name;
	GLenum _target;
	uint _id;
	uint _offset;
	std::map<GLenum, int> _params;
	int _internal_format;
	glm::uvec3 _size;
	GLenum _format;
	GLenum _type;
};


struct GLDrawTexturePool {
	GLDrawTexturePool();
	~GLDrawTexturePool();
	GLDrawTexture * get_texture(std::string name);
	GLDrawTexture * add_texture(std::string name, GLenum target, uint offset, std::map<GLenum, int> params, int internal_format, glm::uvec3 size, GLenum format, GLenum type);
	friend std::ostream & operator << (std::ostream & os, const GLDrawTexturePool & texpool);


	std::vector<GLDrawTexture *> _textures;
};


struct GLDrawContextAttrib {
	GLDrawContextAttrib();
	GLDrawContextAttrib(std::string name, GLint loc, uint size, GLenum type);
	~GLDrawContextAttrib();
	friend std::ostream & operator << (std::ostream & os, const GLDrawContextAttrib & dca);


	std::string _name;
	GLint _loc;
	uint _size;
	uint _offset;
	bool _in_default_buffer;
};


struct GLDrawContextUniform {
	GLDrawContextUniform();
	GLDrawContextUniform(std::string name, GLint loc, GLenum type, GLint size);
	~GLDrawContextUniform();
	friend std::ostream & operator << (std::ostream & os, const GLDrawContextUniform & dcu);


	std::string _name;
	GLint _loc;
	GLenum _type;
	GLint _size;
};


struct GLDrawContextBuffer {
	GLDrawContextBuffer();
	GLDrawContextBuffer(bool is_instanced, GLenum usage);
	~GLDrawContextBuffer();
	friend std::ostream & operator << (std::ostream & os, const GLDrawContextBuffer & dcb);


	GLuint _id;
	std::vector<GLDrawContextAttrib *> _attribs;
	uint _n_attrs_per_pts;
	bool _is_instanced;
	GLenum _usage;
};


class GLDrawContext {
public:
	GLDrawContext();
	GLDrawContext(std::string name, GLuint prog, GLenum draw_mode, std::vector<GLDrawContextBuffer *> buffers, bool active = true);
	~GLDrawContext();
	void set_data(float * data, uint idx_buffer = 0);
	void clear_data(uint idx_buffer = 0);
	void activate();
	void deactivate();
	void draw();
	GLDrawContextUniform * get_uniform(std::string uniform_name);
	void set_uniform(std::string uniform_name, float data);
	void set_uniform(std::string uniform_name, const float * data);
	void set_uniform(std::string uniform_name, int data);
	void set_uniform(std::string uniform_name, const int * data);
	void set_uniform(std::string uniform_name, uint data);
	void set_uniform(std::string uniform_name, const uint * data);
	uint data_size(uint idx_buffer = 0);
	void show_data(uint idx_buffer = 0);
	friend std::ostream & operator << (std::ostream & os, const GLDrawContext & dc);


	std::string _name;
	GLuint _prog;
	GLuint _vao;
	std::vector<GLDrawContextUniform *> _uniforms;
	std::vector<GLDrawContextBuffer *> _buffers;
	std::vector<GLDrawTexture *> _textures;
	uint _n_pts;
	uint _n_instances;
	bool _active;
	GLenum _draw_mode;
};


class GLDrawManager {
public:
	GLDrawManager();
	GLDrawManager(std::string json_path);
	~GLDrawManager();
	GLDrawContext * get_context(std::string context_name);
	void set_data(std::string context_name, uint n_pts, float * data);
	void set_active(std::string context_name);
	void set_inactive(std::string context_name);
	void switch_active(std::string context_name);
	void add_texture(std::string name, GLenum target, uint offset, std::map<GLenum, int> params, int internal_format, glm::uvec3 size, GLenum format, GLenum type);
	void set_texture_data(std::string name, void * data, uint depth, int width = -1, int height = -1);
	void set_texture_data(std::string name, std::vector<std::string> pngs);
	friend std::ostream & operator << (std::ostream & os, const GLDrawManager & gdm);


	std::vector<GLDrawContext *> _contexts;
	GLDrawTexturePool * _texture_pool;
};


#endif
