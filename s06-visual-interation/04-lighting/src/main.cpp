#include "ofMain.h"


class ofApp : public ofBaseApp {
public:
    void setup() {
        width = 640;
        height = 480;
        grabber.initGrabber(width, height);
        depths.resize(width * height);
        ofSetVerticalSync(true);
        glEnable(GL_DEPTH_TEST);
        glPointSize(10);
        
        
        light.enable();
    }
    void update() {
        light.setPosition(mouseX, height - mouseY, 100);
        
        grabber.update();
        if (grabber.isFrameNew()) {
            auto pixels = grabber.getPixels();
            mesh.clear();
            mesh.setMode(OF_PRIMITIVE_POINTS);
            for(int y = 0; y < height; y++) {
                for(int x = 0; x < width; x++) {
                    ofColor cur = pixels.getColor(x, y);
                    float previous_depth = depths[y * width + x];
                    float this_depth = cur.r * 0.28 + cur.g * 0.6 + cur.b * 0.12;
                    float depth = previous_depth * 0.95 + this_depth * 0.05;
                    depths[y * width + x] = depth;
                    if(depth > 20 && depth < 200) {
                            // the alpha value encodes depth, let's remap it to a good depth range
                        float z = ofMap(depth, 0, 255, -100, 100);
                        cur.a = depth;
                        mesh.addColor(cur);
                        ofVec3f pos(x - width / 2, y - height / 2, z);
                        mesh.addVertex(pos);
                    }
                }
            }
        }
    }
    void draw() {
        ofBackground(0);
        cam.begin();
        ofScale(1, -1, 1); // make y point down
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
    ofLight light;
};


    //========================================================================
int main( ){
    ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context
    
        // this kicks off the running of my app
        // can be OF_WINDOW or OF_FULLSCREEN
        // pass in width and height too:
    ofRunApp(new ofApp());
    
}
