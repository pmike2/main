#include <cstdlib>
#include <iostream>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "font.h"

using namespace std;


Text::Text() {

}


Text::Text(string text, glm::vec2 pos, float scale, glm::vec4 color) :
	_text(text), _pos(pos), _scale(scale), _color(color)
{

}


Text::~Text() {

}


// ---------------------------------------------------------------------------------

Font::Font() {

}


Font::Font(GLuint prog_draw, string font_path, unsigned int font_size, ScreenGL * screengl, unsigned int n_text_groups) :
	_prog_draw(prog_draw), _screengl(screengl), _n_text_groups(n_text_groups)
{
	FT_Library ft_lib;
	FT_Face face;

	if (FT_Init_FreeType(&ft_lib)) {
		cout << "Could not init FreeType Library" << endl;
		return;
	}

	if (FT_New_Face(ft_lib, font_path.c_str(), 0, &face)) {
		cout << "Failed to load font" << endl;
		return;
	}

	FT_Set_Pixel_Sizes(face, 0, font_size);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

	// on crée une texture qui va contenir tous les chars dans une bande horizontale
	_tex_width= 0;
	_tex_height= 0;
	for (GLubyte c=32; c<128; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			cout << "Failed to load Glyph" << endl;
			continue;
		}

		// somme des largeurs
		_tex_width+= face->glyph->bitmap.width;
		// max des hauteurs
		_tex_height= std::max(_tex_height, face->glyph->bitmap.rows);
	}

	glGenTextures(1, &_font_texture);
	glBindTexture(GL_TEXTURE_2D, _font_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _tex_width, _tex_height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned int x= 0;
	for (GLubyte c=32; c<128; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			cout << "Failed to load Glyph" << endl;
			continue;
		}

		Character character = {
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			static_cast<GLuint>(face->glyph->advance.x),
			(float)(x)/ (float)(_tex_width)
		};
		_characters.insert(std::pair<GLchar, Character>(c, character));

		glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, _characters[c]._size.x, _characters[c]._size.y, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

		x+= _characters[c]._size.x;
	}

	// debug font ---------------------------------------------
	GLubyte * pixels= new GLubyte[_tex_width* _tex_height];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
	FILE *f;
	f= fopen("font.pgm", "wb");
	fprintf(f, "P5\n%d %d\n%d\n", _tex_width, _tex_height, 255);
	for (int i=0; i<_tex_height; ++i) {
		fwrite(pixels+ i* _tex_width, 1, _tex_width, f);
	}
	fclose(f);
	delete[] pixels;

	GLchar c= 'b';
	std::cout << "size=(" << _characters[c]._size.x << " ; " << _characters[c]._size.y << ")";
	std::cout << " ; bearing=(" <<  _characters[c]._bearing.x << " ; " << _characters[c]._bearing.y << ")";
	std::cout << " ; advance=" << _characters[c]._advance;
	std::cout << " ; xoffset=" <<_characters[c]._xoffset << "\n";
	// ---------------------------------------------------------
	
	glBindTexture(GL_TEXTURE_2D, 0);
	FT_Done_Face(face);
	FT_Done_FreeType(ft_lib);
	
	//glm::mat4 glm_projection= glm::ortho(0.0f, float(_screengl->_screen_width), 0.0f, float(_screengl->_screen_height));
	glm::mat4 glm_projection= glm::ortho(-_screengl->_gl_width* 0.5f, _screengl->_gl_width* 0.5f, -_screengl->_gl_height* 0.5f, _screengl->_gl_height* 0.5f);
	memcpy(_projection, glm::value_ptr(glm_projection), sizeof(float)* 16);
	
	glUseProgram(_prog_draw);
	_vertex_loc= glGetAttribLocation(_prog_draw, "vertex");
	_color_loc= glGetAttribLocation(_prog_draw, "color_vertex");
	_projection_loc= glGetUniformLocation(_prog_draw, "projection");
	glUseProgram(0);

	// pour chaque groupe de textes le nombre de caracteres à dessiner
	_n_chars= new unsigned int[_n_text_groups];

	// chaque groupe de textes a son VBO ce qui permet de ne mettre à jour qu'un groupe de texte à la fois
	_vbos= new GLuint[_n_text_groups];
	glGenBuffers(_n_text_groups, _vbos);

	for (unsigned int idx_text_group=0; idx_text_group<_n_text_groups; ++idx_text_group) {
		_n_chars[idx_text_group]= 0;
		glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_text_group]);
		glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}


