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
	unsigned char _char; // which character
	glm::ivec2 _size;       // Size of glyph
	glm::ivec2 _bearing;    // Offset from baseline to left/top of glyph
	GLuint     _advance;    // Offset to advance to next glyph

	friend std::ostream & operator << (std::ostream & os, const Character & c) {
		os << "char=" << c._char;
		os << " ; size=(" << c._size.x << " ; " << c._size.y << ")";
		os << " ; bearing=(" <<  c._bearing.x << " ; " << c._bearing.y << ")";
		os << " ; advance=" << c._advance;
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
	Font(GLuint prog_font, std::string font_path, unsigned int font_size, ScreenGL * screengl);
	void set_text(std::vector<Text> & texts);
	void set_text(Text & text);
	void clear();
	void draw();
	
	GLuint _texture_id;
	unsigned int _tex_size;
	DrawContext * _context;
	std::map<char, Character> _characters;
	glm::mat4 _camera2clip;
	float _z;
};


#endif
