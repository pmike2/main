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


// validation shaders
void _check_gl_error(const char * file, int line);
#define check_gl_error() _check_gl_error(__FILE__,__LINE__)
void check_gl_program(GLuint prog);

// chargement shader
char * load_source(const char * filename);
GLuint load_shader(GLenum type, const char * filename);

// création programme
GLuint create_prog(fs vs_path, fs fs_path, fs gs_path="");

// récupération des uniforms et des attributs d'un programme
std::vector<GLDrawContextUniform *> active_uniforms(GLuint prog);
std::vector<GLDrawContextAttrib *> active_attribs(GLuint prog);


// Texture
struct GLDrawTexture {
	GLDrawTexture();
	GLDrawTexture(std::string name, GLenum target, uint offset, std::map<GLenum, int> params, int internal_format, glm::uvec3 size, GLenum format, GLenum type);
	~GLDrawTexture();
	// set data ; dans le cas GL_TEXTURE_2D depth est ignoré
	void set_data(void * data, int depth = -1, int width = -1, int height = -1);
	void set_data(fs png);
	// set data avec une liste de PNGs, uniquement pour le cas GL_TEXTURE_2D_ARRAY
	void set_data(std::vector<fs> pngs);
	// export PGM pour debug
	void export2pgm(fs pgm_path);
	// print pour debug
	void print_data();
	friend std::ostream & operator << (std::ostream & os, const GLDrawTexture & tex);


	std::string _name; // nom
	GLenum _target; // target (GL_TEXTURE_2D, GL_TEXTURE_2D_ARRAY, ...)
	uint _id; // id
	uint _offset; // offset
	std::map<GLenum, int> _params; // paramètres
	int _internal_format; // format interne
	glm::uvec3 _size; // taille
	GLenum _format; // format
	GLenum _type; // type
};


// Pool de textures
// c'est _context2textures qui permet de savoir quelle texture est dispo dans quel contexte
// afin de gérer le cas où une même texture (même nom) doit être accessible par plusieurs contextes
// et le cas où plusieurs textures portant le même nom ne doivent être accessibles chacune que par un contexte
struct GLDrawTexturePool {
	GLDrawTexturePool();
	~GLDrawTexturePool();
	// get texture globale
	GLDrawTexture * get_texture(std::string texture_name);
	// get texture spécifique à un contexte
	GLDrawTexture * get_texture(std::string context_name, std::string texture_name);
	// ajout texture
	GLDrawTexture * add_texture(std::string name, GLenum target, uint offset, std::map<GLenum, int> params, int internal_format, glm::uvec3 size, GLenum format, GLenum type);
	// ajout texture spécifique à un contexte
	GLDrawTexture * add_texture(std::string context_name, std::string name, GLenum target, uint offset, std::map<GLenum, int> params, int internal_format, glm::uvec3 size, GLenum format, GLenum type);
	// ajout d'une texture à contexte
	void add_texture2context(std::string context_name, GLDrawTexture * texture);
	
	friend std::ostream & operator << (std::ostream & os, const GLDrawTexturePool & texpool);


	std::vector<GLDrawTexture *> _textures;
	std::map<std::string, std::vector<GLDrawTexture *> > _context2textures;
};


// ===============================================
// TODO : faire un pool de texture buffer comme GLDrawTexturePool 
// ===============================================

// Texture buffer ; https://wikis.khronos.org/opengl/Buffer_Texture
struct GLDrawTextureBuffer {
	GLDrawTextureBuffer();
	GLDrawTextureBuffer(std::string name, GLenum internal_format, uint offset);
	~GLDrawTextureBuffer();
	void set_data(void * data, uint size);


	std::string _name; // nom
	GLenum _internal_format; // format interne
	uint _offset; // offset
	uint _tex_id; // id texture
	uint _buf_id; // id buffer
};


// Attribut
struct GLDrawContextAttrib {
	GLDrawContextAttrib();
	GLDrawContextAttrib(std::string name, GLint loc, uint size, GLenum type);
	~GLDrawContextAttrib();
	friend std::ostream & operator << (std::ostream & os, const GLDrawContextAttrib & dca);


	std::string _name; // nom
	GLint _loc; // location
	uint _size; // taille
	uint _offset; // offset
	bool _in_default_buffer; // est t'il dans le buffer défaut
};


// Uniform
struct GLDrawContextUniform {
	GLDrawContextUniform();
	GLDrawContextUniform(std::string name, GLint loc, GLenum type, GLint size);
	~GLDrawContextUniform();
	friend std::ostream & operator << (std::ostream & os, const GLDrawContextUniform & dcu);


	std::string _name; // nom
	GLint _loc; // location
	GLenum _type; // type (mat4, vec2, ...)
	GLint _size; // taille
};


// Buffer de données
struct GLDrawContextBuffer {
	GLDrawContextBuffer();
	GLDrawContextBuffer(bool is_instanced, GLenum usage);
	~GLDrawContextBuffer();
	friend std::ostream & operator << (std::ostream & os, const GLDrawContextBuffer & dcb);