void Font::set_text_group(unsigned int idx_text_group, vector<Text> & texts) {
	if (idx_text_group>= _n_text_groups) {
		cerr << "Font idx_text trop grand\n";
		return;
	}

	_n_chars[idx_text_group]= 0;
	for (auto & t : texts) {
		_n_chars[idx_text_group]+= t._text.size();
	}
	float vertices[_n_chars[idx_text_group]* 6* 8];

	unsigned int idx= 0;
	for (unsigned int idx_text=0; idx_text<texts.size(); ++idx_text) {
		float x= texts[idx_text]._pos.x;
		for (string::const_iterator c=texts[idx_text]._text.begin(); c!=texts[idx_text]._text.end(); c++) {
			Character ch= _characters[*c];

			//float xpos= x+ (float)(ch._bearing.x)* texts[idx_text]._scale;
			float xpos= x;
			//float ypos= texts[idx_text]._pos.y- (float)(ch._size.y- ch._bearing.y)* texts[idx_text]._scale;
			float ypos= texts[idx_text]._pos.y;

			float w= (float)(ch._size.x)* texts[idx_text]._scale;
			float h= (float)(ch._size.y)* texts[idx_text]._scale;

			float tex_w= (float)(ch._size.x)/ (float)(_tex_width);

			vertices[idx* 6* 8+ 0]= xpos;
			vertices[idx* 6* 8+ 1]= ypos+ h;
			vertices[idx* 6* 8+ 2]= ch._xoffset;
			vertices[idx* 6* 8+ 3]= 0.0f;

			vertices[idx* 6* 8+ 8]= xpos;
			vertices[idx* 6* 8+ 9]= ypos;
			vertices[idx* 6* 8+ 10]= ch._xoffset;
			vertices[idx* 6* 8+ 11]= 1.0f;

			vertices[idx* 6* 8+ 16]= xpos+ w;
			vertices[idx* 6* 8+ 17]= ypos;
			vertices[idx* 6* 8+ 18]= ch._xoffset+ tex_w;
			vertices[idx* 6* 8+ 19]= 1.0f;

			vertices[idx* 6* 8+ 24]= xpos;
			vertices[idx* 6* 8+ 25]= ypos+ h;
			vertices[idx* 6* 8+ 26]= ch._xoffset;
			vertices[idx* 6* 8+ 27]= 0.0f;

			vertices[idx* 6* 8+ 32]= xpos+ w;
			vertices[idx* 6* 8+ 33]= ypos;
			vertices[idx* 6* 8+ 34]= ch._xoffset+ tex_w;
			vertices[idx* 6* 8+ 35]= 1.0f;

			vertices[idx* 6* 8+ 40]= xpos+ w;
			vertices[idx* 6* 8+ 41]= ypos+ h;
			vertices[idx* 6* 8+ 42]= ch._xoffset+ tex_w;
			vertices[idx* 6* 8+ 43]= 0.0f;

			for (unsigned int k=0; k<6; ++k) {
				vertices[idx* 6* 8+ k* 8+ 4]= texts[idx_text]._color.r;
				vertices[idx* 6* 8+ k* 8+ 5]= texts[idx_text]._color.g;
				vertices[idx* 6* 8+ k* 8+ 6]= texts[idx_text]._color.b;
				vertices[idx* 6* 8+ k* 8+ 7]= texts[idx_text]._color.a;
			}

			// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x+= (float)(ch._advance >> 6)* texts[idx_text]._scale; // Bitshift by 6 to get value in pixels (2^6 = 64)

			idx++;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_text_group]);
	glBufferData(GL_ARRAY_BUFFER, _n_chars[idx_text_group]* 6* 8* sizeof(GLfloat), vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// raccourci lorsque l'on a qu'un seul texte à afficher
void Font::set_text_group(unsigned int idx_text_group, Text & text) {
	vector<Text> texts;
	texts.push_back(text);
	set_text_group(idx_text_group, texts);
}


void Font::clear() {
	for (unsigned int idx_text_group=0; idx_text_group<_n_text_groups; ++idx_text_group) {
		_n_chars[idx_text_group]= 0;
		glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_text_group]);
		glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}


void Font::draw() {
	glUseProgram(_prog_draw);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, _font_texture);

	for (unsigned int idx_text_group=0; idx_text_group<_n_text_groups; ++idx_text_group) {
		if (_n_chars[idx_text_group]== 0) {
			continue;
		}

		glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_text_group]);
		glUniformMatrix4fv(_projection_loc, 1, GL_FALSE, _projection);

		glEnableVertexAttribArray(_vertex_loc);
		glEnableVertexAttribArray(_color_loc);
		glVertexAttribPointer(_vertex_loc, 4, GL_FLOAT, GL_FALSE, 8* sizeof(GLfloat), 0);
		glVertexAttribPointer(_color_loc, 4, GL_FLOAT, GL_FALSE, 8* sizeof(GLfloat), (void*)(4* sizeof(float)));
		glDrawArrays(GL_TRIANGLES, 0, _n_chars[idx_text_group]* 6);
		glDisableVertexAttribArray(_vertex_loc);
		glDisableVertexAttribArray(_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}	
	
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

