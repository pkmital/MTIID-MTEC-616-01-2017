#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    
    reader1.open(ofToDataPath("amen.wav"));
    int frame_size = 512;
    int total_frames = reader1.mNumSamples / frame_size;
    pkmMatrix corpus;
    for (int frame = 0; frame < total_frames; frame++) {
        pkmMatrix buffer(1, frame_size);
            // read the audio file's current frame into "buffer"
        reader1.read(buffer.data, frame * frame_size, frame_size);
            // push the new frame into "corpus"
        corpus.push_back(buffer);
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
