#include <iostream>
#include <fstream>
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
#include <glm/glm.hpp>
#include "json.hpp"

#include "mpeg.h"
#include "utile.h"


using namespace std;
using json = nlohmann::json;


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


// -----------------------------------------------------------------------------------
TimeConfig::TimeConfig() : _speed(0.0f) {

}


TimeConfig::TimeConfig(vector<pair<float, float> > time_checkpoints, float speed) :
	_time_checkpoints(time_checkpoints), _speed(speed) 
{
	sort(_time_checkpoints.begin(), _time_checkpoints.end(), [](const pair<float, float> & a, const pair<float, float> & b) -> bool
	{
		return a.first< b.first; 
	});
}


TimeConfig::TimeConfig(const TimeConfig & time_config) :
	_time_checkpoints(time_config._time_checkpoints), _speed(time_config._speed) 
{
	sort(_time_checkpoints.begin(), _time_checkpoints.end(), [](const pair<float, float> & a, const pair<float, float> & b) -> bool
	{
		return a.first< b.first; 
	});
}


TimeConfig::~TimeConfig() {

}


ostream & operator << (ostream & stream, TimeConfig & t) {
	stream << "speed=" << t._speed << "\n";
	stream << "time_checkpoints=";
	for (auto & tc : t._time_checkpoints) {
		stream << "(" << tc.first << " ; " << tc.second << ") ";
	}
	stream << "\n";

	return stream;
}


