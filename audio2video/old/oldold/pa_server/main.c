
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#include "portaudio.h"

#ifdef WIN32
#include <windows.h>

#if PA_USE_ASIO
#include "pa_asio.h"
#endif
#endif


// valeurs a ajuster...
#define MAX_TRIGGER_DURATION 200 // en ms
#define PORT 50000
#define SAMPLE_RATE 48000
#define FRAMES_PRE_BUFFER 256
#define N_MAX_CHANNELS 32
#define AMPLITUDE_TRESHOLD 0.3


struct Channel {
	int sockfd, newsockfd;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;

	int is_triggered;
	struct timeval t;
};


PaStream *stream;
PaError err;

int idx_channels[N_MAX_CHANNELS];
int idx_device= -1;

struct Channel channels[N_MAX_CHANNELS];


// copiés de utile.h de la partie client
unsigned int diff_time_ms(struct timeval * after, struct timeval * before) {
	struct timeval diff;
	timersub(after, before, &diff);
	unsigned long i= diff.tv_sec* 1e6+ diff.tv_usec;
	return i/ 1000;
}


unsigned int diff_time_ms_from_now(struct timeval * begin) {
	struct timeval now;
	gettimeofday(&now, NULL);
	return diff_time_ms(&now, begin);
}
// -------------------------------------


/*
void PrintSupportedStandardSampleRates(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters ) {
	double standardSampleRates[] = { 8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0,
		44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1};
	int i;

	for (i=0; standardSampleRates[i]> 0; i++) {
		err = Pa_IsFormatSupported(inputParameters, outputParameters, standardSampleRates[i]);
		if (err == paFormatIsSupported)
			printf("%8.2f ; ", standardSampleRates[i]);
	}
	printf("\n");
}
*/

