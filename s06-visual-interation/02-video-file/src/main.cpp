#include "ofMain.h"

class ofApp : public ofBaseApp{
    
public:
    
    // redeclaration of functions (declared in base class)
    void setup(){
        
            // set the size of the window
        ofSetWindowShape(320, 240);
        
            // the rate at which the program runs (FPS)
        ofSetFrameRate(30);
        
            // load a movie located in the project's "bin/data" directory
        videoPlayer.load("dumbo.mp4");
        videoPlayer.play();
//        videoPlayer.setVolume(0);
//        videoPlayer.setSpeed(2.0);
    }
    
    void update(){
        
            // update the current frame
        videoPlayer.update();
    }
    
    void draw(){
        ofBackground(0);

        
            // draw the movie a position 0,0
        videoPlayer.draw(0,0);
    }

    
    ofVideoPlayer videoPlayer;
};


int main( ){
    ofSetupOpenGL(1024, 768, OF_WINDOW);
    ofRunApp(new ofApp());
}
