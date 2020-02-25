
#include "font.h"

using namespace std;


Font::Font() {

}


Font::Font(GLuint prog_draw, string font_path, unsigned int font_size, unsigned int screen_width, unsigned int screen_height) : _prog_draw(prog_draw), _alpha(1.0f) {
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

	for (GLubyte c=0; c<128; c++) {
		// Load character glyph 
		if (FT_Load_Char(_face, c, FT_LOAD_RENDER)) {
			cout << "Failed to load Glyph" << endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _face->glyph->bitmap.width, _face->glyph->bitmap.rows, 0,
			GL_RED, GL_UNSIGNED_BYTE, _face->glyph->bitmap.buffer);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			glm::ivec2(_face->glyph->bitmap.width, _face->glyph->bitmap.rows),
			glm::ivec2(_face->glyph->bitmap_left, _face->glyph->bitmap_top),
			static_cast<GLuint>( _face->glyph->advance.x)
		};
		_characters.insert(std::pair<GLchar, Character>(c, character));
	}
	
	FT_Done_Face(_face);
	FT_Done_FreeType(_ft_lib);
	
	glm::mat4 glm_projection= glm::ortho(0.0f, float(screen_width), 0.0f, float(screen_height));
	memcpy(_projection, glm::value_ptr(glm_projection), sizeof(float)* 16);
	
	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* 6* 4, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(_prog_draw);
	_vertex_loc= glGetAttribLocation(_prog_draw, "vertex");
	_text_color_loc= glGetUniformLocation(_prog_draw, "text_color");
	_projection_loc= glGetUniformLocation(_prog_draw, "projection");
	_alpha_loc= glGetUniformLocation(_prog_draw, "alpha");
	glUseProgram(0);
}


void Font::draw(string text, float x, float y, float scale, glm::vec3 color) {
	glUseProgram(_prog_draw);
	glUniform3f(_text_color_loc, color.x, color.y, color.z);
	glUniformMatrix4fv(_projection_loc, 1, GL_FALSE, _projection);
	glUniform1f(_alpha_loc, _alpha);
	glActiveTexture(GL_TEXTURE0);

	string::const_iterator c;
	for (c=text.begin(); c!=text.end(); c++) {
		Character ch= _characters[*c];

		float xpos= x+ ch._bearing.x* scale;
		float ypos= y- (ch._size.y- ch._bearing.y)* scale;

		float w= ch._size.x* scale;
		float h= ch._size.y* scale;

		// Update VBO for each character
		float vertices[6][4]= {
			{ xpos,     ypos + h, 0.0f, 0.0f},
			{ xpos,     ypos,     0.0f, 1.0f},
			{ xpos + w, ypos,     1.0f, 1.0f},
			{ xpos,     ypos + h, 0.0f, 0.0f},
			{ xpos + w, ypos,     1.0f, 1.0f},
			{ xpos + w, ypos + h, 1.0f, 0.0f}
	};

		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch._texture_id);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glEnableVertexAttribArray(_vertex_loc);
		glVertexAttribPointer(_vertex_loc, 4, GL_FLOAT, GL_FALSE, 4* sizeof(GLfloat), 0);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(_vertex_loc);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x+= (ch._advance >> 6)* scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

