#include "ofMain.h"
#include "ofAppGLFWWindow.h"

#include "ofxMaxim.h"
#include "maxiGrains.h"

/*
 maxiSample sample;
 maxiTimePitchStretch<hannWinFunctor, maxiSample> *ts;
 sample1.load(ofToDataPath("amen.wav"));
 ts = new maxiTimePitchStretch<hannWinFunctor, maxiSample>(&sample);
 */

#include "pkmFFT.h"
#include "pkmSTFT.h"
#include "pkmCircularRecorder.h"
#include "pkmAudioFeatures.h"
#include "pkmMatrix.h"
#include "pkmEXTAudioFileWriter.h"


/*
 Pseudocode:
 
 While recording, add new "segments" to library
    1.  Detect new segment using RMS threshold
    2.  Save audio from previous onset to current onset as "wave" file
    3.  Store audio features from each frame in matrix
    4.  Store lookup table of audio features to saved audio file
    5.  Create timestretch object representing new audio segment
 While matching, select "grains" to play from library
    1.  Detect audio features of current frame
    2.  Find closest audio features in library
    3.  Select grain for playback and play position of detected audio frame
 
 */

class OnsetDetector {
public:
    OnsetDetector() {
        rms = pkmMatrix(22050, 1);
    }
    
    bool detectOnset(float sample) {
        rms.getMeanAndStdDev(mean_rms, std_rms);
        sample = sample * sample;
        rms.insertRowCircularly(&sample);
        if (rms.isCircularInsertionFull() &&
            (sample > (mean_rms + 3.0 * std_rms) ||
             sample < (mean_rms - 3.0 * std_rms)))
        {
            rms.resetCircularRowCounter();
            return true;
        }
        else {
            return false;
        }
    }

private:
    pkmMatrix rms;
    float mean_rms, std_rms;
};

class AudioSegment {
public:
    void setup(string path) {
        this->path = ofToDataPath(path, true);
        writer.open(this->path);
    }
    
    void insert(float *buf, int size) {
        writer.write(buf, current_sample, size);
        current_sample += size;
    }
    
    void finalize() {
        writer.close();
        sample = new maxiSample();
        sample->load(path);
        ts = make_shared<maxiTimePitchStretch<hannWinFunctor, maxiSample>>(sample);
    }
    
    void setPosition(float position) {
        ts->setPosition(position);
    }
    
    float play() {
        return ts->play(1.0, 1.0, 0.1, 4);
    }
    
private:
    string path;
    maxiSample *sample;
    shared_ptr<maxiTimePitchStretch<hannWinFunctor, maxiSample>> ts;
    pkmEXTAudioFileWriter writer;
    unsigned long current_sample;
};

class Corpus {
public:
    Corpus(int frame_size=512, int fft_size=2048, int sample_rate=44100) {
        this->fft_size = fft_size;
        this->frame_size = frame_size;
        buffer = pkmMatrix(fft_size / frame_size, frame_size, true);
        feature_analyzer.setup(sample_rate, fft_size);
        createNewSegment();
    }
    
    void addFrame(float *buf, int size) {
        segments.back().insert(buf, size);
        
        buffer.insertRowCircularly(buf);
        pkmMatrix aligned = buffer.getCircularAligned();
        pkmMatrix this_features(1, 13);
        feature_analyzer.computeLFCCF(aligned.data, this_features.data, 13);
        features.back().push_back(this_features);
    }
    
    void finalizeSegment() {
        segments.back().finalize();
    }
    
    void createNewSegment() {
        AudioSegment segment;
        segment.setup(ofToString(segments.size()) + ".wav");
        segments.push_back(segment);

        features.push_back(pkmMatrix());
    }
    
    int size() {
        return segments.size();
    }

private:
    int fft_size, frame_size;
    pkmMatrix buffer;
    vector<pkmMatrix> features;
    vector<AudioSegment> segments;
    pkmAudioFeatures feature_analyzer;
};

class ofApp : public ofBaseApp {
public:
    void setup() {
        is_matching = false;
        
        width = 500;
        height = 500;
        ofSetWindowShape(width, height);
        
        maxiSettings::setup(44100, 1, 512);
        ofSoundStreamSetup(1, 1, 44100, 512, 3);
    }
    
    void update() {
        
    }

    void draw() {
        ofDrawBitmapString(ofToString(corpus.size()), 20, 20);
    }
    
    void audioIn(float *buf, int size, int ch) {
        
    }
    
    void audioIn(float *buf, int size, int ch) {
        if(is_matching) {
            bool detected_onset = false;
            for (int i = 0; i < size; i++) {
                if(detector.detectOnset(buf[i])) {
                    detected_onset = true;
                    cout << "onset" << endl;
                }
            }
            corpus.addFrame(buf, size);
            if (detected_onset) {
                corpus.finalizeSegment();
                corpus.createNewSegment();
                corpus.addFrame(buf, size);
            }
        }
        else {
            
        }
    }
    
private:
    int width, height;
    int is_matching;
    OnsetDetector detector;
    Corpus corpus;
};


//========================================================================
int main() {
    ofGLFWWindowSettings settings;
    settings.width = 500;
    settings.height = 500;
    settings.setPosition(ofVec2f(0, 0));
    settings.resizable = true;
    shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);
    
    shared_ptr<ofApp> mainApp(new ofApp);
    
    ofRunApp(mainWindow, mainApp);
    ofRunMainLoop();
}