	GLuint _id; // id
	std::vector<GLDrawContextAttrib *> _attribs; // attributs
	uint _n_attrs_per_pts; // nombre d'attributs par point
	bool _is_instanced; // est-t'il instanced
	GLenum _usage; // usage (GL_STREAM_DRAW, ...)
};


// Contexte de dessin
struct GLDrawContext {
	GLDrawContext();
	GLDrawContext(std::string name, GLuint prog, GLenum draw_mode, std::vector<GLDrawContextBuffer *> buffers, GLDrawTexturePool * texture_pool, bool active = true);
	~GLDrawContext();
	void set_data(float * data, uint idx_buffer = 0); // set data d'un buffer
	void set_subdata(float * data, uint offset, uint size, uint idx_buffer = 0); // set subdata ; inutilisé car lent...
	void clear_data(uint idx_buffer = 0); // clear data d'un buffer
	bool empty(uint idx_buffer = 0); // est-ce qu'un buffer est vide
	void activate(); // activation contexte
	void deactivate(); // désactivation contexte
	void draw(); // dessin
	void validate(); // validation
	GLDrawContextUniform * get_uniform(std::string uniform_name); // récup uniform

	// set uniform
	void set_uniform(std::string uniform_name, float data);
	void set_uniform(std::string uniform_name, const float * data, uint count = 1);
	void set_uniform(std::string uniform_name, int data);
	void set_uniform(std::string uniform_name, const int * data, uint count = 1);
	void set_uniform(std::string uniform_name, uint data);
	void set_uniform(std::string uniform_name, const uint * data, uint count = 1);

	uint data_size(uint idx_buffer = 0); // taille d'un buffer
	void show_data(uint idx_buffer = 0); // print contenu buffer pour debug
	friend std::ostream & operator << (std::ostream & os, const GLDrawContext & dc);


	std::string _name; // nom
	GLuint _prog; // programme
	GLuint _vao; // VAO
	std::vector<GLDrawContextUniform *> _uniforms; // uniforms
	std::vector<GLDrawContextBuffer *> _buffers; // buffers
	std::vector<GLDrawTextureBuffer *> _texture_buffers; // texture buffers
	uint _n_pts; // nombre de points
	uint _n_instances; // nombre d'instances ; utilisé seulement si un des buffers est instanced
	bool _active; // est-til actif
	GLenum _draw_mode; // mode dessin (GL_TRIANGLES, GL_LINES, ...)
	bool _verbose; // verbosité
	GLDrawTexturePool * _texture_pool; // pool de textures (copie du pointeur de GLDrawManager)
};


// classe principale de gestion de dessin OpenGL
struct GLDrawManager {
	GLDrawManager();
	GLDrawManager(fs json_path);
	~GLDrawManager();
	
	GLDrawContext * get_context(std::string context_name); // récupération contexte
	void set_data(std::string context_name, uint n_pts, float * data, uint idx_buffer = 0); // set data d'un buffer d'un contexte
	void set_active(std::string context_name); // active le contexte
	void set_inactive(std::string context_name); // désactive le contexte
	void switch_active(std::string context_name); // switche l'activation du contexte
	void validate(); // valide les contextes ; à faire quand tout (les textures, ...) a été initialisé, à des fins de debug
	
	// ajout d'une texture potentiellement partagée parmi les contextes
	void add_texture(std::string texture_name, GLenum target, uint offset, std::map<GLenum, int> params, int internal_format, glm::uvec3 size, GLenum format, GLenum type);
	// ajout d'une texture pour un contexte spécifique
	void add_texture(std::string context_name, std::string texture_name, GLenum target, uint offset, std::map<GLenum, int> params, int internal_format, glm::uvec3 size, GLenum format, GLenum type);
	// set texture data pour une texture globale
	void set_texture_data(std::string texture_name, void * data, int depth = -1, int width = -1, int height = -1);
	// set texture data pour une texture spécifique à un contexte
	void set_texture_data(std::string context_name, std::string texture_name, void * data, int depth = -1, int width = -1, int height = -1);

	// pour un PNG, uniquement valide pour le cas GL_TEXTURE_2D
	void set_texture_data(std::string texture_name, fs png);
	void set_texture_data(std::string context_name, std::string texture_name, fs png);

	// la même chose avec des PNGs, uniquement valide pour le cas GL_TEXTURE_2D_ARRAY
	void set_texture_data(std::string texture_name, std::vector<fs> pngs);
	void set_texture_data(std::string context_name, std::string texture_name, std::vector<fs> pngs);

	// ajout texture buffer
	void add_texture_buffer(std::string context_name, std::string texture_buffer_name, GLenum internal_format, uint offset);
	// set texture buffer data
	void set_texture_buffer_data(std::string context_name, std::string texture_buffer_name, void * data, uint size);
	
	// gestion verbosité
	void set_verbose(bool verbose);
	friend std::ostream & operator << (std::ostream & os, const GLDrawManager & gdm);


	std::vector<GLDrawContext *> _contexts;
	GLDrawTexturePool * _texture_pool;
	//GLDrawTextureBufferPool * _texture_buffer_pool;
	bool _verbose;
};


#endif
