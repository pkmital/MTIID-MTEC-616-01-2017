#include "pkmEXTAudioFileWriter.h"
#include <libgen.h>
#include <Accelerate/Accelerate.h>

#ifndef MIN
#define MIN(x,y) ((x) < (y)) ? (x) : (y)
#endif

bool pkmEXTAudioFileWriter::open(string mPath, int frameSize, int sampleRate)
{
	bReady = false;
    this->frameSize = frameSize;
	OSStatus err = noErr;
	CFURLRef mURL = CFURLCreateFromFileSystemRepresentation(NULL,  
															(const UInt8 *) mPath.c_str(), 
															mPath.size(), 
															false);

	mFormat.mSampleRate         = sampleRate;
	mFormat.mFormatID           = kAudioFormatLinearPCM;
	mFormat.mFormatFlags        = kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger; //kAudioFormatFlagsNativeFloatPacked;
	mFormat.mFramesPerPacket    = 1;
	mFormat.mChannelsPerFrame   = 1;
	mFormat.mBitsPerChannel     = sizeof(short) * 8; // sizeof(float) * 8;//16;
	mFormat.mBytesPerPacket     = 2; // 4;//2;
	mFormat.mBytesPerFrame      = 2; // 4;//2;
	mFormat.mReserved           = 0;

	err = ExtAudioFileCreateWithURL(mURL, 
									kAudioFileWAVEType,
									&mFormat,
									NULL,
									kAudioFileFlags_EraseFile, 
									&mSoundID);
	
	CFRelease(mURL); mURL = NULL;
	
	if(err != noErr){
		printf("[pkmEXTAudioFileWriter]: Error opening target file %s for writing: %ld\n", mPath.c_str(), err);
		close();
		return false;
	}
	
	
	err = ExtAudioFileSetProperty(mSoundID, kExtAudioFileProperty_ClientDataFormat, sizeof(mFormat), &mFormat);
	if (err != noErr)
	{
		printf("[pkmEXTAudioFileWriter]: ExtAudioFileSetProperty failed: %ld\n", err);
		close();
		return false;
	}
	
	// pull out basic properties
	mFrameRate		= (unsigned) mFormat.mSampleRate;
	mNumChannels	= (unsigned) mFormat.mChannelsPerFrame;
	mBytesPerSample = (unsigned) mFormat.mBitsPerChannel / 8;
	mNumFrames		= 0;
	//printf("[pkmEXTAudioFileWriter]: opened %s (%d hz, %d ch, %d samples, %d bps)\n", mPath.c_str(), mFrameRate, mNumChannels, mNumFrames, mBytesPerSample);
	
	short_frame = (short *)malloc(sizeof(short) * frameSize);
	float_frame = (float *)malloc(sizeof(float) * frameSize);
	
	bReady = true;
    
    return true;
}

void pkmEXTAudioFileWriter::close()
{
	ExtAudioFileDispose(mSoundID);
    if(bReady)
    {
        free(short_frame);
        free(float_frame);
        bReady = false;
    }
}	

void pkmEXTAudioFileWriter::write(float *frame, long start, long count)
{
    
	if (!bReady) {
		return;
	}
	OSStatus err = noErr;
	UInt32 size = count;
	int offset = 0;
	
	while (offset < count)
	{
		// number of frames
		size = MIN(frameSize, count - offset);	
		
		// convert to 16-bit integer [-32767.0, 32767.0]
		float factor = 32767.0;
		vDSP_vsmul(frame + offset, 1, &factor, float_frame, 1, size);
		vDSP_vfixr16(float_frame, 1, short_frame, 1, size);
		
		AudioBufferList buf;
		buf.mNumberBuffers = 1;
		buf.mBuffers[0].mNumberChannels = 1;
		buf.mBuffers[0].mDataByteSize = size * mNumChannels * mBytesPerSample;
		buf.mBuffers[0].mData = short_frame; //frame + offset;//short_frame;
		err = ExtAudioFileWrite(mSoundID, size, &buf);
		
		mNumFrames += size;
		
		if (err != noErr)
		{
			printf("[pkmEXTAudioFileWriter]: ExtAudioFileWriteAsync failed: %ld\n", err);
			return;
		}	
		else
		{
			offset += size;
			
			if (size == 0)
				break;
		}
	}

}