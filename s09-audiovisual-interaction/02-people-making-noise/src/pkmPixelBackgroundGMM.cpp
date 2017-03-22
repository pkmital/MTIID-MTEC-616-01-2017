//#include "stdafx.h"

// Parag K. Mital
// http://pkmital.com
// 07-04-11
// Goldsmiths, Department of Computing
//
// improvements on zivkovic's original 2005 implementation 

#include "pkmPixelBackgroundGMM.h"


CvPixelBackgroundGMM* cvCreatePixelBackgroundGMM(int width,int height)
{
	CvPixelBackgroundGMM* pGMM=new(CvPixelBackgroundGMM);
	int size=width*height;
	pGMM->nWidth=width;
	pGMM->nHeight=height;
	pGMM->nSize=size;

	pGMM->nNBands=3;//always 3 - not implemented for other values!

	//set parameters
	// K - max number of Gaussians per pixel
	pGMM->nM = 4;			
	// Tb - the threshold - n var
	pGMM->fTb = 4*4;
	// Tbf - the threshold
	pGMM->fTB = 0.9f;//1-cf from the paper 
	// Tgenerate - the threshold
	pGMM->fTg = 3.0f*3.0f;//update the mode or generate new
	pGMM->fSigma= 11.0f;//sigma for the new mode
	// alpha - the learning factor
	pGMM->fAlphaT=0.001f;
	// complexity reduction prior constant
	pGMM->fCT=0.05f;

	//shadow
	// Shadow detection
	pGMM->bShadowDetection = 1;//turn on
	pGMM->fTau = 0.5f;// Tau - shadow threshold


	//GMM for each pixel
	pGMM->rGMM=(CvPBGMMGaussian*) malloc(size * pGMM->nM * sizeof(CvPBGMMGaussian));

	//used modes per pixel
	pGMM->rnUsedModes = (unsigned char* ) malloc(size);
	memset(pGMM->rnUsedModes,0,size);//no modes used
    pGMM->bRemoveForeground=0;
	return pGMM;
}

void cvReleasePixelBackgroundGMM(CvPixelBackgroundGMM** ppGMM)
{
	delete (*ppGMM)->rGMM;
	delete (*ppGMM)->rnUsedModes;
	delete (*ppGMM);
	(*ppGMM)=0;
}


//this might be usefull
void cvSetPixelBackgroundGMM(CvPixelBackgroundGMM* pGMM,unsigned char* data)
{
	int size=pGMM->nSize;
	unsigned char* pDataCurrent=data;
	
	for (int i=0;i<size;i++)
	{
	// retrieve the colors
		float R = *pDataCurrent++;
		float G = *pDataCurrent++;
		float B = *pDataCurrent++;
	
		pGMM->rGMM[i].weight=1.0;
		pGMM->rGMM[i].muR=R;
		pGMM->rGMM[i].muG=G;
		pGMM->rGMM[i].muB=B;
		pGMM->rGMM[i].sigma=pGMM->fSigma;
	}
		
	memset(pGMM->rnUsedModes,1,size);//1 mode used

}

int _cvRemoveShadowGMM(long posPixel, 
								float red, float green, float blue, 
								unsigned char nModes, 
								CvPBGMMGaussian* m_aGaussians,
								int m_nM,
								float m_fTb,
								float m_fTB,	
								float m_fTg,
								float m_fTau)
{
	//calculate distances to the modes (+ sort???)
	//here we need to go in descending order!!!
//	long posPixel = pixel * m_nM;
	long pos;
	float tWeight = 0;
	float numerator, denominator;
	// check all the distributions, marked as background:
	for (int iModes=0;iModes<nModes;iModes++)
	{
		pos=posPixel+iModes;
		float var = m_aGaussians[pos].sigma;
		float muR = m_aGaussians[pos].muR;
		float muG = m_aGaussians[pos].muG;
		float muB = m_aGaussians[pos].muB;
		float weight = m_aGaussians[pos].weight;
		tWeight += weight;
		
		numerator = red * muR + green * muG + blue * muB;
		denominator = muR * muR + muG * muG + muB * muB;
		// no division by zero allowed
		if (denominator == 0)
		{
				break;
		};
		float a = numerator / denominator;

		// if tau < a < 1 then also check the color distortion
		if ((a <= 1) && (a >= m_fTau))//m_nBeta=1
		{
			float dR=a * muR - red;
			float dG=a * muG - green;
			float dB=a * muB - blue;

			//square distance -slower and less accurate
			//float maxDistance = cvSqrt(m_fTb*var);
			//if ((fabs(dR) <= maxDistance) && (fabs(dG) <= maxDistance) && (fabs(dB) <= maxDistance))
			//circle
			float dist=(dR*dR+dG*dG+dB*dB);
			if (dist<m_fTb*var*a*a)
			{
				return 2;
			}
		};
		if (tWeight > m_fTB)
		{
				break;
		};
	};
	return 0;
}




