#include "ofMain.h"


class ofApp : public ofBaseApp {
public:
    void setup() {
        width = 1280;
        height = 720;
        grabber.initGrabber(width, height);
        depths.resize(width * height);
        ofSetVerticalSync(true);
//        ofEnableDepthTest();
//        glPointSize(3);
    }
    void update() {
        grabber.update();
        if (grabber.isFrameNew()) {
            auto pixels = grabber.getPixels();
            mesh.clear();
            mesh.setMode(OF_PRIMITIVE_POINTS);
            for(int y = 0; y < height; y++) {
                for(int x = 0; x < width; x++) {
                    ofColor cur = pixels.getColor(x, y);
                    float previous_depth = depths[y * width + x];
                    float this_depth = cur.getLightness();
                    float depth = previous_depth * 0.5 + this_depth * 0.5;
                    depths[y * width + x] = depth;
                    float z = ofMap(depth, 0, 255, 0, 300);
                    cur.a = depth;
                    mesh.addColor(cur);
                    ofVec3f pos(x - width / 2, y - height / 2, z);
                    mesh.addVertex(pos);
                }
            }
        }
    }
    void draw() {
        ofBackground(0);
        cam.begin();
        ofScale(1, -1, 1);
        mesh.draw();
        cam.end();
    }
    void keyPressed(int key) {
        if(key == 'f'){
            ofToggleFullscreen();
        }
    }
    
    vector<float> depths;
    int width, height;
    ofVideoGrabber grabber;
    ofEasyCam cam;
    ofMesh mesh;
};


//========================================================================
int main( ){
	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
