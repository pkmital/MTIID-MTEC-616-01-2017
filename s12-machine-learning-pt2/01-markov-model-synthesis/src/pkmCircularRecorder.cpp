/*
 *  pkmCircularRecorder.cpp
 *  pkmSourceSeparation
 *
 *  Created by Mr. Magoo on 4/18/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "pkmCircularRecorder.h"

pkmCircularRecorder::pkmCircularRecorder()
{
    
}

void pkmCircularRecorder::setup(int fixed_size, int frame_size)
{
	size = fixed_size;
	sizeOver2 = fixed_size/2;
	data = (float *)malloc((size+1) * sizeof(float));
	memset(data, 0, (size+1)*sizeof(float));
	data_current = data;
	data_end = data + size;		
	frameSize = frame_size;
	currentIdx = 0;
	bRecorded = false;
}

pkmCircularRecorder::~pkmCircularRecorder()
{
	free(data);
	size = 0;
	data_current = data_end = data = NULL;
	
}

void pkmCircularRecorder::clear()
{
    if(size > 0)
        memset(data, 0, (size+1) * sizeof(float));
	currentIdx = 0;
	bRecorded = false;
}

// get last half of the audio, 
// (buf should be size / 2)
void pkmCircularRecorder::getLastHalf(float *buf)
{
	//printf("3-cidx: %d, sizeOver2: %d, size: %d\n", currentIdx, sizeOver2, size);
	if ((int)(currentIdx - sizeOver2) >= 0) {
		cblas_scopy(sizeOver2, data + (currentIdx - sizeOver2), 1, buf, 1);
	}
	else {
		// first part touching end of buffer
		cblas_scopy(sizeOver2 - currentIdx, data + (size - (sizeOver2 - currentIdx)), 1, buf, 1);
		// last part touching beginning of buffer
		cblas_scopy(currentIdx, data, 1, buf + sizeOver2 - currentIdx, 1);
	}
}

// get first half of the audio, 
// (buf should be size / 2)
void pkmCircularRecorder::getFirstHalf(float *buf)
{
	//printf("2-cidx: %d, sizeOver2: %d, size: %d\n", currentIdx, sizeOver2, size);
	if (currentIdx + sizeOver2 < size) {	
		cblas_scopy(sizeOver2, data + currentIdx, 1, buf, 1);
	}
	else {		
		// first part touching end of buffer
		cblas_scopy(size-currentIdx, data + currentIdx, 1, buf, 1);
		// last part in the beginning of the buffer
		cblas_scopy(sizeOver2 - (size - currentIdx), data, 1, buf + (size - currentIdx), 1);
	}
}

float pkmCircularRecorder::backValue()
{
	int offset = currentIdx - 1;
	if (offset < 0) {
		offset += size;
	}
	return *(data + offset);
}

float* pkmCircularRecorder::back()
{
	int offset = currentIdx - 1;
	if (offset < 0) {
		offset += size;
	}
	return (data + offset);
}

float pkmCircularRecorder::frontValue()
{
	int offset = currentIdx;
	
	return *(data + offset);
}

float* pkmCircularRecorder::front()
{
	int offset = currentIdx;
	
	return (data + offset);
}

void pkmCircularRecorder::setFrameSize(int frame_size)
{
	frameSize = frame_size;
}

bool pkmCircularRecorder::isRecorded()
{
	return bRecorded;
}
