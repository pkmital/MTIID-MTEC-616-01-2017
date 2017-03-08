#include "ofMain.h"
#include "ofxOpenCv.h"

class ofApp : public ofBaseApp{
    
public:
    
        // redeclaration of functions (declared in base class)
    void setup(){
        
        // do some initialization
        img_width = 320;
        img_height = 240;
        
        // flag which will change the behavior of our program.  initially false.
        b_capture = false;
        
        // set the size of the window
        ofSetWindowShape(img_width * 3, img_height);
        
        // the rate at which the program runs (FPS)
        ofSetFrameRate(30);
        
        // setup the camera
        camera.initGrabber(img_width, img_height);
        
        // create some ofxOpenCv images.  ofxCvColorImage can hold color images
        rgb.allocate(img_width, img_height);
        gray.allocate(img_width, img_height);
        diff.allocate(img_width, img_height);
    }
    
    void update(){
        camera.update();
        
        if(camera.isFrameNew())
        {
            // copy the pixels from the camera object into an ofxCvColorImage object
            rgb.setFromPixels(camera.getPixels());
            
            // convert our rgb color image to hsv
            rgb.convertRgbToHsv();
            
            // get just the 2nd (0, 1, 2) plane, representing the "V" of the HSV image
            rgb.convertToGrayscalePlanarImage(gray, 2);
            
            
            
            // store as previous image
            prev_gray = gray;
        }
    }
    
    void draw(){
        // background values go to 0
        ofBackground(0);
        
        // draw the camera
        ofSetColor(255, 255, 255);
        camera.draw(0, 0);
        gray.draw(img_width, 0);
        diff.draw(img_width * 2, 0);
        
        
    }
    
    void keyPressed(int key) {
        switch (key) {
            case ' ':
                b_capture = true;
                break;
            default:
                break;
        }
    }
    
private:
    
    ofVideoGrabber camera;
    
    ofxCvColorImage rgb;
    ofxCvGrayscaleImage gray, prev_gray, diff;
    
    int img_width, img_height;
    
    bool b_capture;
};

int main(){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());
}
