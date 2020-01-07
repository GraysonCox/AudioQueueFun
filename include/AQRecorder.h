//
// Created by Grayson Cox on 1/7/20.
//

#ifndef AQRECORDER_H
#define AQRECORDER_H


#include <AudioToolbox/AudioToolbox.h>

void HandleInputBuffer(
		void *aqRecorderState,
		AudioQueueRef inAQ,
		AudioQueueBufferRef inBuffer,
		const AudioTimeStamp *inStartTime,
		UInt32 inNumPackets,
		const AudioStreamPacketDescription *inPacketDesc
);

void DeriveBufferSize(
		AudioQueueRef audioQueue,
		AudioStreamBasicDescription &ASBDescription,
		Float64 seconds,
		UInt32 *outBufferSize
);

OSStatus SetMagicCookieForFile(
		AudioQueueRef inQueue,
		AudioFileID inFile
);


#endif //AQRECORDER_H
