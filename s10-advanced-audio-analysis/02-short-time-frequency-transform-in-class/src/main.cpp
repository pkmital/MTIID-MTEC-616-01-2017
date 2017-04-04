#include "ofMain.h"
#include "pkmFFT.h"
#include "pkmMatrix.h"

class ofApp : public ofBaseApp {
public:
    void setup() {
        width = 500;
        height = 500;
        ofSetWindowShape(width, height);
        
        frame_size = 512;
        fft_size = 512;
        
        fft = make_shared<pkmFFT>(fft_size);
//        stft = make_shared<pkmSTFT>(fft_size, fft_size);
        
        magnitudes = pkmMatrix(1, fft_size);
        phases = pkmMatrix(1, fft_size);
        
        ofSoundStreamSetup(0, 1, 44100, frame_size, 3);
    }
    
    void update() {
        
    }
    
    void draw() {
            // for every value in our magnitudes matrix
        float width_step = width / ((float)fft_size / 2.0);
        for (int i = 1; i < fft_size / 2; i++) {
            ofDrawLine((i - 1) * width_step, height - magnitudes[i - 1],
                       i * width_step, height - magnitudes[i]);
        }
            // draw a line from halfway down the screen
            // up to height determined by the magnitude
            // move over some number of pixels
    }
    
    void audioIn(float *buf, int size, int ch) {
        fft->forward(0, buf, magnitudes.data, phases.data);
    }
    
private:
    int width, height;
    int frame_size, fft_size;
    
        // create an fft object
    shared_ptr<pkmFFT> fft;
//    shared_ptr<pkmSTFT> stft;
    pkmMatrix magnitudes, phases;
    deque<pkmMatrix> stft;
};


//========================================================================
int main( ){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());

}
