#include "ofMain.h"
#include "ofxOpenCv.h"
#include "pkmBlobTracker.h"

class ofApp : public ofBaseApp, public ofxCvBlobListener {
    
public:
    
    void blobOn( int x, int y, int id, int order ) {
        
    }

    void blobMoved( int x, int y, int id, int order ) {
        
    }

    void blobOff( int x, int y, int id, int order ) {
        
    }
    
        // redeclaration of functions (declared in base class)
    void setup(){
        
            // do some initialization
        img_width = 320;
        img_height = 240;
        
            // set the size of the window
        ofSetWindowShape(img_width * 3.5, img_height * 1.5);
        
            // the rate at which the program runs (FPS)
        ofSetFrameRate(30);
        
            // setup the camera
        camera.setup(img_width, img_height);
        
            // setup the tracker
        tracker.setup(img_width, img_height);
        tracker.setListener(this);
    }
    
    void update(){
        camera.update();
        
        tracker.update(camera.getPixels());
    }
    
    void draw(){
            // background values go to 0
        ofBackground(0);
        
        ofSetColor(255);
        camera.draw(0, 0);
        
            // draw the camera
        tracker.draw(0, 0);
    }
    
    void keyPressed(int key) {
        tracker.keyPressed(key);
    }
    
private:
    
    ofVideoGrabber camera;
    pkmBlobTracker tracker;
    
    int img_width, img_height;
    
    bool b_capture;
};

int main(){
    ofSetupOpenGL(1024, 768, OF_WINDOW);
    ofRunApp(new ofApp());
}
