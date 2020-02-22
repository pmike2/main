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



#endif

