#include "ofMain.h"
#include "ofxMaxim.h"


class App : public ofBaseApp{
public:
    void setup() {
        width = 640;
        height = 480;
        ofSetWindowShape(width, height);
        
        ofSetFrameRate(60);
        ofSetBackgroundAuto(false);
        
        sample_rate = 44100;
        buffer_size = 256;
        ofSoundStreamSetup(2, 0, sample_rate, buffer_size, 3);
    }
    
    // i get called in a loop that runs until the program ends
    void update() {
        
    }
    
    // i also get called in a loop that runs until the program ends
    void draw() {
        ofBackground(0);
        
    }
    
    void keyPressed(int k) {

    }
    
    void audioOut(float * output, int buffer_size, int n_channels) {
        for(int i = 0; i < buffer_size; i++)
        {
            output[i] = delay1.dl(osc1.sinewave(osc3.phasor(0.5, 0, 440)) * osc2.sinewave(440),
                                  22050, 0.5);
        }
    }

private:
    int                     width,
                            height;
    
    int                     sample_rate,
                            buffer_size;
    
    ofxMaxiOsc              osc1, osc2, osc3;
    ofxMaxiDelayline        delay1;
};


int main() {
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new App());
}