int list_devices() {
	int	i, numDevices, defaultDisplayed;
	const PaDeviceInfo *deviceInfo;
	PaStreamParameters inputParameters, outputParameters;
	
	numDevices = Pa_GetDeviceCount();
	if (numDevices < 0)
	{
		printf("ERROR: Pa_GetDeviceCount returned %d\n", numDevices);
	}
	
	printf("Number of devices = %d\n", numDevices);
	for (i=0; i<numDevices; i++) {
		deviceInfo= Pa_GetDeviceInfo(i);
		printf( "--------------------------------------- device #%d\n", i);
		
		defaultDisplayed= 0;
		if (i == Pa_GetDefaultInputDevice()) {
			printf("[ Default Input");
			defaultDisplayed = 1;
		}
		else if (i == Pa_GetHostApiInfo(deviceInfo->hostApi)->defaultInputDevice) {
			const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
			printf("[ Default %s Input", hostInfo->name);
			defaultDisplayed = 1;
		}
		
		if (i == Pa_GetDefaultOutputDevice()) {
			printf((defaultDisplayed ? "," : "["));
			printf("Default Output");
			defaultDisplayed = 1;
		}
		else if (i == Pa_GetHostApiInfo( deviceInfo->hostApi )->defaultOutputDevice) {
			const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
			printf((defaultDisplayed ? "," : "["));
			printf(" Default %s Output", hostInfo->name);
			defaultDisplayed = 1;
		}
		
		if (defaultDisplayed)
			printf( " ]\n" );
		
#ifdef WIN32
		/* Use wide char on windows, so we can show UTF-8 encoded device names */
		wchar_t wideName[MAX_PATH];
		MultiByteToWideChar(CP_UTF8, 0, deviceInfo->name, -1, wideName, MAX_PATH- 1);
		wprintf(L"Name = %s\n", wideName);
#else
		printf("Name = %s\n", deviceInfo->name);
#endif
		
		printf("Host API = %s\n", Pa_GetHostApiInfo(deviceInfo->hostApi)->name);
		printf("Max inputs = %d", deviceInfo->maxInputChannels);
		printf(", Max outputs = %d\n", deviceInfo->maxOutputChannels);
		/*printf("Default low input latency	 = %8.4f\n", deviceInfo->defaultLowInputLatency);
		printf("Default low output latency	 = %8.4f\n", deviceInfo->defaultLowOutputLatency);
		printf("Default high input latency	 = %8.4f\n", deviceInfo->defaultHighInputLatency);
		printf("Default high output latency = %8.4f\n", deviceInfo->defaultHighOutputLatency);*/
		
#ifdef WIN32
#if PA_USE_ASIO
		/* ASIO specific latency information */
		/*if (Pa_GetHostApiInfo(deviceInfo->hostApi)->type == paASIO) {
			long minLatency, maxLatency, preferredLatency, granularity;
			err = PaAsio_GetAvailableBufferSizes(i,	&minLatency, &maxLatency, &preferredLatency, &granularity);
			printf("ASIO minimum buffer size = %ld\n", minLatency);
			printf("ASIO maximum buffer size = %ld\n", maxLatency);
			printf("ASIO preferred buffer size = %ld\n", preferredLatency);
			if (granularity == -1) printf("ASIO buffer granularity = power of 2\n");
			else printf("ASIO buffer granularity = %ld\n", granularity);
		}*/
#endif /* PA_USE_ASIO */
#endif /* WIN32 */
		
		/*printf("Default sample rate = %8.2f\n", deviceInfo->defaultSampleRate);
		
		inputParameters.device = i;
		inputParameters.channelCount = deviceInfo->maxInputChannels;
		inputParameters.sampleFormat = paInt16;
		inputParameters.suggestedLatency = 0;
		inputParameters.hostApiSpecificStreamInfo = NULL;
		
		outputParameters.device = i;
		outputParameters.channelCount = deviceInfo->maxOutputChannels;
		outputParameters.sampleFormat = paInt16;
		outputParameters.suggestedLatency = 0;
		outputParameters.hostApiSpecificStreamInfo = NULL;
		
		if (inputParameters.channelCount > 0) {
			printf("Supported standard sample rates\n for half-duplex 16 bit %d channel input = \n", inputParameters.channelCount);
			PrintSupportedStandardSampleRates(&inputParameters, NULL);
		}
		
		if (outputParameters.channelCount > 0) {
			printf("Supported standard sample rates\n for half-duplex 16 bit %d channel output = \n", outputParameters.channelCount);
			PrintSupportedStandardSampleRates(NULL, &outputParameters);
		}
		
		if (inputParameters.channelCount > 0 && outputParameters.channelCount > 0) {
			printf("Supported standard sample rates\n for full-duplex 16 bit %d channel input, %d channel output = \n", inputParameters.channelCount, outputParameters.channelCount);
			PrintSupportedStandardSampleRates(&inputParameters, &outputParameters);
		}*/

	}
	
	printf("----------------------------------------------\n");
	return 0;
}


int pa_callback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {

	float *in = (float *) inputBuffer;
	int *data= (int*)(userData);
	int sockno= *data;
	float input;
	unsigned int i;
	int n;

	if (inputBuffer == NULL)
		return 0;

	if (channels[sockno].is_triggered)
		return 0;

	for (i=0; i<framesPerBuffer; i++) {
		input = *in++;
		
		if (fabs(input)> AMPLITUDE_TRESHOLD) {
			channels[sockno].is_triggered= 1;
			gettimeofday(&(channels[sockno].t), NULL);
			
			char c= '1';
			n = write(channels[sockno].newsockfd, &c, sizeof(c));
			//printf("trig %i\n", sockno);
			if (n < 0)
				printf("ERROR writing to socket\n");
			
			return 0;
		}
	}
	return 0;
}


