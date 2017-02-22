#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    frame = 0;
    reader1.open(ofToDataPath("amen.wav"));
    ofSoundStreamSetup(1, 0, 44100, 512, 3);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
}

void ofApp::audioOut(float *buffer, int buffer_size, int n_channels) {
    reader1.read(buffer, frame * buffer_size, buffer_size);
    frame++;
}
