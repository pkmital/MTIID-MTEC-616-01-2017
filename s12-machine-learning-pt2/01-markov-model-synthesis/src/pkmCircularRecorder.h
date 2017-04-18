/*
 *  pkmCircularRecorder.h
 *  pkmSourceSeparation
 *
 *  Created by Mr. Magoo on 4/18/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


#pragma once

#include <Accelerate/Accelerate.h>
#include <stdlib.h>
#include <string.h>

class pkmCircularRecorder
{
public:
    pkmCircularRecorder();
    ~pkmCircularRecorder();
    
    void setup(int fixed_size = 44100, int frame_size = 512);
    
    inline float * operator[](int idx)
    {
        idx = (idx + currentIdx) % size;
        return data + idx;
    }
	
	void clear();
	
	// add audio
	inline void insertFrame(float *buf)
	{	
		 for (int i = 0; i < frameSize; i++) {
             data[currentIdx] = *buf++;
             currentIdx = (currentIdx + 1) % size;
		 }
		
        /*
		// this seems to be the same speed as the loop + modulo
		if (currentIdx + frameSize >= size) {
			// first part touching the end of the buffer
			cblas_scopy(size-currentIdx, buf, 1, data + currentIdx, 1);
			// second part at the start of the buffer
			cblas_scopy(currentIdx + frameSize - size, buf + (size - currentIdx), 1, data, 1);
		}
		else {
			cblas_scopy(frameSize, buf, 1, data + currentIdx, 1);
		}
		currentIdx = (currentIdx + frameSize) % size;
		*/
		
		if (currentIdx == 0) {
			bRecorded = true;
		}
	}
	
	inline int getLastFrameOffset()
	{
		int offset = (currentIdx - frameSize);
		if (offset < 0) {
			offset += size;
		}
		return offset;
	}
	
	inline float * getBufferPointer()
	{
		return data;
	}
	
	inline void copyAlignedData(float *buf)
	{
		if (currentIdx == 0) {
			cblas_scopy(size, data, 1, buf, 1);
		}
		else if(currentIdx < (size-1)) {
			// first part touching end of buffer
			cblas_scopy(size-currentIdx, data+currentIdx, 1, buf, 1);
			// second part in the beginning of the buffer
			cblas_scopy(currentIdx, data, 1, buf+(size-currentIdx), 1);
		}
        else if(currentIdx == size-1){
			// second part in the beginning of the buffer
			cblas_scopy(currentIdx, data, 1, buf+(size-currentIdx), 1);
        }
		
	}
    
	
	// get last half of the audio, 
	// (buf should be size / 2)
	void getLastHalf(float *buf);
	
	// get first half of the audio, 
	// (buf should be size / 2)
	void getFirstHalf(float *buf);
	
	float backValue();
	float* back();
	float frontValue();
	float* front();
	void setFrameSize(int frame_size);
	bool isRecorded();
	
	float					*data_current, *data_end;
	float					*data;
	int						size, sizeOver2, frameSize;
	int						currentIdx;
	bool					bRecorded;
	
};
