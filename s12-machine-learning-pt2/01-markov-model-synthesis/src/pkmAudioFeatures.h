/*
 *  pkmAudioFeatures.h
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
 
 - on a non-exclusive basis, 
 
 - solely for non-commercial use in the hope that it will be useful, 
 
 - "AS-IS" and in order for the benefit of its educational and research
 purposes, pkmital makes clear that no condition is made or to be
 implied, nor is any representation or warranty given or to be
 implied, as to (i) the quality, accuracy or reliability of the
 Software; (ii) the suitability of the Software for any particular
 use or for use under any specific conditions; and (iii) whether use
 of the Software will infringe third-party rights.
 
 pkmital disclaims: 
 
 - all responsibility for the use which is made of the Software; and
 
 - any liability for the outcomes arising from using the Software.
 
 The Licensee may make public, results or data obtained from, dependent
 on or arising out of the use of the Software provided that any such
 publication includes a prominent statement identifying the Software as
 the source of the results or the data, including the Copyright Notice
 and stating that the Software has been made available for use by the
 Licensee under licence from pkmital and the Licensee provides a copy of
 any such publication to pkmital.
 
 The Licensee agrees to indemnify pkmital and hold them
 harmless from and against any and all claims, damages and liabilities
 asserted by third parties (including claims for negligence) which
 arise directly or indirectly from the use of the Software or any
 derivative of it or the sale of any products based on the
 Software. The Licensee undertakes to make no liability claim against
 any employee, student, agent or appointee of pkmital, in connection 
 with this Licence or the Software.
 
 
 No part of the Software may be reproduced, modified, transmitted or
 transferred in any form or by any means, electronic or mechanical,
 without the express permission of pkmital. pkmital's permission is not
 required if the said reproduction, modification, transmission or
 transference is done without financial return, the conditions of this
 Licence are imposed upon the receiver of the product, and all original
 and amended source code is included in any transmitted product. You
 may be held legally responsible for any copyright infringement that is
 caused or encouraged by your failure to abide by these terms and
 conditions.
 
 You are not permitted under this Licence to use this Software
 commercially. Use for which any financial return is received shall be
 defined as commercial use, and includes (1) integration of all or part
 of the source code or the Software into a product for sale or license
 by or on behalf of Licensee to third parties or (2) use of the
 Software or any derivative of it for research with the final aim of
 developing software products for sale or license to a third party or
 (3) use of the Software or any derivative of it for research with the
 final aim of developing non-software products for sale or license to a
 third party, or (4) use of the Software to provide any service to an
 external organisation for which payment is received. If you are
 interested in using the Software commercially, please contact pkmital to
 negotiate a licence. Contact details are: parag@pkmital.com
 
 *
 *  Usage:
 *
 *  pkmAudioFeatures *af;
 *
 *  void setup()
 *  {
 *		af = new pkmAudioFeatures();
 *		LFCCs = (float *)malloc(sizeof(float) * af->getNumCoefficients());
 *  }
 *
 *  void audioReceived(float *input, int bufferSize, int nChannels)
 *  {
 *		af->computeLFCC(input, LFCCs);
 *  }
 *
 */

#pragma once

#include <Accelerate/Accelerate.h>
#include "pkmMatrix.h"
#include "pkmFFT.h"
#include "stdio.h"
#include "string.h"

#define CQ_ENV_THRESH 0.001   // Sparse matrix threshold (for efficient matrix multiplicaton)	

class pkmAudioFeatures
{
public:
    pkmAudioFeatures();
    
    void setup(int sample_rate = 44100, int fft_size = 2048);
	~pkmAudioFeatures();
    
    // 12 features + 12 optional delta
    void computeMelFeatures(float *inputSignal,
                            float *outputFeatures,
                            int numFilters = -1,
                            bool computeLogAmplitude = true,
                            bool computeNormalization = false,
                            bool computeDeltaFeatures = true);
    
	void computeLFCCF(float *inputSignal, float *outputFeatures, int numLFCCS=-1);
	void computeLFCCD(float *inputSignal, double *outputFeatures, int numLFCCS = -1);
	
	void computeLFCCFromMagnitudesF(float *fftMagnitudes, float *outputFeatures, int numLFCCS=-1);
	void computeLFCCFromMagnitudesD(float *fftMagnitudes, double *outputFeatures, int numLFCCS = -1);
    
    void computeChromagramF(float *inputSignal, float *outputFeatures, bool calculateDeltaFeatures = false);
    void computeChromagramFromMagnitudesF(float *fftMagnitudes, float *outputFeatures, bool calculateDeltaFeatures = false);
    
    // get pointer to calculated magnitude/phase after calculating features
	float *getMagnitudes();
	float *getPhases();
    float *getChromagram();
    
    inline int getMagnitudesLength()
    {
        return fftOutN;
    }
    
	
    // number of coefficients in total dct spectrum
	inline int getNumCoefficients()
	{
		return dctN;
	}
    

    // calculate a 12 + 12 dimensional audio feature vector composed of Mel + Chromagram
    void compute24DimAudioFeaturesF(float *inputSignal, float *outputFeatures);
    
    // calculate a 12 + 12 + 12 dimensional audio feature vector composed of Mel + delta Mel + Chromagram
    void compute36DimAudioFeaturesF(float *inputSignal, float *outputFeatures);
    
    // calculate a 12 + 12 + 12 + 12 dimensional audio feature vector composed of Mel + delta Mel + Chromagram + delta Chromagram
    void compute48DimAudioFeaturesF(float *inputSignal, float *outputFeatures);
	
	static float cosineDistance(float *x, float *y, unsigned int count);
	static float L1Norm(float *buf1, float *buf2, int size);
	
private:
	
	void setupCepstral();
	void setupChromagram();
    
	void createLogFreqMap();
	void createDCT();
	
	float			*sample_data,
					*powerSpectrum;
	
	float			*discreteCosineTransformStorage,		// storage
					*constantQTransformStorage;

	float			*DCT,									// coefficients
					*CQT;
	
	float			*cqtVector,								// transforms
					*dctVector;
	
	int				*cqStart,								// sparse matrix indices
					*cqStop;
	
	float			loEdge,									// tranform range
					hiEdge;
	
	float			fratio;
	
	pkmFFT			*fft;
    
    float           *chroma;
    float           *note;
	
	float			*fft_magnitudes,
					*fft_phases;
    
    float           *previousLFCCs;
    float           *previousDeltaLFCCs;
    
    float           *previousChromas;
    float           *previousDeltaChromas;

	float			*foutput;
	
	int				bpoN,
					cqtN,
					dctN,
					fftN,
					fftOutN,
					sampleRate;
    
    int             numberOfDCTBands;                           //  for returning
    
    bool            is_setup;
    
	
};

