#include "ofMain.h"

class ofApp : public ofBaseApp{
    
public:
    void setup(){
        // set the size of the window
        ofSetWindowShape(320, 240);
        
        // the rate at which the program runs (FPS)
        ofSetFrameRate(60);
        
        // setup the camera
        grabber.setDesiredFrameRate(30);
        grabber.initGrabber(320, 240);
    }
    
    void update(){
        grabber.update();
        if (grabber.isFrameNew()) {
            // do some intensive processing
            
        }
    }
    
    void draw(){

        // draw the camera
        ofSetColor(255);
        
        grabber.draw(0, 0);
    }
    
private:
    ofVideoGrabber grabber;
};


int main( ){
    ofSetupOpenGL(1024, 768, OF_WINDOW);
    ofRunApp(new ofApp());
}
