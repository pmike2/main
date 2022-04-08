#ifndef MPEG_H
#define MPEG_H

#include <vector>
#include <string>

const unsigned int N_MAX_TEXTURES= 8;

class MPEG {
public:
	MPEG();
	MPEG(std::string mpeg_path);
	~MPEG();
	void * get_frame(unsigned int frame_idx);

	unsigned int _n_frames;
	unsigned int _fps;
	unsigned char * _data;
	unsigned int _width;
	unsigned int _height;
	unsigned int _frame_size;
};


class MPEGTextures {
public:
	MPEGTextures();
	MPEGTextures(std::vector<std::string> mpeg_paths, int loc, int base_index);
	~MPEGTextures();
	void prepare2draw();

	unsigned int _ids[N_MAX_TEXTURES];
	int _loc;
	int _base_index;
	GLint _indices[N_MAX_TEXTURES];
	unsigned int _n;
};

#endif
