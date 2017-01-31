#include "ofMain.h"

class ofApp : public ofBaseApp{
public:
    void setup() {
        // do some initialization
        
        // set the size of the window
        ofSetWindowShape(250, 250);
        
        // the rate at which the program runs (FPS)
        ofSetFrameRate(30);
        
        myFont.loadFont("dekar.ttf", 36);
        
        // don't clear the background on each update/draw loop
         ofSetBackgroundAuto(false);
    }
    
    // i get called in a loop that runs until the program ends
    void update() {

    }
    
    // i also get called in a loop that runs until the program ends
    void draw() {
        // set the size of the circle
        float radius = 3.0f;
        
        // draw a circle at the mouse position
        ofCircle(mouseX, mouseY, radius);
        
        myFont.drawString("hello!", 50, 50);
        
        char buf[256];
        sprintf(buf, "%d %d", mouseX, mouseY);
        myFont.drawString(buf, 50, 125);
        ofDisableAlphaBlending();
    }

private:
    ofTrueTypeFont myFont;
};


int main(){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());
}