// -----------------------------------------------------------------------------------
AlphaPolygon::AlphaPolygon() : _fadeout(0.0f), _curve(0.0f), _alpha_max(0.0f), _polygon(Polygon2D()) {

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


ostream & operator << (ostream & stream, AlphaPolygon & a) {
	stream << "fadeout=" << a._fadeout << " ; curve=" << a._curve << " ; alpha_max=" << a._alpha_max << "\n";
	stream << "polygon=";
	for (auto & pt : a._polygon._pts) {
		stream << "(" << pt[0] << " ; " << pt[1] << ") ";
	}
	stream << "\n";

	return stream;
}


// -----------------------------------------------------------------------------------
AlphaConfig::AlphaConfig() : _decrease_speed(0.0f) {

}


AlphaConfig::AlphaConfig(vector<AlphaPolygon> polygons, float decrease_speed) :
	_polygons(polygons), _decrease_speed(decrease_speed) 
{

}


AlphaConfig::AlphaConfig(const AlphaConfig & alpha_config) :
	_polygons(alpha_config._polygons), _decrease_speed(alpha_config._decrease_speed) 
{

}


AlphaConfig::~AlphaConfig() {

}


ostream & operator << (ostream & stream, AlphaConfig & a) {
	stream << "decrease_speed=" << a._decrease_speed << "\n";
	stream << "polygons=\n";
	for (auto p : a._polygons) {
		stream << p;
	}

	return stream;
}


// -----------------------------------------------------------------------------------
ReaderConfig::ReaderConfig() {

}


ReaderConfig::ReaderConfig(AlphaConfig alpha_config, TimeConfig time_config, string mpeg_path, unsigned int key) :
	_alpha_config(alpha_config), _time_config(time_config), _mpeg_path(mpeg_path), _key(key)
{

}


ReaderConfig::ReaderConfig(const ReaderConfig & reader_config) : 
	_alpha_config(reader_config._alpha_config), _time_config(reader_config._time_config), _mpeg_path(reader_config._mpeg_path), _key(reader_config._key)
{

}


ReaderConfig::~ReaderConfig() {

}


ostream & operator << (ostream & stream, ReaderConfig & r) {
	stream << "mpeg_path=" << r._mpeg_path << " ; key=" << r._key << "\n";
	stream << "alpha_config=\n";
	stream << r._alpha_config;
	stream << "time_config=\n";
	stream << r._time_config;

	return stream;
}


// -----------------------------------------------------------------------------------
ModifierConfig::ModifierConfig() :
	_movie_speed(0.0f), _alpha_speed(0.0f), _time_mult(1.0f), _time_add(0.0f), _time_speed(0.0f),
	_idx_track(0), _key(0)
{
	_movie_mult[0]= 1.0f; _movie_mult[1]= 0.0f; _movie_mult[2]= 0.0f; _movie_mult[3]= 1.0f;
	_movie_add[0]= 0.0f; _movie_add[1]= 0.0f;
	_alpha_mult[0]= 1.0f; _alpha_mult[1]= 0.0f; _alpha_mult[2]= 0.0f; _alpha_mult[3]= 1.0f;
	_alpha_add[0]= 0.0f; _alpha_add[1]= 0.0f;
}


ModifierConfig::ModifierConfig(const ModifierConfig & modifier_config) :
	_movie_speed(modifier_config._movie_speed), _alpha_speed(modifier_config._alpha_speed),
	_time_mult(modifier_config._time_mult), _time_add(modifier_config._time_add), _time_speed(modifier_config._time_speed),
	_idx_track(modifier_config._idx_track), _key(modifier_config._key)
{
	for (unsigned int i=0; i<4; ++i) {
		_movie_mult[i]= modifier_config._movie_mult[i];
		_alpha_mult[i]= modifier_config._alpha_mult[i];
	}
	for (unsigned int i=0; i<2; ++i) {
		_movie_add[i]= modifier_config._movie_add[i];
		_alpha_add[i]= modifier_config._alpha_add[i];
	}
}


ModifierConfig::~ModifierConfig() {

}


ostream & operator << (ostream & stream, ModifierConfig & g) {
	stream << "idx_track=" << g._idx_track << " ; key=" << g._key << "\n";
	stream << "movie_mult=(" << g._movie_mult[0] << " ; " << g._movie_mult[1] << " ; " << g._movie_mult[2] << " ; " << g._movie_mult[3] << ")\n";
	stream << "movie_add=(" << g._movie_add[0] << " ; " << g._movie_add[1] << ")\n";
	stream << "movie_speed=" << g._movie_speed << "\n";
	stream << "alpha_mult=(" << g._alpha_mult[0] << " ; " << g._alpha_mult[1] << " ; " << g._alpha_mult[2] << " ; " << g._alpha_mult[3] << ")\n";
	stream << "alpha_add=(" << g._alpha_add[0] << " ; " << g._alpha_add[1] << ")\n";
	stream << "alpha_speed=" << g._alpha_speed << "\n";
	stream << "time_mult=" << g._time_mult << " ; time_add=" << g._time_add << " ; time_speed=" << g._time_speed << "\n";
	return stream;
}


// -----------------------------------------------------------------------------------
GlobalConfig::GlobalConfig() :
	_alpha_width(0), _alpha_height(0), _alpha_depth(0), _time_width(0), _time_height(0), _index_time_width(0), _index_movie_width(0)
{

}


GlobalConfig::GlobalConfig(unsigned int alpha_width, unsigned int alpha_height, unsigned int alpha_depth, unsigned int alpha_depth0,
	unsigned int time_width, unsigned int time_height, unsigned int time_height0,
	unsigned int index_time_width, unsigned int index_movie_width, unsigned int index_movie_width0,
	unsigned int global_alpha_width, unsigned int modifier_width, unsigned int modifier_height,
	vector<ReaderConfig> reader_configs, vector<ModifierConfig> modifier_configs) :
	_alpha_width(alpha_width), _alpha_height(alpha_height), _alpha_depth(alpha_depth), _alpha_depth0(alpha_depth0),
	_time_width(time_width), _time_height(time_height), _time_height0(time_height0),
	_index_time_width(index_time_width), _index_movie_width(index_movie_width), _index_movie_width0(index_movie_width0), 
	_global_alpha_width(global_alpha_width), _modifier_width(modifier_width), _modifier_height(modifier_height),
	_reader_configs(reader_configs), _modifier_configs(modifier_configs)
{

}


GlobalConfig::GlobalConfig(const GlobalConfig & config) :
	_alpha_width(config._alpha_width), _alpha_height(config._alpha_height), _alpha_depth(config._alpha_depth), _alpha_depth0(config._alpha_depth0), 
	_time_width(config._time_width), _time_height(config._time_height), _time_height0(config._time_height0),
	_index_time_width(config._index_time_width), _index_movie_width(config._index_movie_width), _index_movie_width0(config._index_movie_width0),
	_global_alpha_width(config._global_alpha_width), _modifier_width(config._modifier_width), _modifier_height(config._modifier_height),
	_reader_configs(config._reader_configs), _modifier_configs(config._modifier_configs)
{

}


GlobalConfig::~GlobalConfig() {

}


GlobalConfig & GlobalConfig::operator=(const GlobalConfig & rhs) {
	if (this== &rhs) {
		return *this;
	}
	
	_alpha_width= rhs._alpha_width;
	_alpha_height= rhs._alpha_height;
	_alpha_depth= rhs._alpha_depth;
	_alpha_depth0= rhs._alpha_depth0;
	_time_width= rhs._time_width;
	_time_height= rhs._time_height;
	_time_height0= rhs._time_height0;
	_index_time_width= rhs._index_time_width;
	_index_movie_width= rhs._index_movie_width;
	_index_movie_width0= rhs._index_movie_width0;
	_global_alpha_width= rhs._global_alpha_width;
	_modifier_width= rhs._modifier_width;
	_modifier_height= rhs._modifier_height;
	
	_reader_configs.clear();
	for (auto & rc : rhs._reader_configs) {
		_reader_configs.push_back(ReaderConfig(rc));
	}
	
	_modifier_configs.clear();
	for (auto & mc : rhs._modifier_configs) {
		_modifier_configs.push_back(ModifierConfig(mc));
	}

	return *this;
}


ostream & operator << (ostream & stream, GlobalConfig & g) {
	stream << "alpha_width=" << g._alpha_width << " ; alpha_height=" << g._alpha_height << " ; alpha_depth=" << g._alpha_depth << " ; alpha_depth0=" << g._alpha_depth0 << "\n";
	stream << "time_width=" << g._time_width << " ; time_height=" << g._time_height << " ; time_height0=" << g._time_height0 << "\n";
	stream << "index_time_width=" << g._index_time_width << " ; index_movie_width=" << g._index_movie_width << " ; index_movie_width0=" << g._index_movie_width0 << "\n";
	stream << "global_alpha_width=" << g._global_alpha_width << "\n";
	stream << "modifier_width=" << g._modifier_width << "modifier_height=" << g._modifier_height << "\n";
	stream << "reader_configs=\n";
	for (auto & r : g._reader_configs) {
		stream << r;
	}
	stream << "modifier_configs=\n";
	for (auto & m : g._modifier_configs) {
		stream << m;
	}

	return stream;
}


// -----------------------------------------------------------------------------------
MPEGReaders::MPEGReaders() {

}


MPEGReaders::MPEGReaders(unsigned int base_index, int movie_loc, int alpha_loc, int time_loc,
	int index_time_loc, int index_movie_loc, int global_alpha_loc, int modifier_loc) :
	_movie_loc(movie_loc),
	_alpha_data0(0), _alpha_data(0), _alpha_texture_index(base_index+ N_MAX_MOVIES), _alpha_loc(alpha_loc),
	_time_data0(0), _time_data(0), _time_texture_index(base_index+ N_MAX_MOVIES+ 1), _time_loc(time_loc),
	_index_time_data(0), _index_time_texture_index(base_index+ N_MAX_MOVIES+ 2), _index_time_loc(index_time_loc),
	_index_movie_data0(0), _index_movie_data(0), _index_movie_texture_index(base_index+ N_MAX_MOVIES+ 3), _index_movie_loc(index_movie_loc),
	_global_alpha_data(0), _global_alpha_texture_index(base_index+ N_MAX_MOVIES+ 4), _global_alpha_loc(global_alpha_loc),
	_modifier_data(0), _modifier_texture_index(base_index+ N_MAX_MOVIES+ 5), _modifier_loc(modifier_loc)
{
	for (unsigned int i=0; i<N_MAX_MOVIES; ++i) {
		_movies_ids[i]= 0;
		_movie_textures_indices[i]= base_index+ i;
	}

	for (unsigned int i=0; i<N_TRACKS; ++i) {
		_index_reader[i]= -1;
		_note_on[i]= false;
	}
}


MPEGReaders::~MPEGReaders() {
	delete[] _alpha_data;
	delete[] _alpha_data0;
	delete[] _time_data;
	delete[] _time_data0;
	delete[] _index_time_data;
	delete[] _index_movie_data;
	delete[] _index_movie_data0;
	delete[] _global_alpha_data;
	delete[] _modifier_data;
}


void MPEGReaders::set_config(GlobalConfig config) {
	_config= config;
	
	load_mpegs();
	init_arrays();
	compute_alpha_data0();
	compute_time_data0();
	compute_index_movie_data0();
	compute_modifier_data();
	init_alpha_texture();
	init_time_texture();
	init_index_time_texture();
	init_index_movie_texture();
	init_global_alpha_texture();
	init_modifier_texture();
}


void MPEGReaders::load_json(string json_path) {
	ifstream istr(json_path);
	json js;
	istr >> js;
	
	vector<string> mpeg_paths;
	for (auto & js_mpeg : js["mpegs"]) {
		mpeg_paths.push_back(js_mpeg);
	}

	vector<ReaderConfig> reader_configs_availables;
	unsigned int compt= 0;
	for (auto & js_conf : js["reader_configs"]) {

		AlphaConfig alpha_config;
		json js_alpha= js_conf["alpha"];
		alpha_config._decrease_speed= js_alpha["decrease_speed"].get<float>();
		for (auto & js_poly : js_alpha["alpha_polygons"]) {
			AlphaPolygon alpha_polygon;
			alpha_polygon._fadeout= js_poly["fadeout"].get<float>();
			alpha_polygon._curve= js_poly["curve"].get<float>();
			alpha_polygon._alpha_max= js_poly["alpha_max"].get<float>();
			vector<glm::vec2> points;
			for (unsigned int i=0; i<js_poly["polygon"].size(); ++i) {
				points.push_back(glm::vec2(js_poly["polygon"][i][0].get<float>(), js_poly["polygon"][i][1].get<float>()));
			}
			alpha_polygon._polygon.set_points(points);
			alpha_config._polygons.push_back(AlphaPolygon(alpha_polygon));
		}
		
		json js_time= js_conf["time"];
		TimeConfig time_config;
		time_config._speed= js_time["speed"].get<float>();
		for (auto & js_cp : js_time["checkpoints"]) {
			time_config._time_checkpoints.push_back(make_pair(js_cp[0].get<float>(), js_cp[1].get<float>()));
		}
		
		reader_configs_availables.push_back(ReaderConfig(alpha_config, time_config));
	}

	vector<ReaderConfig> reader_configs;
	for (auto & mapping : js["keymapping"].items()) {
		unsigned int key= mapping.key().c_str()[0];
		json js_val= mapping.value();
		unsigned int mpeg_idx= js_val["mpeg"].get<unsigned int>();
		unsigned int config_idx= js_val["config"].get<unsigned int>();

		if (mpeg_idx>= mpeg_paths.size()) {
			cout << "mpeg_idx invalide\n";
			continue;
		}
		if (config_idx>= reader_configs_availables.size()) {
			cout << "config_idx invalide\n";
			continue;
		}

		ReaderConfig reader_config(reader_configs_availables[config_idx]);
		reader_config._mpeg_path= mpeg_paths[mpeg_idx];
		reader_config._key= key;
		reader_configs.push_back(ReaderConfig(reader_config));
	}

	vector<ModifierConfig> modifier_configs_availables;
	for (auto & js_mod : js["modifiers"]) {
		ModifierConfig modifier_config;
		modifier_config._movie_mult[0]= js_mod["movie"]["mult"][0].get<float>();
		modifier_config._movie_mult[1]= js_mod["movie"]["mult"][1].get<float>();
		modifier_config._movie_mult[2]= js_mod["movie"]["mult"][2].get<float>();
		modifier_config._movie_mult[3]= js_mod["movie"]["mult"][3].get<float>();
		modifier_config._movie_add[0] = js_mod["movie"]["add"][0].get<float>();
		modifier_config._movie_add[1] = js_mod["movie"]["add"][1].get<float>();
		modifier_config._movie_speed  = js_mod["movie"]["speed"].get<float>();

		modifier_config._alpha_mult[0]= js_mod["alpha"]["mult"][0].get<float>();
		modifier_config._alpha_mult[1]= js_mod["alpha"]["mult"][1].get<float>();
		modifier_config._alpha_mult[2]= js_mod["alpha"]["mult"][2].get<float>();
		modifier_config._alpha_mult[3]= js_mod["alpha"]["mult"][3].get<float>();
		modifier_config._alpha_add[0] = js_mod["alpha"]["add"][0].get<float>();
		modifier_config._alpha_add[1] = js_mod["alpha"]["add"][1].get<float>();
		modifier_config._alpha_speed  = js_mod["alpha"]["speed"].get<float>();

		modifier_config._time_mult= js_mod["time"]["mult"].get<float>();
		modifier_config._time_add = js_mod["time"]["add"].get<float>();
		modifier_config._time_speed = js_mod["time"]["speed"].get<float>();

		modifier_configs_availables.push_back(ModifierConfig(modifier_config));
	}

	vector<ModifierConfig> modifier_configs;
	for (auto & mapping : js["keymapping_modifier"].items()) {
		unsigned int key= mapping.key().c_str()[0];
		json js_val= mapping.value();
		unsigned int track_idx= js_val["track"].get<unsigned int>();
		unsigned int modifier_idx= js_val["modifier"].get<unsigned int>();

		if (track_idx>= N_TRACKS) {
			cout << "track_idx invalide\n";
			continue;
		}
		if (modifier_idx>= modifier_configs_availables.size()) {
			cout << "modifier_idx invalide\n";
			continue;
		}

		ModifierConfig modifier_config(modifier_configs_availables[modifier_idx]);
		modifier_config._idx_track= track_idx;
		modifier_config._key= key;
		modifier_configs.push_back(ModifierConfig(modifier_config));
	}

	unsigned int alpha_width= js["alpha_width"].get<int>();
	unsigned int alpha_height= js["alpha_height"].get<int>();
	unsigned int alpha_depth= N_TRACKS;
	unsigned int alpha_depth0= reader_configs.size();
	unsigned int time_width= js["time_width"].get<int>();
	unsigned int time_height= N_TRACKS;
	unsigned int time_height0= reader_configs.size();
	unsigned int index_time_width= N_TRACKS;
	unsigned int index_movie_width= N_TRACKS;
	unsigned int index_movie_width0= reader_configs.size();
	unsigned int global_alpha_width= N_TRACKS;
	unsigned int modifier_width= 32; // + petite puissance de 2 > 17 = (4+ 2+ 1)* 2+ 3
	unsigned int modifier_height= N_TRACKS;

	GlobalConfig config(alpha_width, alpha_height, alpha_depth, alpha_depth0, time_width, time_height, time_height0,
		index_time_width, index_movie_width, index_movie_width0, global_alpha_width, modifier_width, modifier_height,
		reader_configs, modifier_configs);
	//cout << config;
	set_config(config);
}


void MPEGReaders::randomize() {

	vector<string> mpeg_paths= list_files("../data/video_samples", "mov");

	unsigned int n_reader_configs= 3;
	vector<ReaderConfig> reader_configs;
	for (unsigned int idx_reader=0; idx_reader<n_reader_configs; ++idx_reader) {
		AlphaConfig alpha_config;
		unsigned int n_polygons= rand_int(1, 10);
		for (unsigned int i=0; i<n_polygons; ++i) {
			alpha_config._decrease_speed= rand_float(0.01f, 0.1f);

			float xsize= rand_float(0.01f, 0.3f);
			float ysize= rand_float(0.01f, 0.3f);
			float xmin= rand_float(0.0f, 1.0f- xsize);
			float ymin= rand_float(0.0f, 1.0f- ysize);
			float points[8]= {xmin, ymin, xmin+ xsize, ymin, xmin+ xsize, ymin+ ysize, xmin, ymin+ ysize};
			float fadeout= rand_float(0.0f, 0.1f);
			float curve= rand_float(1.0f, 5.0f);
			float alpha_max= rand_float(0.3f, 1.0f);
			alpha_config._polygons.push_back(AlphaPolygon(points, 4, fadeout, curve, alpha_max));
		}

		TimeConfig time_config;
		unsigned int n_checkpoints= rand_int(2, 10);
		for (unsigned int j=0; j<n_checkpoints; ++j) {
			time_config._time_checkpoints.push_back(make_pair(rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f)));
		}
		time_config._speed= rand_float(0.01f, 0.05f);

		string mpeg_path= mpeg_paths[rand_int(0, mpeg_paths.size()- 1)];
		unsigned int key= 'a'+ idx_reader;

		reader_configs.push_back(ReaderConfig(alpha_config, time_config, mpeg_path, key));
	}

	unsigned int n_modifier_configs= 3;
	vector<ModifierConfig> modifier_configs;
	for (unsigned int idx_modifier=0; idx_modifier<n_modifier_configs; ++idx_modifier) {
		ModifierConfig modifier_config;
		for (unsigned int i=0; i<4; i++) {
			modifier_config._movie_mult[i]= rand_float(-2.0f, 2.0f);
			modifier_config._alpha_mult[i]= rand_float(-2.0f, 2.0f);
		}
		for (unsigned int i=0; i<2; i++) {
			modifier_config._movie_add[i]= rand_float(-1.0f, 1.0f);
			modifier_config._alpha_add[i]= rand_float(-1.0f, 1.0f);
		}
		modifier_config._movie_speed= rand_float(0.005f, 0.01f);
		modifier_config._alpha_speed= rand_float(0.005f, 0.01f);
		modifier_config._time_mult= rand_float(-2.0f, 2.0f);
		modifier_config._time_add= rand_float(-1.0f, 1.0f);
		modifier_config._time_speed= rand_float(0.005f, 0.01f);

		modifier_config._idx_track= idx_modifier % N_TRACKS;
		modifier_config._key= 'a'+ n_reader_configs+ idx_modifier;

		modifier_configs.push_back(ModifierConfig(modifier_config));
	}

	unsigned int alpha_width= 256;
	unsigned int alpha_height= 256;
	unsigned int alpha_depth= N_TRACKS;
	unsigned int alpha_depth0= n_reader_configs;
	unsigned int time_width= 512;
	unsigned int time_height= N_TRACKS;
	unsigned int time_height0= n_reader_configs;
	unsigned int index_time_width= N_TRACKS;
	unsigned int index_movie_width= N_TRACKS;
	unsigned int index_movie_width0= n_reader_configs;
	unsigned int global_alpha_width= N_TRACKS;
	unsigned int modifier_width= 32; // + petite puissance de 2 > 17 = (4+ 2+ 1)* 2+ 3
	unsigned int modifier_height= N_TRACKS;

	GlobalConfig config(alpha_width, alpha_height, alpha_depth, alpha_depth0, time_width, time_height, time_height0,
		index_time_width, index_movie_width, index_movie_width0, global_alpha_width, modifier_width, modifier_height,
		reader_configs, modifier_configs);
	//cout << config;
	set_config(config);
}


