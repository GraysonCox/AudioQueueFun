//
// Created by Grayson Cox on 1/7/20.
//

#include <AQRecorderState.h>
#include "AQRecorder.h"

void HandleInputBuffer(
		void *aqRecorderState,
		AudioQueueRef inAQ,
		AudioQueueBufferRef inBuffer,
		const AudioTimeStamp *inStartTime,
		UInt32 inNumPackets,
		const AudioStreamPacketDescription *inPacketDesc) {
	AQRecorderState *pAqRecorderState = (AQRecorderState *) aqRecorderState;

	// If the audio queue buffer contains CBR data, calculate the number of packets in the buffer. This number equals
	// the total bytes of data in the buffer divided by the (constant) number of bytes per packet. For VBR data, the
	// audio queue supplies the number of packets in the buffer when it invokes the callback.
	if (inNumPackets == 0 && pAqRecorderState->mDataFormat.mBytesPerPacket != 0)
		inNumPackets = inBuffer->mAudioDataByteSize / pAqRecorderState->mDataFormat.mBytesPerPacket;

	// Write audio queue buffer to disk.
	if (AudioFileWritePackets(
			pAqRecorderState->mAudioFile,
			false,
			inBuffer->mAudioDataByteSize,
			inPacketDesc,
			pAqRecorderState->mCurrentPacket,
			&inNumPackets,
			inBuffer->mAudioData
	) == noErr) {
		pAqRecorderState->mCurrentPacket += inNumPackets;
	}

	// Return if the audio queue has stopped.
	if (!pAqRecorderState->mIsRunning)
		return;

	// Enqueue audio queue buffer.
	AudioQueueEnqueueBuffer(
			pAqRecorderState->mQueue,
			inBuffer,
			0,
			NULL
	);
}

void DeriveBufferSize(AudioQueueRef audioQueue, AudioStreamBasicDescription &ASBDescription, Float64 seconds,
					  UInt32 *outBufferSize) {
	static const int maxBufferSize = 0x50000;

	int maxPacketSize = ASBDescription.mBytesPerPacket;
	if (maxPacketSize == 0) {
		UInt32 maxVBRPacketSize = sizeof(maxPacketSize);
		AudioQueueGetProperty(
				audioQueue,
				kAudioQueueProperty_MaximumOutputPacketSize,
				// TODO: in Mac OS X v10.5, instead use
				//   kAudioConverterPropertyMaximumOutputPacketSize
				&maxPacketSize,
				&maxVBRPacketSize
		);
	}

	Float64 numBytesForTime = ASBDescription.mSampleRate * maxPacketSize * seconds;
	*outBufferSize = UInt32(numBytesForTime < maxBufferSize ? numBytesForTime : maxBufferSize);
}

OSStatus SetMagicCookieForFile(AudioQueueRef inQueue, AudioFileID inFile) {
	OSStatus result = noErr;
	UInt32 cookieSize;

	if (AudioQueueGetPropertySize(
			inQueue,
			kAudioQueueProperty_MagicCookie,
			&cookieSize) == noErr) {
		char *magicCookie = (char *) malloc(cookieSize);
		if (AudioQueueGetProperty(
				inQueue,
				kAudioQueueProperty_MagicCookie,
				magicCookie,
				&cookieSize
		) == noErr)
			result = AudioFileSetProperty(
					inFile,
					kAudioFilePropertyMagicCookieData,
					cookieSize,
					magicCookie
			);
		free(magicCookie);
	}
	return result;
}
