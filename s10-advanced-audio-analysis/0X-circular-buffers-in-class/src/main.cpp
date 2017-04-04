#include "ofMain.h"
#include "pkmFFT.h"
#include "pkmMatrix.h"
#include "pkmCircularRecorder.h"
#include <deque>

class ofApp : public ofBaseApp {
public:
    void setup() {
        width = 500;
        height = 500;
        ofSetWindowShape(width, height);
        
        frame_size = 128;
        fft_size = 16384;
        recorder.setup(fft_size, frame_size);
        
        n_windows = 100;
        fft = make_shared<pkmFFT>(fft_size);
        magnitudes.resize(1, fft_size / 2);
        phases.resize(1, fft_size / 2);
        buffer.resize(1, fft_size);
        
        ofSoundStreamSetup(0, 1, 44100, frame_size, 3);
        
    }
    
    void update() {
        if (recorder.isRecorded()) {
            recorder.copyAlignedData(buffer.data);
        }
        fft->forward(0, buffer.data, magnitudes.data, phases.data);
        
        stft.push_back(magnitudes);
        if (stft.size() > n_windows) {
            stft.pop_front();
        }
    }
    
    void draw() {
        float width_step = width / (float)(fft_size / 2);
        float height_scale = height / 100.0;
        
        for (int window_i = 0; window_i < stft.size(); window_i++)
        {
            float height_offset = height * (window_i / (float)n_windows);
            ofSetColor(200 * (window_i / (float)n_windows), 100, 100);
            for (int i = 1; i < fft_size / 2; i++) {
                ofDrawLine((i - 1) * width_step, height_offset - stft[window_i][i - 1] * height_scale,
                           i * width_step, height_offset - stft[window_i][i] * height_scale);
            }
        }
        
    }
    
    void audioIn(float *buf, int size, int ch) {
        recorder.insertFrame(buf);
    }
    
private:
    int width, height;
    int frame_size, fft_size, n_windows;
    
    shared_ptr<pkmFFT> fft;
    pkmMatrix magnitudes, phases, buffer;
    pkmCircularRecorder recorder;
    std::deque<pkmMatrix> stft;
};


    //========================================================================
int main( ){
    ofSetupOpenGL(1024, 768, OF_WINDOW);
    ofRunApp(new ofApp());
    
}
