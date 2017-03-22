#include "ofMain.h"
#include "ofAppGlutWindow.h"
#include "maximilian.h"

class ofApp : public ofBaseApp{
    
public:
    
    void setup() {
            // do some initialization
            // set the size of the window
        width = 320;
        height = 240;
        ofSetWindowShape(width, height);
        
            // the rate at which the program runs (FPS)
        ofSetFrameRate(30);
        
            // setup the camera
        camera.initGrabber(width, height);
        
        prev_audio = 0;
        
        line.setAttack(200);
        line.setRelease(200);
        
        sample.load(ofToDataPath("amen.wav"));

            // setup the sound
        int sampleRate = 44100;
        int bufferSize = 256;
        ofSoundStreamSetup(1,				// output channels
                           0,				// input channels
                           sampleRate,		// how many samples (readings) per second
                           bufferSize,		// size of each copy of audio
                           4);				// latency of audio
    }
    
    void update(){
        camera.update();
    }
    
    void draw(){
        ofBackground(0);
        
        // draw the camera
        camera.draw(0, 0);
    }
    
    void audioOut(float *buf, int size, int ch) {
        auto pixels = camera.getPixels().getLine(height / 2).asPixels();
        float ratio = (width - 1) / (float)(size - 1);
        for (int i = 0; i < size; i++)
        {
            float brightness = pixels.getColor(i).getBrightness();
            float speed = ofMap(brightness,
                                0, 255,
                                0.0, 2.0);
            speed = line.play(speed);
            buf[i] = sample.play(prev_audio * 0.99 + speed * 0.01);
            prev_audio = speed;
        }
    }
    
private:
    
    int width, height;
    ofVideoGrabber camera;
    maxiOsc osc;
    maxiSample sample;
    maxiEnvelopeFollower line;
    float prev_audio;
};


//========================================================================
int main(){
    ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());
}
