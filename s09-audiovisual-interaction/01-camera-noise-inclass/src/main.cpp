#include "ofMain.h"
#include "maximilian.h"

class ofApp : public ofBaseApp{
public:
    void setup() {
        cam.setup(width, height);
        ofSetWindowShape(width, height);
        ofSetFrameRate(60);
        
        ofSoundStreamSetup(1, 0, 44100, 512, 3);
    }
    
    void update() {
        cam.update();
    }
    
    void draw() {
        cam.draw(0, 0, ofGetWidth(), ofGetHeight());
    }
    
    void keyPressed(int key) {
        if (key == 'f') {
            ofToggleFullscreen();
        }
    }
    
    void mouseMoved( int x, int y ){
        
    }
    
    void audioOut(float *buf, int size, int ch) {
        ofPixels pix = cam.getPixels();
        float ratio = width / (float)size;
        for (int i = 0; i < size; i++) {
            // 0 - 255
            ofColor color = pix.getColor(i * ratio, height / 2);
            buf[i] = oscs[0].sinewave(color.r + 255);
            buf[i] += oscs[1].sinewave((color.g + 255) * 2);
            buf[i] += oscs[2].sinewave((color.b + 255) * 3);
            maxiSample
                // play
                // rate
//            maxiDelayline delay
        }
    }

private:
    maxiOsc oscs[3];
    
    ofVideoGrabber cam;
    int width = 640;
    int height = 480;
};


int main(){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());
}
