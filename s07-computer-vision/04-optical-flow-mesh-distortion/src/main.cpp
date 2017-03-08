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
        ofSetWindowShape(img_width * 2, img_height * 2);
        
        // the rate at which the program runs (FPS)
        ofSetFrameRate(30);
        
        // setup the camera
        camera.initGrabber(img_width, img_height);
        
        flow = cv::createOptFlow_DualTVL1();
        flow->set("iterations", 10);
        flow->set("nscales", 3);
        flow->set("warps", 3);

        // create some ofxOpenCv images.  ofxCvColorImage can hold color images
        img.allocate(img_width, img_height);
        gray.allocate(img_width, img_height);
        prev_gray.allocate(img_width, img_height);
        mag.allocate(img_width, img_height);
        
        // create a mesh from the camera
        mesh.setMode(OF_PRIMITIVE_TRIANGLES);
        step = 2;
        
        for(int y = 0; y < img_height; y+=step) {
            for(int x = 0; x < img_width; x+=step) {
                mesh.addVertex(ofVec2f(x - img_width / 2, y - img_height / 2));
                mesh.addTexCoord(ofVec2f(x, y));
            }
        }
        
        for(int y = 0; y < (img_height - 1) / step; y++) {
            for(int x = 0; x < (img_width - 1) / step; x++) {
                int nw = y * img_width / step + x;
                int ne = nw + 1;
                int sw = nw + img_width / step;
                int se = sw + 1;
                mesh.addIndex(nw);
                mesh.addIndex(ne);
                mesh.addIndex(se);
                mesh.addIndex(nw);
                mesh.addIndex(se);
                mesh.addIndex(sw);
            }
        }
    }
    
    void update(){
        camera.update();
        
        if(camera.isFrameNew())
        {
            // copy the pixels from the camera object into an ofxCvColorImage object
            img.setFromPixels(camera.getPixels());
            
            // convert our rgb color image to hsv
            img.convertRgbToHsv();
            
            // get just the 2nd (0, 1, 2) plane, representing the "V" of the HSV image
            img.convertToGrayscalePlanarImage(gray, 2);
            
            // calculate optical flow
            cv::Mat gray_mat(gray.getCvImage()),
                prev_gray_mat(prev_gray.getCvImage()),
                mag_mat(mag.getCvImage()),
                phase_mat,
                flow_mat,
                flows[2];
            
            flow->calc(prev_gray_mat, gray_mat, flow_mat);
            cv::split(flow_mat, flows);
            cv::cartToPolar(flows[0], flows[1], mag_mat, phase_mat, true);
            mag_mat /= 20.0;
            
            mag.flagImageChanged();
            
            // store as previous image
            prev_gray = gray;
            
            for(int y = 0; y < img_height; y+=step) {
                for(int x = 0; x < img_width; x+=step) {
                    ofVec3f position(x - img_width / 2, y - img_height / 2);
                    float dx = flows[0].at<float>(y, x);
                    float dy = flows[1].at<float>(y, x);
                    float dz = sqrt(dx*dx + dy*dy) * 10.0;
                    ofVec3f offset(dx, dy, dz);
                    int idx = (y / step * img_width / step + x / step);
                    mesh.setVertex(idx, position + offset);

                }
            }
        }
    }
    
    void draw(){
        // background values go to 0
        ofBackground(0);
        
        // draw the camera
        ofSetColor(255, 255, 255);
        
        scene.begin();
        ofPushMatrix();
        camera.getTexture().bind();
        mesh.draw();
        camera.getTexture().unbind();
        ofPopMatrix();
        scene.end();
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
    
    ofxCvColorImage img;
    ofxCvGrayscaleImage gray, prev_gray;
    ofxCvFloatImage mag;
    
    cv::Ptr<cv::DenseOpticalFlow> flow;
    
    ofMesh mesh;
    int step;
    
    ofEasyCam scene;
    
    int img_width, img_height;
    
    bool b_capture;
};

int main(){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());
}
