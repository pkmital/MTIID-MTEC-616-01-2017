/*
 Corpus Based Concatenative Synthesis w/ TSNE Browser
 
 1. analysis:
    - load a corpus of sound
    - store them using frames (e.g. 2048 samples of audio)
    - associate each frame with some audio features
 2. matching:
    - look at each frame of audio (2048 samples)
    - calculate the audio features
    - find the k-NN, NN - find the nearest audio segment based on
      the smallest distance to every possible audio feature.
    - playback the nearest segment(s).
 
 extend to do, e.g.:
    mfcc, delta mfcc, delta delta mfcc features
    granular synthesis
    circular buffers w/ smaller frame sizes
    onset detection to determine recordings rather than using every frame
    for variable length recordings, you could e.g. use the mean MFCC as well as the first frame's MFCC to have 26 instead of 13 features
    you could add chromagrams
    andrew's idea of detecting pitched content and using or not using chromas/mfccs.
 
 for TSNE:
    use 3-dimensions
 
 */

#include "ofMain.h"
#include "pkmFFT.h"
#include "pkmSTFT.h"
#include "pkmCircularRecorder.h"
#include "pkmAudioFeatures.h"
#include "pkmMatrix.h"
#include "ofxTSNE.h"

class Recording {
public:
    Recording(pkmMatrix buf, pkmMatrix feats) {
        buffer = buf;
        features = feats;
    }
    pkmMatrix buffer, features;
};

class Corpus {
public:
    void setup(int segment_size = 2048){
        analyzer.setup(44100, segment_size);
    }
    
    float * getNearestRecording(float *buf, int size) {
        pkmMatrix buffer(1, size, buf);
        pkmMatrix features(1, 13);
        analyzer.computeLFCCF(buffer.data, features.data, 13);
        
            // look at every single recording's features
            // calculate the distance to it
        
        float best_distance = HUGE_VALF;
        int best_idx = 0;

        for(int recording_i = 0; recording_i < corpora.size(); recording_i++) {
            float this_distance = 0;
            for(int feature_i = 0; feature_i < 13; feature_i++){
                this_distance += abs(features[feature_i] - corpora[recording_i].features[feature_i]);
            }
            if (this_distance < best_distance) {
                best_distance = this_distance;
                best_idx = recording_i;
            }
        }
        
        if (corpora.size()) {
            return corpora[best_idx].buffer.data;
        }
        else {
            return NULL;
        }
    }
    
    void addRecording(float *buf, int size){
        pkmMatrix buffer(1, size, buf);
        pkmMatrix features(1, 13);
        analyzer.computeLFCCF(buffer.data, features.data, 13);
        Recording r(buffer, features);
        corpora.push_back(r);
    }
    
    int size() {
        return corpora.size();
    }
    
    float * getFeatures(int i){
        return corpora[i].features.data;
    }
    
    float * getAudio(int i){
        return corpora[i].buffer.data;
    }
    
private:
    pkmAudioFeatures analyzer;
    vector<Recording> corpora;
};

class ofApp : public ofBaseApp {
public:
    void setup() {
        is_matching = false;
        is_recording = false;
        
        best_idx = -1;
        
        width = 500;
        height = 500;
        
        ofSetWindowShape(width, height);
        
        buffer = pkmMatrix(1, 2048);
        
        corpus.setup(2048);
        
        ofSoundStreamSetup(1, 2, 44100, 2048, 3);
    }
    
    void update() {
    }
    
    void draw() {
        
        for (int rec_i = 0; rec_i < pts.size(); rec_i++) {
            double x = pts[rec_i][0] * width;
            double y = pts[rec_i][1] * height;
            if(rec_i == best_idx)
                ofSetColor(255);
            else
                ofSetColor(pts[rec_i][0] * 200, pts[rec_i][1] * 200, pts[rec_i][2] * 200);
            ofDrawCircle(x, y, 4);
        }
        
        ofDrawBitmapString(ofToString(corpus.size()), 20, 20);
    }
    
    void audioIn(float *buf, int size, int ch) {
        if (is_recording) {
                // get 2048 samples of audio and
                // store in corpus.
            corpus.addRecording(buf, size);
        }
        
        for (int i = 0; i < size; i++) {
            buffer[i] = buf[i];
        }
    }
    
    void audioOut(float *buf, int size, int ch) {
        if (is_matching) {
            
            float best_distance = HUGE_VALF;
            
            for(int rec_i = 0; rec_i < corpus.size(); rec_i++) {
                float this_distance = abs(pts[rec_i][0] - mouseX / (float)width) + abs(pts[rec_i][1] - mouseY / (float)height);
                if (this_distance < best_distance) {
                    best_distance = this_distance;
                    best_idx = rec_i;
                }
            }
            
            float *audio = corpus.getAudio(best_idx);
            for (int i = 0; i < size; i++) {
                buf[i] = audio[i];
            }
        }
    }
    
    void keyPressed(int k) {
        if(k == 'r') {
            is_recording = !is_recording;
            if(is_recording)
                is_matching = false;
        }
        else if(k == 'm') {
            is_matching = !is_matching;
            if(is_recording)
                is_recording = false;
        }
        else if(k == ' ') {
            vector<vector<float>> features;
            
            for (int rec_i = 0; rec_i < corpus.size(); rec_i++) {
                vector<float> this_feature;
                float *f = corpus.getFeatures(rec_i);
                for (int feat_i = 0; feat_i < 13; feat_i++) {
                    this_feature.push_back(f[feat_i]);
                }
                features.push_back(this_feature);
            }
            
            pts = tsne.run(features, 3);
        }
    }
    
    
private:
    
    ofxTSNE tsne;
    vector<vector<double>> pts;
    int best_idx;
    
    pkmMatrix buffer;
    
    Corpus corpus;
    
    int width, height;
    
    bool is_matching;
    bool is_recording;
};


//========================================================================
int main( ){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());

}
