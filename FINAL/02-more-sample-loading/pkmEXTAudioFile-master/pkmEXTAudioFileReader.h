#pragma once

#include <AudioToolbox/ExtendedAudioFile.h>
#include <string>

using namespace std;

class pkmEXTAudioFileReader
{
	public:
		pkmEXTAudioFileReader();
		~pkmEXTAudioFileReader();

		bool open(string path, int sampleRate = 44100);
		bool read(float *target, long start, long count, int sampleRate = 44100);
		void close();
	
		ExtAudioFileRef					mSoundID;
	
		unsigned long					mFrameRate,
										mNumChannels,
										mNumSamples,
										mBytesPerSample;	
    
        AudioStreamBasicDescription     dstFormat;
};
