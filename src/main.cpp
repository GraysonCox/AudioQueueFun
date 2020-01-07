//
// Created by Grayson Cox on 1/7/20.
//

#include <string>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioFile.h>
#include <CoreFoundation/CFURL.h>
#include <AQRecorderState.h>
#include <AQRecorder.h>

using namespace std;

const string OUTPUT_FILE_PATH = "output.wav";

int main() {

	// Set up audio format for recording.
	AQRecorderState aqData;
	aqData.mDataFormat.mFormatID = kAudioFormatLinearPCM;
	aqData.mDataFormat.mSampleRate = 44100.0;
	aqData.mDataFormat.mChannelsPerFrame = 2;
	aqData.mDataFormat.mBitsPerChannel = 16;
	aqData.mDataFormat.mBytesPerPacket =
	aqData.mDataFormat.mBytesPerFrame = aqData.mDataFormat.mChannelsPerFrame * sizeof(SInt16);
	aqData.mDataFormat.mFramesPerPacket = 1;
	AudioFileTypeID fileType = kAudioFileAIFFType;
	aqData.mDataFormat.mFormatFlags = kLinearPCMFormatFlagIsBigEndian | kLinearPCMFormatFlagIsSignedInteger
									  | kLinearPCMFormatFlagIsPacked;

	// Create recording audio queue.
	AudioQueueNewInput(
			&aqData.mDataFormat,
			HandleInputBuffer,
			&aqData,
			NULL,
			kCFRunLoopCommonModes,
			0,
			&aqData.mQueue
	);

	// Get full audio queue format from audio queue.
	UInt32 dataFormatSize = sizeof(aqData.mDataFormat);
	AudioQueueGetProperty(
			aqData.mQueue,
			kAudioQueueProperty_StreamDescription,
			// TODO: in Mac OS X, instead use
			//    kAudioConverterCurrentInputStreamDescription
			&aqData.mDataFormat,
			&dataFormatSize
	);

	// Create audio file.
	CFURLRef audioFileURL =
			CFURLCreateFromFileSystemRepresentation(
					NULL,
					(const UInt8 *) OUTPUT_FILE_PATH.c_str(),
					strlen(OUTPUT_FILE_PATH.c_str()),
					false
			);
	AudioFileCreateWithURL(
			audioFileURL,
			fileType,
			&aqData.mDataFormat,
			kAudioFileFlags_EraseFile,
			&aqData.mAudioFile
	);

	// Set audio queue buffer size.
	DeriveBufferSize(
			aqData.mQueue,
			aqData.mDataFormat,
			0.5,
			&aqData.bufferByteSize
	);

	// Prepare set of audio queue buffers.
	for (int i = 0; i < kNumberBuffers; ++i) {
		AudioQueueAllocateBuffer(
				aqData.mQueue,
				aqData.bufferByteSize,
				&aqData.mBuffers[i]
		);

		AudioQueueEnqueueBuffer(
				aqData.mQueue,
				aqData.mBuffers[i],
				0,
				NULL
		);
	}

	// Record audio.
	aqData.mCurrentPacket = 0;
	aqData.mIsRunning = true;
	AudioQueueStart(
			aqData.mQueue,
			NULL
	);
	// Wait on user interface thread until user stops the recording.
	sleep(2);
	AudioQueueStop(
			aqData.mQueue,
			true
	);
	aqData.mIsRunning = false;

	// Clean up after recording.
	AudioQueueDispose(
			aqData.mQueue,
			true
	);

	AudioFileClose(aqData.mAudioFile);

	return 0;
}