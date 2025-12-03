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
#include "typedefs.h"


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


class Text3D {
public:
	Text3D();
	Text3D(std::string text, glm::vec3 pos, float scale, glm::vec4 color);
	~Text3D();

	std::string _text;
	glm::vec3 _pos;
	float _scale;
	glm::vec4 _color;
};




class Font {
public:
	Font();
	Font(std::map<std::string, GLuint> progs, std::string font_path, unsigned int font_size, ScreenGL * screengl, mat_4d * world2clip=NULL);
	void set_text(std::vector<Text> & texts);
	void set_text(Text & text);
	void set_text(std::vector<Text3D> & texts);
	void set_text(Text3D & text);
	void clear();
	void draw();
	void draw_3d();
	
	GLuint _texture_id;
	unsigned int _tex_size;
	std::map<std::string, DrawContext *> _contexts;
	std::map<char, Character> _characters;
	mat_4d _camera2clip;
	mat_4d * _world2clip;
	float _z;
};


#endif
