#ifndef MPEG_H
#define MPEG_H

#include <string>

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
	MPEGWriter(unsigned int width, unsigned int height, unsigned int fps, unsigned int bitrate, std::string output_path);
	~MPEGWriter();
	void push_frame(unsigned char * data);
	void finish();


	unsigned int _width;
	unsigned int _height;
	unsigned int _fps;
	unsigned int _bitrate;
	std::string _output_path;
	
	AVFrame * _frame;
	AVFrame * _frame_rgb;
	AVFrame * _filtered_frame;
	AVCodecContext * _codec_context;
	AVStream * _stream;
	SwsContext * _sws_context;
	AVFormatContext * _format_context;
	const AVOutputFormat * _output_format;
	const AVCodec * _codec;
	unsigned int _frame_count;
	AVPacket * _pkt;
	
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

#endif
