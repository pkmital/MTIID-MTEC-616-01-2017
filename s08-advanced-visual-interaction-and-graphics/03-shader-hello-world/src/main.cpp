/*
  Versions
 OpenGL GLSL
 2.0	110
 2.1	120
 3.0	130
 3.1	140
 3.2	150
 3.3	330
 4.0	400
 4.1	410
 4.2	420
 4.3	430
 */

#include "ofMain.h"
#include "ofAppGLFWWindow.h"

class ofApp : public ofBaseApp{
    
public:
    void setup() {
        width = ofGetWidth();
        height = ofGetHeight();

        shader.load("hello.vert", "hello.frag");
    }

    void update() {
        
    }

    void draw() {
        shader.begin();
        
        ofDrawRectangle(0, 0, width, height);
        
        shader.end();
    }
    
private:
    
    int width, height;
    ofShader shader;
    
};

//========================================================================
int main( ){
    ofGLFWWindowSettings settings;
    settings.width = 320;
    settings.height = 240;
    settings.setPosition(ofVec2f(20, 20));
    settings.resizable = true;
    settings.setGLVersion(2, 1);
    shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);
    
    shared_ptr<ofApp> mainApp(new ofApp);
    ofRunApp(mainWindow, mainApp);
    ofRunMainLoop();
}