void MPEGReaders::load_mpegs() {
	glDeleteTextures(N_MAX_MOVIES, _movies_ids);
	
	glGenTextures(N_MAX_MOVIES, _movies_ids);
	vector<string> mpegs_paths= get_mpegs_paths();
	for (unsigned int i=0; i<N_MAX_MOVIES; ++i) {
		if (i>= mpegs_paths.size()) {
			_movies_ids[i]= _movies_ids[i % mpegs_paths.size()];
			continue;
		}
		
		MPEG * mpeg= new MPEG(mpegs_paths[i]);
		
		glActiveTexture(GL_TEXTURE0+ _movie_textures_indices[i]);
		glBindTexture(GL_TEXTURE_3D, _movies_ids[i]);

		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, mpeg->_width, mpeg->_height, mpeg->_n_frames, 0, GL_RGB, GL_UNSIGNED_BYTE, mpeg->_data);
		delete mpeg;

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R    , GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S    , GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);
		
		glBindTexture(GL_TEXTURE_3D, 0);
	}

	glUniform1iv(_movie_loc, N_MAX_MOVIES, &_movie_textures_indices[0]);
}


void MPEGReaders::init_arrays() {
	if (_alpha_data0) {
		delete[] _alpha_data0;
	}
	_alpha_data0= new float[_config._alpha_width* _config._alpha_height* _config._alpha_depth0];
	for (unsigned int i=0; i<_config._alpha_width* _config._alpha_height* _config._alpha_depth0; ++i) {
		_alpha_data0[i]= 0.0f;
	}

	if (_alpha_data) {
		delete[] _alpha_data;
	}
	_alpha_data= new float[_config._alpha_width* _config._alpha_height* _config._alpha_depth];
	for (unsigned int i=0; i<_config._alpha_width* _config._alpha_height* _config._alpha_depth; ++i) {
		_alpha_data[i]= 0.0f;
	}

	if (_time_data0) {
		delete[] _time_data0;
	}
	_time_data0= new float[_config._time_width* _config._time_height0];
	for (unsigned int i=0; i<_config._time_width* _config._time_height0; ++i) {
		_time_data0[i]= 0.0f;
	}

	if (_time_data) {
		delete[] _time_data;
	}
	_time_data= new float[_config._time_width* _config._time_height];
	for (unsigned int i=0; i<_config._time_width* _config._time_height; ++i) {
		_time_data[i]= 0.0f;
	}

	if (_index_time_data) {
		delete[] _index_time_data;
	}
	_index_time_data= new float[_config._index_time_width];
	for (unsigned int i=0; i<_config._index_time_width; ++i) {
		_index_time_data[i]= 0.0f;
	}

	if (_index_movie_data0) {
		delete[] _index_movie_data0;
	}
	_index_movie_data0= new unsigned int[_config._index_movie_width0];
	for (unsigned int i=0; i<_config._index_movie_width0; ++i) {
		_index_movie_data0[i]= 0;
	}

	if (_index_movie_data) {
		delete[] _index_movie_data;
	}
	_index_movie_data= new unsigned int[_config._index_movie_width];
	for (unsigned int i=0; i<_config._index_movie_width; ++i) {
		_index_movie_data[i]= 0;
	}

	if (_global_alpha_data) {
		delete[] _global_alpha_data;
	}
	_global_alpha_data= new float[_config._global_alpha_width];
	for (unsigned int i=0; i<_config._global_alpha_width; ++i) {
		_global_alpha_data[i]= 0.0f;
	}

	if (_modifier_data) {
		delete[] _modifier_data;
	}
	_modifier_data= new float[_config._modifier_width* _config._modifier_height];
	for (unsigned int i=0; i<_config._modifier_width* _config._modifier_height; ++i) {
		_modifier_data[i]= 0.0f;
	}
}


