#include "engine.h"
#include "game\includes.h"

Engine::App* app = NULL;

#ifndef SAMPLE_RATE
	#define SAMPLE_RATE 48000
#endif

int main(int argc,char** argv) {
    #ifndef APP_CLASS

        #error APP_CLASS must be defined

    #else
    	#ifdef USE_AUDIO
		Engine::AudioContext audioContext(SAMPLE_RATE);
		#endif

        Engine::App::setAppParams(argc,argv);

        APP_CLASS application;
        ((Engine::App*)&application)->run();

    #endif // APP_CLASS

    return 0;
}
