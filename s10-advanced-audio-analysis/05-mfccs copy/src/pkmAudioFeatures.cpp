/*
 *  pkmAudioFeatures.cpp
 *
 *  Created by Parag K. Mital - http://pkmital.com 
 *  Contact: parag@pkmital.com
 *
 *  Copyright 2011 Parag K. Mital. All rights reserved.
 * 
 
 Copyright (C) 2011 Parag K. Mital
 
 The Software is and remains the property of Parag K Mital
 ("pkmital") The Licensee will ensure that the Copyright Notice set
 out above appears prominently wherever the Software is used.
 
 The Software is distributed under this Licence: 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 *****
 CONTRIB: 
 
 Code based on Michael Casey's code for LFCC computation in SoundSpotter
 Optimizations on his code make most computations vectorized, and make use of 
 my own personal fft library built on top of Apple's Accelerate framework.
 
 As SoundSpotter is GPL v2, this optimized version of Michael Casey's LFCC 
 computation is also GPL v2.
 http://mp7c.svn.sourceforge.net/viewvc/mp7c/trunk/soundspotter/
 
 
 */

#include "pkmAudioFeatures.h"

pkmAudioFeatures::pkmAudioFeatures() {
    allocated = false;
}

void pkmAudioFeatures::setup(int sample_rate, int fft_size)
{
	sampleRate = sample_rate;
	fftN = fft_size;
	
	setupCepstral();
    setupChromagram();
    
    allocated = true;
}

pkmAudioFeatures::~pkmAudioFeatures()
{
    if(allocated)
    {
        free(CQT);
        free(cqStart);
        free(cqStop);
        
        free(DCT);
        
        free(cqtVector);
        free(dctVector);
        
        free(fft);
        free(fft_magnitudes);
        free(fft_phases);
        
        free(previousLFCCs);
        free(previousDeltaLFCCs);
        
        free(previousChromas);
        free(previousDeltaChromas);
        
        free(foutput);
        
        free(note);
        free(chroma);
    }
}

void pkmAudioFeatures::setupCepstral()
{
	bpoN = 12;
	
	fft = new pkmFFT(fftN);
	fftOutN = fft->fftSizeOver2;
	fft_magnitudes = (float *)malloc(sizeof(float) * fftOutN);
	fft_phases = (float *)malloc(sizeof(float) * fftOutN);
	
	// low C minus quater tone
	loEdge = 55.0 * pow(2.0, 2.5/12.0);		//55.0
	hiEdge = 8000;						//8000.0
	
	// Constant-Q bandwidth
	fratio = pow(2.0, 1.0/(float)bpoN);				
	cqtN = (int) floor(log(hiEdge/loEdge)/log(fratio));
	
	if(cqtN<1)
		printf("warning: cqtN not positive definite\n");
	
	// The transformation matrix (mel filters)
	CQT = (float *)malloc(sizeof(float)*cqtN*fftOutN);			
	
	// Sparse matrix coding indices
	cqStart = (int *)malloc(sizeof(int)*cqtN);					
	cqStop = (int *)malloc(sizeof(int)*cqtN);					
	
	// Full spectrum DCT matrix
	dctN = cqtN; 
	DCT = (float *)malloc(sizeof(float)*cqtN*dctN);
	
	// Our transforms (mel bands)
	cqtVector = (float *)malloc(sizeof(float)*cqtN);	
	dctVector = (float *)malloc(sizeof(float)*dctN);	
	
	foutput = (float *)malloc(sizeof(float) * dctN);
    
    previousLFCCs = (float *)malloc(sizeof(float) * dctN);
    memset(previousLFCCs, 0, sizeof(float) * dctN);
	
    previousDeltaLFCCs = (float *)malloc(sizeof(float) * dctN);
    memset(previousDeltaLFCCs, 0, sizeof(float) * dctN);
    
    numberOfDCTBands = 12;
    
	// initialize maps
	createLogFreqMap();
	createDCT();
}

