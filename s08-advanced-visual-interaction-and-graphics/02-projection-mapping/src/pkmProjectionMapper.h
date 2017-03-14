/*
 *  pkmProjectionMapper.h

 *  Created by Parag K. Mital - http://pkmital.com 
 *  Contact: parag@pkmital.com
 *
 *  Copyright 2011 Parag K. Mital. All rights reserved.
 * 
 *	Permission is hereby granted, free of charge, to any person
 *	obtaining a copy of this software and associated documentation
 *	files (the "Software"), to deal in the Software without
 *	restriction, including without limitation the rights to use,
 *	copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the
 *	Software is furnished to do so, subject to the following
 *	conditions:
 *	
 *	The above copyright notice and this permission notice shall be
 *	included in all copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,	
 *	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *	OTHER DEALINGS IN THE SOFTWARE.
 
 */

#pragma once

#include "ofMain.h"

class pkmProjectionMapper
{
	public:
	
	pkmProjectionMapper()
	{
		initialize(ofGetWidth(), ofGetHeight());
	}
	
	void initialize(int w, int h, int start_x=0, int start_y=0)
	{
		src[0].x = 0;
        src[0].y = 0;
        src[1].x = w;
        src[1].y = 0;
        src[2].x = w;
        src[2].y = h;
        src[3].x = 0;
        src[3].y = h;
		
		dst[0].x = start_x;
        dst[0].y = start_y;
        dst[1].x = w + start_x;
        dst[1].y = start_y;
        dst[2].x = w + start_x;
        dst[2].y = h + start_y;
        dst[3].x = start_x;
        dst[3].y = h + start_y;
		
		p_dst[0] = ofPoint(0,0);
		p_dst[1] = ofPoint(0,0);
		p_dst[2] = ofPoint(0,0);
		p_dst[3] = ofPoint(0,0);
		
		radius = 10;
		
		bNeedsUpdate = true;
		
		bSelected[0] = bSelected[1] = bSelected[2] = bSelected[3] = false;
		bSelectedCenter = false;
	}

	void update()
	{
		// find transformation matrix
		if (p_dst[0] != dst[0] || p_dst[1] != dst[1] || p_dst[2] != dst[2] || p_dst[3] != dst[3]) {
			findHomography(src, dst, homography);
			center = findIntersectionPoint(dst[0], dst[2], dst[1], dst[3]);
			
			p_dst[0] = dst[0];
			p_dst[1] = dst[1];
			p_dst[2] = dst[2];
			p_dst[3] = dst[3];
		}
	}
	
	void drawPoint(ofPoint p, float radius, bool selected)
	{
		if (selected) {
			ofSetColor(255, 150, 150);
		}
		else {
			ofSetColor(200, 100, 100);
		}
		ofCircle(p.x, p.y, radius);
	}
	
	void drawBoundingBox()
	{
        ofPushStyle();
		ofSetLineWidth(2.5f);
		ofSetRectMode(OF_RECTMODE_CORNER);
		ofNoFill();
		ofSetColor(255, 255, 255);
		ofBeginShape();
			ofVertex(dst[0]);
			ofVertex(dst[1]);
			ofVertex(dst[2]);
			ofVertex(dst[3]);
		ofEndShape(true);
        ofPopStyle();
        
        ofPushStyle();
		ofSetColor(200, 100, 100);
		ofLine(dst[0].x, dst[0].y, dst[2].x, dst[2].y);
		ofLine(dst[1].x, dst[1].y, dst[3].x, dst[3].y);
		
		ofSetRectMode(OF_RECTMODE_CENTER);
			drawPoint(dst[0], radius*.66, bSelected[0]);
			drawPoint(dst[1], radius*.66, bSelected[1]);
			drawPoint(dst[2], radius*.66, bSelected[2]);
			drawPoint(dst[3], radius*.66, bSelected[3]);
			drawPoint(center, radius*.66, bSelectedCenter);
        ofSetRectMode(OF_RECTMODE_CORNER);
        ofPopStyle();
        
        if (bSelected[0] || bSelected[1] || bSelected[2] || bSelected[3] || bSelectedCenter) {
            
            ofPushStyle();
            ofEnableAlphaBlending();
            ofFill();
            ofSetColor(200, 100, 100, 100);
            ofBeginShape();
            ofVertex(dst[0]);
            ofVertex(dst[1]);
            ofVertex(dst[2]);
            ofVertex(dst[3]);
            ofEndShape(true);
            ofPopStyle();
        }
		
	}
	
