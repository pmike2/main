#include <iostream>
#include <algorithm>

#define __STDC_CONSTANT_MACROS
extern "C" {
#include<libavutil/avutil.h>
#include<libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <OpenGL/gl3.h>

#include "mpeg.h"
#include "utile.h"


using namespace std;


MPEG::MPEG() : _n_frames(0), _fps(0), _data(0), _width(0), _height(0), _frame_size(0) {

}


MPEG::MPEG(string mpeg_path) {
	// Format I/O context
	AVFormatContext * ctx_format = 0;

	// main external API structure
	AVCodecContext * ctx_codec = 0;

	// Codec
	const AVCodec * codec = 0;

	// This structure describes decoded (raw) audio or video data
	AVFrame * frame= av_frame_alloc();
	AVFrame * frame_rgb= av_frame_alloc();

	// Structure permettant de manipuler la data
	SwsContext * ctx_sws = 0;

	// Stream
	AVStream * vid_stream = 0;

	/*
	This structure stores compressed data.
	It is typically exported by demuxers and then passed as input to decoders, or received as output from encoders and then passed to muxers.
	For video, it should typically contain one compressed frame. For audio it may contain several compressed frames.
	Encoders are allowed to output empty packets, with no compressed data, containing only side data (e.g. to update some stream parameters at the end of encoding).
	*/
	AVPacket * pkt = av_packet_alloc();

	// buffer qui servira a stocker les données de frame_rgb après conversion du frame
	unsigned char * buffer_rgb= 0;
	int buffer_rgb_size= 0;

	// servira a faire la conversion de format de couleur entre frame et frame_rgb
	struct SwsContext * sws_ctx= 0;

	// indice du stream video du fichier en entrée
	int stream_idx= 0;

	// valeurs de retour des fonctions
	int ret= 0;

	/*
	Allocate memory for AVFormatContext.
	Read the probe_size about of data from the file (input url)
	Tries to guess the input file format, codec parameter for the input file. This is done by calling read_probe function pointer for each of the demuxer.
	Allocate the codec context, demuxed context, I/O context.
	*/
	ret = avformat_open_input(&ctx_format, mpeg_path.c_str(), 0, 0);
	if (ret!= 0) {
		cout << "ERREUR avformat_open_input\n";
		return;
	}

	/*
	Read packets of a media file to get stream information.
	This is useful for file formats with no headers such as MPEG. This function also computes the real framerate in case of MPEG-2 repeat frame mode.
	The logical file position is not changed by this function; examined packets may be buffered for later processing.
	*/
	ret= avformat_find_stream_info(ctx_format, 0);
	if (ret< 0) {
		cout << "ERREUR avformat_find_stream_info\n";
		return; // Couldn't find stream information
	}

	// Print detailed information about the input or output format, such as duration, bitrate, streams, container, programs, metadata, side data, codec and time base.
	//av_dump_format(ctx_format, 0, sample_path.c_str(), false);

	// pour chaque stream, si c'est un stream video on l'assigne à vid_stream
	for (int i = 0; i < ctx_format->nb_streams; i++)
		if (ctx_format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			stream_idx = i;
			vid_stream = ctx_format->streams[i];
			break;
	}
	if (vid_stream== 0) {
		cout << "ERREUR pas de stream video trouvé\n";
		return;
	}

	_fps= av_q2d(ctx_format->streams[stream_idx]->r_frame_rate);

	// 	Find a registered decoder with a matching codec ID
	codec= avcodec_find_decoder(vid_stream->codecpar->codec_id);
	if (!codec) {
		cout << "ERREUR avcodec_find_decoder\n";
		return;
	}

	// Allocate an AVCodecContext and set its fields to default values.
	ctx_codec= avcodec_alloc_context3(codec);

	// 	Fill the codec context based on the values from the supplied codec parameters
	ret= avcodec_parameters_to_context(ctx_codec, vid_stream->codecpar);
	if (ret< 0) {
		cout << "ERREUR avcodec_parameters_to_context\n";
		return;
	}
	
	// Initialize the codec context to use the given AVCodec.
	ret= avcodec_open2(ctx_codec, codec, 0);
	if (ret< 0) {
		cout << "ERREUR avcodec_open2\n";
		return;
	}

	_width= ctx_codec->width;
	_height= ctx_codec->height;

	// Determine required buffer size and allocate buffer
	buffer_rgb_size= av_image_get_buffer_size(AV_PIX_FMT_RGB24, _width, _height, 32);
	_frame_size= buffer_rgb_size* sizeof(unsigned char);
	buffer_rgb= (unsigned char *)av_malloc(_frame_size);

	// Assign appropriate parts of buffer to image planes in frame_rgb
	av_image_fill_arrays(frame_rgb->data, frame_rgb->linesize, buffer_rgb, AV_PIX_FMT_RGB24, _width, _height, 1);

	// servira a faire la conversion de format de couleur
	sws_ctx= sws_getContext(_width, _height, ctx_codec->pix_fmt, _width, _height, AV_PIX_FMT_RGB24, SWS_BILINEAR, 0, 0, 0);

	_n_frames= ctx_format->streams[stream_idx]->nb_frames;
	//cout << "NB Frames estimated = " << _n_frames << "\n";
	_data= (unsigned char *)av_malloc(_frame_size* _n_frames);

	// Return the next frame of a stream
	while (av_read_frame(ctx_format, pkt)>= 0){
		// s'il s'agit du bon stream
		if (pkt->stream_index == stream_idx) {
			// Supply raw packet data as input to a decoder
			ret= avcodec_send_packet(ctx_codec, pkt);
			if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
				cout << "avcodec_send_packet: " << ret << "\n";
				break;
			}

			while (ret>= 0) {
				// Return decoded output data from a decoder
				// AVERROR_EOF: the decoder has been fully flushed, and there will be no more output frames
				// AVERROR(EAGAIN): output is not available in this state
				ret= avcodec_receive_frame(ctx_codec, frame);
				if ((ret== AVERROR_EOF) || (ret== AVERROR(EAGAIN))) {
					//cout << "avcodec_receive_frame: " << ret << "\n";
					break;
				}

				//cout << "frame: " << ctx_codec->frame_number << "\n";
				_n_frames= ctx_codec->frame_number;

				// conversion de frame vers frame_rgb
				sws_scale(sws_ctx, (unsigned char const * const *)frame->data, frame->linesize, 0, _height, frame_rgb->data, frame_rgb->linesize);
	
				memcpy(_data+ (ctx_codec->frame_number- 1)* _frame_size, buffer_rgb, _frame_size);
			}

		}

		// Wipe the packet. Unreference the buffer referenced by the packet and reset the remaining packet fields to their default values.
		av_packet_unref(pkt);
	}

	// close format context
	avformat_close_input(&ctx_format);
	av_packet_unref(pkt);
	// Free the codec context and everything associated with it and write NULL to the provided pointer
	avcodec_free_context(&ctx_codec);
	// Free an AVFormatContext and all its streams.
	avformat_free_context(ctx_format);
}


