#pragma once

class pkmAudioWaveformDrawer {
    int                         x,y;
    int                         width, height;
    float                       resolution;
    int                         frameSize;
    float                       seconds;
    unsigned long               numBins;
    int                         binSize;
    vector<float>               binnedSamples;
    ofFbo                       waveformFbo;
    
public:
    void setup(int px = 0, int py = 0, int w = 1280, int h = 100, int fS = 1024, float r = 1.5) {
        
        x = px;
        y = py;
        width = w;
        height = h - gui_bar_height;
        frameSize = fS;
        buffer = (float *)malloc(sizeof(float) * fS);
        vDSP_vclr(buffer, 1, fS);
        resolution = r;

        waveformFbo.allocate(width, height, GL_RGBA, 8);
        waveformFbo.begin();
        ofDisableAlphaBlending();
        ofSetColor(255);
        ofBackground(0);
        ofDrawRectangle(0, 0, width, height);
        waveformFbo.end();
    }
    void update() {
        
            // how many of them to keep for drawing
        binSize = MAX(ratio * resolution, 1);
        
            // get the resampled audio file
        numBins = numSamplesToRead / (float)binSize;
        if(binnedSamples.size() != numBins)
            binnedSamples.resize(numBins);
        
            //cout << "Samples per bin: " << binSize << " Number of bins: : " << numBins << endl;
        
        float bin[binSize];
        float max;
        unsigned long bin_i = 0;
        
        if(!rereader.open(audioFilename))
        {
            cerr << "Could not read from file!" << endl;
            currentSampleStart = currentSampleEnd = 0;
            bNeedsUpdate = false;
            return;
        }
        
        for (unsigned long i = sampleStart; i < sampleEnd; i += binSize) {
            rereader.read(bin, i, binSize);
            vDSP_maxv(bin, 1, &max, binSize);
            binnedSamples[bin_i++] = max;
        }
    }
};
