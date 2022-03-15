#define __STDC_CONSTANT_MACROS
extern "C" {
#include<libavutil/avutil.h>
#include<libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <iostream>


static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize, char *filename) {
	FILE *f;
	int i;

	f = fopen(filename,"wb");
	fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
	for (i = 0; i < ysize; i++) {
		fwrite(buf + i * wrap, 1, xsize, f);
	}
	fclose(f);
}


int main(int argc, char **argv) {
    AVFormatContext* ctx_format = nullptr;
    AVCodecContext* ctx_codec = nullptr;
    const AVCodec* codec = nullptr;
    AVFrame* frame = av_frame_alloc();
    int stream_idx;
    SwsContext* ctx_sws = nullptr;
    const char* fin = argv[1];
    const char* out = argv[2];
    AVStream *vid_stream = nullptr;
    AVPacket* pkt = av_packet_alloc();


    if (int ret = avformat_open_input(&ctx_format, fin, nullptr, nullptr) != 0) {
        std::cout << 1 << std::endl;
        return ret;
    }
    if (avformat_find_stream_info(ctx_format, nullptr) < 0) {
        std::cout << 2 << std::endl;
        return -1; // Couldn't find stream information
    }
    av_dump_format(ctx_format, 0, fin, false);

    for (int i = 0; i < ctx_format->nb_streams; i++)
        if (ctx_format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            stream_idx = i;
            vid_stream = ctx_format->streams[i];
            break;
    }
    if (vid_stream == nullptr) {
        std::cout << 4 << std::endl;
        return -1;
    }

    codec = avcodec_find_decoder(vid_stream->codecpar->codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }
    ctx_codec = avcodec_alloc_context3(codec);

    if(avcodec_parameters_to_context(ctx_codec, vid_stream->codecpar)<0)
        std::cout << 512;
    if (avcodec_open2(ctx_codec, codec, nullptr)<0) {
        std::cout << 5;
        return -1;
    }

    //av_new_packet(pkt, pic_size);

	while(av_read_frame(ctx_format, pkt) >= 0){
		if (pkt->stream_index == stream_idx) {
			int ret = avcodec_send_packet(ctx_codec, pkt);
			if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
				std::cout << "avcodec_send_packet: " << ret << std::endl;
				break;
			}
			while (ret  >= 0) {
				ret = avcodec_receive_frame(ctx_codec, frame);
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
					//std::cout << "avcodec_receive_frame: " << ret << std::endl;
					break;
				}
				std::cout << "frame: " << ctx_codec->frame_number << std::endl;

                char buf[1024];
                snprintf(buf, sizeof(buf), "%s-%d.pgm", out, ctx_codec->frame_number);
            	pgm_save(frame->data[0], frame->linesize[0], frame->width, frame->height, buf);

			}
		}
		av_packet_unref(pkt);
	}

    avformat_close_input(&ctx_format);
    av_packet_unref(pkt);
    avcodec_free_context(&ctx_codec);
    avformat_free_context(ctx_format);
    return 0;
}
