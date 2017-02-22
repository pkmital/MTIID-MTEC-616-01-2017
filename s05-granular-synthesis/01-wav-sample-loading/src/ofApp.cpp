#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    sample1.load(ofToDataPath("amen.wav"));
    
    ofSoundStreamSetup(1, 0, 44100, 512, 3);
//    maxiSettings::setup(44100, 1, 3);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

}

void ofApp::audioOut(float *buffer, int buffer_size, int n_channels) {
    for (int i = 0; i < buffer_size; i++) {
        buffer[i] = sample1.play();
    }
}
