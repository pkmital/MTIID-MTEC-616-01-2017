# Session 03 - 02/07/17 - Audio Interaction

<!-- MarkdownTOC -->

- [Session Overview](#session-overview)
- [Topics](#topics)
    - [A Note on Coding Style](#a-note-on-coding-style)
    - [Button Pad Array](#button-pad-array)
    - [Multisampler](#multisampler)
    - [RMS](#rms)
    - [Audio Segmentation](#audio-segmentation)
    - [Basic GUI](#basic-gui)
    - [Multiple Windows](#multiple-windows)
    - [Lab](#lab)
    - [Homework](#homework)

<!-- /MarkdownTOC -->


<a name="session-overview"></a>
## Session Overview

We'll continue with developing a multisampler that can record/playback audio, automatically segment audio, and apply some simple filters.
 
<a name="topics"></a>
## Topics

<a name="a-note-on-coding-style"></a>
### A Note on Coding Style

This is really up to you and the project you are working on.  You may be contrainsed to a particular coding convention based on the API you are expected to build, the clients you are writing for, or an existing code base that you are working with.  In this course, from now on, I'll try to stick to this coding style convention:

#### snake_case

Variables, e.g.:

```cpp
int pt_x, pt_y;
```

Also for objects such as:

```cpp
class Point {
public:
    int x, int y;
};

Point pt[2];
pt[0].x = 5;
pt[0].y = 10;
pt[1].x = 2;
pt[1].y = 20;
```

Note that openFrameworks in general uses camelCase for their variable names instead of snake_case.  However, I prefer to use snake_case since it_is_a_lot_easier_to_read_and_clearly_separates_functions_from_variables.

#### camelCase

Function Names, e.g.:

```cpp
void audioIn(...)
void audioOut(...)
```

Same for class functions, e.g.:

```cpp
class Point {
public:
    void moveTo(int x, int y)
    {
        _x = x;
        _y = y;
    }
private:
    int _x, int _y;
};
```

#### PascalCase

Class Names, e.g.:

```cpp
class Box : public Point {
public:
    int size;
};
```

#### Boolean Variables

Will be preprended w/ `is_`, e.g. `is_something` or `is_recording`.

#### Count Variables

Will be prepended w/ `n_`, e.g. `n_buttons` or `n_samples`.

<a name="button-pad-array"></a>
### Button Pad Array

```cpp
#include "ofMain.h"


// declare the padbutton class and methods
class padButton {
public:
    // enumerators allow us to assign more interesting names to values of an integer
    // we could use an integer to the same effect,
    // e.g. "int button_state = 0", when our button is down
    // and "int button_state = 1", when our button is normal,
    // but enumerators allow us to instead say
    // "BUTTON_STATE button_state = BUTTON_DOWN", when our button is down,
    // "BUTTON_STATE button_state = NORMAL", when our button is normal.
    enum BUTTON_STATE {
        BUTTON_DOWN,
        NORMAL
    };
    
    // default constructor definition
    padButton()
    {
        // by default, we have a very boring button
        button_x = 0;
        button_y = 0;
        button_width = 0;
        button_height = 0;
        button_state = NORMAL;
    }

    // methods which our button class will define
    // one for loading images for each of the button states
    void load(string state_normal, string state_down)
    {
        // load the images for our buttons
        button_image_normal.load(state_normal);
        button_image_down.load(state_down);
    }
    
    // setters, to set internal variables
    // the position
    void setPosition(int x, int y)
    {
        // set our internal variables
        button_x = x;
        button_y = y;
    }
    
    // the size
    void setSize(int w, int h)
    {
        // set our internal variables
        button_width = w;
        button_height = h;
    }

    // drawing the buttons
    void draw()
    {
        // allow alpha transparency
        ofEnableAlphaBlending();
        
        // if our button is normal
        if(button_state == NORMAL)
        {
            // draw the normal button image
            button_image_normal.draw(button_x,
                                     button_y,
                                     button_width,
                                     button_height);
        }
        else
        {
            // draw the down image
            button_image_down.draw(button_x,
                                   button_y,
                                   button_width,
                                   button_height);
        }
        
        // ok done w/ alpha blending
        ofDisableAlphaBlending();
    }

    // and interaction with the button
    bool pressed(int x, int y)
    {
        // compound boolean expressions to determine,
        // is our x,y input within the bounds of the button
        // we have to check the left, the top, the right, and the bottom sides
        // of the button, respectively.
        if( x > button_x && y > button_y
           && x < (button_x + button_width)
           && y < (button_y + button_height) )
        {
            button_state = BUTTON_DOWN;
            
            // return yes since the user pressed the button
            return true;
        }
        else {
            // no the user didn't press the button
            return false;
        }
    }

    bool released(int x, int y)
    {
        // ok back to normal
        button_state = NORMAL;
        
        // we always return true since that is how our buttons work.
        return true;
    }
private:
    
    // images for drawing
    ofImage button_image_normal, button_image_down;
    
    // our position
    int button_x, button_y;
    
    // size
    int button_width, button_height;
    
    // and internal state of the button
    BUTTON_STATE button_state;
};

class ofApp : public ofBaseApp{
public:
    void setup() {
        width = 500;
        height = 500;
        
        // do some initialization
        ofSetWindowShape(width, height);
        
        n_buttons = 4;
        button_width = width / n_buttons;
        button_height = height / n_buttons;
        
        // we allow our vector to have 4 padButtons, which we index from 0 - 3
        buttons.resize(n_buttons * n_buttons);
        
        // use a nested loop to initialize our buttons, setting their positions as a square matrix
        // - this outer-loop iterates over ROWS
        for (int j = 0; j < n_buttons; j++)
        {
            // - this inner-loop iterates over COLUMNS
            for (int i = 0; i < n_buttons; i = i + 1)
            {
                // notice how we use the loop variables, i and j, in setting the x,y positions of each button
                buttons[j * n_buttons + i].setPosition(button_width * i,
                                                       button_height * j);
                buttons[j * n_buttons + i].setSize(button_width,
                                                   button_height);
                buttons[j * n_buttons + i].load("button.png",
                                                "button-down.png");
            }
        }
    }
    
    // i get called in a loop that runs until the program ends
    void update() {
        
    }
    
    // i also get called in a loop that runs until the program ends
    void draw() {
        ofBackground(0);
        
        // draw our buttons
        for (int i = 0; i < buttons.size(); i++) {
            buttons[i].draw();
        }
    }
    
    void mousePressed(int x, int y, int button) {
        for (int i = 0; i < buttons.size(); i++) {
            buttons[i].pressed(x, y);
        }
    }
    
    void mouseReleased(int x, int y, int button) {
        for (int i = 0; i < buttons.size(); i++) {
            buttons[i].released(x, y);
        }
    }
    

private:
    int                 width,
                        height;

    // a vector is a c-standard library implementation of an array
    // this allows us to create multiple buttons
    vector<padButton>   buttons;
    int                 n_buttons,
                        button_width,
                        button_height;
};


int main(){
    ofSetupOpenGL(1024, 768, OF_WINDOW);
    ofRunApp(new ofApp());
}
```

<a name="multisampler"></a>
### Multisampler

```cpp
#include "ofMain.h"


// declare the ButtonPad class and methods
class ButtonPad {
public:
    // enumerators allow us to assign more interesting names to values of an integer
    // we could use an integer to the same effect,
    // e.g. "int button_state = 0", when our button is down
    // and "int button_state = 1", when our button is normal,
    // but enumerators allow us to instead say
    // "BUTTON_STATE button_state = BUTTON_DOWN", when our button is down,
    // "BUTTON_STATE button_state = NORMAL", when our button is normal.
    enum BUTTON_STATE {
        BUTTON_DOWN,
        NORMAL
    };
    
    // default constructor definition
    ButtonPad() {
        // by default, we have a very boring button
        button_x = 0;
        button_y = 0;
        button_width = 0;
        button_height = 0;
        button_state = NORMAL;
    }

    // methods which our button class will define
    // one for loading images for each of the button states
    void load(string state_normal, string state_down) {
        // load the images for our buttons
        button_image_normal.load(state_normal);
        button_image_down.load(state_down);
    }
    
    // setters, to set internal variables
    // the position
    void setPosition(int x, int y) {
        // set our internal variables
        button_x = x;
        button_y = y;
    }
    
    // the size
    void setSize(int w, int h) {
        // set our internal variables
        button_width = w;
        button_height = h;
    }

    // drawing the buttons
    void draw() {
        // if our button is normal
        if(button_state == NORMAL) {
            // draw the normal button image
            button_image_normal.draw(button_x,
                                     button_y,
                                     button_width,
                                     button_height);
        }
        else {
            // draw the down image
            button_image_down.draw(button_x,
                                   button_y,
                                   button_width,
                                   button_height);
        }
    }
    
    bool isPressed() {
        return button_state == BUTTON_DOWN;
    }

    // and interaction with the button
    bool pressed(int x, int y) {
        // compound boolean expressions to determine,
        // is our x,y input within the bounds of the button
        // we have to check the left, the top, the right, and the bottom sides
        // of the button, respectively.
        if( x > button_x && y > button_y
           && x < (button_x + button_width)
           && y < (button_y + button_height) ) {
            
            button_state = BUTTON_DOWN;
            
            // return yes since the user pressed the button
            return true;
        }
        else {
            // no the user didn't press the button
            return false;
        }
    }

    bool released(int x, int y) {
        if( x > button_x && y > button_y
           && x < (button_x + button_width)
           && y < (button_y + button_height) ) {
            
            // ok back to normal
            button_state = NORMAL;
            
            // we always return true since that is how our buttons work.
            return true;
        }
        
        return false;
    }
private:
    
    // images for drawing
    ofImage button_image_normal, button_image_down;
    
    // our position
    int button_x, button_y;
    
    // size
    int button_width, button_height;
    
    // and internal state of the button
    BUTTON_STATE button_state;
};

class AudioPad : public ButtonPad {
public:
    AudioPad() {
        frame = 0;
        n_frames = 0;
        is_recording = false;
        is_playing = false;
        is_recorded = false;
    }
    
    void setRecording(bool recording) {
        is_recording = recording;
    }
    
    bool isRecording() {
        return is_recording;
    }
    
    void toggleRecording() {
        is_recording = !is_recording;
    }
    
    void setPlaying(bool playing) {
        is_playing = playing;
    }
    
    bool isPlaying() {
        return is_playing;
    }
    
    void togglePlaying() {
        is_playing = !is_playing;
    }
    
    bool hasSamples() {
        return buffer.size() > 0;
    }
    
    void setKey(char k) {
        key = k;
    }
    
    bool keyPressed(int k) {
        if(key == k)
            return true;
        else
            return false;
    }
    
    bool keyReleased(int k) {
        if(key == k)
            return true;
        else
            return false;
    }
    
    void audioOut(float * output, int buffer_size, int n_channels) {

        // if we are playing back audio (if the user is pressing the play button)
        if(is_playing && is_recorded) {
            // we set the output to be our recorded buffer
            for (int i = 0; i < buffer_size; i++){
                // so we have to access the current "playback frame" which is a variable
                // "frame".  this variable helps us determine which frame we should play back.
                // because one frame is only 512 samples, or 1/90th of a second of audio, we would like
                // to hear more than just that one frame.  so we playback not just the first frame,
                // but every frame after that... after 90 frames of audio, we will have heard
                // 1 second of the recording...
                output[i] += buffer[i + frame*buffer_size];
            }
            
            // increment the frame counter
            frame = frame + 1;
            
            // check if the frame counter has reached the end of the recorded
            // buffer.  if so, then we should go back to the start again
            if (frame > (buffer.size() / buffer_size)) {
                frame = 0;
            }
        }
        // else don't output anything to the speaker
        else {
            memset(output, 0, n_channels * buffer_size * sizeof(float));
        }
    }
    
    void audioIn(float * input, int buffer_size, int n_channels) {
        
        // if we are recording
        if(is_recording) {
            // let's add the current frame of audio input to our recording buffer.  this is 512 samples.
            // (note: another way to do this is to copy the whole chunk of memory using memcpy)
            for (int i = 0; i < buffer_size; i++)
            {
                // we will add a sample at a time to the back of the buffer, increasing the size of "buffer"
                buffer.push_back(input[i]);
            }
            
            // we also need to keep track of how many audio "frames" we have.  this is how many times
            // we have recorded a chunk of 512 samples.  we refer to that chunk of 512 samples as 1 frame.
            n_frames++;
            
            is_recorded = true;
        }
        // otherwise we set the input to 0
        else {
            // set the chunk in memory pointed to by "input" to 0.  the
            // size of the chunk is the 3rd argument.
            memset(input, 0, n_channels * buffer_size * sizeof(float));
        }
    }
    
    char                key;
    
    // this vector will store our audio recording
    vector<float>       buffer;
    
    // frame will tell us "what chunk of audio are we currently playing back".
    // as we record and play back in "chunks" also called "frames", we will need
    // to keep track of which frame we are playing during playback
    int                 frame;
    
    // frame will run until it hits the final recorded frame, which is n_frames
    // we increment this value every time we record a new frame of audio
    int                 n_frames;
    
    // determined based on whether the user has pressed the play or record buttons
    bool                is_recording, is_playing, is_recorded;

};


class App : public ofBaseApp{
public:
    void setup() {
        // do some initialization
        
        n_buttons = 4;
        button_width = 100;
        button_height = 100;
        
        width = button_width * n_buttons;
        height = button_height * n_buttons + button_height;
        ofSetWindowShape(width, height);
        
        is_recording = true;
        
        // we allow our vector to have 4 ButtonPads, which we index from 0 - 3
        buttons.resize(n_buttons * n_buttons);
        
        // make two buttons in the same position, and we'll toggle between these
        button_record.load("button-record.png", "button-record.png");
        button_record.setPosition(width / 2 - button_width / 2,
                                  height - button_height);
        button_record.setSize(button_width, button_height);

        button_stop.load("button-stop.png", "button-stop.png");
        button_stop.setPosition(width / 2 - button_width / 2,
                                height - button_height);
        button_stop.setSize(button_width, button_height);
        
        vector<vector<char>> keys = {
            {'1', '2', '3', '4'},
            {'q', 'w', 'e', 'r'},
            {'a', 's', 'd', 'f'},
            {'z', 'x', 'c', 'v'}};
        
        // use a nested loop to initialize our buttons, setting their positions as a square matrix
        // - this outer-loop iterates over ROWS
        for (int j = 0; j < n_buttons; j++) {
            // - this inner-loop iterates over COLUMNS
            for (int i = 0; i < n_buttons; i = i + 1) {
                // notice how we use the loop variables, i and j, in setting the x,y positions of each button
                buttons[j * n_buttons + i].setPosition(button_width * i,
                                                       button_height * j);
                buttons[j * n_buttons + i].setSize(button_width,
                                                   button_height);
                buttons[j * n_buttons + i].load("button.png",
                                                "button-down.png");
                buttons[j * n_buttons + i].setKey(keys[j][i]);
            }
        }
        
        ofSoundStreamSetup(1, 1, 44100, 512, 3);
    }
    
    // i get called in a loop that runs until the program ends
    void update() {
        
    }
    
    // i also get called in a loop that runs until the program ends
    void draw() {
        ofBackground(0);
        ofEnableAlphaBlending();
        ofSetColor(255);
        
        if(is_recording) {
            button_stop.draw();
        }
        else {
            button_record.draw();
        }
        
        // draw our buttons
        for (int i = 0; i < buttons.size(); i++) {
            if(buttons[i].isRecording()) {
                ofSetColor(200, 100, 100);
                buttons[i].draw();
            }
            else if(buttons[i].isPlaying()){
                ofSetColor(100, 200, 100);
                buttons[i].draw();
            }
            else {
                ofSetColor(200, 200, 200);
                buttons[i].draw();
            }
        }
        
        ofDisableAlphaBlending();
    }
    
    void keyPressed(int k) {
        for (int i = 0; i < buttons.size(); i++) {
            if(buttons[i].keyPressed(k)) {
                if(is_recording) {
                    buttons[i].setRecording(true);
                }
                else {
                    buttons[i].setPlaying(true);
                }
            }
        }
    }
    
    void keyReleased(int k) {
        for (int i = 0; i < buttons.size(); i++) {
            if(buttons[i].keyReleased(k)) {
                if(is_recording) {
                    buttons[i].setRecording(false);
                }
                else {
                    buttons[i].setPlaying(false);
                    buttons[i].frame = 0;
                }
            }
        }
    }
    
    void mousePressed(int x, int y, int button) {
        if(is_recording && button_stop.pressed(x, y)) {
            is_recording = false;
        }
        else if(!is_recording && button_record.pressed(x, y)) {
            is_recording = true;
        }

        for (int i = 0; i < buttons.size(); i++) {
            if(buttons[i].pressed(x, y)) {
                if(is_recording) {
                    buttons[i].buffer.resize(0);
                    buttons[i].setRecording(true);
                }
                else {
                    buttons[i].setPlaying(true);
                }
            }
        }
    }
    
    void mouseReleased(int x, int y, int button) {
        for (int i = 0; i < buttons.size(); i++) {
            if(buttons[i].released(x, y)) {
                if(is_recording) {
                    buttons[i].setRecording(false);
                }
                else {
                    buttons[i].setPlaying(false);
                    buttons[i].frame = 0;
                }
            }
        }
    }
    
    void audioOut(float * output, int bufferSize, int nChannels) {
        for (int i = 0; i < buttons.size(); i++) {
            if(buttons[i].isPlaying() && buttons[i].hasSamples()) {
                buttons[i].audioOut(output, bufferSize, nChannels);
            }
        }
    }
    
    void audioIn(float * input, int bufferSize, int nChannels) {
        for (int i = 0; i < buttons.size(); i++) {
            if(buttons[i].isRecording()) {
                buttons[i].audioIn(input, bufferSize, nChannels);
            }
        }
    }
    

private:
    int                     width,
                            height;

    vector<AudioPad>        buttons;

    int                     n_buttons,
                            button_width,
                            button_height;
    
    bool                    is_recording;
    ButtonPad               button_record,
                            button_stop;
};


int main(){
    ofSetupOpenGL(1024, 768, OF_WINDOW);
    ofRunApp(new App());
}

```

<a name="rms"></a>
### RMS

```cpp
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
        ofEnableAlphaBlending();
        ofSetColor(0, 0, 0, 10);
        ofDrawRectangle(0, 0, width, height);
        
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
```

<a name="audio-segmentation"></a>
### Audio Segmentation

```cpp
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
                if (i == current_grain_i && j >= current_frame_i * buffer_size && j <= (current_frame_i + 1) * buffer_size) {
                    this_grain = true;
                    ofSetColor(255, 100, 100, 30);
                    ofDrawLine(samp_i * width_step, 0, (samp_i + 1) * width_step, height);
                }
                ofSetColor(255);
                ofLine(samp_i * width_step, -grains[i][j - 1] * height / 2 + height / 2,
                       (samp_i + 1) * width_step, -grains[i][j] * height / 2 + height / 2);
                samp_i++;
            }
            if(this_grain) {
                ofSetColor(100, 255, 100, 30);
                ofSetRectMode(OF_RECTMODE_CORNER);
                ofDrawRectangle(prev_grain_start * width_step, 0,
                                (samp_i - prev_grain_start) * width_step, height);
            }
        }
    }
    
    void keyPressed(int k) {
        if(k == ' '){
            b_recording = !b_recording;
        }
    }
    
    void audioOut(float * output, int buffer_size, int n_channels)
    {
        if (b_recording) {
            return;
        }
        
        if(grains.size()) {
            
            for (int i = 0; i < buffer_size; i++) {
                output[i] = grains[current_grain_i][i + current_frame_i * buffer_size];
            }
            
            current_frame_i += 1;
            
            if(current_frame_i * buffer_size > min_samples_per_grain ||
               current_frame_i >= grains[current_grain_i].size() / buffer_size) {
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
    
    void audioIn(float * input, int buffer_size, int n_channels)
    {
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


int main(){
    ofSetupOpenGL(1024, 768, OF_WINDOW);
    ofRunApp(new App());
}
```

<a name="basic-gui"></a>
### Basic GUI

`ofParameter<...>`
`ofParameterGroup`
`ofxGui`

<a name="multiple-windows"></a>
### Multiple Windows

Sometimes you might want a second window.  This could be useful for instance if you wanted a GUI in one window and some audiovisuals going in another window.

```cpp
#include "ofMain.h"
#include "ofApp.h"
#include "ofAppGLFWWindow.h"

//========================================================================
int main( ){
    ofGLFWWindowSettings settings;
    settings.width = 600;
    settings.height = 600;
    settings.setPosition(ofVec2f(300,0));
    settings.resizable = true;
    shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);

    settings.width = 300;
    settings.height = 300;
    settings.setPosition(ofVec2f(0,0));
    settings.resizable = false;
    // uncomment next line to share main's OpenGL resources with gui
    //settings.shareContextWith = mainWindow;   
    shared_ptr<ofAppBaseWindow> guiWindow = ofCreateWindow(settings);
    guiWindow->setVerticalSync(false);

    shared_ptr<ofApp> mainApp(new ofApp);
    mainApp->setupGui();
        // everytime the gui window needs to draw,
        // we're going to tell the instance of ofApp, mainApp
        // to call it's "drawGui" function
    ofAddListener(guiWindow->events().draw, mainApp.get(), &ofApp::drawGui);

    ofRunApp(mainWindow, mainApp);
    ofRunMainLoop();

}
```

<a name="lab"></a>
### Lab

<a name="homework"></a>
### Homework

Write proposal for project.  Include basic interaction, design, and related ideas.  Start on proof on concept using sketches, storyboards, and anything else that might help you understand the basic logic of the program you will write.  Your program should be capable of audio playback using some sort of interaction, whether by algorithmic/generative, keyboard, mouse, or other means.
