#include "ofMain.h"

class ofApp : public ofBaseApp{
public:
    void setup() {
        // do some initialization
        
        // set the size of the window
        ofSetWindowShape(width, height);
        
        // the rate at which the program runs (FPS)
        ofSetFrameRate(30);
        
        // don't clear the background on each update/draw loop
//         ofSetBackgroundAuto(false);
    }
    
    // i get called in a loop that runs until the program ends
    void update() {

    }
    
    // i also get called in a loop that runs until the program ends
    void draw() {
        // clear the background to black
        ofBackground(0);
        
        float frequency = 2.0;
        float amplitude = height / 2.0;
        float phase = ofGetElapsedTimef();
        
        ofSetColor(100, 100, 100);
        ofDrawLine(0, height / 2, width, height / 2);
        
        ofSetColor(200, 200, 200);
        for (int i = 1; i < width; i++) {
            
            float x1 = (float)i / width;
            float y1 = amplitude * sin( phase + frequency * x1 * 2.0*PI );
            float x2 = (float)(i - 1) / width;
            float y2 = amplitude * sin( phase + frequency * x2 * 2.0*PI );
            
            ofDrawLine(x1 * width, -y1 + height / 2.0,
                       x2 * width, -y2 + height / 2.0);
        }
    }
    
    void keyPressed(int key) {
        if (key == 'f') {
            ofToggleFullscreen();
        }
    }
    
    void mouseMoved( int x, int y ){

    }

private:
    int width = 640;
    int height = 480;
};


int main(){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());
}
