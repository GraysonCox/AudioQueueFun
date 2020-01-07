//
// Created by Grayson Cox on 1/7/20.
//

#ifndef AQRECORDERSTATE_H
#define AQRECORDERSTATE_H

#include <AudioToolbox/AudioToolbox.h>

static const int kNumberBuffers = 3;

struct AQRecorderState {
	AudioStreamBasicDescription mDataFormat;
	AudioQueueRef mQueue;
	AudioQueueBufferRef mBuffers[kNumberBuffers];
	AudioFileID mAudioFile;
	UInt32 bufferByteSize;
	SInt64 mCurrentPacket;
	bool mIsRunning;
};


#endif //AQRECORDERSTATE_H
