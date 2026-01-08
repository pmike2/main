#ifndef PA_UTILS_H
#define PA_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "portaudio.h"

#ifdef WIN32
#include <windows.h>
#if PA_USE_ASIO
#include "pa_asio.h"
#endif
#endif


void print_supported_standard_sample_rates(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters);

int list_devices();

uint get_n_input_channels(int idx_device);
uint get_n_output_channels(int idx_device);

PaStream * pa_init(int idx_device_input, int idx_device_output, const uint sample_rate, const uint frames_per_buffer,
	int (* callback)(const void * input, void * output, unsigned long sample_count, const PaStreamCallbackTimeInfo * time_info, PaStreamCallbackFlags status_flags, void * user_data),
	void * user_data);

void pa_close(PaStream * stream);

#endif