void MPEGReaders::compute_alpha_data0() {
	for (unsigned int i=0; i<_config._alpha_width* _config._alpha_height* _config._alpha_depth0; ++i) {
		_alpha_data0[i]= 0.0f;
	}

	for (unsigned int idx_reader=0; idx_reader<_config._reader_configs.size(); ++idx_reader) {
		for (unsigned int idx_poly=0; idx_poly<_config._reader_configs[idx_reader]._alpha_config._polygons.size(); ++idx_poly) {
			AlphaPolygon alpha_poly= _config._reader_configs[idx_reader]._alpha_config._polygons[idx_poly];

			float xmin= alpha_poly._polygon._aabb->_pos.x;
			float ymin= alpha_poly._polygon._aabb->_pos.y;
			float xmax= xmin+ alpha_poly._polygon._aabb->_size.x;
			float ymax= ymin+ alpha_poly._polygon._aabb->_size.y;

			int imin= (int)((xmin- alpha_poly._fadeout)* (float)(_config._alpha_width));
			if (imin< 0) {
				imin= 0;
			}
			int imax= (int)((xmax+ alpha_poly._fadeout)* (float)(_config._alpha_width));
			if (imax>= _config._alpha_width) {
				imax= _config._alpha_width- 1;
			}
			int jmin= (int)((ymin- alpha_poly._fadeout)* (float)(_config._alpha_height));
			if (jmin< 0) {
				jmin= 0;
			}
			int jmax= (int)((ymax+ alpha_poly._fadeout)* (float)(_config._alpha_height));
			if (jmax>= _config._alpha_height) {
				jmax=_config. _alpha_height- 1;
			}

			for (unsigned int i=imin; i<=imax; ++i) {
				for (unsigned int j=jmin; j<=jmax; ++j) {
					glm::vec2 pt((float)(i)/ (float)(_config._alpha_width)+ 0.5f/ _config._alpha_width, (float)(j)/ (float)(_config._alpha_height)+ 0.5f/ _config._alpha_height);
					float d= distance_poly_pt(&alpha_poly._polygon, pt, 0);
					if ((alpha_poly._fadeout> 1e-5) && (d<= alpha_poly._fadeout)) {
						_alpha_data0[_config._alpha_width* _config._alpha_height* idx_reader+ i* _config._alpha_height+ j]+= (1.0f- pow(d/ alpha_poly._fadeout, alpha_poly._curve))* alpha_poly._alpha_max;
					}
					else if (d< 1e-5) {
						_alpha_data0[_config._alpha_width* _config._alpha_height* idx_reader+ i* _config._alpha_height+ j]= 1.0f;
					}
				}
			}
		}
	}

	for (unsigned int i=0; i<_config._alpha_width* _config._alpha_height* _config._alpha_depth0; ++i) {
		if (_alpha_data0[i]> 1.0f) {
			_alpha_data0[i]= 1.0f;
		}
	}
}


