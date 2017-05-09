#include "pkmEXTAudioFileReader.h"
#include <libgen.h>
#include <Accelerate/Accelerate.h>

pkmEXTAudioFileReader::pkmEXTAudioFileReader() : mSoundID(NULL)
{
}

pkmEXTAudioFileReader::~pkmEXTAudioFileReader()
{
    close();
}

bool pkmEXTAudioFileReader::open(string mPath, int sampleRate)
{
	OSStatus err = noErr;
	const char *filename = basename((char *) mPath.c_str());
	
	CFURLRef mURL = CFURLCreateFromFileSystemRepresentation(NULL,  
															(const UInt8 *) mPath.c_str(), 
															mPath.size(), 
															false);
	
	err = ExtAudioFileOpenURL(mURL, &mSoundID);
	CFRelease(mURL); mURL = NULL;
	if (err != noErr)
	{
		printf("[pkmEXTAudioFileReader]: ExtAudioFileOpenURL %s: err %d (file: '%s')\n", filename, (int) err, mPath.c_str());
		return false;
	}
	
	SInt64 fileSize;	
	UInt32 propertySize = sizeof(fileSize);
	
	err = ExtAudioFileGetProperty(mSoundID, 
								  kExtAudioFileProperty_FileLengthFrames , 
								  &propertySize, 
								  &fileSize);
	
	if (err != noErr)
	{ 
		printf("[pkmEXTAudioFileReader]: ExtAudioFileGetProperty FileLengthFrames: err %d (file: '%s')\n", (int) err, mPath.c_str());
		return false;
	}
	
	// get the input file format
	AudioStreamBasicDescription	format;
	propertySize = sizeof(format);
	err = ExtAudioFileGetProperty(mSoundID, 
								  kExtAudioFileProperty_FileDataFormat, 
								  &propertySize, 
								  &format);
	if (err != noErr)
	{ 
		printf("[pkmEXTAudioFileReader]: ExtAudioFileGetProperty FileDataFormat: err %d (file: '%s')\n", (int) err, mPath.c_str());
		return false;
	}
	
	mFrameRate = (unsigned) format.mSampleRate;
	mNumChannels = (unsigned) format.mChannelsPerFrame;
	mNumSamples = (unsigned) fileSize * (sampleRate / format.mSampleRate);
	mBytesPerSample = (unsigned) format.mBitsPerChannel / 8;
	
	printf("[pkmEXTAudioFileReader]: opened %s (%d hz, %d ch, %d samples, %d bps)\n", mPath.c_str(), mFrameRate, mNumChannels, mNumSamples, mBytesPerSample*8);
	
	memset(&dstFormat, 0, sizeof(AudioStreamBasicDescription));
	UInt32 propSize;
	
	propSize = sizeof(dstFormat);
	ExtAudioFileGetProperty(mSoundID, 
							kExtAudioFileProperty_FileDataFormat, 
							&propSize, 
							&dstFormat);
	
	dstFormat.mSampleRate			= sampleRate;
	dstFormat.mFormatID				= kAudioFormatLinearPCM;
	dstFormat.mBitsPerChannel		= sizeof(float) * 8;
	dstFormat.mChannelsPerFrame		= 1; // set this to 2 for stereo
	dstFormat.mFramesPerPacket		= 1;
	//dstFormat.mBytesPerPacket		= dstFormat.mFramesPerPacket * dstFormat.mBytesPerFrame;
	//dstFormat.mBytesPerFrame		= sizeof(Float32) * dstFormat.mChannelsPerFrame;
	dstFormat.mFormatFlags			= kAudioFormatFlagsNativeFloatPacked;//kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked | kAudioFormatFlagsNativeEndian;
	
	dstFormat.mBytesPerFrame		= 4;//dstFormat.mBitsPerChannel * dstFormat.mChannelsPerFrame / 8;
	dstFormat.mBytesPerPacket		= 4;//dstFormat.mFramesPerPacket * dstFormat.mBytesPerFrame;
	dstFormat.mReserved				= 0;
	
	err = ExtAudioFileSetProperty(mSoundID, 
								  kExtAudioFileProperty_ClientDataFormat, 
								  propSize, 
								  &dstFormat);
	if (err != noErr)
	{
		printf("[pkmEXTAudioFileReader]: ExtAudioFileSetProperty failed: %ld\n", err);
		return false;
	}
	
	
	return true;
}

void pkmEXTAudioFileReader::close()
{
	ExtAudioFileDispose(mSoundID);	
}	

bool pkmEXTAudioFileReader::read(float *target, long start, long count, int sampleRate)
{
	//printf("reading %ld - %ld samples\n", start, start+count);
	
	UInt32 size;
	OSStatus err = noErr;
	
	long offset = 0;
	err = ExtAudioFileSeek(mSoundID, start);
	if (err != noErr)
	{
		printf("[pkmEXTAudioFileReader]: ExtAudioFileSeek failed: %ld\n", err);
		return false;
	}
	
	while (offset < count)
	{
		size = count - offset;
		
		AudioBufferList buf;
		buf.mNumberBuffers					= 1;
		buf.mBuffers[0].mNumberChannels		= dstFormat.mChannelsPerFrame;
		buf.mBuffers[0].mDataByteSize		= (count - offset) * dstFormat.mChannelsPerFrame * (unsigned) (dstFormat.mBitsPerChannel / 8);
		buf.mBuffers[0].mData				= target + offset;
		
		err = ExtAudioFileRead(mSoundID, &size, &buf);
		
		if (err != noErr)
		{
			printf("[pkmEXTAudioFileReader]: ExtAudioFileRead failed: %ld\n", err);
			return false;
		}	
		else
		{
			offset += size;
			
			// EOF
			if (size == 0)
				break;
		}
	}
    return true;
}


