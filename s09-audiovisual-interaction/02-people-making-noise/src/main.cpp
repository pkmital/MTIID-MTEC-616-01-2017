#include "ofMain.h"
#include "ofxOpenCv.h"
#include "maximilian.h"
#include "maxiGrains.h"
#include "pkmBlobTracker.h"

const int W = 320;
const int H = 240;
const int WINDOW_WIDTH = W*3 + 40*2;
const int WINDOW_HEIGHT = H*1.5;


class ofApp : public ofBaseApp, public ofxCvBlobListener {
    
public:
    
    void blobOn( int x, int y, int id, int order )
    {
        snd_mapping[id] = curr_sound;
        int this_sound = snd_mapping[id];

        px[this_sound] = x;
        py[this_sound] = y;
        visible[this_sound] = true;
        
        curr_sound = (curr_sound + 1) % n_sounds;
    }
    
    void blobMoved( int x, int y, int id, int order )
    {
        int this_sound = snd_mapping[id];
        int previous_x = px[this_sound];
        int previous_y = py[this_sound];
        
        float speed = sqrtf((x - previous_x)*(x - previous_x) +
                            (y - previous_y)*(y - previous_y));
        
        px[this_sound] = x;
        py[this_sound] = y;
        
        velocities[this_sound] = speed;
    }
    
    void blobOff( int x, int y, int id, int order )
    {
        int this_sound = snd_mapping[id];
        velocities[this_sound] = 0.0;
        visible[this_sound] = false;
    }
    
        // redeclaration of functions (declared in base class)
    void setup(){
        
        camera.initGrabber(W, H);
        
        ofSetWindowShape(WINDOW_WIDTH, WINDOW_HEIGHT);
        ofSetFrameRate(120);
        ofSetBackgroundAuto(true);
        ofBackground(0,0,0);
        
        tracker.setup(W, H);
        tracker.setListener(this);
        
        vector<string> samples =
        {
            "pads.wav",
            "bass.wav",
            "snare.wav",
            "breathe.wav",
            "street-footsteps.wav",
            "tile-footsteps.wav",
            "mosquito1.wav",
            "mosquito2.wav"
        };
        
        curr_sound = 0;
        n_sounds = samples.size();
        
        visible.resize(n_sounds);
        delays.resize(n_sounds);
        sounds.resize(n_sounds);
        lines.resize(n_sounds);
        velocities.resize(n_sounds);
        px.resize(n_sounds);
        py.resize(n_sounds);
        ts.resize(n_sounds);
        
        for (int i = 0; i < n_sounds; i++) {
            sounds[i].load(ofToDataPath(samples[i]));
            ts[i] = new maxiTimePitchStretch<hannWinFunctor, maxiSample>(&sounds[i]);
            velocities[i] = 0.0;
            px[i] = W / 2;
            py[i] = H / 2;
        }
        
        maxiSettings::setup(44100, 1, 512);
        ofSoundStreamSetup(1, 0, 44100, 512, 3);
    }
    
    void update(){
        ofBackground(0,0,0);
        
        camera.update();
        if (camera.isFrameNew()) {
            tracker.update(camera.getPixels());
            
        }
        
    }
    
    void draw(){
            // background values go to 0
        ofBackground(0);
        
        ofSetColor(255);
        tracker.draw(0, 0);
    }
    
    void keyPressed(int key) {
        switch (key){
            case 'f':
                ofToggleFullscreen();
                break;
            default:
                tracker.keyPressed(key);
                break;
        }
    }
    
    void audioOut(float *buf, int buffer_size, int ch) {
        for (int sample_i = 0; sample_i < buffer_size; sample_i++) {
            buf[sample_i] = 0.0;
            for (int sound_i = 0; sound_i < n_sounds; sound_i++) {
                float speed = lines[sound_i].play(ofMap(ofClamp(velocities[sound_i], 0.0, 10.0), 0.0, 10.0, 0.0, 2.0));
                if(visible[sound_i])
                    buf[sample_i] += (*ts[sound_i]).play(1.0, 1.0 - speed, 0.2, 3);
            }
        }
    }
    
private:
    
    int                             img_width, img_height;

    ofVideoGrabber                  camera;
    pkmBlobTracker                  tracker;
    
    vector<maxiDelayline>           delays;
    vector<float>                   velocities;
    vector<bool>                    visible;
    vector<int>                     px, py;
    vector<maxiEnvelopeFollower>    lines;

    vector<maxiSample>              sounds;
    vector<maxiTimePitchStretch<hannWinFunctor, maxiSample> *> ts;
    int                             n_sounds;
    int                             curr_sound;
    
    map<int, int>                   snd_mapping;
};

int main(){
    ofSetupOpenGL(1024, 768, OF_WINDOW);
    ofRunApp(new ofApp());
}
