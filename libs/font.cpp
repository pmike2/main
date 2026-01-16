#include <cstdlib>
#include <iostream>
#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "font.h"


Text::Text() {

}


Text::Text(std::string text, glm::vec2 pos, float scale, glm::vec4 color, float wrapping) :
	_text(text), _pos(pos), _scale(scale), _color(color), _wrapping(wrapping)
{

}


Text::~Text() {

}


// ---------------------------------------------------------------------------------
Text3D::Text3D() {

}


Text3D::Text3D(std::string text, glm::vec3 pos, float scale, glm::vec4 color, float wrapping) :
	_text(text), _pos(pos), _scale(scale), _color(color), _wrapping(wrapping)
{

}


Text3D::~Text3D() {

}


// ---------------------------------------------------------------------------------
Font::Font() {

}


Font::Font(std::map<std::string, GLuint> progs, std::string font_path, uint font_size, ScreenGL * screengl) : _z(0.0) {
	FT_Library ft_lib;
	FT_Face face;

	if (FT_Init_FreeType(&ft_lib)) {
		std::cout << "Could not init FreeType Library\n";
		return;
	}

	if (FT_New_Face(ft_lib, font_path.c_str(), 0, &face)) {
		std::cout << "Failed to load font\n";
		return;
	}

	FT_Set_Pixel_Sizes(face, 0, font_size);
	
	// Disable byte-alignment restriction ; nécessaire !
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// on crée une texture qui va contenir tous les chars dans une bande horizontale
	uint max_width= 0;
	uint max_height= 0;
	for (unsigned char c=0; c<128; ++c) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cout << "Failed to load Glyph\n";
			continue;
		}

		if (face->glyph->bitmap.width> max_width) {
			max_width= face->glyph->bitmap.width;
		}
		if (face->glyph->bitmap.rows> max_height) {
			max_height= face->glyph->bitmap.rows;
		}
	}
	_tex_size= pow(2, int(ceil(log2(std::max(max_width, max_height)))));

	/*std::cout << "max_width = " << max_width << " ; max_height = " << max_height << "\n";
	std::cout << "tex_size=" << tex_size << "\n";*/

	glGenTextures(1, &_texture_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RED, _tex_size, _tex_size, 128, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);

	uint compt= 0;
	for (unsigned char c=0; c<128; ++c) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cout << "Failed to load Glyph\n";
			continue;
		}

		Character character = {
			c,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			static_cast<GLuint>(face->glyph->advance.x)
		};
		_characters.insert(std::pair<GLchar, Character>(c, character));

		glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
						0,                           // mipmap number
						0, 0, compt,                 // xoffset, yoffset, zoffset
						_characters[c]._size.x, _characters[c]._size.y, 1, // width, height, depth
						GL_RED,                      // format
						GL_UNSIGNED_BYTE,            // type
						face->glyph->bitmap.buffer); // pointer to data
		compt++;
	}

	// à réactiver pour visualiser les textures
	//export_texture_array2pgm("/Volumes/Data/tmp/test", tex_size, tex_size, 128);
	
	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	FT_Done_Face(face);
	FT_Done_FreeType(ft_lib);

	_contexts["font"]= new DrawContext(progs["font"], 
		std::vector<std::string>{"vertex_in:4", "color_in:4", "current_layer_in:1"},
		std::vector<std::string>{"camera2clip_matrix", "z", "texture_array"});

	_contexts["font3d"]= new DrawContext(progs["font3d"], 
		std::vector<std::string>{"vertex_in:3", "tex_in:2", "color_in:4", "current_layer_in:1"},
		std::vector<std::string>{"world2clip_matrix", "texture_array"});

	_camera2clip = glm::ortho(-screengl->_gl_width* 0.5, screengl->_gl_width* 0.5, -screengl->_gl_height* 0.5, screengl->_gl_height* 0.5);
}


void Font::set_text(std::vector<Text> & texts) {
	DrawContext * context = _contexts["font"];

	const uint n_pts_per_char= 6;
	context->_n_pts= 0;
	for (auto text : texts) {
		context->_n_pts+= n_pts_per_char* text._text.size();
	}

	uint n_attrs_per_pts = context->_buffers[0]._n_attrs_per_pts;
	
	float data[context->_n_pts * n_attrs_per_pts];

	uint idx= 0;
	for (auto text: texts) {
		float x0= text._pos.x;
		float y0= text._pos.y;
		
		for (std::string::const_iterator c=text._text.begin(); c!=text._text.end(); c++) {
			Character ch= _characters[*c];
			
			// affichage attributs lettres
			//std::cout << ch << "\n";

			float xpos= x0+ (float)(ch._bearing.x)* text._scale;
			float ypos= y0- (float)(ch._size.y- ch._bearing.y)* text._scale;

			float w= (float)(ch._size.x)* text._scale;
			float h= (float)(ch._size.y)* text._scale;

			float u= (float)(ch._size.x)/ float(_tex_size);
			float v= (float)(ch._size.y)/ float(_tex_size);

			float positions[n_pts_per_char][4]= {
				{xpos, ypos, 0.0, v},
				{xpos+ w, ypos, u, v},
				{xpos+ w, ypos+ h, u, 0.0},

				{xpos, ypos, 0.0, v},
				{xpos+ w, ypos+ h, u, 0.0},
				{xpos, ypos+ h, 0.0, 0.0},
			};
			for (int i=0; i<n_pts_per_char; ++i) {
				for (int j=0; j<4; ++j) {
					data[idx * n_pts_per_char * n_attrs_per_pts + i * n_attrs_per_pts + j]= positions[i][j];
				}
				for (int k=0; k<4; ++k) {
					data[idx * n_pts_per_char * n_attrs_per_pts + i * n_attrs_per_pts + 4 + k]= text._color[k];
				}
				data[idx * n_pts_per_char * n_attrs_per_pts + i * n_attrs_per_pts + 8]= float(ch._char);
			}

			// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x0+= (float)(ch._advance >> 6)* text._scale; // Bitshift by 6 to get value in pixels (2^6 = 64)

			// retour à la ligne si wrapping > 0 (pas par défaut)
			if (text._wrapping > 0.0 && x0 - text._pos.x > text._wrapping) {
				x0 = text._pos.x;
				y0 -= 0.1; // à mettre en argument ?
			}

			idx++;
		}
	}

	context->set_data(data);
	/*glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* n_attrs_per_pts, data, context->_usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);*/
}