int _cvUpdatePixelBackgroundGMM(long posPixel, 
								float red, float green, float blue, 
								unsigned char* pModesUsed, 
								CvPBGMMGaussian* m_aGaussians,
								int m_nM,
								float m_fAlphaT,
								float m_fTb,
								float m_fTB,	
								float m_fTg,
								float m_fSigma,
								float m_fPrune)
{
	//calculate distances to the modes (+ sort???)
	//here we need to go in descending order!!!
	
	//long posPixel = pixel * m_nM;
	long pos;

//	long pos=posPixel-1;//because of ++ at the end
	
	bool bFitsPDF=0;
	bool bBackground=0;

	float m_fOneMinAlpha=1-m_fAlphaT;

	//bool bPrune=0;
	int nModes=*pModesUsed;
	float totalWeight=0.0f;

	//////
	//go through all modes
	for (int iModes=0;iModes<nModes;iModes++)
	{
		pos=posPixel+iModes;
		float weight = m_aGaussians[pos].weight;

		////
		//fit not found yet
		if (!bFitsPDF)
		{
			//check if it belongs to some of the modes
			//calculate distance
			float var = m_aGaussians[pos].sigma;
			float muR = m_aGaussians[pos].muR;
			float muG = m_aGaussians[pos].muG;
			float muB = m_aGaussians[pos].muB;
		
			float dR=muR - red;
			float dG=muG - green;
			float dB=muB - blue;

			///////
			//check if it fits the current mode (Factor * sigma)
			
			//square distance -slower and less accurate
			//float maxDistance = cvSqrt(m_fTg*var);
			//if ((fabs(dR) <= maxDistance) && (fabs(dG) <= maxDistance) && (fabs(dB) <= maxDistance))
			//circle
			float dist=(dR*dR+dG*dG+dB*dB);
			//background? - m_fTb
			if ((totalWeight<m_fTB)&&(dist<m_fTb*var))
					bBackground=1;
			//check fit
			if (dist<m_fTg*var)
			{
				/////
				//belongs to the mode
				bFitsPDF=1;

				//update distribution
				float k = m_fAlphaT/weight;
				weight=m_fOneMinAlpha*weight+m_fPrune;
				weight+=m_fAlphaT;
				m_aGaussians[pos].muR = muR - k*(dR);
				m_aGaussians[pos].muG = muG - k*(dG);
				m_aGaussians[pos].muB = muB - k*(dB);

				//limit update speed for cov matrice
				//not needed
				//k=k>20*m_fAlphaT?20*m_fAlphaT:k;
				//float sigmanew = var + k*((0.33*(dR*dR+dG*dG+dB*dB))-var);
				//float sigmanew = var + k*((dR*dR+dG*dG+dB*dB)-var);
				//float sigmanew = var + k*((0.33*dist)-var);
				float sigmanew = var + k*(dist-var);
				//limit the variance
				//m_aGaussians[pos].sigma = sigmanew>70?70:sigmanew;
				//m_aGaussians[pos].sigma = sigmanew>5*m_fSigma?5*m_fSigma:sigmanew;
				m_aGaussians[pos].sigma =sigmanew< 4 ? 4 : sigmanew>5*m_fSigma?5*m_fSigma:sigmanew;
				//m_aGaussians[pos].sigma =sigmanew< 4 ? 4 : sigmanew>3*m_fSigma?3*m_fSigma:sigmanew;
				//m_aGaussians[pos].sigma = m_fSigma;
				//sort
				//all other weights are at the same place and 
				//only the matched (iModes) is higher -> just find the new place for it
				for (int iLocal = iModes;iLocal>0;iLocal--)
				{
					long posLocal=posPixel + iLocal;
					if (weight < (m_aGaussians[posLocal-1].weight))
					{
						break;
					}
					else
					{
						//swap
						CvPBGMMGaussian temp = m_aGaussians[posLocal];
						m_aGaussians[posLocal] = m_aGaussians[posLocal-1];
						m_aGaussians[posLocal-1] = temp;
					}
				}

				//belongs to the mode
				/////
			}
			else
			{
				weight=m_fOneMinAlpha*weight+m_fPrune;
				//check prune
				if (weight<-m_fPrune)
				{
					weight=0.0;
					nModes--;
				//	bPrune=1;
					//break;//the components are sorted so we can skip the rest
				}
			}
			//check if it fits the current mode (2.5 sigma)
			///////
		}
		//fit not found yet
		/////
		else
		{
				weight=m_fOneMinAlpha*weight+m_fPrune;
				//check prune
				if (weight<-m_fPrune)
				{
					weight=0.0;
					nModes--;
					//bPrune=1;
					//break;//the components are sorted so we can skip the rest
				}
		}
		totalWeight+=weight;
		m_aGaussians[pos].weight=weight;
	}
	//go through all modes
	//////

	//renormalize weights
	for (int iLocal = 0; iLocal < nModes; iLocal++)
	{
		m_aGaussians[posPixel+ iLocal].weight = m_aGaussians[posPixel+ iLocal].weight/totalWeight;
	}
	
	//make new mode if needed and exit
	if (!bFitsPDF)
	{
		if (nModes==m_nM)
		{
           //replace the weakest
		}
		else
		{
           //add a new one
			//totalWeight+=m_fAlphaT;
			//pos++;
			nModes++;
		}
		pos=posPixel+nModes-1;

      	if (nModes==1)
			m_aGaussians[pos].weight=1;
		else
			m_aGaussians[pos].weight=m_fAlphaT;

		//renormalize weights
		int iLocal;
		for (iLocal = 0; iLocal < nModes-1; iLocal++)
		{
			m_aGaussians[posPixel+ iLocal].weight *=m_fOneMinAlpha;
		}

		m_aGaussians[pos].muR=red;
		m_aGaussians[pos].muG=green;
		m_aGaussians[pos].muB=blue;
		m_aGaussians[pos].sigma=m_fSigma;

		//sort
		//find the new place for it
		for (iLocal = nModes-1;iLocal>0;iLocal--)
		{
			long posLocal=posPixel + iLocal;
			if (m_fAlphaT < (m_aGaussians[posLocal-1].weight))
			{
						break;
			}
			else
			{
				//swap
				CvPBGMMGaussian temp = m_aGaussians[posLocal];
				m_aGaussians[posLocal] = m_aGaussians[posLocal-1];
				m_aGaussians[posLocal-1] = temp;
			}
		}
	}

	//set the number of modes
	*pModesUsed=nModes;

    return bBackground;
}

