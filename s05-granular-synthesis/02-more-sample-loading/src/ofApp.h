#pragma once

#include "ofMain.h"
#include "pkmEXTAudioFileReader.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

    void audioOut(float *buffer, int buffer_size, int n_channels);
		
    pkmEXTAudioFileReader reader1;
    int frame;
};
