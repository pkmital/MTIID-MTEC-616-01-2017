
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
