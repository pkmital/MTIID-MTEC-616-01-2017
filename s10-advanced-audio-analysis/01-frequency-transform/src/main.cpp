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
        fft = make_shared<pkmFFT>(frame_size);
        magnitudes.resize(1, frame_size);
        phases.resize(1, frame_size);
        
        ofSoundStreamSetup(0, 1, 44100, frame_size, 3);
        
    }
    
    void update() {
        
    }
    
    void draw() {
        float width_step = width / (float)frame_size;
        float height_scale = height / 100.0;
        
        for (int i = 1; i < frame_size; i++) {
            ofDrawLine((i - 1) * width_step, height / 2 - magnitudes[i - 1] * height_scale,
                       i * width_step, height / 2 - magnitudes[i] * height_scale);
        }
    }
    
    void audioIn(float *buf, int size, int ch) {
        fft->forward(0, buf, magnitudes.data, phases.data);
    }
    
private:
    int width, height;
    int frame_size;
    
    shared_ptr<pkmFFT> fft;
    pkmMatrix magnitudes, phases;
};


//========================================================================
int main( ){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());

}
