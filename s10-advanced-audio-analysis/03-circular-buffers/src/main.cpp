#include "ofMain.h"
#include "pkmFFT.h"
#include "pkmMatrix.h"
#include "pkmCircularRecorder.h"
#include <deque>

class ofApp : public ofBaseApp {
public:
    void setup() {
        width = 500;
        height = 500;
        ofSetWindowShape(width, height);
        
        frame_size = 128;
        
        ofSoundStreamSetup(0, 1, 44100, frame_size, 3);
        
    }
    
    void update() {

    }
    
    void draw() {
        
    }
    
    void audioIn(float *buf, int size, int ch) {

    }
    
private:
    int width, height;
    int frame_size, buffer_size;
    pkmCircularRecorder recorder;
    pkmMatrix buffer;
};


    //========================================================================
int main( ){
    ofSetupOpenGL(1024, 768, OF_WINDOW);
    ofRunApp(new ofApp());
    
}