void _cvReplacePixelBackgroundGMM(long pos, 
								unsigned char* pData, 
								CvPBGMMGaussian* m_aGaussians)
{
	pData[0]=(unsigned char) m_aGaussians[pos].muR;
	pData[1]=(unsigned char) m_aGaussians[pos].muG;
	pData[2]=(unsigned char) m_aGaussians[pos].muB;
}


void cvUpdatePixelBackgroundGMM(CvPixelBackgroundGMM* pGMM,unsigned char* data,unsigned char* output)
{
	int size=pGMM->nSize;
	unsigned char* pDataCurrent=data;
	unsigned char* pUsedModes=pGMM->rnUsedModes;
	unsigned char* pDataOutput=output;
	//some constants
	int m_nM=pGMM->nM;
	float m_fAlphaT=pGMM->fAlphaT;
	float m_fTb=pGMM->fTb;//Tb - threshold on the Mahalan. dist.
	float m_fTB=pGMM->fTB;//1-TF from the paper
	float m_fTg=pGMM->fTg;//Tg - when to generate a new component
	float m_fSigma=pGMM->fSigma;//initial sigma
	float m_fCT=pGMM->fCT;//CT - complexity reduction prior 
	float m_fPrune=-m_fAlphaT*m_fCT;
	float m_fTau=pGMM->fTau;
	CvPBGMMGaussian* m_aGaussians=pGMM->rGMM;
	long posPixel;
	int m_bShadowDetection=pGMM->bShadowDetection;

	//go through the image
	for (int i=0;i<size;i++)
	{
		// retrieve the colors
		float red = *pDataCurrent++;
		float green = *pDataCurrent++;
		float blue = *pDataCurrent++;
		
		//update model+ background subtract
		;
		posPixel=i*m_nM;
		int result = _cvUpdatePixelBackgroundGMM(posPixel, red, green, blue,pUsedModes,m_aGaussians,
			m_nM,m_fAlphaT, m_fTb, m_fTB, m_fTg, m_fSigma, m_fPrune);
		int nMLocal=*pUsedModes;
		pUsedModes++;
		if (m_bShadowDetection)
				if (!result)
				{
					result= _cvRemoveShadowGMM(posPixel, red, green, blue,nMLocal,m_aGaussians,
								m_nM,
								m_fTb,
								m_fTB,	
								m_fTg,
								m_fTau);
				}

		
		switch (result)
		{
			case 0:
				//foreground
				(* pDataOutput)=255;
				if (pGMM->bRemoveForeground) 
				{
					_cvReplacePixelBackgroundGMM(posPixel,pDataCurrent-3,m_aGaussians);
				}
				break;
			case 1:
				//background
				(* pDataOutput)=0;
				break;
			case 2:
				//shadow
				(* pDataOutput)=125;
				if (pGMM->bRemoveForeground) 
				{
					_cvReplacePixelBackgroundGMM(posPixel,pDataCurrent-3,m_aGaussians);
				}

				break;
		}
		//(* pDataOutput)=nM*30;
		pDataOutput++;
	}
}

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
					float m_fPrune)
{
	long pos;
	bool bFitsPDF=0;
	bool bBackground=0;
	
	float m_fOneMinAlpha=1-m_fAlphaT;
	
	//bool bPrune=0;
	int nModes=*pModesUsed;
	float totalWeight=0.0f;
	
	//////
	//go through all modes
	for (int iModes=0;iModes<nModes;iModes++)
	{
		pos=posPixel+iModes;
		float weight = m_aGaussians[pos].weight;
		
		////
		//fit not found yet
		if (!bFitsPDF)
		{
			//check if it belongs to some of the modes
			//calculate distance
			float var = m_aGaussians[pos].sigma;
			float muR = m_aGaussians[pos].muR;
			float muG = m_aGaussians[pos].muG;
			float muB = m_aGaussians[pos].muB;
			
			float dR=muR - red;
			float dG=muG - green;
			float dB=muB - blue;
			
			float dist=(dR*dR+dG*dG+dB*dB);
			//background? - m_fTb
			if ((totalWeight<m_fTB)&&(dist<m_fTb*var))
				bBackground=1;
			//check fit
			if (dist<m_fTg*var)
			{
				/////
				//belongs to the mode
				bFitsPDF=1;
				return 1;
			}
				
			else
			{
				weight=m_fOneMinAlpha*weight+m_fPrune;
				//check prune
				if (weight<-m_fPrune)
				{
					weight=0.0;
					nModes--;
				}
			}
		}
		//fit not found yet
		/////
		else
		{
			weight=m_fOneMinAlpha*weight+m_fPrune;
			//check prune
			if (weight<-m_fPrune)
			{
				weight=0.0;
				nModes--;
				//bPrune=1;
				//break;//the components are sorted so we can skip the rest
			}
		}
		totalWeight+=weight;
		m_aGaussians[pos].weight=weight;
	}
	
    return 0;
}

