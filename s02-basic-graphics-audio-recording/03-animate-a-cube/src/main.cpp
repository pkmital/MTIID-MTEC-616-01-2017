#include "ofMain.h"

class ofApp : public ofBaseApp{
public:
    void setup() {
        // do some initialization
        
        // set the size of the window
        ofSetWindowShape(250, 250);
        
        // the rate at which the program runs (FPS)
        ofSetFrameRate(30);
        
        // we are going to increment this variable each "frame"
        counter = 0;
        
        ofResetElapsedTimeCounter();
    }
    
    // i get called in a loop that runs until the program ends
    void update() {
        // update some variables
        counter = counter + 1;
    }
    
    // i also get called in a loop that runs until the program ends
    void draw() {

        ofBackground(0);
        
        ofPushStyle();
        {
            ofPushStyle();
            {
                unsigned char brightness = sin(ofGetElapsedTimef() * 5.0) * 128.0 + 128.0;
                printf("%f\n", brightness);
                ofSetColor(brightness, 0, 0);
                
                ofDrawRectangle(20, 20, 100, 100);
                ofNoFill();
            }
            ofPopStyle();
            
        }
        ofPopStyle();
    }


private:
    int counter;
};


int main(){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());
}
