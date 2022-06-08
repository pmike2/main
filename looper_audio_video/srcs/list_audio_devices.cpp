#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "portaudio.h"
#ifdef WIN32
#include <windows.h>
#if PA_USE_ASIO
#include "pa_asio.h"
#endif
#endif

#include "pa_utils.h"


using namespace std;



int main(int argc, char **argv) {
	list_devices();

	return 0;
}