void MPEGReaders::compute_time_data0() {
	for (unsigned int i=0; i<_config._time_width* _config._time_height0; ++i) {
		_time_data0[i]= 0.0f;
	}
	for (unsigned int idx_reader=0; idx_reader<_config._reader_configs.size(); ++idx_reader) {
		for (unsigned int k=0; k<_config._reader_configs[idx_reader]._time_config._time_checkpoints.size()- 1; ++k) {
			float t0= _config._reader_configs[idx_reader]._time_config._time_checkpoints[k].first;
			float t1= _config._reader_configs[idx_reader]._time_config._time_checkpoints[k+ 1].first;
			float val0= _config._reader_configs[idx_reader]._time_config._time_checkpoints[k].second;
			float val1= _config._reader_configs[idx_reader]._time_config._time_checkpoints[k+ 1].second;
			unsigned int j0= (unsigned int)(t0* (float)(_config._time_width));
			unsigned int j1= (unsigned int)(t1* (float)(_config._time_width));
			for (unsigned int j=j0; j<j1; ++j) {
				_time_data0[_config._time_width* idx_reader+ j]= val0+ ((float)(j)/ (float)(_config._time_width)- t0)* (val1- val0)/ (t1- t0);
			}
		}
	}
}


void MPEGReaders::compute_index_movie_data0() {
	vector<string> mpegs_paths= get_mpegs_paths();
	for (unsigned int idx_reader=0; idx_reader<_config._reader_configs.size(); ++idx_reader) {
		unsigned int idx_mpeg= find(mpegs_paths.begin(), mpegs_paths.end(), _config._reader_configs[idx_reader]._mpeg_path)- mpegs_paths.begin();
		_index_movie_data0[idx_reader]= idx_mpeg;
	}
}