void cvPixelBackgroundGMMSubtraction(CvPixelBackgroundGMM* pGMM,unsigned char* data,unsigned char* output)
{
	int size=pGMM->nSize;
	unsigned char* pDataCurrent=data;
	unsigned char* pUsedModes=pGMM->rnUsedModes;
	unsigned char* pDataOutput=output;
	//some constants
	int m_nM=pGMM->nM;
	float m_fAlphaT=pGMM->fAlphaT;
	float m_fTb=pGMM->fTb;//Tb - threshold on the Mahalan. dist.
	float m_fTB=pGMM->fTB;//1-TF from the paper
	float m_fTg=pGMM->fTg;//Tg - when to generate a new component
	float m_fSigma=pGMM->fSigma;//initial sigma
	float m_fCT=pGMM->fCT;//CT - complexity reduction prior 
	float m_fPrune=-m_fAlphaT*m_fCT;
	float m_fTau=pGMM->fTau;
	CvPBGMMGaussian* m_aGaussians=pGMM->rGMM;
	long posPixel;
	int m_bShadowDetection=pGMM->bShadowDetection;
	
	//go through the image
	for (int i=0;i<size;i++)
	{
		// retrieve the colors
		float red = *pDataCurrent++;
		float green = *pDataCurrent++;
		float blue = *pDataCurrent++;
		
		//update model+ background subtract
		posPixel=i*m_nM;
		int result = _cvCheckPixel(posPixel, red, green, blue,pUsedModes,m_aGaussians,
								   m_nM,m_fAlphaT, m_fTb, m_fTB, m_fTg, m_fSigma, m_fPrune);
		if(result == 0)
		{
			int nMLocal=*pUsedModes;
			result = _cvRemoveShadowGMM(posPixel, red, green, blue,nMLocal,m_aGaussians,
								   m_nM,
								   m_fTb,
								   m_fTB,	
								   m_fTg,
								   m_fTau);
		}
		(* pDataOutput)=255 - 255 * result;
		pDataOutput++;
	}
}