void pkmAudioFeatures::setupChromagram()
{
    
    float base = 130.81278265;
    note = (float *)malloc(sizeof(float) *12);
    chroma = (float *)malloc(sizeof(float) *12);
    previousChromas = (float *)malloc(sizeof(float) *12);
    memset(previousChromas, 0, sizeof(float) * 12);
    previousDeltaChromas = (float *)malloc(sizeof(float) *12);
    memset(previousDeltaChromas, 0, sizeof(float) * 12);
    
    for (int i = 0;i < 12;i++)
    {
        note[i] = base*pow(2,(((float) i)/12));
    }
}

void pkmAudioFeatures::createLogFreqMap()
{
	// loop variables
	int	i = 0,
	j = 0;
	
	float *fftfrqs = (float *)malloc(sizeof(float)*fftOutN);		// Actual number of real FFT coefficients
	float *logfrqs = (float *)malloc(sizeof(float)*cqtN);			// Number of constant-Q spectral bins
	float *logfbws = (float *)malloc(sizeof(float)*cqtN);			// Bandwidths of constant-Q bins
	float *mxnorm =  (float *)malloc(sizeof(float)*cqtN);			// CQ matrix normalization coefficients
	
	float N = (float)fftN;
	for(i = 0; i < fftOutN; i++)
		fftfrqs[i] = i * sampleRate / N;
	
	for(i = 0; i < cqtN; i++)
	{
		logfrqs[i] = loEdge * powf(2.0,(float)i/bpoN);
		logfbws[i] = MAX(logfrqs[i] * (fratio - 1.0), sampleRate / N);
	}
	
	float ovfctr = 0.5475;					// Norm constant so CQT'*CQT close to 1.0
	float tmp,tmp2;
	float *ptr;
	float cqEnvThresh = CQ_ENV_THRESH;		// Sparse matrix threshold (for efficient matrix multiplicaton)	
	
	assert(CQT);
	
	// Build the constant-Q transform (CQT)
	ptr = CQT;
	for(i = 0; i < cqtN; i++)
	{
		mxnorm[i] = 0.0;
		tmp2 = 1.0 / (ovfctr * logfbws[i]);
		for(j = 0; j < fftOutN; j++, ptr++)
		{
			tmp = (logfrqs[i] - fftfrqs[j])*tmp2;
			tmp = expf(-0.5 * tmp*tmp);
			*ptr = tmp;								// row major transform
			mxnorm[i] += tmp*tmp;
		}      
		mxnorm[i] = 2.0 * sqrtf(mxnorm[i]);
	}
	
	// Normalize transform matrix for identity inverse
	ptr = CQT;    
	for(i = 0; i < cqtN; i++)
	{
		cqStart[i] = 0;
		cqStop[i] = 0;
		tmp = 1.0/mxnorm[i];
		for(j = 0; j < fftOutN; j++, ptr++)
		{
			*ptr *= tmp;
			if( (!cqStart[i]) && 
			   (cqEnvThresh < *ptr) )
			{
				cqStart[i] = j;
			}
			else if( (!cqStop[i]) && 
					(cqStart[i]) && 
					(*ptr < cqEnvThresh) )
			{
				cqStop[i] = j;
			}
		}
	}
	
	// cleanup local dynamic memory
	free(fftfrqs);
	free(logfrqs);
	free(logfbws);
	free(mxnorm);
}

void pkmAudioFeatures::createDCT()
{
	
	int i,j;
	float nm = 1 / sqrtf( cqtN / 2.0 );
	
	assert( DCT );
	
	for( i = 0 ; i < dctN ; i++ )
		for ( j = 0 ; j < cqtN ; j++ )
			DCT[ i * cqtN + j ] = nm * cosf( i * (2.0 * j + 1) * M_PI / 2.0 / (float)cqtN );
	for ( j = 0 ; j < cqtN ; j++ )
		DCT[ j ] *= sqrtf(2.0) / 2.0;
	
}

