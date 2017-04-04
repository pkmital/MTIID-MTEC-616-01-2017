#include "ofMain.h"
#include "pkmFFT.h"
#include "pkmSTFT.h"
#include "pkmCircularRecorder.h"
#include "pkmMatrix.h"

class ofApp : public ofBaseApp {
public:
    void setup() {
        width = 500;
        height = 500;
        ofSetWindowShape(width, height);
        
        n_frames = 100;
        frame_size = 512;
        fft_size = 4096;
        buffer_size = frame_size * n_frames;
        
        stft = make_shared<pkmSTFT>(fft_size);
        magnitudes.resize(stft->getNumWindows(buffer_size), fft_size / 2);
        phases.resize(stft->getNumWindows(buffer_size), fft_size / 2);
        
        recorder.setup(buffer_size, frame_size);
        buffer.resize(1, buffer_size);
        
        ofSoundStreamSetup(0, 1, 44100, frame_size, 3);

    }
    
    void update() {
        
    }
    
    void draw() {
        float width_step = width / (float)frame_size;
        float height_scale = height / 100.0;
        
        for (int window_i = 0; window_i < magnitudes.rows; window_i++)
        {
            float height_offset = height * (window_i / (float)magnitudes.rows);
            ofSetColor(200 * (window_i / (float)magnitudes.rows), 100, 100);
            for (int i = 1; i < magnitudes.cols; i++)
            {
                ofDrawLine((i - 1) * width_step, height_offset - magnitudes.row(window_i)[i - 1] * height_scale,
                           i * width_step, height_offset - magnitudes.row(window_i)[i] * height_scale);
            }
        }
    }
    
    void audioIn(float *buf, int size, int ch) {
        recorder.insertFrame(buf);
        if (recorder.isRecorded()) {
            recorder.copyAlignedData(buffer.data);
            stft->STFT(buffer.data, n_frames * frame_size, magnitudes, phases);
        }
    }
    
private:
    int width, height;
    
    int buffer_size, fft_size, frame_size, n_frames;
    
    shared_ptr<pkmSTFT> stft;
    pkmMatrix magnitudes, phases, buffer;
    pkmCircularRecorder recorder;
};


//========================================================================
int main( ){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());

}
