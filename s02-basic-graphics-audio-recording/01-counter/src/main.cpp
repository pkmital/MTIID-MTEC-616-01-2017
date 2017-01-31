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
        
        // create our font
        myFont.loadFont("dekar.ttf",    // name of the font file
                        36);            // size of the font
    }
    
    // i get called in a loop that runs until the program ends
    void update() {
        // update some variables
        counter = counter + 1;
//        counter++;
//        counter += 1;
    }
    
    // i also get called in a loop that runs until the program ends
    void draw() {
        // write the variable counter into a string
        char buf[80];
        sprintf(buf, "%d", counter);
        
        // draw the current counter
        myFont.drawString(buf, 100, 125);
    }

private:
    // declaration of variables
    int counter;
    ofTrueTypeFont myFont;
};


int main(){
    ofSetupOpenGL(1024, 768, OF_WINDOW);
    ofRunApp(new ofApp());
}
