/*
 Markov Chain Concatenative Synthesis
 
 1. recording:
    - load a corpus of sound
    - perform onset detection
    - store as new audio segment
    - window segment
 2. analysis:
    - find mean MFCCs of each segment
    - "cluster" audio features
    - describe each audio segment by its cluster, e.g. segment 120 belongs to cluster 3
    - build transition matrix describing how often there is a transition between each possible cluster
    - convert transition matrix to probabilities by dividing by its sum
 3. synthesis:
    - sample from the transition matrix probabilities to play back an audio segment
 
 */

#include "ofMain.h"
#include "pkmFFT.h"
#include "pkmSTFT.h"
#include "pkmCircularRecorder.h"
#include "pkmAudioFeatures.h"
#include "pkmMatrix.h"
#include "ofxLearn.h"

////////////////////////////////////////////////////////////////////////////
class AudioSegmenter {
public:
    AudioSegmenter() {
    }
    
    void setup(int sample_rate = 44100,
               int frame_size = 1024,
               int min_samples_per_segment = 11025)
    {
        this->sample_rate = sample_rate;
        this->frame_size = frame_size;
        this->min_samples_per_segment = min_samples_per_segment;
        rms_values = pkmMatrix(44100 / frame_size, 1);
    }
    
    bool segment(float *input, int buffer_size) {
        bool segmented = false;
        
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
        float rms = sqrt(total);
        
            // calculate the basic statistics of the rms values
        float average_rms, std_rms;
        rms_values.getMeanAndStdDev(average_rms, std_rms);
        
            // now we see if the current value is outside the mean + variance
            // basic statistics tells us a normally distributed function
            // has a mean and a deviation where 97% of the data is explained by
            // 3 standard deviations.  we use this principle here in detecting
            // the the current rms reading is outside this probability
        float min_rms = 0.01;
        if (rms > (average_rms + 2.0 * std_rms) &&
            rms > min_rms &&
            current_segment.size() > min_samples_per_segment)
        {
            stored_segments.push_back(current_segment);
            current_segment = pkmMatrix();
            segmented = true;
        }
        
            // add current frame of audio to current segment
        current_segment.push_back(input, buffer_size);
        
            // add the current rms value
        rms_values.insertRowCircularly(&rms);
        
        return segmented;
    }
    
    pkmMatrix getLastSegment() {
        return stored_segments.back();
    }
    
private:
    int                     sample_rate, frame_size, min_samples_per_segment;
    pkmMatrix               rms_values;
    pkmMatrix               current_segment;
    vector<pkmMatrix>       stored_segments;
};


////////////////////////////////////////////////////////////////////////////
class Recording {
public:
    Recording(pkmMatrix buf, pkmMatrix feats) {
        buffer = buf;
        features = feats;
    }
    pkmMatrix buffer, features;
};


////////////////////////////////////////////////////////////////////////////
class Corpus {
public:
    Corpus() {
    }
    
    void setup(int sample_rate, int frame_size) {
        analyzer.setup(sample_rate, frame_size);
    }
    
    void addRecording(pkmMatrix recording) {
        pkmMatrix features;
        for (int row_i = 0; row_i < recording.rows; row_i++) {
            pkmMatrix this_features(1, 13);
            analyzer.computeLFCCF(recording.row(row_i), this_features.data, 13);
            features.push_back(this_features);
        }
        pkmMatrix combined_feature = features.rowRange(0, 1);
        combined_feature.push_back(features.mean());
        Recording r(recording, combined_feature);
        corpora.push_back(r);
    }
    
    int size() {
        return corpora.size();
    }
    
private:
    pkmAudioFeatures analyzer;

protected:
    vector<Recording> corpora;
};


////////////////////////////////////////////////////////////////////////////
class MarkovCorpus : public Corpus {
public:
    MarkovCorpus() {
        current_cluster = 0;
        previous_cluster = 0;
        setNumClusters(10);
    }
    
    void setNumClusters(int n) {
        n_clusters = n;
    }
    
