/*
 *  pkmBlobTracker.h
 *  Created by Parag K. Mital - http://pkmital.com
 *  Contact: parag@pkmital.com
 *
 
 Copyright (C) 2011 Parag K. Mital
 
 MIT LICENSE
 
 */


#pragma once

#include "ofxOpenCv.h"

#include "pkmPixelBackgroundGMM.h"
#include "ofxCvBlobTracker.h"
#include "ofMain.h"

class pkmBlobTracker : public ofxCvBlobListener
{
public:
    pkmBlobTracker(){
        
    }
    
    void setup(int width = 320, int height = 240)
    {
        W = width;
        H = height;
        
        center_x = center_y = width = height = 0;
        orientation = 0;
        init_orientation = 0;
        bInit = false;
        
        initializeLKFlow(W, H);
        
        color_img.allocate(W,H);
        gray_img.allocate(W,H);
        warped_img.allocate(W,H);
        
            // Background model variables
        gray_bg.allocate(W,H);
        gray_diff.allocate(W,H);
        
        gmm=cvCreatePixelBackgroundGMM(W,H);
        gmm_timing = 200.0;
        gmm->fAlphaT = 1. / gmm_timing;
        gmm->fTb = 5*4;
        gmm->fSigma = 30;
        gmm->bShadowDetection = true;
        b_learn_background = true;
        threshold = 20;
        block_size = 9;
        
        low_threshold = 16;	frame_num = 0;
        
        b_learn_background = true;
        threshold = 15;
        option1 = true;	// learn a background once
        
        pixel_buffer.resize(W*H*3);
        
        tracker.setListener( this );
        listener = NULL;
        orientation = 0.0f;
        
        bBlob = false;
    }
    
    ~pkmBlobTracker()
    {
        cvReleaseImage(&cv_prev_gray_img);
        cvReleaseImage(&cv_gray_img);
            //cvReleaseImage(prev_gray_pyr);
            //cvReleaseImage(gray_pyr);
    }
    
    void setListener(ofxCvBlobListener* _listener)
    {
        listener = _listener;
            //		tracker.setListener(_listener);
    }
    
    void initializeLKFlow(int width, int height)
    {
        cv_prev_gray_img = cvCreateImage(cvSize(width, height), 8, 1);
        cv_gray_img = cvCreateImage(cvSize(width, height), 8, 1);
        
        cvZero(cv_prev_gray_img);
        cvZero(cv_gray_img);
        
            //prev_gray_pyr = cvCreateImage(cvSize(width, height), 8, 1);
            //gray_pyr = cvCreateImage(cvSize(width, height), 8, 1);
        
        grid_size = 4;													// Size of the grid for feature points A
        levels = 2;														// Number of levels in the pyramid
        winSize = cvSize(2,2);											// Size of the search window
        int type = CV_TERMCRIT_ITER|CV_TERMCRIT_EPS;
        eps = 0.01;
        iter = 10;
        crit = cvTermCriteria(type,iter,eps);
        internal_flags = 0;
    }
    
    void updateImage(const IplImage *img)
    {
        swap(cv_prev_gray_img, cv_gray_img);
        
        cvCopyImage(img, cv_gray_img);
    }
    
    void update(const ofPixels &pixels) {
        update(pixels.getData(), pixels.getWidth(), pixels.getHeight());
    }
    
    void update(const unsigned char *pixels, int w, int h)
    {
            //inImage = pixels;
        color_img.setFromPixels(pixels, w, h);
        gray_img = color_img;
        updateImage(gray_img.getCvImage());
        
        if (b_learn_background){
            cvUpdatePixelBackgroundGMM(gmm, color_img.getPixels().getData(), &(pixel_buffer[0]));
            gray_diff.setFromPixels(&(pixel_buffer[0]), w, h);
        }
        else {
            cvPixelBackgroundGMMSubtraction(gmm, color_img.getPixels().getData(), &(pixel_buffer[0]));
            gray_diff.setFromPixels(&(pixel_buffer[0]), w, h);
            
        }
        
        gray_bg = gray_diff;
        
            // subtraction and threshold
        gray_diff.blur( block_size );
        gray_diff.threshold( threshold );
        
            // blob tracking
        contours.findContours(gray_diff, 150, w*h/3, 10, false);
        tracker.trackBlobs( contours.blobs );
    }
    