MPEG::~MPEG() {
	av_free(_data);
}


void * MPEG::get_frame(unsigned int frame_idx) {
	return (void *)(_data+ frame_idx* _frame_size* sizeof(unsigned char));
}



// -------------------------------------------------------------
MPEGTextures::MPEGTextures() {

}


MPEGTextures::MPEGTextures(vector<string> mpeg_paths, int loc, int base_index) :
	_base_index(base_index), _loc(loc)
{
	glGenTextures(N_READERS, _ids);
	for (unsigned int i=0; i<N_READERS; ++i) {
		_indices[i]= _base_index+ i;
	}

	for (unsigned int i=0; i<N_READERS; ++i) {
		if (i>= mpeg_paths.size()) {
			_ids[i]= _ids[i % mpeg_paths.size()];
			continue;
		}
		MPEG * mpeg= new MPEG(mpeg_paths[i]);
		
		glActiveTexture(GL_TEXTURE0+ _base_index+ i);
		glBindTexture(GL_TEXTURE_3D, _ids[i]);

		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, mpeg->_width, mpeg->_height, mpeg->_n_frames, 0, GL_RGB, GL_UNSIGNED_BYTE, mpeg->_data);
		delete mpeg;

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);
		
		glBindTexture(GL_TEXTURE_3D, 0);
	}

	glUniform1iv(_loc, N_READERS, &_indices[0]);
}


