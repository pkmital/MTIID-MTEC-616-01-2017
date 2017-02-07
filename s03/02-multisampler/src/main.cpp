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
