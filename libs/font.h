#ifndef FONT_H
#define FONT_H

#include <iostream>
#include <string>
#include <map>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>

#include "gl_utils.h"


// classe d'affichage de texte ; utilise la lib freetype
// cf https://learnopengl.com/In-Practice/Text-Rendering


struct Character {
	char _char; // which character
	glm::ivec2 _size;       // Size of glyph
	glm::ivec2 _bearing;    // Offset from baseline to left/top of glyph
	GLuint     _advance;    // Offset to advance to next glyph
	float _xoffset;

	friend std::ostream & operator << (std::ostream & os, const Character & c) {
		os << "char=" << c._char;
		os << " ; size=(" << c._size.x << " ; " << c._size.y << ")";
		os << " ; bearing=(" <<  c._bearing.x << " ; " << c._bearing.y << ")";
		os << " ; advance=" << c._advance;
		os << " ; xoffset=" << c._xoffset;
		return os;
	}
};


class Text {
public:
	Text();
	Text(std::string text, glm::vec2 pos, float scale, glm::vec4 color);
	~Text();

	std::string _text;
	glm::vec2 _pos;
	float _scale;
	glm::vec4 _color;
};


class Font {
public:
	Font();
	Font(GLuint prog_draw, std::string font_path, unsigned int font_size, ScreenGL * screengl, unsigned int n_text_groups = 1);
	void set_text_group(unsigned int idx_text_group, std::vector<Text> & texts);
	void set_text_group(unsigned int idx_text_group, Text & text);
	void clear();
	void draw();
	
	GLuint _font_texture;
	std::map<char, Character> _characters;
	float _projection[16];
	GLuint * _vbos;
	GLuint _prog_draw;
	GLint _vertex_loc, _color_loc, _projection_loc;
	
	unsigned int _tex_width;
	unsigned int _tex_height;
	ScreenGL * _screengl;
	unsigned int _n_text_groups;
	unsigned int * _n_chars;
};


#endif
