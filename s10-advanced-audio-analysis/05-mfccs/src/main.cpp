#include "ofMain.h"
#include "ofAppGLFWWindow.h"
#include "pkmFFT.h"
#include "pkmSTFT.h"
#include "pkmCircularRecorder.h"
#include "pkmAudioFeatures.h"
#include "pkmMatrix.h"

class ofApp : public ofBaseApp {
public:
    void setup() {
        width = 500;
        height = 500;
        ofSetWindowShape(width, height);
        
        n_frames = 50;
        frame_size = 512;
        fft_size = 2048;
        buffer_size = frame_size * n_frames;
        
        stft = make_shared<pkmSTFT>(fft_size);
        magnitudes.resize(stft->getNumWindows(buffer_size), fft_size / 2);
        phases.resize(stft->getNumWindows(buffer_size), fft_size / 2);
        mels.resize(1, 60);
        
        features.setup(44100, fft_size);
        recorder.setup(buffer_size, frame_size);
        buffer.resize(1, buffer_size);
        
        ofSoundStreamSetup(0, 1, 44100, frame_size, 3);

    }
    
    void update() {
        
    }

    void drawMFCCs(ofEventArgs &args) {
        float width_step = width / (float)mels.size();
        float height_scale = height / 30.0;
        for (int i = 1; i < mels.size(); i ++) {
            ofDrawLine((i - 1) * width_step, height / 2 - mels[i - 1] * height_scale,
                       i * width_step, height / 2 - mels[i] * height_scale);
        }
    }
    
    void drawSignal(ofEventArgs &args) {
        float width_step = width / (float)buffer.size();
        float height_scale = height / 2;
        for (int i = 1; i < buffer.size(); i ++) {
//            ofSetColor(200 * (i / (float)buffer.size()), 100, 100);
            ofSetColor(200 * abs(buffer[i]) * 10.0, 100, 100);
            ofDrawLine((i - 1) * width_step, height / 2 - buffer[i - 1] * height_scale,
                       i * width_step, height / 2 - buffer[i] * height_scale);
        }
    }
    
    void drawSpectrum(ofEventArgs &args) {
        float width_step = width / (float)frame_size;
        float height_scale = height / 100.0;
        
        for (int window_i = 0; window_i < magnitudes.rows; window_i++)
        {
            float height_offset = height * (window_i / (float)magnitudes.rows);
//            ofSetColor(200 * (window_i / (float)magnitudes.rows), 100, 100);
            for (int i = 1; i < magnitudes.cols; i++)
            {
                ofSetColor(200 * magnitudes.row(window_i)[i] / 10.0, 100, 100, 200 * magnitudes.row(window_i)[i] / 10.0);
                ofDrawLine((i - 1) * width_step, height_offset - magnitudes.row(window_i)[i - 1] * height_scale,
                           i * width_step, height_offset - magnitudes.row(window_i)[i] * height_scale);
            }
        }
    }
    
    void audioIn(float *buf, int size, int ch) {
        recorder.insertFrame(buf);
        if (recorder.isRecorded()) {
            recorder.copyAlignedData(buffer.data);
            stft->STFT(buffer.data, n_frames * frame_size, magnitudes, phases);
            features.computeMelFeatures(buffer.data, mels.data, 60);
        }
    }
    
private:
    int width, height;
    int buffer_size, fft_size, frame_size, n_frames;
    
    shared_ptr<pkmSTFT> stft;
    
    pkmMatrix magnitudes, phases, mels, buffer;
    pkmCircularRecorder recorder;
    pkmAudioFeatures features;
};


//========================================================================
int main( ){
    ofGLFWWindowSettings settings;
    settings.width = 500;
    settings.height = 500;
    settings.setPosition(ofVec2f(0, 0));
    settings.resizable = true;
    shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);
    
    settings.width = 500;
    settings.height = 500;
    settings.setPosition(ofVec2f(100,0));
    settings.resizable = false;
    shared_ptr<ofAppBaseWindow> spectrumWindow = ofCreateWindow(settings);
    
    settings.width = 500;
    settings.height = 500;
    settings.setPosition(ofVec2f(200,0));
    settings.resizable = false;
    shared_ptr<ofAppBaseWindow> signalWindow = ofCreateWindow(settings);
    
    shared_ptr<ofApp> mainApp(new ofApp);
    ofAddListener(spectrumWindow->events().draw, mainApp.get(), &ofApp::drawSpectrum);
    ofAddListener(signalWindow->events().draw, mainApp.get(), &ofApp::drawSignal);
    ofAddListener(mainWindow->events().draw, mainApp.get(), &ofApp::drawMFCCs);
    
    ofRunApp(mainWindow, mainApp);
    ofRunMainLoop();
}
