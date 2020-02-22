#include "pa_utils.h"


void print_supported_standard_sample_rates(const PaStreamParameters *input_parameters, const PaStreamParameters *output_parameters ) {
	double standard_sample_rates[] = { 8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0,
		44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1};
	int i;
	PaError err;

	for (i=0; standard_sample_rates[i]> 0; i++) {
		err= Pa_IsFormatSupported(input_parameters, output_parameters, standard_sample_rates[i]);
		if (err== paFormatIsSupported)
			printf("%8.2f ; ", standard_sample_rates[i]);
	}
	printf("\n");
}


int list_devices() {
	int	i, numDevices, defaultDisplayed;
	const PaDeviceInfo *deviceInfo;
	PaStreamParameters inputParameters, outputParameters;
	
	numDevices= Pa_GetDeviceCount();
	if (numDevices< 0)	{
		printf("ERROR: Pa_GetDeviceCount returned %d\n", numDevices);
	}
	
	printf("Number of devices = %d\n", numDevices);
	for (i=0; i<numDevices; i++) {
		deviceInfo= Pa_GetDeviceInfo(i);
		printf( "--------------------------------------- device #%d\n", i);
		
		defaultDisplayed= 0;
		if (i== Pa_GetDefaultInputDevice()) {
			printf("[ Default Input");
			defaultDisplayed = 1;
		}
		else if (i== Pa_GetHostApiInfo(deviceInfo->hostApi)->defaultInputDevice) {
			const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
			printf("[ Default %s Input", hostInfo->name);
			defaultDisplayed = 1;
		}
		
		if (i== Pa_GetDefaultOutputDevice()) {
			printf((defaultDisplayed ? "," : "["));
			printf("Default Output");
			defaultDisplayed = 1;
		}
		else if (i== Pa_GetHostApiInfo( deviceInfo->hostApi )->defaultOutputDevice) {
			const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
			printf((defaultDisplayed ? "," : "["));
			printf(" Default %s Output", hostInfo->name);
			defaultDisplayed = 1;
		}
		
		if (defaultDisplayed)
			printf(" ]\n");
		
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
		printf("Default sample rate = %8.2f\n", deviceInfo->defaultSampleRate);
		/*printf("Default low input latency	 = %8.4f\n", deviceInfo->defaultLowInputLatency);
		printf("Default low output latency	 = %8.4f\n", deviceInfo->defaultLowOutputLatency);
		printf("Default high input latency	 = %8.4f\n", deviceInfo->defaultHighInputLatency);
		printf("Default high output latency = %8.4f\n", deviceInfo->defaultHighOutputLatency);*/
		
#ifdef WIN32
#if PA_USE_ASIO
		/* ASIO specific latency information */
		if (Pa_GetHostApiInfo(deviceInfo->hostApi)->type == paASIO) {
			long minLatency, maxLatency, preferredLatency, granularity;
			err = PaAsio_GetAvailableBufferSizes(i,	&minLatency, &maxLatency, &preferredLatency, &granularity);
			printf("ASIO minimum buffer size = %ld\n", minLatency);
			printf("ASIO maximum buffer size = %ld\n", maxLatency);
			printf("ASIO preferred buffer size = %ld\n", preferredLatency);
			if (granularity == -1) printf("ASIO buffer granularity = power of 2\n");
			else printf("ASIO buffer granularity = %ld\n", granularity);
		}
#endif
#endif
		/*
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
			//PrintSupportedStandardSampleRates(&inputParameters, NULL);
		}
		
		if (outputParameters.channelCount > 0) {
			printf("Supported standard sample rates\n for half-duplex 16 bit %d channel output = \n", outputParameters.channelCount);
			//PrintSupportedStandardSampleRates(NULL, &outputParameters);
		}
		
		if (inputParameters.channelCount > 0 && outputParameters.channelCount > 0) {
			printf("Supported standard sample rates\n for full-duplex 16 bit %d channel input, %d channel output = \n", inputParameters.channelCount, outputParameters.channelCount);
			//PrintSupportedStandardSampleRates(&inputParameters, &outputParameters);
		}
		*/

		printf("\n");

	}
	
	printf("----------\n");
	return 0;
}
