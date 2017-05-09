#include "ofMain.h"


class App : public ofBaseApp{
public:
    void setup() {
        width = 640;
        height = 480;
        ofSetWindowShape(width, height);
        
        ofSetFrameRate(60);
        ofSetBackgroundAuto(false);
        
        sample_rate = 44100;
        buffer_size = 512;
        audio_input.resize(buffer_size);
        ofSoundStreamSetup(0, 1, sample_rate, buffer_size, 3);
    }
    
    // i get called in a loop that runs until the program ends
    void update() {
        
    }
    
    // i also get called in a loop that runs until the program ends
    void draw() {
        ofBackground(0);
        
        // draw a line across the middle of the screen
        ofSetColor(100, 100, 100);
        ofDrawLine(0, height / 2, width, height / 2);
        
        // we draw the audio input as before
        ofSetColor(200, 200, 200);
        float amplitude = 100.0f;
        for (int i = 1; i < audio_input.size(); i++) {
            
            // get two pairs of points
            float x1 = i * width / (float)audio_input.size();
            float y1 = amplitude * audio_input[i];
            float x2 = (i - 1) * width / (float)audio_input.size();
            float y2 = amplitude * audio_input[i-1];
            
            // draw a tiny segment of the overall line
            ofDrawLine(x1, -y1 + height / 2,
                       x2, -y2 + height / 2);
        }
        
        // draw a circle in the middle of the screen with the size
        // set by the rms value
        ofSetRectMode(OF_RECTMODE_CENTER);
        ofDrawCircle(width / 2, height / 2, rms * width);

        if(rms_values.size() > 0)
        {
            ofSetRectMode(OF_RECTMODE_CORNER);
            
            for (int i = 1; i < rms_values.size(); i++) {
                ofDrawLine(i * width / (float)rms_values.size(),
                           -rms_values[i] * 1000.0 + height,
                           (i - 1) * width / (float)rms_values.size(),
                           -rms_values[i - 1] * 1000.0 + height);
            }

            // calculate the average of the rms values
            float average_rms = 0.0f;
            for (int i = 0; i < rms_values.size(); i++) {
                average_rms = average_rms + rms_values[i];
            }
            average_rms = average_rms / rms_values.size();
            
            // calculate the variance of the rms values
            float var_rms = 0.0f;
            for (int i = 0; i < rms_values.size(); i++) {
                var_rms = var_rms + abs(rms_values[i] - average_rms);
            }
            var_rms = var_rms / rms_values.size();
            
            // now we see if the current value is outside the mean + variance
            // basic statistics tells us a normally distributed function
            // has a mean and a variance where 97% of the data is explained by
            // 3 standard deviations.  we use this principle here in detecting 
            // the the current rms reading is outside this probability
            float min_rms = 0.01;
            if (rms_values.back() > (average_rms + 2.0 * var_rms) &&
                rms_values.back() > min_rms) {
                // draw a rectangle to denote the detection of an onset
                ofDrawRectangle(0, 0, width, height);
            }
        }
        ofDisableAlphaBlending();
    }
    
    void audioIn(float * input, int buffer_size, int n_channels)
    {
        // copy the data into our variable, audioInput
        std::memcpy(&audio_input[0], input, sizeof(float) * buffer_size);
        
        // add all the audio input values
        float total = 0;
        for (int i = 0; i < buffer_size; i++) {
            // we add the "square" of each value so that negative numbers
            // become positive.  this is like thinking of the "magnitude"
            total = total + (input[i] * input[i]);
        }
        // the "mean" part of the RMS, we divide by the number of audio input samples
        // we added in the for loop above
        total = total / (float)buffer_size;
        
        // the "root" part of RMS, we take the square root to get our RMS reading for the
        // current chunk of audio input values
        rms = sqrt(total);
        
        // add the current rms value
        rms_values.push_back(rms);
        
        // we only keep a maximum of 1 second of the rms readings
        if (rms_values.size() > (sample_rate / buffer_size)) {
            // then we delete the first one
            rms_values.erase(rms_values.begin(), rms_values.begin() + 1);
        }
    }
    

private:
    int                     width,
                            height;
    
    vector<float>           audio_input;
    
    int                     sample_rate,
                            buffer_size;
    
    float                   rms;
    
    vector<float>           rms_values;
};


int main(){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new App());
}