void MPEGReaders::compute_modifier_data() {
	for (unsigned int idx_track=0; idx_track<N_TRACKS; ++idx_track) {
		int idx_modifier= get_idx_modifier(idx_track);

		if (idx_modifier< 0) {
			_modifier_data[_config._modifier_width* idx_track+ 0]= 1.0f;
			_modifier_data[_config._modifier_width* idx_track+ 1]= 0.0f;
			_modifier_data[_config._modifier_width* idx_track+ 2]= 0.0f;
			_modifier_data[_config._modifier_width* idx_track+ 3]= 1.0f;
			_modifier_data[_config._modifier_width* idx_track+ 4]= 0.0f;
			_modifier_data[_config._modifier_width* idx_track+ 5]= 0.0f;
			_modifier_data[_config._modifier_width* idx_track+ 6]= 1.0f;

			_modifier_data[_config._modifier_width* idx_track+ 7]= 1.0f;
			_modifier_data[_config._modifier_width* idx_track+ 8]= 0.0f;
			_modifier_data[_config._modifier_width* idx_track+ 9]= 0.0f;
			_modifier_data[_config._modifier_width* idx_track+ 10]= 1.0f;
			_modifier_data[_config._modifier_width* idx_track+ 11]= 0.0f;
			_modifier_data[_config._modifier_width* idx_track+ 12]= 0.0f;
			_modifier_data[_config._modifier_width* idx_track+ 13]= 1.0f;

			_modifier_data[_config._modifier_width* idx_track+ 14]= 1.0f;
			_modifier_data[_config._modifier_width* idx_track+ 15]= 0.0f;
			_modifier_data[_config._modifier_width* idx_track+ 16]= 1.0f;
		}
		else {
			_modifier_data[_config._modifier_width* idx_track+ 0]= _config._modifier_configs[idx_modifier]._movie_mult[0];
			_modifier_data[_config._modifier_width* idx_track+ 1]= _config._modifier_configs[idx_modifier]._movie_mult[1];
			_modifier_data[_config._modifier_width* idx_track+ 2]= _config._modifier_configs[idx_modifier]._movie_mult[2];
			_modifier_data[_config._modifier_width* idx_track+ 3]= _config._modifier_configs[idx_modifier]._movie_mult[3];
			_modifier_data[_config._modifier_width* idx_track+ 4]= _config._modifier_configs[idx_modifier]._movie_add[0];
			_modifier_data[_config._modifier_width* idx_track+ 5]= _config._modifier_configs[idx_modifier]._movie_add[1];
			_modifier_data[_config._modifier_width* idx_track+ 6]= 1.0f;

			_modifier_data[_config._modifier_width* idx_track+ 7]= _config._modifier_configs[idx_modifier]._alpha_mult[0];
			_modifier_data[_config._modifier_width* idx_track+ 8]= _config._modifier_configs[idx_modifier]._alpha_mult[1];
			_modifier_data[_config._modifier_width* idx_track+ 9]= _config._modifier_configs[idx_modifier]._alpha_mult[2];
			_modifier_data[_config._modifier_width* idx_track+ 10]= _config._modifier_configs[idx_modifier]._alpha_mult[3];
			_modifier_data[_config._modifier_width* idx_track+ 11]= _config._modifier_configs[idx_modifier]._alpha_add[0];
			_modifier_data[_config._modifier_width* idx_track+ 12]= _config._modifier_configs[idx_modifier]._alpha_add[1];
			_modifier_data[_config._modifier_width* idx_track+ 13]= 1.0f;

			_modifier_data[_config._modifier_width* idx_track+ 14]= _config._modifier_configs[idx_modifier]._time_mult;
			_modifier_data[_config._modifier_width* idx_track+ 15]= _config._modifier_configs[idx_modifier]._time_add;
			_modifier_data[_config._modifier_width* idx_track+ 16]= 1.0f;
		}
	}
}