    float getOrientation(ofxCvTrackedBlob &blob)
    {
        int n_pts = blob.pts.size();
        char *status = new char[n_pts];
        float *err = new float[n_pts];
        CvPoint2D32f *feat_a = new CvPoint2D32f[n_pts];
        CvPoint2D32f *feat_b = new CvPoint2D32f[n_pts];
        
        for(int i = 0; i < n_pts; i++)
        {
            feat_a[i].x = MAX(MIN(blob.pts[i].x, W - 3), 3);
            feat_a[i].y = MAX(MIN(blob.pts[i].y, H - 3), 3);
        }
        
        cvCalcOpticalFlowPyrLK(cv_prev_gray_img, cv_gray_img,
                               NULL, NULL,
                               &(feat_a[0]), &(feat_b[0]),
                               n_pts, cvSize(12,12), levels, status, err, crit, internal_flags);
        
        float ori = 0.0f;
        int pts = 0;
        for (int i = 0; i < n_pts; i++) {
            
            CvPoint2D32f vecA, vecB, vecAB;
            vecA.x = feat_a[i].x;
            vecA.y = feat_a[i].y;
            
            vecB.x = feat_b[i].x;
            vecB.y = feat_b[i].y;
            
                //atan2f((float)vecA.x, (float)vecA.y) - atan2f((float)vecB.x, (float)vecB.y);
                //float normA = sqrtf(vecA.x*vecA.x + vecA.y*vecA.y);
                //float normB = sqrtf(vecB.x*vecB.x + vecB.y*vecB.y);
                //acosf( (vecA.x*vecB.x + vecA.y*vecB.y) / (normA * normB) );
            float o =  fmod(atan2f((float)(vecB.y - vecA.y), (float)(vecB.x - vecA.x)), (float)(2.0f*PI));
            
            if(!isnan(o))
            {
                //printf("%f\n", (float)(o*180.0f/PI));
                ori += o;
                pts++;
            }
            
        }
        ori /= pts;
        if (bInit) {
            orientation = fmod((float)(0.6*orientation + 0.4*(orientation + (ori-orientation))), (float)(2.0f*PI));
        }
        else {
            orientation = ori;
            bInit = true;
        }
        center_x = blob.centroid.x;
        center_y = blob.centroid.y;
        width = blob.boundingRect.width;
        height = blob.boundingRect.height;
        
        delete [] status;
        delete [] err;
        delete [] feat_a;
        delete [] feat_b;
        
        return orientation;
    }

    
    void draw(int x, int y)
    {
        ofSetColor(255, 255, 255);
        
        color_img.draw(20,20);
        
        gray_diff.draw(20+W+20+W+20,20);
        
            // debug text output
        ofSetColor(0,255,131);
        char buf[256];
        if (b_learn_background) {
            sprintf(buf, "learning background...\n['+'] or ['-'] to change threshold: %d\n", threshold);
        }
        else {
            sprintf(buf, "['space'] to learn background\n['+'] or ['-'] to change threshold: %d\n", threshold);
        }
        ofDrawBitmapString( buf, 20,290 );
        
        gray_bg.draw(20+W+20, 20);
        ofSetColor(0,255,131);
        sprintf(buf, "['9'] or ['0'] to change gmm timing: %f\n['['] or [']'] to change block size: %d", gmm_timing, block_size);
        ofDrawBitmapString( buf, 20,316 );
        
        
            // blob drawing
        tracker.draw(20+W+20+W+20, 20);
        
        if (bBlob) {
            ofSetColor(200, 20, 20);
            ofNoFill();
            ofPushMatrix();
            ofDrawCircle(x + center_x, y + center_y, height/2.0f);
            ofTranslate(x+center_x, y+center_y, 0);
            ofRotate(orientation * 180.0f / PI, 0, 0, 1);
            ofDrawLine(0, 0, height/2.0f, 0);
            ofPopMatrix();
        }
    }
    
    void reInit()
    {
        bInit = false;
    }
    
