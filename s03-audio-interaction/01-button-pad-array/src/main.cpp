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
    ButtonPad()
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

class App : public ofBaseApp{
public:
    void setup() {
        width = 500;
        height = 500;
        
            // do some initialization
        ofSetWindowShape(width, height);
        
        n_buttons = 4;
        button_width = width / n_buttons;
        button_height = height / n_buttons;
        
            // we allow our vector to have 4 ButtonPads, which we index from 0 - 3
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
    vector<ButtonPad>   buttons;
    int                 n_buttons,
    button_width,
    button_height;
};


int main(){
    ofSetupOpenGL(1024, 768, OF_WINDOW);
    ofRunApp(new App());
}