	void startMapping()
	{
		glPushMatrix();
		glMultMatrixf(homography);
	}
	
	void stopMapping()
	{
		glPopMatrix();
	}
	
	void mousePressed(int x, int y)
	{
		for (int i = 0; i < 4; i++) 
		{
			if (ofInRange(x, dst[i].x-radius, dst[i].x+radius) &&
				ofInRange(y, dst[i].y-radius, dst[i].y+radius))
			{
				bSelected[i] = true;
				start_x = x;
				start_y = y;
				return;
			}
		}
		
		if (ofInRange(x, center.x - radius, center.x + radius) &&
			ofInRange(y, center.y - radius, center.y + radius)) {
			bSelectedCenter = true;
			start_x = x;
			start_y = y;
		}
		
	}
	
	void mouseDragged(int x, int y)
	{
		for (int i = 0; i < 4; i++) 
		{
			if (bSelected[i])
			{
				dst[i].x = dst[i].x + (x - start_x);
				dst[i].y = dst[i].y + (y - start_y);
				start_x = x;
				start_y = y;
				return;
			}
		}
		
		if (bSelectedCenter) {
			for (int i = 0; i < 4; i++) {
				dst[i].x = dst[i].x + (x - start_x);
				dst[i].y = dst[i].y + (y - start_y);
			}
			start_x = x;
			start_y = y;
		}
		
	}
	
	void mouseReleased(int x, int y)
	{
		for (int i = 0; i < 4; i++) {
			bSelected[i] = false;
		}
		bSelectedCenter = false;
		
	}
	
	
	void save(string filename)
	{
		FILE *fp = fopen(filename.c_str(), "w");
		if (fp != NULL) {
			fprintf(fp, "%f,%f,%f,%f,%f,%f,%f,%f,\n", 
					src[0].x, src[0].y, src[1].x, src[1].y, src[2].x, src[2].y, src[3].x, src[3].y);
			fprintf(fp, "%f,%f,%f,%f,%f,%f,%f,%f", 
					dst[0].x, dst[0].y, dst[1].x, dst[1].y, dst[2].x, dst[2].y, dst[3].x, dst[3].y);
			fclose(fp);
		}
	}
	
	void load(string filename)
	{
		FILE *fp = fopen(filename.c_str(), "r");
		if (fp != NULL) {
			fscanf(fp, "%f,%f,%f,%f,%f,%f,%f,%f,\n", 
				   &(src[0].x), &(src[0].y), &(src[1].x), &(src[1].y), &(src[2].x), &(src[2].y), &(src[3].x), &(src[3].y));
			fscanf(fp, "%f,%f,%f,%f,%f,%f,%f,%f", 
				   &(dst[0].x), &(dst[0].y), &(dst[1].x), &(dst[1].y), &(dst[2].x), &(dst[2].y), &(dst[3].x), &(dst[3].y));
			fclose(fp);
			
			bNeedsUpdate = true;
			bSelected[0] = bSelected[1] = bSelected[2] = bSelected[3] = false;
			bSelectedCenter = false;
		}	
	}

	ofPoint					src[4],			
							dst[4],
							p_dst[4],
							center;
	float					homography[16];
	bool					bSelected[4];
	bool					bSelectedCenter;
	bool					bNeedsUpdate;
	int						start_x, start_y;
	float					radius;
	
protected:
	// taken from lpmt
	void gaussian_elimination(float *input, int n)
	{
		// ported to c from pseudocode in
		// http://en.wikipedia.org/wiki/Gaussian_elimination
		
		float * A = input;
		int i = 0;
		int j = 0;
		int m = n-1;
		while (i < m && j < n)
		{
			// Find pivot in column j, starting in row i:
			int maxi = i;
			for(int k = i+1; k<m; k++)
			{
				if(fabs(A[k*n+j]) > fabs(A[maxi*n+j]))
				{
					maxi = k;
				}
			}
			if (A[maxi*n+j] != 0)
			{
				//swap rows i and maxi, but do not change the value of i
				if(i!=maxi)
					for(int k=0; k<n; k++)
					{
						float aux = A[i*n+k];
						A[i*n+k]=A[maxi*n+k];
						A[maxi*n+k]=aux;
					}
				//Now A[i,j] will contain the old value of A[maxi,j].
				//divide each entry in row i by A[i,j]
				float A_ij=A[i*n+j];
				for(int k=0; k<n; k++)
				{
					A[i*n+k]/=A_ij;
				}
				//Now A[i,j] will have the value 1.
				for(int u = i+1; u< m; u++)
				{
					//subtract A[u,j] * row i from row u
					float A_uj = A[u*n+j];
					for(int k=0; k<n; k++)
					{
						A[u*n+k]-=A_uj*A[i*n+k];
					}
					//Now A[u,j] will be 0, since A[u,j] - A[i,j] * A[u,j] = A[u,j] - 1 * A[u,j] = 0.
				}
				
				i++;
			}
			j++;
		}
		
		//back substitution
		for(int i=m-2; i>=0; i--)
		{
			for(int j=i+1; j<n-1; j++)
			{
				A[i*n+m]-=A[i*n+j]*A[j*n+m];
				//A[i*n+j]=0;
			}
		}
	}
	