MPEGTextures::~MPEGTextures() {

}


void MPEGTextures::prepare2draw() {
	glUniform1iv(_loc, N_READERS, &_indices[0]);
	for (unsigned int i=0; i<N_READERS; ++i) {
		glActiveTexture(GL_TEXTURE0+ _base_index+ i);
		glBindTexture(GL_TEXTURE_3D, _ids[i]);
	}
}



// -----------------------------------------------------------------------------------
TimeConfig::TimeConfig() {

}


TimeConfig::TimeConfig(vector<pair<float, float> > time_checkpoints, float speed) :
	_time_checkpoints(time_checkpoints), _speed(speed) 
{
	sort(_time_checkpoints.begin(), _time_checkpoints.end(), [](const pair<float, float> & a, const pair<float, float> & b) -> bool
	{
		return a.first< b.first; 
	});
}


TimeConfig::~TimeConfig() {

}



// -----------------------------------------------------------------------------------
AlphaPolygon::AlphaPolygon() {

}


AlphaPolygon::AlphaPolygon(float * points, unsigned int n_points, float fadeout, float curve, float alpha_max) :
	_fadeout(fadeout), _curve(curve), _alpha_max(alpha_max), _polygon(Polygon2D())
{
	_polygon.set_points(points, n_points, false);
}


AlphaPolygon::AlphaPolygon(const AlphaPolygon & alpha_polygon) :
	_polygon(alpha_polygon._polygon), _fadeout(alpha_polygon._fadeout), _curve(alpha_polygon._curve), _alpha_max(alpha_polygon._alpha_max)
{
	
}


AlphaPolygon::~AlphaPolygon() {

}


// -----------------------------------------------------------------------------------
AlphaConfig::AlphaConfig() {

}


AlphaConfig::AlphaConfig(vector<AlphaPolygon> polygons, float decrease_speed) :
	_polygons(polygons), _decrease_speed(decrease_speed) 
{

}


AlphaConfig::~AlphaConfig() {

}


// -----------------------------------------------------------------------------------
GlobalConfig::GlobalConfig() {

}


GlobalConfig::GlobalConfig(unsigned int alpha_width, unsigned int alpha_height, unsigned int time_width, 
	vector<AlphaConfig> alpha_configs, vector<TimeConfig> time_configs) : 
	_alpha_width(alpha_width), _alpha_height(alpha_height), _time_width(time_width), _alpha_configs(alpha_configs), _time_configs(time_configs)
{

}


GlobalConfig::~GlobalConfig() {

}


// -----------------------------------------------------------------------------------
MPEGReaders::MPEGReaders() {

}


