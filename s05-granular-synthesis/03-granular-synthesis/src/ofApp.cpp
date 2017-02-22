#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    speed = rate = 1.0;
    sample1.load(ofToDataPath("amen.wav"));
    
    ts = new maxiTimePitchStretch<hannWinFunctor, maxiSample>(&sample1);

    ofSoundStreamSetup(1, 0, 44100, 512, 3);
//    maxiSettings::setup(44100, 1, 3);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    speed = mouseX / (float)ofGetWidth();
    rate = mouseY / (float)ofGetHeight();
    ofBackground(0);
    ofDrawBitmapString("Speed: " + ofToString(speed), 20, 20);
    ofDrawBitmapString("Rate: " + ofToString(rate), 20, 40);
}

void ofApp::audioOut(float *buffer, int buffer_size, int n_channels) {
    for (int i = 0; i < buffer_size; i++) {
        buffer[i] = (*ts).play(speed, rate, 0.1, 4);
    }
}











