#include "ofMain.h"

class ofApp : public ofBaseApp{
public:
    void setup() {
        // do some initialization
        
        // the rate at which the program runs (FPS)
        ofSetFrameRate(30);
        
        width = 480;
        height = 320;
        ofSetWindowShape(width, height);
        
        sampleRate = 44100;
        initialBufferSize = 512;
        ofSoundStreamSetup(0, 1, this, sampleRate, initialBufferSize, 4);
        
        buffer.resize(initialBufferSize);
    }
    
    // i get called in a loop that runs until the program ends
    void update() {
        
    }
    
    // i also get called in a loop that runs until the program ends
    void draw() {
        
        ofBackground(0);
        
        // change future drawing commands to be white
        ofSetColor(255, 255, 255, 200);

        // move the drawing down halfway
        ofTranslate(0, height/2);
        
        // this is going to store how many pixels we multiply our drawing by
        float amplitude = 100;
        
        // conversion from samples to pixels
        float width_ratio = width / (float)(initialBufferSize);
        
        // draw our audio buffer
        for (int i = 1; i < initialBufferSize; i++)
        {
            ofLine((i - 1) * width_ratio,
                   buffer[(i-1)] * amplitude,
                   i * width_ratio,
                   buffer[i] * amplitude);
        }
        
    }
    
    void keyPressed(int key) {
        if (key == 'f') {
            ofToggleFullscreen();
        }
    }
    

    void audioIn(float * input, int bufferSize, int nChannels) {
        
        if( initialBufferSize != bufferSize ){
            ofLog(OF_LOG_ERROR, "your buffer size was set to %i - but the stream needs a buffer size of %i", initialBufferSize, bufferSize);
            return;
        }

        // let's add the current frame of audio input to our recording buffer.  this is 512 samples.
        // (note: another way to do this is to copy the whole chunk of memory using memcpy)
        for (int i = 0; i < bufferSize; i++)
        {
            // we will set the value at each sample
            buffer[i] = input[i];
        }

    }
    

private:
    int                 width,
                        height;
    
    // variables for audio
    int                 initialBufferSize,
                        sampleRate;
    
    // this vector will store our audio recording
    vector<float>       buffer;

};


int main(){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());
}