// raccourci lorsque l'on a qu'un seul texte à afficher
void Font::set_text(Text & text) {
	std::vector<Text> texts;
	texts.push_back(text);
	set_text(texts);
}


void Font::set_text(std::vector<Text3D> & texts) {
	DrawContext * context = _contexts["font3d"];

	const uint n_pts_per_char= 6;
	context->_n_pts= 0;
	for (auto text : texts) {
		context->_n_pts+= n_pts_per_char* text._text.size();
	}

	uint n_attrs_per_pts = context->_buffers[0]._n_attrs_per_pts;

	float data[context->_n_pts* n_attrs_per_pts];

	uint idx= 0;
	for (auto text: texts) {
		float x0= text._pos.x;
		float y0= text._pos.y;
		float z = text._pos.z;
		
		for (std::string::const_iterator c=text._text.begin(); c!=text._text.end(); c++) {
			Character ch= _characters[*c];
			
			// affichage attributs lettres
			//std::cout << ch << "\n";

			float xpos= x0+ (float)(ch._bearing.x)* text._scale;
			float ypos= y0- (float)(ch._size.y- ch._bearing.y)* text._scale;

			float w= (float)(ch._size.x)* text._scale;
			float h= (float)(ch._size.y)* text._scale;

			float u= (float)(ch._size.x)/ float(_tex_size);
			float v= (float)(ch._size.y)/ float(_tex_size);

			float positions[n_pts_per_char][5]= {
				{xpos, ypos, z, 0.0, v},
				{xpos+ w, ypos, z, u, v},
				{xpos+ w, ypos+ h, z, u, 0.0},

				{xpos, ypos, z, 0.0, v},
				{xpos+ w, ypos+ h, z, u, 0.0},
				{xpos, ypos+ h, z, 0.0, 0.0},
			};
			for (int i=0; i<n_pts_per_char; ++i) {
				for (int j=0; j<5; ++j) {
					data[idx * n_pts_per_char * n_attrs_per_pts + i * n_attrs_per_pts + j]= positions[i][j];
				}
				for (int k=0; k<4; ++k) {
					data[idx * n_pts_per_char * n_attrs_per_pts+ i * n_attrs_per_pts + 5 + k]= text._color[k];
				}
				data[idx * n_pts_per_char * n_attrs_per_pts + i * n_attrs_per_pts + 9]= float(ch._char);
			}

			// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x0+= (float)(ch._advance >> 6)* text._scale; // Bitshift by 6 to get value in pixels (2^6 = 64)

			// retour à la ligne si wrapping > 0 (pas par défaut)
			if (text._wrapping > 0.0 && x0 - text._pos.x > text._wrapping) {
				x0 = text._pos.x;
				y0 -= 0.1; // à mettre en argument ?
			}

			idx++;
		}
	}

	context->set_data(data);
	/*glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* n_attrs_per_pts, data, context->_usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);*/
}


// raccourci lorsque l'on a qu'un seul texte à afficher
void Font::set_text(Text3D & text) {
	std::vector<Text3D> texts;
	texts.push_back(text);
	set_text(texts);
}


void Font::clear() {
	/*glBindBuffer(GL_ARRAY_BUFFER, _contexts["font"]->_buffer);
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, _contexts["font"]->_usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, _contexts["font3d"]->_buffer);
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, _contexts["font3d"]->_usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);*/
	_contexts["font"]->clear_data();
	_contexts["font3d"]->clear_data();
}


void Font::draw() {
	DrawContext * context = _contexts["font"];
	
	context->activate();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id);
	glActiveTexture(0);

	glUniform1i(context->_locs_uniform["texture_array"], 0); //Sampler refers to texture unit 0
	glUniform1f(context->_locs_uniform["z"], _z);
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(_camera2clip)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	context->deactivate();
}


void Font::draw_3d(const mat_4d & world2clip) {
	DrawContext * context = _contexts["font3d"];

	context->activate();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id);
	glActiveTexture(0);

	glUniform1i(context->_locs_uniform["texture_array"], 0); //Sampler refers to texture unit 0
	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(world2clip)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	context->deactivate();
}