MPEGReaders::MPEGReaders(
		unsigned int alpha_texture_index, unsigned int alpha_loc,
		unsigned int time_texture_index, unsigned int time_loc,
		unsigned int index_time_texture_index, unsigned int index_time_loc) :
	_alpha_depth(N_READERS), _alpha_texture_index(alpha_texture_index), _alpha_loc(alpha_loc),
	_time_height(N_READERS), _time_texture_index(time_texture_index), _time_loc(time_loc),
	_index_time_width(N_READERS), _index_time_texture_index(index_time_texture_index), _index_time_loc(index_time_loc)
{
	_alpha_data0= new float[_alpha_width* _alpha_height* _alpha_depth];
	_alpha_data= new float[_alpha_width* _alpha_height* _alpha_depth];

	for (unsigned int i=0; i<_alpha_width* _alpha_height* _alpha_depth; ++i) {
		_alpha_data0[i]= 0.0f;
		_alpha_data[i]= 0.0f;
	}

	_time_data= new float[_time_width* _time_height];
	for (unsigned int i=0; i<_time_width* _time_height; ++i) {
		_time_data[i]= 0.0f;
	}

	_index_time_data= new float[_index_time_width];
	for (unsigned int i=0; i<_index_time_width; ++i) {
		_index_time_data[i]= 0.0f;
	}

	for (unsigned int i=0; i<N_READERS; ++i) {
		_note_ons.push_back(false);
	}
}


MPEGReaders::~MPEGReaders() {
	delete[] _alpha_data;
	delete[] _time_data;
	delete[] _index_time_data;
}


void MPEGReaders::set_config(GlobalConfig config) {

}


void MPEGReaders::parse_json(std::string json_path) {

}


void MPEGReaders::compute_alpha_data0() {
	for (unsigned int i=0; i<_alpha_width* _alpha_height* _alpha_depth; ++i) {
		_alpha_data0[i]= 0.0f;
	}
	for (unsigned int idx_reader=0; idx_reader<_alpha_depth; ++idx_reader) {
		//cout << _alpha_configs[idx_reader]._polygons.size() << "\n";
		for (unsigned int idx_poly=0; idx_poly<_alpha_configs[idx_reader]._polygons.size(); ++idx_poly) {
			AlphaPolygon alpha_poly= _alpha_configs[idx_reader]._polygons[idx_poly];
			//cout << alpha_poly._fadeout << " ; " << alpha_poly._curve << " ; " << alpha_poly._alpha_max << "\n";
			float xmin= alpha_poly._polygon._aabb->_pos.x;
			float ymin= alpha_poly._polygon._aabb->_pos.y;
			float xmax= xmin+ alpha_poly._polygon._aabb->_size.x;
			float ymax= ymin+ alpha_poly._polygon._aabb->_size.y;
			//cout << xmin << " ; " << ymin << " ; " << xmax << " ; " << ymax << "\n";
			int imin= (int)((xmin- alpha_poly._fadeout)* (float)(_alpha_width));
			if (imin< 0) {
				imin= 0;
			}
			int imax= (int)((xmax+ alpha_poly._fadeout)* (float)(_alpha_width));
			if (imax>= _alpha_width) {
				imax= _alpha_width- 1;
			}
			int jmin= (int)((ymin- alpha_poly._fadeout)* (float)(_alpha_height));
			if (jmin< 0) {
				jmin= 0;
			}
			int jmax= (int)((ymax+ alpha_poly._fadeout)* (float)(_alpha_height));
			if (jmax>= _alpha_height) {
				jmax= _alpha_height- 1;
			}
			for (unsigned int i=imin; i<=imax; ++i) {
				for (unsigned int j=jmin; j<=jmax; ++j) {
					glm::vec2 pt((float)(i)/ (float)(_alpha_width)+ 0.5f/ _alpha_width, (float)(j)/ (float)(_alpha_height)+ 0.5f/ _alpha_height);
					float d= distance_poly_pt(&alpha_poly._polygon, pt, 0);
					if ((alpha_poly._fadeout> 1e-5) && (d<= alpha_poly._fadeout)) {
						_alpha_data0[_alpha_width* _alpha_height* idx_reader+ i* _alpha_height+ j]+= (1.0f- pow(d/ alpha_poly._fadeout, alpha_poly._curve))* alpha_poly._alpha_max;
					}
					else if (d< 1e-5) {
						_alpha_data0[_alpha_width* _alpha_height* idx_reader+ i* _alpha_height+ j]= 1.0f;
					}
				}
			}
		}
	}
	for (unsigned int i=0; i<_alpha_width* _alpha_height* _alpha_depth; ++i) {
		if (_alpha_data0[i]> 1.0f) {
			_alpha_data0[i]= 1.0f;
		}
	}

	/*unsigned int depth= 0;
	for (unsigned int i=0; i<_alpha_width; ++i) {
		for (unsigned int j=0; j<_alpha_height; ++j) {
			cout << _alpha_data[_alpha_width* _alpha_height* depth+ i* _alpha_height+ j] << "|";
		}
		cout << "\n";
	}*/
}


