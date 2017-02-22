#pragma once

#include "ofMain.h"
#include "ofxMaxim.h"
#include "maxiGrains.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

    void audioOut(float *buffer, int buffer_size, int n_channels);
		
    maxiSample sample1;
    maxiTimePitchStretch<hannWinFunctor, maxiSample> *ts;
    float rate, speed;
};
