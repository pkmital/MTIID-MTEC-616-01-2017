#include "ofMain.h"
#include "ofxOpenCv.h"
#include "pkmBlobTracker.h"
#include "maximilian.h"


class ofApp : public ofBaseApp, public ofxCvBlobListener {
    
public:
    
    void blobOn( int x, int y, int id, int order ) {
            // trigger envelope/start sound
        snd_mapping[id] = curr_sound;
        
        sounds[snd_mapping[id]].play();
        
        curr_sound = (curr_sound + 1) % n_sounds;
    }

    void blobMoved( int x, int y, int id, int order ) {
            // change parameters based on blob properties
    }

    void blobOff( int x, int y, int id, int order ) {
            // kill it.
        
        sounds[snd_mapping[id]].stop();
    }
    
        // redeclaration of functions (declared in base class)
    void setup(){
        
            // do some initialization
        img_width = 320;
        img_height = 240;
        
        curr_sound = 0;
        
            // load each of our samples into 'sounds'
        ofDirectory dir;
        dir.open(ofToDataPath(""));
        vector<ofFile> files = dir.getFiles();
        n_sounds = 0;
        for (int i = 0; i < files.size(); i++) {
            if (files[i].getExtension() == "wav") {
                ofSoundPlayer player;
                player.load(files[i].getAbsolutePath());
                player.setMultiPlay(true);
                sounds.push_back(player);
                
                n_sounds ++;
            }
        }
        
            // set the size of the window
        ofSetWindowShape(img_width * 3.5, img_height * 1.5);
        
            // the rate at which the program runs (FPS)
        ofSetFrameRate(30);
        
            // setup the camera
        camera.setup(img_width, img_height);
        
            // setup the tracker
        tracker.setup(img_width, img_height);
        tracker.setListener(this);
    }
    
    void update(){
        camera.update();
        
        tracker.update(camera.getPixels());
    }
    
    void draw(){
            // background values go to 0
        ofBackground(0);
        
        ofSetColor(255);
        
            // draw the camera
        tracker.draw(0, 0);
    }
    
    void keyPressed(int key) {
        tracker.keyPressed(key);
    }
    
private:
    int n_sounds;
    vector<ofSoundPlayer> sounds;
    
    map<int, int> snd_mapping;
        // snd_mapping[blob_id] = sound_id;
    int curr_sound;
    
    ofVideoGrabber camera;
    pkmBlobTracker tracker;
    
    int img_width, img_height;
    
    bool b_capture;
};

int main(){
    ofSetupOpenGL(1024, 768, OF_WINDOW);
    ofRunApp(new ofApp());
}
