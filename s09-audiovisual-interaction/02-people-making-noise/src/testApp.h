#pragma once

#include "ofMain.h"

const int W = 1024;
const int H = 768;

const int WINDOW_WIDTH = W*3 + 40*2;
const int WINDOW_HEIGHT = H*1.5;

#include "ofVideoGrabber.h"
#include "pkmBlobTracker.h"

class testApp : public ofBaseApp, public ofCvBlobListener {
	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);


	void blobOn( int x, int y, int id, int order );
	void blobMoved( int x, int y, int id, int order );
	void blobOff( int x, int y, int id, int order );
	
	ofVideoGrabber			vidGrabber;
	pkmBlobTracker			orientationTracker;
	
	vector<float>			velocities;
	vector<int>				px;
	vector<int>				py;
	ofSoundPlayer			sound[10];
	int						numSounds;
	int						currentSound;
	
	map<int, int>			soundMapping;
	map<int, int>			velocityMapping;
	
};