void init_server_socket(int portno, int sockno) {
	channels[sockno].sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (channels[sockno].sockfd < 0) 
		printf("ERROR opening socket\n");

	int optval = 1;
    setsockopt(channels[sockno].sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
		
	bzero((char *) &(channels[sockno].serv_addr), sizeof(channels[sockno].serv_addr));
	channels[sockno].serv_addr.sin_family = AF_INET;
	channels[sockno].serv_addr.sin_addr.s_addr = INADDR_ANY;
	channels[sockno].serv_addr.sin_port = htons(portno);
	if (bind(channels[sockno].sockfd, (struct sockaddr *) &(channels[sockno].serv_addr), sizeof(channels[sockno].serv_addr)) < 0) 
		printf("ERROR on binding\n");
	
	listen(channels[sockno].sockfd, SOMAXCONN);
	channels[sockno].clilen = sizeof(channels[sockno].cli_addr);
	channels[sockno].newsockfd = accept(channels[sockno].sockfd, (struct sockaddr *) &(channels[sockno].cli_addr), &(channels[sockno].clilen));
	if (channels[sockno].newsockfd < 0)
		printf("ERROR on accept\n");
	
	// pour rendre le IO non bloquant (a faire du cote serveur ET client)
	int flags = fcntl(channels[sockno].newsockfd, F_GETFL, 0);
	fcntl(channels[sockno].newsockfd, F_SETFL, flags | O_NONBLOCK);
}


void init() {
	int i;
	// sert au passage d'argument au callback (voir Pa_OpenStream + bas)
	for (i=0; i<N_MAX_CHANNELS; i++)
		idx_channels[i]= i;

	for (i=0; i<N_MAX_CHANNELS; i++) {
		channels[i].is_triggered= 0;
	}

	err = Pa_Initialize();
	if (err != paNoError) {
		printf("ERROR: Pa_Initialize returned 0x%x\n", err);
	}
	//printf("PortAudio version number = %d\nPortAudio version text = '%s'\n", Pa_GetVersion(), Pa_GetVersionText());
	
	// si pas d'argument donné à l'executable
	if (idx_device< 0) {
		list_devices();
		printf("Quel device ? ");
		scanf("%d", &idx_device);
	}
	
	const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(idx_device);
	int idx_channel;
	// channel va de 1 à n_channels inclus
	for (idx_channel=1; idx_channel<=deviceInfo->maxInputChannels; idx_channel++) {
		PaStreamParameters inputParameters;
		bzero(&inputParameters, sizeof(inputParameters));
		inputParameters.device = idx_device;
		inputParameters.channelCount = idx_channel;
		inputParameters.hostApiSpecificStreamInfo = NULL;
		inputParameters.sampleFormat = paFloat32;
		inputParameters.suggestedLatency = Pa_GetDeviceInfo(idx_device)->defaultLowInputLatency ;
		inputParameters.hostApiSpecificStreamInfo = NULL;
	
		err = Pa_OpenStream(&stream, &inputParameters, 0, SAMPLE_RATE, FRAMES_PRE_BUFFER, paNoFlag,	pa_callback, &idx_channels[idx_channel- 1]);
		Pa_StartStream(stream);
	}
	
	// on emet de PORT_NUMBER à PORT_NUMBER+ N_MAX_CHANNELS- 1
	for (i=0; i<N_MAX_CHANNELS; i++) {
		init_server_socket(PORT+ i, i);
	}
}


void main_loop() {
	unsigned int i;
	while (1) {
		for (i=0; i<N_MAX_CHANNELS; i++) {
			if (channels[i].is_triggered) {
				unsigned int diff_ms= diff_time_ms_from_now(&(channels[i].t));
				if (diff_ms> MAX_TRIGGER_DURATION) {
					channels[i].is_triggered= 0;
				}
			}
		}
	}
}


void clean() {
	unsigned int i;
	
	err= Pa_StopStream(stream);
	Pa_CloseStream(stream);
	Pa_Terminate();

	for (i=0; i<N_MAX_CHANNELS; i++) {
		close(channels[i].newsockfd);
		close(channels[i].sockfd);
	}
}


int main(int argc, char *argv[])
{
	if (argc> 1) {
		sscanf(argv[1], "%d", &idx_device);
	}
	init();
	main_loop();
	clean();

	return 0;
}

