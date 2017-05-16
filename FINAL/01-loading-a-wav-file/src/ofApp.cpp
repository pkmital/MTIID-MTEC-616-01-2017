#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    
    reader1.open(ofToDataPath("amen.wav"));
    int frame_size = 512;
    int total_frames = reader1.mNumSamples / frame_size;
    pkmMatrix corpus(total_frames, frame_size);
    for (int frame = 0; frame < total_frames; frame++) {
        reader1.read(corpus.row(frame), frame * frame_size, frame_size);
    }
    
    ofSoundStreamSetup(1, 0, 44100, 512, 3);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
}

void ofApp::audioOut(float *buffer, int buffer_size, int n_channels) {

}