void MPEGReaders::compute_time_data() {
	for (unsigned int i=0; i<_time_width* _time_height; ++i) {
		_time_data[i]= 0.0f;
	}
	for (unsigned int i=0; i<_time_height; ++i) {
		for (unsigned int k=0; k<_time_configs[i]._time_checkpoints.size()- 1; ++k) {
			float t0= _time_configs[i]._time_checkpoints[k].first;
			float t1= _time_configs[i]._time_checkpoints[k+ 1].first;
			float val0= _time_configs[i]._time_checkpoints[k].second;
			float val1= _time_configs[i]._time_checkpoints[k+ 1].second;
			unsigned int j0= (unsigned int)(t0* (float)(_time_width));
			unsigned int j1= (unsigned int)(t1* (float)(_time_width));
			for (unsigned int j=j0; j<j1; ++j) {
				_time_data[_time_width* i+ j]= val0+ ((float)(j)/ (float)(_time_width)- t0)* (val1- val0)/ (t1- t0);
			}
		}
	}

	//cout << _time_width << " ; "  << _time_height << "\n";
	/*for (unsigned int i=0; i<_time_height; ++i) {
		for (unsigned int j=0; j<_time_width; ++j) {
			cout << _time_data[_time_width* i+ j] << " ; ";
		}
		cout << "\n";
	}*/
}


void MPEGReaders::init_alpha_texture() {
	glGenTextures(1, &_alpha_id);

	glActiveTexture(GL_TEXTURE0+ _alpha_texture_index);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _alpha_id);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R32F, _alpha_width, _alpha_height, _alpha_depth, 0, GL_RED, GL_FLOAT, _alpha_data);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	glUniform1i(_alpha_loc, _alpha_texture_index);
}


void MPEGReaders::init_time_texture() {
	glGenTextures(1, &_time_id);

	glActiveTexture(GL_TEXTURE0+ _time_texture_index);
	glBindTexture(GL_TEXTURE_1D_ARRAY, _time_id);
	glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, GL_R32F, _time_width, _time_height, 0, GL_RED, GL_FLOAT, _time_data);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_1D_ARRAY, 0);

	glUniform1i(_time_loc, _time_texture_index);
}


void MPEGReaders::init_index_time_texture() {
	glGenTextures(1, &_index_time_id);

	glActiveTexture(GL_TEXTURE0+ _index_time_texture_index);
	glBindTexture(GL_TEXTURE_1D, _index_time_id);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, _index_time_width, 0, GL_RED, GL_FLOAT, _index_time_data);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_1D, 0);

	glUniform1i(_index_time_loc, _index_time_texture_index);
}


