#ifndef FONT_H
#define FONT_H

#include <string>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>



// classe d'affichage de texte ; utilise la lib freetype
// cf https://learnopengl.com/In-Practice/Text-Rendering


struct Character {
	GLuint     _texture_id; // ID handle of the glyph texture
	glm::ivec2 _size;       // Size of glyph
	glm::ivec2 _bearing;    // Offset from baseline to left/top of glyph
	GLuint     _advance;    // Offset to advance to next glyph
};


class Font {
public:
	Font();
	Font(GLuint prog_draw, std::string font_path, unsigned int font_size, unsigned int screen_width, unsigned int screen_height);
	void draw(std::string text, float x, float y, float scale, glm::vec3 color);
	
	FT_Library _ft_lib;
	FT_Face _face;
	std::map<GLchar, Character> _characters;
	float _projection[16];
	GLuint _vbo;
	GLuint _prog_draw;
	GLint _vertex_loc, _text_color_loc, _projection_loc, _alpha_loc;
	float _alpha;
};


#endif