void MPEGReaders::init_alpha_texture() {
	glGenTextures(1, &_alpha_id);

	glActiveTexture(GL_TEXTURE0+ _alpha_texture_index);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _alpha_id);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R32F, _config._alpha_width, _config._alpha_height, _config._alpha_depth, 0, GL_RED, GL_FLOAT, _alpha_data);
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
	glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, GL_R32F, _config._time_width, _config._time_height, 0, GL_RED, GL_FLOAT, _time_data);
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
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, _config._index_time_width, 0, GL_RED, GL_FLOAT, _index_time_data);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_1D, 0);

	glUniform1i(_index_time_loc, _index_time_texture_index);
}


void MPEGReaders::init_index_movie_texture() {
	glGenTextures(1, &_index_movie_id);

	glActiveTexture(GL_TEXTURE0+ _index_movie_texture_index);
	glBindTexture(GL_TEXTURE_1D, _index_movie_id);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R32UI, _config._index_movie_width, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, _index_movie_data);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_1D, 0);

	glUniform1i(_index_movie_loc, _index_movie_texture_index);
}


void MPEGReaders::init_global_alpha_texture() {
	glGenTextures(1, &_global_alpha_id);

	glActiveTexture(GL_TEXTURE0+ _global_alpha_texture_index);
	glBindTexture(GL_TEXTURE_1D, _global_alpha_id);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, _config._global_alpha_width, 0, GL_RED, GL_FLOAT, _global_alpha_data);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_1D, 0);

	glUniform1i(_global_alpha_loc, _global_alpha_texture_index);
}


void MPEGReaders::init_modifier_texture() {
	glGenTextures(1, &_modifier_id);

	glActiveTexture(GL_TEXTURE0+ _modifier_texture_index);
	glBindTexture(GL_TEXTURE_1D_ARRAY, _modifier_id);
	glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, GL_R32F, _config._modifier_width, _config._modifier_height, 0, GL_RED, GL_FLOAT, _modifier_data);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_1D_ARRAY, 0);

	glUniform1i(_modifier_loc, _modifier_texture_index);
}


void MPEGReaders::prepare2draw() {
	glUniform1iv(_movie_loc, N_MAX_MOVIES, &_movie_textures_indices[0]);
	for (unsigned int i=0; i<N_MAX_MOVIES; ++i) {
		glActiveTexture(GL_TEXTURE0+ _movie_textures_indices[i]);
		glBindTexture(GL_TEXTURE_3D, _movies_ids[i]);
	}

	glUniform1i(_alpha_loc, _alpha_texture_index);
	glActiveTexture(GL_TEXTURE0+ _alpha_texture_index);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _alpha_id);

	glUniform1i(_time_loc, _time_texture_index);
	glActiveTexture(GL_TEXTURE0+ _time_texture_index);
	glBindTexture(GL_TEXTURE_1D_ARRAY, _time_id);

	glUniform1i(_index_time_loc, _index_time_texture_index);
	glActiveTexture(GL_TEXTURE0+ _index_time_texture_index);
	glBindTexture(GL_TEXTURE_1D, _index_time_id);

	glUniform1i(_index_movie_loc, _index_movie_texture_index);
	glActiveTexture(GL_TEXTURE0+ _index_movie_texture_index);
	glBindTexture(GL_TEXTURE_1D, _index_movie_id);

	glUniform1i(_global_alpha_loc, _global_alpha_texture_index);
	glActiveTexture(GL_TEXTURE0+ _global_alpha_texture_index);
	glBindTexture(GL_TEXTURE_1D, _global_alpha_id);
}


void MPEGReaders::update() {
	for (unsigned int idx_track=0; idx_track<N_TRACKS; ++idx_track) {
		next_index_modifier(idx_track);
		if (_index_reader[idx_track]>= 0) {
			next_index_time(idx_track);
			if (!_note_on[idx_track]) {
				decrease_alpha(idx_track);
			}
		}
	}
}


void MPEGReaders::update_alpha_texture(unsigned int idx_track) {
	glBindTexture(GL_TEXTURE_2D_ARRAY, _alpha_id);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, idx_track, _config._alpha_width, _config._alpha_height, 1, GL_RED, GL_FLOAT, _alpha_data+ _config._alpha_width* _config._alpha_height* idx_track);
}


void MPEGReaders::update_time_texture(unsigned int idx_track) {
	glBindTexture(GL_TEXTURE_1D_ARRAY, _time_id);
	glTexSubImage2D(GL_TEXTURE_1D_ARRAY, 0, 0, idx_track, _config._time_width, 1, GL_RED, GL_FLOAT, _time_data+ _config._time_width* idx_track);
}


void MPEGReaders::update_index_time_texture(unsigned int idx_track) {
	glBindTexture(GL_TEXTURE_1D, _index_time_id);
	glTexSubImage1D(GL_TEXTURE_1D, 0, idx_track, 1, GL_RED, GL_FLOAT, &_index_time_data[idx_track]);
}


void MPEGReaders::update_index_movie_texture(unsigned int idx_track) {
	glBindTexture(GL_TEXTURE_1D, _index_movie_id);
	glTexSubImage1D(GL_TEXTURE_1D, 0, idx_track, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &_index_movie_data[idx_track]);
}