    void cluster() {
            // use the ofxlearn kmeans object
        ofxLearnKMeans kmeans;
        kmeans.setNumClusters(n_clusters);
        
            // gather all features in the correct format (double precision)
        for(int i = 0; i < corpora.size(); i++) {
            vector<double> features;
            features.resize(corpora[i].features.size());
            corpora[i].features.copyToDouble(&(features[0]));
            
                // add each feature to the kmeans class
            kmeans.addSample(features);
        }
        
            // now train with all the data we just added
        kmeans.train();
        
            // get the cluster centers (which cluster does each segment belong to)
        clusters = kmeans.getClusters();
        
            // we'll create a "transition matrix" or "transition table" which says
            // from any given "cluster", how likely are we to go to another "cluster"
        transition_table = pkmMatrix(n_clusters, n_clusters, 0.0f);
        
            // we'll also store all the recordings that belong to any given cluster
        lut.resize(0); lut.resize(n_clusters);
        
            // we'll loop through all our recordings, see which cluster they belong to
            // and create our transition table based on which cluster we end up
            // transitioning to.
        int prev_cluster = -1;
        for(int i = 0; i < corpora.size(); i++) {
            cout << clusters[i] << endl;
            
                // keep a look up table which says which recordings (i) belong to
                // the current cluster (clusters[i]).
            lut[clusters[i]].push_back(i);
            
            if(prev_cluster != -1) {
                    // each time we observe a transition from the previous cluster
                    // to the current cluster, we increment that position in the matrix
                    // by 1.
                transition_table.row(prev_cluster)[clusters[i]] += 1;
            }
            prev_cluster = clusters[i];
        }
        
            // we will normalize by rows, so each row can be treated as probabilities
        transition_table.divideEachVecBySum(true);
        
            // this will just print to the console so we can see what it looks like
        transition_table.print();
    }
    
    int transition() {
        pkmMatrix row = transition_table.rowRange(previous_cluster, previous_cluster + 1);
        float r = (rand() % 100) / 100.0;
        float cumsum = 0;
        for(int c = 0; c < row.cols; c++) {
            cumsum += row[c];
            if(r < cumsum) {
                previous_cluster = c;
                return c;
            }
        }
    }
    
    pkmMatrix sample(int cluster) {
        int rand_idx = rand() % lut[cluster].size();
        return corpora[lut[cluster][rand_idx]].buffer;
    }
    
private:
    int previous_cluster, current_cluster;
    int n_clusters;
    vector<int> clusters;
    vector<vector<int>> lut;
    pkmMatrix transition_table;
};


////////////////////////////////////////////////////////////////////////////
class ofApp : public ofBaseApp {
public:
    void setup() {
        is_synthesizing = false;
        is_recording = false;
        
        sample_rate = 44100;
        frame_size = 512;

        width = 500;
        height = 500;
        ofSetWindowShape(width, height);
        
        corpus.setup(sample_rate, frame_size);
        segmenter.setup(sample_rate, frame_size);
        ofSoundStreamSetup(1, 1, sample_rate, frame_size, 3);
    }
    
    void update() {
    }
    
    void draw() {
        ofDrawBitmapString((is_recording ? "recording: on" : "recording: off"), 20, 20);
        ofDrawBitmapString("size: " + ofToString(corpus.size()), 20, 60);
    }
    
    void audioIn(float *buf, int size, int ch) {
        if (is_recording) {
            if (segmenter.segment(buf, size)) {
                pkmMatrix segment = segmenter.getLastSegment();
                corpus.addRecording(segment);
            }
        }
    }

    void audioOut(float *buf, int size, int ch) {
        if(is_synthesizing) {
            if(current_frame < segment.rows) {
                for (int i = 0; i < size; i++) {
                    buf[i] = segment.row(current_frame)[i];
                }
                current_frame++;
            }
            else {
                corpus.transition();
                segment = corpus.sample();
                current_frame = 0;
                for (int i = 0; i < size; i++) {
                    buf[i] = segment.row(current_frame)[i];
                }
                current_frame++;
            }
        }
    }
    
    void keyPressed(int k) {
        if(k == 'r') {
            is_recording = !is_recording;
            if(is_recording)
                is_synthesizing = false;
        }
        else if(k == 'm') {
            is_synthesizing = !is_synthesizing;
            if(is_synthesizing)
                is_recording = false;
        }
        else if(k == 'c') {
            corpus.cluster();
        }
        else if(k == ' ') {
            current_cluster = corpus.transition();
            segment = corpus.sample(current_cluster);
            is_synthesizing = true;
            is_recording = false;
        }
    }
    
private:
    
    int sample_rate, frame_size;
    
    AudioSegmenter segmenter;
    MarkovCorpus corpus;
    pkmMatrix segment;
    int current_cluster, current_frame;
    
    int width, height;
    
    bool is_synthesizing;
    bool is_recording;
};


//========================================================================
int main( ){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());

}