void MPEGReaders::randomize() {
	_time_configs.clear();
	for (unsigned int i=0; i<_time_height; ++i) {
		vector<pair<float, float> > checkpoints;
		unsigned int n_checkpoints= rand_int(2, 10);
		for (unsigned int j=0; j<n_checkpoints; ++j) {
			checkpoints.push_back(make_pair(rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f)));
		}
		float speed= rand_float(0.01f, 0.1f);
		_time_configs.push_back(TimeConfig(checkpoints, speed));
	}

	_alpha_configs.clear();
	for (unsigned int i=0; i<_alpha_depth; ++i) {
		vector<AlphaPolygon> alpha_polygons;
		float xsize= rand_float(0.01f, 0.3f);
		float ysize= rand_float(0.01f, 0.3f);
		float xmin= rand_float(0.0f, 1.0f- xsize);
		float ymin= rand_float(0.0f, 1.0f- ysize);
		float points[8]= {xmin, ymin, xmin+ xsize, ymin, xmin+ xsize, ymin+ ysize, ymin+ ysize};
		float fadeout= rand_float(0.0f, 0.4f);
		float curve= rand_float(1.0f, 5.0f);
		float alpha_max= rand_float(0.6f, 1.0f);
		AlphaPolygon alpha_polygon(points, 4, fadeout, curve, alpha_max);
		alpha_polygons.push_back(AlphaPolygon(alpha_polygon));
		float decrease_speed= rand_float(0.01f, 0.1f);
		AlphaConfig alpha_config(alpha_polygons, decrease_speed);
		_alpha_configs.push_back(alpha_config);
	}
}


void MPEGReaders::prepare2draw() {
	glUniform1i(_alpha_loc, _alpha_texture_index);
	glActiveTexture(GL_TEXTURE0+ _alpha_texture_index);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _alpha_id);

	glUniform1i(_time_loc, _time_texture_index);
	glActiveTexture(GL_TEXTURE0+ _time_texture_index);
	glBindTexture(GL_TEXTURE_1D_ARRAY, _time_id);

	glUniform1i(_index_time_loc, _index_time_texture_index);
	glActiveTexture(GL_TEXTURE0+ _index_time_texture_index);
	glBindTexture(GL_TEXTURE_1D, _index_time_id);
}


void MPEGReaders::update() {
	for (int i=0; i<_alpha_depth; ++i) {
		decrease_alpha(i);
		next_index_time(i);
	}
}


void MPEGReaders::update_alpha_texture(unsigned int depth) {
	glBindTexture(GL_TEXTURE_2D_ARRAY, _alpha_id);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, depth, _alpha_width, _alpha_height, 1, GL_RED, GL_FLOAT, _alpha_data+ _alpha_width* _alpha_height* depth);
}


void MPEGReaders::update_index_time_texture(unsigned int depth) {
	glBindTexture(GL_TEXTURE_1D, _index_time_id);
	glTexSubImage1D(GL_TEXTURE_1D, 0, depth, 1, GL_RED, GL_FLOAT, &_index_time_data[depth]);
}


void MPEGReaders::decrease_alpha(unsigned int depth) {
	if (_note_ons[depth]) {
		return;
	}

	for (unsigned int i=0; i<_alpha_width* _alpha_height; ++i) {
		_alpha_data[_alpha_width* _alpha_height* depth+ i]-= _alpha_configs[depth]._decrease_speed;
		if (_alpha_data[_alpha_width* _alpha_height* depth+ i]< 0.0f) {
			_alpha_data[_alpha_width* _alpha_height* depth+ i]= 0.0f;
		}
	}

	update_alpha_texture(depth);
}


void MPEGReaders::next_index_time(unsigned int depth) {
	if (!_note_ons[depth]) {
		return;
	}

	_index_time_data[depth]+= _time_configs[depth]._speed;
	if (_index_time_data[depth]>= 1.0f) {
		_index_time_data[depth]= 0.0f;
	}
	update_index_time_texture(depth);
}


void MPEGReaders::note_on(unsigned int depth) {
	if (_note_ons[depth]) {
		return;
	}

	//cout << "note_on " << depth << "\n";

	_note_ons[depth]= true;

	_index_time_data[depth]= 0.0f;
	update_index_time_texture(depth);
	
	for (unsigned int i=0; i<_alpha_width* _alpha_height; ++i) {
		_alpha_data[_alpha_width* _alpha_height* depth+ i]= _alpha_data0[_alpha_width* _alpha_height* depth+ i];
	}
	update_alpha_texture(depth);
}


void MPEGReaders::note_off(unsigned int depth) {
	//cout << "note_off " << depth << "\n";
	
	_note_ons[depth]= false;
}

