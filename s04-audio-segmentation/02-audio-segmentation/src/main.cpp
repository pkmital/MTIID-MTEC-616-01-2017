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
        buffer_size = 256;
        
        min_samples_per_grain = 0.2 * sample_rate;
        current_frame_i = 0;
        current_grain_i = 0;
        total_samples = 0;
        
        audio_input.resize(buffer_size);
        rms_values.resize(sample_rate / buffer_size);
        ofSoundStreamSetup(1, 1, sample_rate, buffer_size, 3);
    }
    
    // i get called in a loop that runs until the program ends
    void update() {
        
    }
    
    // i also get called in a loop that runs until the program ends
    void draw() {
        ofBackground(0);
        
        if(b_recording) {
            ofDrawBitmapString("Recording", 20, 20);
        }
        else {
            ofDrawBitmapString("Playing", 20, 20);
        }
        ofDrawBitmapString("Grains: " + ofToString(grains.size()), 20, 40);
        ofDrawBitmapString("Current Grain: " + ofToString(current_grain_i), 20, 60);
        ofDrawBitmapString("Current Frame: " + ofToString(current_frame_i), 20, 80);
        
        float skip = 4;
        float width_step = width / (float)(total_samples / skip);
        int samp_i = 0;
        for(int i = 0; i < grains.size(); i++) {
            bool this_grain = false;
            int prev_grain_start = samp_i;
            for(int j = 1; j < grains[i].size(); j+=4) {
                if (i == current_grain_i &&
                    j >= current_frame_i * buffer_size &&
                    j <= (current_frame_i + 1) * buffer_size)
                {
                    this_grain = true;
                    ofSetColor(255, 100, 100, 30);
                    ofDrawLine(samp_i * width_step,
                               0,
                               (samp_i + 1) * width_step,
                               height);
                }
                ofSetColor(255);
                ofDrawLine(samp_i * width_step,
                           -grains[i][j - 1] * height / 2 + height / 2,
                           (samp_i + 1) * width_step,
                           -grains[i][j] * height / 2 + height / 2);
                samp_i++;
            }
            if(this_grain) {
                ofSetColor(100, 255, 100, 30);
                ofSetRectMode(OF_RECTMODE_CORNER);
                ofDrawRectangle(prev_grain_start * width_step,
                                0,
                                (samp_i - prev_grain_start) * width_step,
                                height);
            }
        }
    }
    
    void keyPressed(int k) {
        if(k == ' '){
            b_recording = !b_recording;
        }
    }
    
    void audioOut(float * output, int buffer_size, int n_channels) {
        if (b_recording) {
            return;
        }
        
        if(grains.size()) {
            
            for (int i = 0; i < buffer_size; i++) {
                output[i] = grains[current_grain_i][i + current_frame_i * buffer_size];
            }
            
            current_frame_i += 1;
            
            if(current_frame_i * buffer_size > min_samples_per_grain ||
               current_frame_i >= grains[current_grain_i].size() / buffer_size)
            {
                if(rand() % 100 > 75) {
                    current_frame_i = 0;
                }
                else {
                    current_grain_i = rand() % grains.size();
                    current_frame_i = 0;
                }
            }
        }
    }
    
    void audioIn(float * input, int buffer_size, int n_channels) {
        // copy the data into our variable, audioInput
        std::memcpy(&audio_input[0], input, sizeof(float) * buffer_size);

        if (!b_recording) {
            return;
        }
        
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
        if (rms > (average_rms + 2.0 * var_rms) &&
            rms > min_rms &&
            current_grain.size() > min_samples_per_grain) {
            grains.push_back(current_grain);
            current_grain.resize(0);
        }
        
        for (int i = 0; i < buffer_size; i++) {
            current_grain.push_back(input[i]);
        }
        total_samples += buffer_size;
        
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
    
    vector<vector<float>>   grains;
    vector<float>           current_grain;
    int                     total_samples;

    int                     current_frame_i;
    int                     current_grain_i;
    int                     min_samples_per_grain;
    
    bool                    b_recording;
};


int main() {
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new App());
}