        //--------------------------------------------------
    void blobOn( int x, int y, int id, int order ) {
        
        printf("blobOn() - id:%d\n", id);
        blob_locations[id] = tracked_blobs.size();
        tracked_blobs.push_back(tracker.getById(id));
        
        bBlob = true;
        
        if (listener != NULL) {
            listener->blobOn(x, y, id, order);
        }
    }
    
    void blobMoved( int x, int y, int id, int order) {
        printf("blobMoved() - id:%d\n", id);
        
            // full access to blob object ( get a reference)
        ofxCvTrackedBlob blob = tracker.getById( id );
        
        orientation = getOrientation(blob) * 180.0f / PI;
        tracked_blobs[blob_locations[id]].orientation =  orientation;
        
        printf("ori: %f\n", orientation);
        /*
         ofxOscMessage m;
         m.setAddress("/listener/position");
         blob_x = (x / (float)W - 0.5) * 5.0f;
         blob_y = (y / (float)H - 0.5) * 5.0f;
         //printf("%f, %f\n", blob_x, blob_y);
         m.addFloatArg(blob_x);
         m.addFloatArg(blob_y);
         m.addFloatArg(1.0f);
         sender.sendMessage(m);
         */
        bBlob = true;
        
        if (listener != NULL) {
            listener->blobMoved(x, y, id, order);
        }
    }
    
    void blobOff( int x, int y, int id, int order )
    {
        bBlob = false;
        int sub = blob_locations[id];
        blob_locations.erase(id);
        tracked_blobs.erase(tracked_blobs.begin() + sub);
        map<int,int>::iterator it = blob_locations.begin();
        while (it != blob_locations.end()) {
            if (it->second > sub) {
                assert(it->second > 0);
                it->second--;
            }
            it++;
        }
        printf("blobOff() - id:%d\n", id);
        
        
        if (listener != NULL) {
            listener->blobOff(x, y, id, order);
        }
    }
    
    void keyPressed(int key) {
        
        switch (key) {
            case ' ':
                b_learn_background = !b_learn_background;
                break;
            case '=': case '+':
                threshold ++;
                if (threshold > 255) threshold = 255;
                break;
            case '-':
                threshold --;
                if (threshold < 0) threshold = 0;
                break;
            case '9':
                gmm_timing -= 1.0;
                gmm_timing = MAX(5,gmm_timing);
                gmm->fAlphaT = 1. / gmm_timing;
                break;
            case '0':
                gmm_timing += 1.0;
                gmm_timing = MIN(320.0,gmm_timing);
                gmm->fAlphaT = 1. / gmm_timing;
                break;
            case '[':
                block_size = MAX(5,block_size-=2);
                break;
            case ']':
                block_size = MIN(53,block_size+=2);
                break;
        }
    }
    
    
    IplImage				*cv_prev_gray_img, *cv_gray_img, 
    *prev_gray_pyr, *gray_pyr;
    CvTermCriteria			crit;
    int						levels;
    int						grid_size;
    int						type;
    double					eps;
    int						iter;
    int						internal_flags;
    CvSize					winSize;
    
    float					orientation, init_orientation;
    float					center_x, center_y, width, height;
    bool					bInit;
    
        // ----------------------------------------------------------
    
    map<int,int>			blob_locations;
    vector<ofxCvTrackedBlob>	tracked_blobs;
    
    ofxCvBlobListener		*listener;
    
    ofxCvColorImage			color_img;
    ofxCvGrayscaleImage 	gray_img;
    ofxCvGrayscaleImage 	warped_img;
    
    
    ofxCvGrayscaleImage		gray_bg;
    ofxCvGrayscaleImage		gray_diff;
    CvPixelBackgroundGMM	*gmm;
    int						threshold;
    int						low_threshold, high_threshold, block_size;
    double					gmm_timing;
    unsigned long			frame_num;
    bool					option1;
    
    ofxCvContourFinder		contours;
    ofxCvBlobTracker			tracker;	
    float					blob_x, blob_y;
    
    bool					b_learn_background;
    vector<unsigned char>	pixel_buffer;
    
    int W,H;
    
    
    bool bBlob;
    
};