	ofPoint findIntersectionPoint(ofPoint p1, ofPoint p2, ofPoint p3, ofPoint p4)
	{
        
		ofPoint intersection;
		
		float ua = ((p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x)) /
		((p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y));
		float ub = ((p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x)) /
		((p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y));
		intersection.x = p1.x + ua * (p2.x - p1.x);
		intersection.y = p1.y + ub * (p2.y - p1.y);
		
		return intersection;
	}
	
	// taken from lpmt
	void findHomography(ofPoint src[4], ofPoint dst[4], float homography[16])
	{
		
		// create the equation system to be solved
		//
		// from: Multiple View Geometry in Computer Vision 2ed
		//       Hartley R. and Zisserman A.
		//
		// x' = xH
		// where H is the homography: a 3 by 3 matrix
		// that transformed to inhomogeneous coordinates for each point
		// gives the following equations for each point:
		//
		// x' * (h31*x + h32*y + h33) = h11*x + h12*y + h13
		// y' * (h31*x + h32*y + h33) = h21*x + h22*y + h23
		//
		// as the homography is scale independent we can let h33 be 1 (indeed any of the terms)
		// so for 4 points we have 8 equations for 8 terms to solve: h11 - h32
		// after ordering the terms it gives the following matrix
		// that can be solved with gaussian elimination:
		
		float P[8][9]=
		{
			{-src[0].x, -src[0].y, -1,   0,   0,  0, src[0].x*dst[0].x, src[0].y*dst[0].x, -dst[0].x }, // h11
			{  0,   0,  0, -src[0].x, -src[0].y, -1, src[0].x*dst[0].y, src[0].y*dst[0].y, -dst[0].y }, // h12
			
			{-src[1].x, -src[1].y, -1,   0,   0,  0, src[1].x*dst[1].x, src[1].y*dst[1].x, -dst[1].x }, // h13
			{  0,   0,  0, -src[1].x, -src[1].y, -1, src[1].x*dst[1].y, src[1].y*dst[1].y, -dst[1].y }, // h21
			
			{-src[2].x, -src[2].y, -1,   0,   0,  0, src[2].x*dst[2].x, src[2].y*dst[2].x, -dst[2].x }, // h22
			{  0,   0,  0, -src[2].x, -src[2].y, -1, src[2].x*dst[2].y, src[2].y*dst[2].y, -dst[2].y }, // h23
			
			{-src[3].x, -src[3].y, -1,   0,   0,  0, src[3].x*dst[3].x, src[3].y*dst[3].x, -dst[3].x }, // h31
			{  0,   0,  0, -src[3].x, -src[3].y, -1, src[3].x*dst[3].y, src[3].y*dst[3].y, -dst[3].y }, // h32
		};
		
		gaussian_elimination(&P[0][0],9);
		
		// gaussian elimination gives the results of the equation system
		// in the last column of the original matrix.
		// opengl needs the transposed 4x4 matrix:
		float aux_H[]= { P[0][8], P[3][8], 0, P[6][8],  // h11  h21 0 h31
			P[1][8], P[4][8], 0, P[7][8],  // h12  h22 0 h32
			0      ,       0, 0, 0,        // 0    0   0 0
			P[2][8], P[5][8], 0, 1			// h13  h23 0 h33
		};      
		
		for(int i=0; i<16; i++) homography[i] = aux_H[i];
	}
};
