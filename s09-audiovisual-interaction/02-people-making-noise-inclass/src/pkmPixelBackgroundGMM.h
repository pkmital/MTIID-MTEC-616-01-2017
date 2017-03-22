#ifndef __CVPIXELBACKGROUND_H
#define __CVPIXELBACKGROUND_H
// Parag K. Mital
// http://pkmital.com
// 07-04-11
// Goldsmiths, Department of Computing
//
// improvements on zivkovic's original 2005 implementation 


//Implementation of the Gaussian mixture model background subtraction from:
//
//"Improved adaptive Gausian mixture model for background subtraction"
//Z.Zivkovic 
//International Conference Pattern Recognition, UK, August, 2004
//
// and
//
//"Efficient Adaptive Density Estimapion per Image Pixel for the Task of Background Subtraction"
//Z.Zivkovic, F. van der Heijden 
//Pattern Recognition Letters, vol. 27, no. 7, pages 773-780, 2006.
//
//The algorithm similar to the standard Stauffer&Grimson algorithm with
//additional selection of the number of the Gaussian components based on:
//
//"Recursive unsupervised learning of finite mixture models "
//Z.Zivkovic, F.van der Heijden 
//IEEE Trans. on Pattern Analysis and Machine Intelligence, vol.26, no.5, pages 651-656, 2004 
//
//
//Example usage:
// //Somewhere during initalization:
// #include "CvPixelBacgroundGMM.h"
// CvPixelBackgroundGMM* pGMM=0;
// pGMM=cvCreatePixelBackgroundGMM(width,height);//reserve memory
// //you migh want to change some parameters of pGMM here...
// ....
//
// //repeat for each frame
// //you migh want to change some parameters of pGMM here...
// 	cvUpdatePixelBackgroundGMM(pGMM,NewRGBImageDataPointer,SegmentedImageGrayDataPointer);
// ....
// 
// //at the end when the progam terminates do not forget to release the reseved memory
// 	cvReleasePixelBackgroundGMM(&pGMM);
//
//
//Author: Z.Zivkovic, www.zoranz.net
//University of Amsterdam, The Netherlands
//Date: 27-April-2005, Version:0.9
///////////

#include <stdlib.h>
#include <memory.h>

typedef struct CvPBGMMGaussian
{
	float sigma;
	float muR;
	float muG;
	float muB;
	float weight;
} CvPBGMMGaussian;

typedef struct CvPixelBackgroundGMM
{
	/////////////////////////
	//very important parameters - things you will change
	////////////////////////
	float fAlphaT;
	//alpha - speed of update - if the time interval you want to average over is T
	//set alpha=1/T. It is also usefull at start to make T slowly increase
	//from 1 until the desired T
	float fTb;
	//Tb - threshold on the squared Mahalan. dist. to decide if it is well described
	//by the background model or not. Related to Cthr from the paper.
	//This does not influence the update of the background. A typical value could be 4 sigma
	//and that is Tb=4*4=16;
	
	/////////////////////////
	//less important parameters - things you might change but be carefull
	////////////////////////
	float fTg;
	//Tg - threshold on the squared Mahalan. dist. to decide 
	//when a sample is close to the existing components. If it is not close
	//to any a new component will be generated. I use 3 sigma => Tg=3*3=9.
	//Smaller Tg leads to more generated components and higher Tg might make
	//lead to small number of components but they can grow too large
	float fTB;//1-cf from the paper
	//TB - threshold when the component becomes significant enough to be included into
	//the background model. It is the TB=1-cf from the paper. So I use cf=0.1 => TB=0.
	//For alpha=0.001 it means that the mode should exist for approximately 105 frames before
	//it is considered foreground
	float fSigma;
	//initial standard deviation  for the newly generated components. 
	//It will will influence the speed of adaptation. A good guess should be made. 
	//A simple way is to estimate the typical standard deviation from the images.
	//I used here 10 as a reasonable value
	float fCT;//CT - complexity reduction prior
	//this is related to the number of samples needed to accept that a component
	//actually exists. We use CT=0.05 of all the samples. By setting CT=0 you get
	//the standard Stauffer&Grimson algorithm (maybe not exact but very similar)

	//even less important parameters
	int nM;//max number of modes - const - 4 is usually enough

	//shadow detection parameters
	int bShadowDetection;//do shadow detection
	float fTau;
	// Tau - shadow threshold. The shadow is detected if the pixel is darker
	//version of the background. Tau is a threshold on how much darker the shadow can be.
	//Tau= 0.5 means that if pixel is more than 2 times darker then it is not shadow
	//See: Prati,Mikic,Trivedi,Cucchiarra,"Detecting Moving Shadows...",IEEE PAMI,2003.
	
	//data
	int nNBands;//only RGB now ==3
	int nWidth;//image size
	int nHeight;
	int nSize;
	// dynamic array for the mixture of Gaussians
	CvPBGMMGaussian* rGMM;
	unsigned char* rnUsedModes;//number of Gaussian components per pixel
	bool bRemoveForeground;
} CvPixelBackgroundGMM;


void cvUpdatePixelBackgroundGMM(CvPixelBackgroundGMM* pGMM,unsigned char* data,unsigned char* output);
//Input:
//	pGMM - a pointer to an alrady initialized GMM
//  data - a pointer to the data of a RGB image of the same size
//Output:
//  out - a pointer to the data of a gray value image of the same size (the memory should already be reserved) 
//		  values: 255-foreground, 125-shadow, 0-background
///////////
void cvUpdatePixelBackgroundGMMTiled(CvPixelBackgroundGMM* pGMM,unsigned char* data,unsigned char* output);
//Use in case R G B images are separate - e.g. calling from Matlab 

void cvPixelBackgroundGMMSubtraction(CvPixelBackgroundGMM* pGMM,unsigned char* data,unsigned char* output);
int _cvCheckPixel(long posPixel, 
				  float red, float green, float blue, 
				  unsigned char* pModesUsed, 
				  CvPBGMMGaussian* m_aGaussians,
				  int m_nM,
				  float m_fAlphaT,
				  float m_fTb,
				  float m_fTB,	
				  float m_fTg,
				  float m_fSigma,
				  float m_fPrune);


#if 1

CvPixelBackgroundGMM* cvCreatePixelBackgroundGMM(int width,int height);

void cvReleasePixelBackgroundGMM(CvPixelBackgroundGMM** ppGMM);

//this might be usefull
void cvSetPixelBackgroundGMM(CvPixelBackgroundGMM* pGMM,unsigned char* data);
#endif



#endif