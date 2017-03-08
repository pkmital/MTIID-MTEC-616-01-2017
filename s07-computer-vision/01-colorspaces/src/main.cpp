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
        b_show_blended = false;
        
        // set the size of the window
        ofSetWindowShape(img_width * 6, img_height);
        
        // the rate at which the program runs (FPS)
        ofSetFrameRate(30);
        
        // setup the camera
        camera.initGrabber(img_width, img_height);
        
        // create some ofxOpenCv images.  ofxCvColorImage can hold color images
        im_color.allocate(img_width, img_height);
        
        // while ofxCvGrayscaleImage can only hold 1 plane of image information,
        // such as just the "R" channel of an RGB image
        im_red.allocate(img_width, img_height);
        im_green.allocate(img_width, img_height);
        im_blue.allocate(img_width, img_height);
        
        // or a grayscale image
        im_gray.allocate(img_width, img_height);
        
        // or just the "V" plane of a HSV image
        im_value.allocate(img_width, img_height);
    }
    
    void update(){
        camera.update();
        
        if(camera.isFrameNew())
        {
            // copy the pixels from the camera object into an ofxCvColorImage object
            im_color.setFromPixels(camera.getPixels());
            
            // convert the color image to a gray color image... as simple as that.
            im_gray = im_color;
            
            // get each color channel from the color image
            im_color.convertToGrayscalePlanarImages(im_red, im_green, im_blue);
            
            // convert our rgb color image to hsv
            im_color.convertRgbToHsv();
            
            // get just the 2nd (0, 1, 2) plane, representing the "V" of the HSV image
            im_color.convertToGrayscalePlanarImage(im_value, 2);
        }
    }
    
    void draw(){
        // background values go to 0
        ofBackground(0);
        
        // draw the camera
        ofSetColor(255, 255, 255);
        camera.draw(0, 0);
        
        if(b_show_blended)
        {
            // blending mode for adding pictures together
            ofEnableAlphaBlending();
            ofEnableBlendMode(OF_BLENDMODE_ADD);

            // full red energy
            ofSetColor(255, 0, 0);
            im_red.draw(img_width * 1,0);
            
            // full green energy
            ofSetColor(0, 255, 0);
            
            // draw using an offset from the center of the screen determined by the mouse position
            im_green.draw(img_width * 1 + (mouseX - ofGetScreenWidth()/2) / 10.0,0);
            
            // full blue energy
            ofSetColor(0, 0, 255);
            
            // offset just like above, but 2x as much
            im_blue.draw(img_width * 1 + (mouseX - ofGetScreenWidth()/2) / 5.0,0);
            
            ofDisableAlphaBlending();
        }
        else
        {
            ofSetColor(255, 0, 0);
            im_red.draw(img_width * 1,0);
            
            ofSetColor(0, 255, 0);
            im_green.draw(img_width * 2,0);
            
            ofSetColor(0, 0, 255);
            im_blue.draw(img_width * 3,0);
        }
        
        ofSetColor(255, 255, 255);
        im_gray.draw(img_width * 4, 0);
        im_value.draw(img_width * 5, 0);
    }
    
    void keyPressed(int key) {
        switch (key) {
            case ' ':
                b_show_blended = !b_show_blended;
                break;
            default:
                break;
        }
    }
    
private:
    
    ofVideoGrabber camera;
    
    ofxCvColorImage im_color;
    ofxCvGrayscaleImage im_red, im_green, im_blue;
    ofxCvGrayscaleImage im_gray;
    ofxCvGrayscaleImage im_value;
    
    int img_width, img_height;
    
    bool b_show_blended;
};

int main(){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());
}
