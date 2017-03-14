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
        
        fbo.allocate(width, height);
        grabber.setup(width, height);

        shader.load("hello.vert", "hello.frag");
    }

    void update() {
        grabber.update();
    }

    void draw() {
        fbo.begin();
        shader.begin();
        shader.setUniformTexture("tex", grabber.getTexture(), 0);
        ofDrawRectangle(0, 0, width, height);
        shader.end();
        fbo.end();
        
        ofPushMatrix();
        fbo.draw(0, 0);
        ofPopMatrix();
    }
    
private:
    
    int width, height;
    ofFbo fbo;
    ofShader shader;
    ofVideoGrabber grabber;
};

//========================================================================
int main( ){
    ofGLFWWindowSettings settings;
    settings.width = 640;
    settings.height = 480;
    settings.setPosition(ofVec2f(20, 20));
    settings.resizable = true;
    settings.setGLVersion(2, 1);
    shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);
    
    shared_ptr<ofApp> mainApp(new ofApp);
    ofRunApp(mainWindow, mainApp);
    ofRunMainLoop();
}