void MPEGReaders::update_global_alpha_texture(unsigned int idx_track) {
	glBindTexture(GL_TEXTURE_1D, _global_alpha_id);
	glTexSubImage1D(GL_TEXTURE_1D, 0, idx_track, 1, GL_RED, GL_FLOAT, &_global_alpha_data[idx_track]);
}


void MPEGReaders::update_modifier_texture(unsigned int idx_track) {
	glBindTexture(GL_TEXTURE_1D_ARRAY, _modifier_id);
	glTexSubImage2D(GL_TEXTURE_1D_ARRAY, 0, 0, idx_track, _config._modifier_width, 1, GL_RED, GL_FLOAT, _modifier_data+ _config._modifier_width* idx_track);
}


void MPEGReaders::decrease_alpha(unsigned int idx_track) {
	bool active= false;
	for (unsigned int i=0; i<_config._alpha_width* _config._alpha_height; ++i) {
		_alpha_data[_config._alpha_width* _config._alpha_height* idx_track+ i]-= _config._reader_configs[_index_reader[idx_track]]._alpha_config._decrease_speed;
		if (_alpha_data[_config._alpha_width* _config._alpha_height* idx_track+ i]> 0.0f) {
			active= true;
		}
		else {
			_alpha_data[_config._alpha_width* _config._alpha_height* idx_track+ i]= 0.0f;
		}
	}
	if (!active) {
		_index_reader[idx_track]= -1;
	}

	update_alpha_texture(idx_track);
}


void MPEGReaders::next_index_time(unsigned int idx_track) {
	_index_time_data[idx_track]+= _config._reader_configs[_index_reader[idx_track]]._time_config._speed;
	while (_index_time_data[idx_track]> 1.0f) {
		_index_time_data[idx_track]-= 1.0f;
	}
	update_index_time_texture(idx_track);
}


void MPEGReaders::next_index_modifier(unsigned int idx_track) {
	int idx_modifier= get_idx_modifier(idx_track);
	if (idx_modifier< 0) {
		return;
	}
	_modifier_data[_config._modifier_width* idx_track+ 6 ]+= _config._modifier_configs[idx_modifier]._movie_speed;
	_modifier_data[_config._modifier_width* idx_track+ 13]+= _config._modifier_configs[idx_modifier]._alpha_speed;
	_modifier_data[_config._modifier_width* idx_track+ 16]+= _config._modifier_configs[idx_modifier]._time_speed;
	if (_modifier_data[_config._modifier_width* idx_track+ 6]> 1.0f) {
		_modifier_data[_config._modifier_width* idx_track+ 6]= 1.0f;
	}
	if (_modifier_data[_config._modifier_width* idx_track+ 13]> 1.0f) {
		_modifier_data[_config._modifier_width* idx_track+ 13]= 1.0f;
	}
	if (_modifier_data[_config._modifier_width* idx_track+ 16]> 1.0f) {
		_modifier_data[_config._modifier_width* idx_track+ 16]= 1.0f;
	}
	update_modifier_texture(idx_track);
}


void MPEGReaders::note_on(unsigned int idx_track, unsigned int key, float amplitude) {
	int idx_reader= get_idx_reader(key);

	if (idx_reader>= 0) {
		_index_reader[idx_track]= idx_reader;
		_note_on[idx_track]= true;

		_index_time_data[idx_track]= 0.0f;
		update_index_time_texture(idx_track);
		
		memcpy(_alpha_data+ _config._alpha_width* _config._alpha_height* idx_track, _alpha_data0+ _config._alpha_width* _config._alpha_height* _index_reader[idx_track], _config._alpha_width* _config._alpha_height* sizeof(float));
		update_alpha_texture(idx_track);

		memcpy(_time_data+ _config._time_width* idx_track, _time_data0+ _config._time_width* _index_reader[idx_track], _config._time_width* sizeof(float));
		update_time_texture(idx_track);

		_index_movie_data[idx_track]= _index_movie_data0[_index_reader[idx_track]];
		update_index_movie_texture(idx_track);

		_global_alpha_data[idx_track]= amplitude;
		update_global_alpha_texture(idx_track);
	}

	int idx_modifier= get_idx_modifier_by_key(key);
	if (idx_modifier>= 0) {
		unsigned int idx_track_modified= _config._modifier_configs[idx_modifier]._idx_track;
		_modifier_data[_config._modifier_width* idx_track_modified+ 6 ]= 0.0f;
		_modifier_data[_config._modifier_width* idx_track_modified+ 13]= 0.0f;
		_modifier_data[_config._modifier_width* idx_track_modified+ 16]= 0.0f;
		update_modifier_texture(idx_track_modified);
	}
}


void MPEGReaders::note_off(unsigned int idx_track) {
	_note_on[idx_track]= false;
}


int MPEGReaders::get_idx_reader(unsigned int key) {
	for (int idx_reader=0; idx_reader<_config._reader_configs.size(); ++idx_reader) {
		if (_config._reader_configs[idx_reader]._key== key) {
			return idx_reader;
		}
	}
	return -1;
}


int MPEGReaders::get_idx_modifier(unsigned int idx_track) {
	for (int idx_modifier=0; idx_modifier<_config._modifier_configs.size(); ++idx_modifier) {
		if (_config._modifier_configs[idx_modifier]._idx_track== idx_track) {
			return idx_modifier;
		}
	}
	return -1;
}


int MPEGReaders::get_idx_modifier_by_key(unsigned int key) {
	for (int idx_modifier=0; idx_modifier<_config._modifier_configs.size(); ++idx_modifier) {
		if (_config._modifier_configs[idx_modifier]._key== key) {
			return idx_modifier;
		}
	}
	return -1;
}


vector<string> MPEGReaders::get_mpegs_paths() {
	vector<string> result;
	for (auto & rc : _config._reader_configs) {
		if (find(result.begin(), result.end(), rc._mpeg_path)== result.end()) {
			result.push_back(rc._mpeg_path);
		}
	}
	return result;
}