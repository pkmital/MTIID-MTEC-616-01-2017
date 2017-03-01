#include "ofMain.h"
#include "ofxGui.h"


class ofApp : public ofBaseApp {
public:
    void setup() {
        width = 640;
        height = 480;
        grabber.initGrabber(width, height);
        
            // setup the gui:
        gui.setup("panel");
        gui.add(point_size.set("point-size", 1.0, 0.0, 10.0));
    }
    
    void update() {
        grabber.update();
        
        glPointSize(point_size);
        
        if (grabber.isFrameNew())
        {
            auto pixels = grabber.getPixels();
            meshes.clear();
            for(int y = 0; y < height; y++) {
                ofMesh mesh;
                mesh.setMode(OF_PRIMITIVE_POINTS);
                for(int x = 0; x < width; x++) {
                    ofColor color = pixels.getColor(x, y);
                    float z = color.getLightness();
                    mesh.addColor(color);
                    mesh.addVertex(ofVec3f(x, y, z));
                }
                meshes.push_back(mesh);
            }
        }
    }
    
    void draw() {
        ofBackground(0);
        
        cam.begin();
        for (int i = 0; i < meshes.size(); i++) {
            meshes[i].draw();
        }
        cam.end();
        
        gui.draw();
        
        ofSetWindowTitle(ofToString(ofGetFrameRate()));
    }
    
    void keyPressed(int key) {
        if(key == 'f'){
            ofToggleFullscreen();
        }
    }
    
    void mousePressed(ofMouseEventArgs &args) {
        gui.mousePressed(args);
    }
    
    ofxPanel gui;
    ofParameter<float> point_size;
    
    int width, height;
    ofVideoGrabber grabber;
    ofEasyCam cam;
    ofLight light;
    vector<ofMesh> meshes;
};


//========================================================================
int main( ){
	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
