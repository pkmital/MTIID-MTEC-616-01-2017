#include "ofMain.h"
#include "pkmProjectionMapper.h"


class ofApp : public ofBaseApp{
public:
    void setup() {
        // start off as debug
        b_debug = true;
        
        screen_width = ofGetScreenWidth();
        screen_height = ofGetScreenHeight();
        ofSetWindowShape(screen_width, screen_height);
        ofSetFrameRate(60);
        
        n_mappers = 6;
        obj_width = 640;
        obj_height = 480;
        
        cam.setup(obj_width, obj_height);
        
        // this is how we setup our projection mapper
        // we give it our windows dimensions, and a starting x, y (0,0)
        for (int i = 0; i < n_mappers; i++) {
            mappers[i].initialize(obj_width, obj_height,  i*40, i*40);
            char buf[256];
            sprintf(buf, "mapper%d.txt", i);
            mappers[i].load(ofToDataPath(buf));
        }
        
        ofSetBackgroundAuto(false);
        
        // fullscreen to start with
        ofSetFullscreen(true);
        
        // this changes our drawing rate to match our screen's refresh rate
        ofSetVerticalSync(true);
    }
    
    void update() {
        for (int i = 0; i < n_mappers; i++) {
            mappers[i].update();
        }
        
        cam.update();
    }
    
    void drawBox() {
        ofPushStyle();
        ofPushMatrix();
        ofSetRectMode(OF_RECTMODE_CENTER);
        ofTranslate(obj_width / 2.0, obj_height / 2.0, 0);
        ofNoFill();
        float s = fmod(ofGetElapsedTimef(), 1.0);
        ofScale(s, s);
        ofDrawRectangle(0, 0, obj_width, obj_height);
        ofSetRectMode(OF_RECTMODE_CORNER);
        ofPopMatrix();
        ofPopStyle();
    }
    
    void drawCamera(){
        ofPushStyle();
        ofPushMatrix();
        ofSetRectMode(OF_RECTMODE_CENTER);
        ofTranslate(obj_width / 2.0, obj_height / 2.0, 0);
        ofFill();
        ofSetColor(255, 200);
        cam.draw(0, 0, obj_width, obj_height);
        ofSetRectMode(OF_RECTMODE_CORNER);
        ofPopMatrix();
        ofPopStyle();
        
    }
    
    void draw() {
        ofSetColor(200, 100, 100, 50);
        ofDrawRectangle(0, 0, screen_width, screen_height);
//        ofBackground(0);
        ofSetColor(255, 255, 255);
        
        for (int i = n_mappers - 1; i >= 0; i--) {
            mappers[i].startMapping();
            if(i == 0)
                drawCamera();
            else
                drawBox();
            mappers[i].stopMapping();
        }
        
            // we can draw the bounding boxes around the projection mapper
        if (b_debug) {
            for (int i = 0; i < n_mappers; i++) {
                mappers[i].drawBoundingBox();
            }
        }
    }
    
    void keyPressed(int key) {
        if (key == 'f') {
            ofToggleFullscreen();
        }
        
            // if the keyboard is pressed with b
        else if (key == 'b') {
                // we start/stop drawing bounding boxes
            b_debug = !b_debug;
        }
        
        else if(key == 'l') {
            for (int i = 0; i < n_mappers; i++) {
                char buf[256];
                sprintf(buf, "mapper%d.txt", i);
                mappers[i].load(ofToDataPath(buf));
            }
        }
        
        else if(key == 's') {
            for (int i = 0; i < n_mappers; i++) {
                char buf[256];
                sprintf(buf, "mapper%d.txt", i);
                mappers[i].save(ofToDataPath(buf));
            }
        }
    }
    
        //--------------------------------------------------------------
    void mouseDragged(int x, int y, int button){
        for (int i = 0; i < n_mappers; i++) {
            mappers[i].mouseDragged(x, y);
        }
    }
    
        //--------------------------------------------------------------
    void mousePressed(int x, int y, int button){
        for (int i = 0; i < n_mappers; i++) {
            mappers[i].mousePressed(x, y);
        }
    }
    
        //--------------------------------------------------------------
    void mouseReleased(int x, int y, int button){
        for (int i = 0; i < n_mappers; i++) {
            mappers[i].mouseReleased(x, y);
        }
    }

private:
    int obj_width;
    int obj_height;
    int screen_width;
    int screen_height;
    
    ofVideoGrabber cam;
    
    ofFbo fbos[8];
    float brightness;
    
    int	n_mappers;
    pkmProjectionMapper mappers[8];
    bool b_debug;
};


int main(){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());
}
