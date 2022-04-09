#ifndef MPEG_H
#define MPEG_H

#include <vector>
#include <string>

const unsigned int N_MAX_TEXTURES= 4;
const unsigned int N_MAX_READERS= 4;

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
	int _indices[N_MAX_TEXTURES];
	unsigned int _n;
};


class MPEGReaders {
public:
	MPEGReaders();
	MPEGReaders(int width, int height, int depth, int loc, int index_loc, int base_index);
	~MPEGReaders();
	void linear_index(float * data);
	void total_rand_index(float * data);
	void time_rand_index(float * data);
	void position_rand_index(float * data);
	void smooth_rand_time_index(float * data, int m);
	void mosaic_index(float * data, int size);
	void prepare2draw();
	void next();

	unsigned int _ids[N_MAX_READERS];
	int _loc, _index_loc;
	int _base_index;
	int _indices[N_MAX_READERS];
	float _index_indices[N_MAX_READERS];
	int _width, _height, _depth;
	unsigned int _idx;
};


#endif
