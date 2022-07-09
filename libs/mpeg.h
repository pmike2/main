#ifndef MPEG_H
#define MPEG_H

#include <string>
#include <chrono>
#include <vector>
#include <thread>
#include <utility>

extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}

#include "thread.h"


class MPEGReader {
public:
	MPEGReader();
	MPEGReader(std::string input_path);
	~MPEGReader();
	unsigned char * get_frame(unsigned int frame_idx);


	unsigned int _n_frames;
	unsigned int _fps;
	unsigned char * _data;
	unsigned int _width;
	unsigned int _height;
	unsigned int _frame_size;
};


class MPEGWriter {
public:
	MPEGWriter();
	MPEGWriter(unsigned int width, unsigned int height, unsigned int fps, unsigned int bitrate, std::string output_path, bool vflip, bool use_global_fps);
	~MPEGWriter();
	void init_filters();
	void push_frame(unsigned char * data, unsigned int ms);
	void finish();


	unsigned int _width;
	unsigned int _height;
	unsigned int _fps;
	unsigned int _bitrate;
	std::string _output_path;
	std::string _output_path_vflipped;
	bool _vflip;
	bool _use_global_fps;
	
	AVFrame * _frame;
	AVFrame * _frame_rgb;
	AVCodecContext * _codec_context;
	AVStream * _stream;
	SwsContext * _sws_context;
	AVFormatContext * _format_context;
	const AVOutputFormat * _output_format;
	const AVCodec * _codec;
	unsigned int _frame_count;
	AVPacket * _pkt;
	
	// a revoir
	AVFrame * _filtered_frame;
	const AVFilter * _filter_buffer_sink;
	const AVFilter * _filter_buffer;
	const AVFilter * _filter_vflip;
	AVFilterInOut * _filter_out;
	AVFilterInOut * _filter_in;
	AVFilterGraph * _filter_graph;
	AVFilterContext * _filter_context_buffer;
	AVFilterContext * _filter_context_buffer_sink;
	AVFilterContext * _filter_context_vflip;
};


class MPEGWriterHelper : public MPEGWriter {
public:
	MPEGWriterHelper();
	MPEGWriterHelper(unsigned int width, unsigned int height, unsigned int fps, unsigned int bitrate, std::string output_path, bool vflip, bool use_global_fps);
	~MPEGWriterHelper();
	void add2queue(unsigned char * data);
	void read_queue();


	std::thread _writing_thread;
	SafeQueue<std::pair<unsigned char *, unsigned int> > _safe_queue;
	std::chrono::system_clock::time_point _start_point;
	std::atomic_bool _stop_thr;
};


#endif