void pkmAudioFeatures::computeMelFeatures(float *input, float *output, int numFilters, bool computeLogAmplitude, bool computeNormalization, bool computeDeltaFeatures)
{
    // should window input buffer before FFT
	fft->forward(0, input, fft_magnitudes, fft_phases);
	
	// sparse matrix product of CQT * FFT
    if (numFilters == -1) {
        vDSP_mmul(fft_magnitudes, 1, CQT, 1, output, 1, 1, cqtN, fftOutN);
    }
    else if (numFilters <= cqtN)
    {
        vDSP_mmul(fft_magnitudes, 1, CQT, 1, cqtVector, 1, 1, cqtN, fftOutN);
        cblas_scopy(numFilters, cqtVector, 1, output, 1);
    }
    else {
        std::cerr << "[ERROR]: pkmAudioFeatures: Incorrect number of filters" << std::endl;
    }
    
    if(computeLogAmplitude)
    {
        int a = 0;
        float *ptr1 = 0;
        
        // log amplitude
        a = cqtN;
        ptr1 = output;
        while( a-- ){
            float f = *ptr1;
            *ptr1++ = f == 0 ? 0 : log10f( f*f );
        }
    }

    if(computeNormalization)
    {
        // Normalize
        pkm::Mat outputMat(1, numFilters, output, false);
        outputMat.divideEachVecByMaxVecElement(true);
    }
    
    if(computeDeltaFeatures)
    {
        // lfcc'
        vDSP_vsub(output, 1,
                  previousLFCCs, 1,
                  output + numFilters, 1,
                  numFilters);
        
        // Normalize
        pkm::Mat outputMat2(1, numFilters, output + numFilters, false);
        outputMat2.divideEachVecByMaxVecElement(true);
        
        // store
        cblas_scopy(numFilters, output, 1, previousLFCCs, 1);
    }
}

void pkmAudioFeatures::computeLFCCF(float *input, float *output, int numLFCCS)
{
	// should window input buffer before FFT
	fft->forward(0, input, fft_magnitudes, fft_phases);
	
	// sparse matrix product of CQT * FFT
	int a = 0;
	float *ptr1 = 0;
	
	vDSP_mmul(fft_magnitudes, 1, CQT, 1, cqtVector, 1, 1, cqtN, fftOutN);
	
	// LFCC 
	a = cqtN;
	ptr1 = cqtVector;
	while( a-- ){
		float f = *ptr1;
		*ptr1++ = f == 0 ? 0 : log10f( f*f );
	}
    
	if (numLFCCS == -1) {
		vDSP_mmul(cqtVector, 1, DCT, 1, output, 1, 1, dctN, cqtN);
		
		float n = dctN;
		vDSP_vsdiv(output, 1, &n, output, 1, dctN);
		
	}
	else {
		vDSP_mmul(cqtVector, 1, DCT, 1, foutput, 1, 1, dctN, cqtN);
		cblas_scopy(numLFCCS, foutput, 1, output, 1);
		
		float n = dctN;
		vDSP_vsdiv(output, 1, &n, output, 1, numLFCCS);
		
	}
	
	//float n = (float) dctN;
	//vDSP_vsdiv(output, 1, &n, output, 1, dctN);
	
}

void pkmAudioFeatures::compute24DimAudioFeaturesF(float *inputSignal, float *outputFeatures)
{
    // write 12 features for Mel and another 12 for Delta Mel
	computeMelFeatures(inputSignal, outputFeatures, 12,  true, false, false);
    
    // write last 12 for Chromagram
    computeChromagramFromMagnitudesF(getMagnitudes(), outputFeatures + 12, false);
}


void pkmAudioFeatures::compute36DimAudioFeaturesF(float *inputSignal, float *outputFeatures)
{
    // write 12 features for Mel and another 12 for Delta Mel
    computeMelFeatures(inputSignal, outputFeatures, 12, true, false, true);
    
    // write last 12 for Chromagram
    computeChromagramFromMagnitudesF(getMagnitudes(), outputFeatures + 24, false);
}

void pkmAudioFeatures::compute48DimAudioFeaturesF(float *inputSignal, float *outputFeatures)
{
    // write 12 features for Mel and another 12 for Delta Mel
    computeMelFeatures(inputSignal, outputFeatures, 12, true, false, true);
    
    // write last 12 for Chromagram and 12 for delta Chromagram
    computeChromagramFromMagnitudesF(getMagnitudes(), outputFeatures + 24, true);
}

