#include "ofMain.h"

class ofApp : public ofBaseApp{
    
public:
    void setup(){
        // set the size of the window
        ofSetWindowShape(320, 240);
        
        // the rate at which the program runs (FPS)
        ofSetFrameRate(60);
        
        font.load("Dekar.ttf", 12);
        font_large.load("Dekar.ttf", 24);
        
            // setup the camera
        grabber.setDesiredFrameRate(10);
        grabber.initGrabber(320, 240);
        
        fbo.allocate(320, 240);
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
        
        fbo.begin();
            ofPushStyle();
                ofSetColor(0, 10);
                ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
            ofPopStyle();
            cam.begin();
                grabber.draw(0, 0);
            cam.end();
        fbo.end();
        
        fbo.draw(0, 0, ofGetWidth(), ofGetHeight());
        
        fbo.draw(0, 0, 50, 50);
        
        ofSetWindowTitle(ofToString(ofGetFrameRate()));
        ofDrawBitmapString(ofToString(ofGetFrameRate()), 20, 20);
        font.drawString(ofToString(ofGetFrameRate()), 20, 40);
        font_large.drawString(ofToString(ofGetFrameRate()), 20, 60);
    }
    
private:
    ofVideoGrabber grabber;
    ofEasyCam cam;
    ofFbo fbo;
    ofTrueTypeFont font, font_large;
};


int main( ){
    ofSetupOpenGL(1024, 768, OF_WINDOW);
    ofRunApp(new ofApp());
}