void cvUpdatePixelBackgroundGMMTiled(CvPixelBackgroundGMM* pGMM,unsigned char* data,unsigned char* output)
{
	int size=pGMM->nSize;
	unsigned char* pDataCurrentR=data;//separate R G and B images
	unsigned char* pDataCurrentG=data+size;
	unsigned char* pDataCurrentB=data+2*size;

	unsigned char* pUsedModes=pGMM->rnUsedModes;
	unsigned char* pDataOutput=output;
	//some constants
	int m_nM=pGMM->nM;
	float m_fAlphaT=pGMM->fAlphaT;
	float m_fTb=pGMM->fTb;//Tb - threshold on the Mahalan. dist.
	float m_fTB=pGMM->fTB;//1-TF from the paper
	float m_fTg=pGMM->fTg;//Tg - when to generate a new component
	float m_fSigma=pGMM->fSigma;//initial sigma
	float m_fCT=pGMM->fCT;//CT - complexity reduction prior 
	float m_fPrune=-m_fAlphaT*m_fCT;
	float m_fTau=pGMM->fTau;
	CvPBGMMGaussian* m_aGaussians=pGMM->rGMM;
	long posPixel;
	int m_bShadowDetection=pGMM->bShadowDetection;

	//go through the image
	for (int i=0;i<size;i++)
	{
		// retrieve the colors
		float red = *pDataCurrentR++;
		float green = *pDataCurrentG++;
		float blue = *pDataCurrentB++;
		
		//update model+ background subtract
		;
		posPixel=i*m_nM;
		int result = _cvUpdatePixelBackgroundGMM(posPixel, red, green, blue,pUsedModes,m_aGaussians,
			m_nM,m_fAlphaT, m_fTb, m_fTB, m_fTg, m_fSigma, m_fPrune);
		int nMLocal=*pUsedModes;
		pUsedModes++;
		if (m_bShadowDetection)
				if (!result)
				{
					result= _cvRemoveShadowGMM(posPixel, red, green, blue,nMLocal,m_aGaussians,
								m_nM,
								m_fTb,
								m_fTB,	
								m_fTg,
								m_fTau);
				}

		
		switch (result)
		{
			case 0:
				//foreground
				(* pDataOutput)=255;
				if (pGMM->bRemoveForeground) 
				{
//					_cvReplacePixelBackgroundGMM(posPixel,pDataCurrent-3,m_aGaussians);
				}
				break;
			case 1:
				//background
				(* pDataOutput)=0;
				break;
			case 2:
				//shadow
				(* pDataOutput)=125;
				if (pGMM->bRemoveForeground) 
				{
//					_cvReplacePixelBackgroundGMM(posPixel,pDataCurrent-3,m_aGaussians);
				}

				break;
		}
		//(* pDataOutput)=nM*30;
		pDataOutput++;
	}
}