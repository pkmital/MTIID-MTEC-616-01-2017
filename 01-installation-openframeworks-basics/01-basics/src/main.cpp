#include "ofMain.h"

class ofApp : public ofBaseApp{
public:
    void setup() {
        cam.setup(width, height);
        ofSetWindowShape(width, height);
        ofSetFrameRate(60);
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

private:
    ofVideoGrabber cam;
    int width = 640;
    int height = 480;
};


int main(){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());
}
