#include "ofMain.h"
#include "pkmFFT.h"
#include "pkmSTFT.h"
#include "pkmCircularRecorder.h"
#include "pkmMatrix.h"

class Recording {
public:
    Recording() {
        current_frame = 0;
        total_frames = 0;
    }

    Recording(pkmMatrix &mags, pkmMatrix &buf) {
        buffer = buf;
        features = mags.mean();
        current_frame = 0;
        total_frames = buffer.rows;
    }

    pkmMatrix& getFeatures() {
        return features;
    }

    const float* getCurrentFrame() {
        if(current_frame < total_frames) {
            const float *buf = buffer.row(current_frame);
            current_frame += 1;
            return buf;
        }
        else {
            current_frame = 0;
            return NULL;
        }
    }

private:
    int current_frame, total_frames;
    pkmMatrix buffer;
    pkmMatrix features;
};

class Corpus {
public:
    Corpus() {
        
    }
    
    ~Corpus() {
        
    }
    
    void addRecording(pkmMatrix &magnitudes, pkmMatrix &buffer) {
        recordings.push_back(make_shared<Recording>(magnitudes, buffer));
    }
    
    shared_ptr<Recording> getMostSimilarRecording(pkmMatrix &magnitudes) {
        pkmMatrix mean_magnitudes = magnitudes.mean();
        float best_dist = HUGE_VALF;
        int best_idx = 0;
        for (int i = 0; i < recordings.size(); i++) {
            pkmMatrix this_mean_magnitudes = recordings[i]->getFeatures();
            float dist = pkm::Mat::l1norm(mean_magnitudes.data,
                                          this_mean_magnitudes.data,
                                          mean_magnitudes.size());
            if(dist < best_dist){
                best_dist = dist;
                best_idx = i;
            }
        }
        return recordings[best_idx];
    }
    
    int size() {
        return recordings.size();
    }
    
private:
    vector<shared_ptr<Recording>> recordings;
};


class ofApp : public ofBaseApp {
public:
    void setup() {
        is_recording = false;
        is_playing = false;
        
        width = 500;
        height = 500;
        ofSetWindowShape(width, height);
        
        n_frames = 100;
        frame_size = 512;
        fft_size = 4096;
        buffer_size = frame_size * n_frames;
        
        stft = make_shared<pkmSTFT>(fft_size);
        
        ofSoundStreamSetup(1, 1, 44100, frame_size, 3);

    }
    
    void update() {

    }
    
    void draw() {
        
        float width_step = width / (float)magnitudes.cols;
        float height_scale = height / 100.0;
        
        for (int window_i = 0; window_i < magnitudes.rows; window_i++)
        {
            float height_offset = height * (window_i / (float)magnitudes.rows);
            ofSetColor(200 * (window_i / (float)magnitudes.rows), 100, 100);
            for (int i = 1; i < magnitudes.cols; i++)
            {
                ofDrawLine((i - 1) * width_step,
                           height_offset - magnitudes.row(window_i)[i - 1] * height_scale,
                           i * width_step,
                           height_offset - magnitudes.row(window_i)[i] * height_scale);
            }
        }
        
        if (is_recording) {
            ofDrawBitmapString("Recording: True", 20, 20);
        }
        else {
            ofDrawBitmapString("Recording: False", 20, 20);
        }
        
        if (is_playing) {
            ofDrawBitmapString("Playing: True", 20, 40);
        }
        else {
            ofDrawBitmapString("Playing: False", 20, 40);
        }
        
        ofDrawBitmapString("Corpus: " + ofToString(corpus.size()), 20, 60);
    }
    
    void audioIn(float *buf, int size, int ch) {
        if(is_recording)
            recording.push_back(buf, size);
        else if(is_playing)
            target.push_back(buf, size);
    }
    
    void audioOut(float *buf, int size, int ch) {
        memset(buf, 0, sizeof(float) * size);
        if (!is_recording) {
            if (!is_playing && match != nullptr) {
                const float *frame = match->getCurrentFrame();
                if(frame != NULL)
                    memcpy(buf, frame, sizeof(float) * size);
            }
        }
    }
    
    void keyPressed(int k) {
        if (k == 'r') {
            is_recording = !is_recording;
            if(!is_recording) {
                magnitudes.resize(stft->getNumWindows(recording.size()), fft_size / 2);
                phases.resize(stft->getNumWindows(recording.size()), fft_size / 2);
                stft->STFT(recording.data, recording.size(), magnitudes, phases);
                corpus.addRecording(magnitudes, recording);
            }
            recording = pkmMatrix();
        }
        else if (k == 'p') {
            is_playing = !is_playing;
            if(!is_playing && target.size()) {
                magnitudes.resize(stft->getNumWindows(target.size()), fft_size / 2);
                phases.resize(stft->getNumWindows(target.size()), fft_size / 2);
                stft->STFT(target.data, target.size(), magnitudes, phases);
                match = corpus.getMostSimilarRecording(magnitudes);
            }
            target = pkmMatrix();
        }
    }
    
private:
    int width, height;
    
    int buffer_size, fft_size, frame_size, n_frames;
    
    shared_ptr<pkmSTFT> stft;
    pkmMatrix magnitudes, phases;
    pkmMatrix recording, target;
    
    Corpus corpus;
    shared_ptr<Recording> match;
    
    bool is_recording, is_playing;
};


//========================================================================
int main( ){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());

}
