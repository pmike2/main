/*

http://dranger.com/ffmpeg/tutorial01.html

Utilisation :
./decode_mp4 ../data/test.mp4 ../data/result

*/

#define __STDC_CONSTANT_MACROS
extern "C" {
#include<libavutil/avutil.h>
#include<libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <iostream>

using namespace std;


/*
PGM  = fichier en niveaux de gris
PPM = couleur
*/

/*
static void pgm_save(unsigned char * buf, int wrap, int xsize, int ysize, char * filename) {
	FILE *f;

	f = fopen(filename, "wb");
	fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
	for (int i= 0; i< ysize; i++) {
		fwrite(buf + i * wrap, 1, xsize, f);
	}
	fclose(f);
}
*/

void ppm_save(AVFrame * frame_rgb, int width, int height, char * filename) {
	FILE *f;

	f = fopen(filename, "wb");
	fprintf(f, "P6\n%d %d\n255\n", width, height);
	for (int y=0; y<height; y++) {
		fwrite(frame_rgb->data[0]+ y* frame_rgb->linesize[0], 1, width* 3, f);
	}
	fclose(f);
}


int main(int argc, char **argv) {
	if (argc!= 3) {
		cerr << "Donner en argument le fichier mpeg en entrée et le dossier en sortie\n";
		return 1;
	}
	// fichier mp4 en entrée
	const char * file_in= argv[1];

	// dossier en sortie
	const char * dir_out= argv[2];

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
	uint8_t * buffer_rgb= 0;
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
	ret = avformat_open_input(&ctx_format, file_in, 0, 0);
	if (ret!= 0) {
		cout << "ERREUR avformat_open_input\n";
		return ret;
	}

	/*
	Read packets of a media file to get stream information.
	This is useful for file formats with no headers such as MPEG. This function also computes the real framerate in case of MPEG-2 repeat frame mode.
	The logical file position is not changed by this function; examined packets may be buffered for later processing.
	*/
	ret= avformat_find_stream_info(ctx_format, 0);
	if (ret< 0) {
		cout << "ERREUR avformat_find_stream_info\n";
		return ret; // Couldn't find stream information
	}

	// Print detailed information about the input or output format, such as duration, bitrate, streams, container, programs, metadata, side data, codec and time base.
	av_dump_format(ctx_format, 0, file_in, false);

	// pour chaque stream, si c'est un stream video on l'assigne à vid_stream
	for (int i = 0; i < ctx_format->nb_streams; i++)
		if (ctx_format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			stream_idx = i;
			vid_stream = ctx_format->streams[i];
			break;
	}
	if (vid_stream== 0) {
		cout << "ERREUR pas de stream video trouvé\n";
		return 1;
	}

	// 	Find a registered decoder with a matching codec ID
	codec= avcodec_find_decoder(vid_stream->codecpar->codec_id);
	if (!codec) {
		cout << "ERREUR avcodec_find_decoder\n";
		return 1;
	}

	// Allocate an AVCodecContext and set its fields to default values.
	ctx_codec= avcodec_alloc_context3(codec);

	// 	Fill the codec context based on the values from the supplied codec parameters
	ret= avcodec_parameters_to_context(ctx_codec, vid_stream->codecpar);
	if (ret< 0) {
		cout << "ERREUR avcodec_parameters_to_context\n";
		return ret;
	}
	
	// Initialize the codec context to use the given AVCodec.
	ret= avcodec_open2(ctx_codec, codec, 0);
	if (ret< 0) {
		cout << "ERREUR avcodec_open2\n";
		return ret;
	}

	// Determine required buffer size and allocate buffer
	buffer_rgb_size= av_image_get_buffer_size(AV_PIX_FMT_RGB24, ctx_codec->width, ctx_codec->height, 32);
	buffer_rgb= (uint8_t *)av_malloc(buffer_rgb_size* sizeof(uint8_t));

	// Assign appropriate parts of buffer to image planes in frame_rgb
	av_image_fill_arrays(frame_rgb->data, frame_rgb->linesize, buffer_rgb, AV_PIX_FMT_RGB24, ctx_codec->width, ctx_codec->height, 1);

	// servira a faire la conversion de format de couleur
	sws_ctx= sws_getContext(ctx_codec->width, ctx_codec->height, ctx_codec->pix_fmt, ctx_codec->width, ctx_codec->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, 0, 0, 0);

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

				cout << "frame: " << ctx_codec->frame_number << "\n";

				// conversion de frame vers frame_rgb
				sws_scale(sws_ctx, (uint8_t const * const *)frame->data, frame->linesize, 0, ctx_codec->height, frame_rgb->data, frame_rgb->linesize);

				// sauvegarde du frame dans un fichier PPM
				char file_out[1024];
				snprintf(file_out, sizeof(file_out), "%s/frame-%d.ppm", dir_out, ctx_codec->frame_number);
				//pgm_save(frame->data[0], frame->linesize[0], frame->width, frame->height, file_out);
				ppm_save(frame_rgb, ctx_codec->width, ctx_codec->height, file_out);
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
	
	return 0;
}