void pkmAudioFeatures::computeLFCCFromMagnitudesF(float *fft_magnitudes, float *outputFeatures, int numLFCCS)
{
	// sparse matrix product of CQT * FFT
	int a = 0;
	float *ptr1 = 0;
	
	
	vDSP_mmul(fft_magnitudes, 1, CQT, 1, cqtVector, 1, 1, cqtN, fftOutN);
	
	// LFCC 
	a = cqtN;
	ptr1 = cqtVector;
	while( a-- ){
		float f = *ptr1;
		*ptr1++ = log10f( f*f );
	}
	
	if (numLFCCS == -1) {
		vDSP_mmul(cqtVector, 1, DCT, 1, outputFeatures, 1, 1, dctN, cqtN);
		
		float n = dctN;
		vDSP_vsdiv(outputFeatures, 1, &n, outputFeatures, 1, dctN);
		
	}
	else {
		vDSP_mmul(cqtVector, 1, DCT, 1, foutput, 1, 1, dctN, cqtN);
		cblas_scopy(numLFCCS, foutput, 1, outputFeatures, 1);
		
		float n = dctN;
		vDSP_vsdiv(outputFeatures, 1, &n, outputFeatures, 1, numLFCCS);
		
	}
	
	
}

float * pkmAudioFeatures::getMagnitudes()
{
	return fft_magnitudes;
}

float * pkmAudioFeatures::getPhases()
{
	return fft_phases;
}

void pkmAudioFeatures::computeLFCCD(float *input, double* output, int numLFCCS)
{
	
	// should window input buffer before FFT
	fft->forward(0, input, fft_magnitudes, fft_phases);
	
	// sparse matrix product of CQT * FFT
	int a = 0;
	float *ptr1 = 0;
	
	vDSP_mmul(fft_magnitudes, 1, CQT, 1, cqtVector, 1, 1, cqtN, fftOutN);
	
	// LFCC 
	a = cqtN;
	ptr1 = cqtVector;
	while( a-- ){
		float f = *ptr1;
		*ptr1++ = log10f( f*f );
	}
	
	vDSP_mmul(cqtVector, 1, DCT, 1, foutput, 1, 1, dctN, cqtN);
	if (numLFCCS == -1) {
		vDSP_vspdp(foutput, 1, output, 1, dctN);
		
		double n = dctN;
		vDSP_vsdivD(output, 1, &n, output, 1, dctN);
		//double rms_feature = pkm::Mat::rms(output, cqtN);
		//output[1] = expf(rms_feature);
	}
	else {
		vDSP_vspdp(foutput, 1, output, 1, numLFCCS);
		
		double n = dctN;
		vDSP_vsdivD(output, 1, &n, output, 1, numLFCCS);
		//double rms_feature = pkm::Mat::rms(output, cqtN);
		//output[1] = expf(rms_feature);
	}
	
}

void pkmAudioFeatures::computeLFCCFromMagnitudesD(float *fft_magnitudes, double* output, int numLFCCS)
{
	
	// sparse matrix product of CQT * FFT
	int a = 0;
	float *ptr1 = 0;
	
	vDSP_mmul(fft_magnitudes, 1, CQT, 1, cqtVector, 1, 1, cqtN, fftOutN);
	
	// LFCC 
	a = cqtN;
	ptr1 = cqtVector;
	while( a-- ){
		float f = *ptr1;
		*ptr1++ = log10f( f*f );
	}
	
	vDSP_mmul(cqtVector, 1, DCT, 1, foutput, 1, 1, dctN, cqtN);
	if (numLFCCS == -1) {
		vDSP_vspdp(foutput, 1, output, 1, dctN);
		
		double n = dctN;
		vDSP_vsdivD(output, 1, &n, output, 1, dctN);
		//double rms_feature = pkm::Mat::rms(output, cqtN);
		//output[1] = expf(rms_feature);
	}
	else {
		vDSP_vspdp(foutput, 1, output, 1, numLFCCS);
		
		double n = dctN;
		vDSP_vsdivD(output, 1, &n, output, 1, numLFCCS);
		//double rms_feature = pkm::Mat::rms(output, cqtN);
		//output[1] = expf(rms_feature);
	}
	
}

