#include <cstdlib>
#include <iostream>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "font.h"

using namespace std;


Font::Font() {

}


Font::Font(GLuint prog_draw, string font_path, unsigned int font_size, ScreenGL * screengl, unsigned int n_texts) :
	_prog_draw(prog_draw), _screengl(screengl), _n_texts(n_texts)
{
	if (FT_Init_FreeType(&_ft_lib)) {
		cout << "Could not init FreeType Library" << endl;
		return;
	}

	if (FT_New_Face(_ft_lib, font_path.c_str(), 0, &_face)) {
		cout << "Failed to load font" << endl;
		return;
	}

	FT_Set_Pixel_Sizes(_face, 0, font_size);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

	_tex_width= 0.0f;
	_tex_height= 0.0f;
	for (GLubyte c=32; c<128; c++) {
		if (FT_Load_Char(_face, c, FT_LOAD_RENDER)) {
			cout << "Failed to load Glyph" << endl;
			continue;
		}

		_tex_width+= _face->glyph->bitmap.width;
		_tex_height= std::max(_tex_height, _face->glyph->bitmap.rows);
	}
	//cout << _tex_width << " ; " << _tex_height << "\n";

	glGenTextures(1, &_font_texture);
	glBindTexture(GL_TEXTURE_2D, _font_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _tex_width, _tex_height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned int x= 0;
	for (GLubyte c=32; c<128; c++) {
		if (FT_Load_Char(_face, c, FT_LOAD_RENDER)) {
			cout << "Failed to load Glyph" << endl;
			continue;
		}

		Character character = {
			glm::ivec2(_face->glyph->bitmap.width, _face->glyph->bitmap.rows),
			glm::ivec2(_face->glyph->bitmap_left, _face->glyph->bitmap_top),
			static_cast<GLuint>(_face->glyph->advance.x),
			(float)(x)/ (float)(_tex_width)
		};
		_characters.insert(std::pair<GLchar, Character>(c, character));

		//glTexSubImage2D(GL_TEXTURE_2D, 0, characters[c]._offset.x, characters[c]._offset.y, characters[c]._size.x, characters[c]._size.y, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, _characters[c]._size.x, _characters[c]._size.y, GL_RED, GL_UNSIGNED_BYTE, _face->glyph->bitmap.buffer);

		x+= _characters[c]._size.x;
	}
	
	FT_Done_Face(_face);
	FT_Done_FreeType(_ft_lib);
	
	glm::mat4 glm_projection= glm::ortho(0.0f, float(_screengl->_screen_width), 0.0f, float(_screengl->_screen_height));
	//cout << glm::to_string(glm_projection);
	memcpy(_projection, glm::value_ptr(glm_projection), sizeof(float)* 16);
	
	glUseProgram(_prog_draw);
	_vertex_loc= glGetAttribLocation(_prog_draw, "vertex");
	_color_loc= glGetAttribLocation(_prog_draw, "color_vertex");
	_projection_loc= glGetUniformLocation(_prog_draw, "projection");
	//_alpha_loc= glGetUniformLocation(_prog_draw, "alpha");
	//cout << _vertex_loc << " ; " << _text_color_loc << " ; " << _projection_loc << " ; " << _alpha_loc << "\n";
	glUseProgram(0);

	_n_chars= new unsigned int[_n_texts];

	_vbos= new GLuint[_n_texts];
	glGenBuffers(_n_texts, _vbos);

	/*glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* _n_chars_max* 6* 8, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);*/
	for (unsigned int idx_text=0; idx_text<_n_texts; ++idx_text) {
		_n_chars[idx_text]= 0;
		glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_text]);
		glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}


void Font::set_text(std::string text, unsigned int i, unsigned int j, float scale, glm::vec4 color, unsigned int idx_text) {
	if (idx_text>= _n_texts) {
		cerr << "Font idx_text trop grand\n";
		return;
	}

	_n_chars[idx_text]= text.size();
	float vertices[_n_chars[idx_text]* 6* 8];

	unsigned int idx= 0;
	for (string::const_iterator c=text.begin(); c!=text.end(); c++) {
		Character ch= _characters[*c];

		float xpos= (float)(i)+ (float)(ch._bearing.x)* scale;
		float ypos= (float)(j)- (float)(ch._size.y- ch._bearing.y)* scale;

		float w= (float)(ch._size.x)* scale;
		float h= (float)(ch._size.y)* scale;

		float tex_w= (float)(ch._size.x)/ (float)(_tex_width);

		//cout << idx << " ; " << xpos << " ; " << ypos << " ; " << w << " ; " << h << " ; " << ch._xoffset << " ; " << tex_w << "\n";

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
			vertices[idx* 6* 8+ k* 8+ 4]= color.r;
			vertices[idx* 6* 8+ k* 8+ 5]= color.g;
			vertices[idx* 6* 8+ k* 8+ 6]= color.b;
			vertices[idx* 6* 8+ k* 8+ 7]= color.a;
		}

		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		i+= (ch._advance >> 6)* scale; // Bitshift by 6 to get value in pixels (2^6 = 64)

		idx++;
	}
	/*for (int i=0; i<6; ++i) {
		for (int j=0; j<4; ++j) {
			cout << vertices[4* i+ j] << " ; ";
		}
		cout << "\n";
	}*/

	/*int idx_text= -1;
	for (unsigned int k=0; k<_texts.size(); ++k) {
		if ((_texts[k]._i== i) && (_texts[k]._j== j)) {
			idx_text= k;
			break;
		}
	}
	if (idx_text< 0) {
		_n_chars+= text.size();
		if (_n_chars> _n_chars_max) {
			cerr << "Nein trop de texte !\n";
		}
		Text txt= {i, j, };
		_texts.push_back(txt);
		idx_text= _texts.size()- 1;
	}
*/

	glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_text]);
	//glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)* _texts[idx_text]._buffer_offset, sizeof(vertices), vertices);
	glBufferData(GL_ARRAY_BUFFER, _n_chars[idx_text]* 6* 8* sizeof(GLfloat), vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Font::clear() {
	for (unsigned int idx_text=0; idx_text<_n_texts; ++idx_text) {
		_n_chars[idx_text]= 0;
		glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_text]);
		glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}


void Font::draw() {
	glUseProgram(_prog_draw);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, _font_texture);

	for (unsigned int idx_text=0; idx_text<_n_texts; ++idx_text) {
		glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_text]);

		//glUniform3f(_text_color_loc, color.x, color.y, color.z);
		//glUniform3f(_text_color_loc, 1.0f, 0.0f, 0.0f);
		glUniformMatrix4fv(_projection_loc, 1, GL_FALSE, _projection);
		//glUniform1f(_alpha_loc, _alpha);

		glEnableVertexAttribArray(_vertex_loc);
		glVertexAttribPointer(_vertex_loc, 4, GL_FLOAT, GL_FALSE, 8* sizeof(GLfloat), 0);
		glVertexAttribPointer(_color_loc, 4, GL_FLOAT, GL_FALSE, 8* sizeof(GLfloat), (void*)(4* sizeof(float)));
		glDrawArrays(GL_TRIANGLES, 0, _n_chars[idx_text]* 6);
		glDisableVertexAttribArray(_vertex_loc);
		glDisableVertexAttribArray(_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}	
	
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