void pkmAudioFeatures::computeChromagramF(float *inputSignal, float *outputFeatures, bool calculateDeltaFeatures)
{
    // should window input buffer before FFT
    fft->forward(0, inputSignal, fft_magnitudes, fft_phases);
    
    int octaves = 2;
    int harmonics = 2;
    int search = 2;
    int searchlength;
    
    float *mag = fft_magnitudes;
    
    float ratio = sampleRate / (float) fftN;
    
    for (int i = 0; i < 12; i++)
    {
        float sum = 0;
        for (int oct = 1;oct <= octaves;oct++)
        {
            float noteval = (note[i]/ratio)*((float) oct);
            float notesum = 0;
            
            for (int h = 1;h <= harmonics;h++)
            {
                int index = round(noteval*((float) h));
                
                searchlength = search*h;
                float maxval = 0;
                for (int n = (index-searchlength);n <= index+searchlength;n++)
                {
                    if (mag[n] > maxval)
                    {
                        maxval = mag[n];
                    }
                }
                notesum = notesum+(maxval*(1/((float) h)));
            }
            sum = sum + notesum;
            
        }
        chroma[i] = sum;
        outputFeatures[i] = sum;
    }
    
    pkm::Mat outputMat(1, 12, outputFeatures, false);
    outputMat.divideEachVecByMaxVecElement(true);
    
    if(calculateDeltaFeatures)
    {
            // chromas'
        vDSP_vsub(outputFeatures, 1,
                  previousChromas, 1,
                  outputFeatures + 12, 1,
                  12);
        
            // Normalize
        pkm::Mat outputMat2(1, 12, outputFeatures + 12, false);
        outputMat.divideEachVecByMaxVecElement(true);
        
            // store
        cblas_scopy(12, outputFeatures, 1, previousChromas, 1);
    }

}
void pkmAudioFeatures::computeChromagramFromMagnitudesF(float *fftMagnitudes, float *outputFeatures, bool calculateDeltaFeatures)
{
    int octaves = 2;
    int harmonics = 2;
    int search = 2;
    int searchlength;
    
    float *mag = fftMagnitudes;
    
    float ratio = sampleRate / (float) fftN;
    
    for (int i = 0; i < 12; i++)
    {
        float sum = 0;
        for (int oct = 1;oct <= octaves;oct++)
        {
            float noteval = (note[i]/ratio)*((float) oct);
            float notesum = 0;
            
            for (int h = 1;h <= harmonics;h++)
            {
                int index = round(noteval*((float) h));
                
                searchlength = search*h;
                float maxval = 0;
                for (int n = (index-searchlength);n <= index+searchlength;n++)
                {
                    if (mag[n] > maxval)
                    {
                        maxval = mag[n];
                    }
                }
                notesum = notesum+(maxval*(1/((float) h)));
            }
            sum = sum + notesum;
            
        }
        chroma[i] = sum;
        outputFeatures[i] = sum;
    }
    
    pkm::Mat outputMat(1, 12, outputFeatures, false);
    outputMat.divideEachVecByMaxVecElement(true);
    
    if(calculateDeltaFeatures)
    {
        // chromas'
        vDSP_vsub(outputFeatures, 1,
                  previousChromas, 1,
                  outputFeatures + 12, 1,
                  12);
        
        // Normalize
        pkm::Mat outputMat2(1, 12, outputFeatures + 12, false);
        outputMat.divideEachVecByMaxVecElement(true);
        
        // store
        cblas_scopy(12, outputFeatures, 1, previousChromas, 1);
    }
}


float pkmAudioFeatures::cosineDistance(float *x, float *y, unsigned int count) {
	float dotProd, magX, magY;
	float *tmp = (float*)malloc(count * sizeof(float));
	
	vDSP_dotpr(x, 1, y, 1, &dotProd, count);
	
	vDSP_vsq(x, 1, tmp, 1, count);
	vDSP_sve(tmp, 1, &magX, count);
	magX = sqrt(magX);
	
	vDSP_vsq(y, 1, tmp, 1, count);
	vDSP_sve(tmp, 1, &magY, count);
	magY = sqrt(magY);
	
	free(tmp);
	
	return 1.0 - (dotProd / (magX * magY));
}

float pkmAudioFeatures::L1Norm(float *buf1, float *buf2, int size)
{
	int a = size;
	float diff = 0;
	float *p1 = buf1, *p2 = buf2;
	while (a) {
		diff += fabs(*p1++ - *p2++);
		a--;
	}
	return diff/(float)size;
}
