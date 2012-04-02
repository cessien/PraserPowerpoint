#include <revel.h>
#include <queue>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
//#include <boost/bind.hpp>
#include "revelrecorder.h"

extern int width, height;
Revel_Error revError;
Revel_Params revParams;
Revel_VideoFrame frame;
const char *filename = "checkers.avi";
int encoderHandle;
int audioBits, audioChannels, audioFormat, audioRate, audioBufferSize;
char *audioBuffer;
int numFrames = 0;


// Function to load a sound file for the sample movie's audio track.
// Once again, this is completely unrelated to using Revel, so don't
// worry about understanding it fully.
void LoadAudio(bool *hasAudioa, int *audioBitsa, int *audioChannelsa,
               int *audioFormata, int *audioRatea, char **audioBuffera,
               int *audioBufferSizea)
{
    // chimes.raw is raw sample data.  The details of its sample format are
    // hard-coded into this file, so that I don't have to clutter
    // up the example code with a WAV-loading function.
    // ENDIAN ISSUE: chimes.raw is stored in little-endian format.  Running
    // this code on a big-endian system won't crash, but the audio will sound
    // pretty awful.  You've been warned!
    const char *audioFilename = "chimes.raw";
    *audioBitsa = 16;
    *audioChannelsa = 2;
    *audioFormata = REVEL_ASF_PCM;
    *audioRatea = 22050;
    *hasAudioa = false;
    FILE *audioFile = fopen(audioFilename, "rb");
    if (audioFile != NULL)
    {
        fseek(audioFile, 0, SEEK_END);
        *audioBufferSizea = ftell(audioFile);
        fseek(audioFile, 0, SEEK_SET);
        *audioBuffera = new char[*audioBufferSizea];
        *hasAudioa = (fread(*audioBuffera, 1, *audioBufferSizea, audioFile) == *audioBufferSizea);
        fclose(audioFile);
    }
    
}

void initRecord(){

	// Attempt to load some sound data, to encode into the output movie's
	// audio stream.
	bool hasAudio = false;

	audioBuffer = NULL;
	LoadAudio(&hasAudio, &audioBits, &audioChannels, &audioFormat, &audioRate,
		&audioBuffer, &audioBufferSize);
	if (!hasAudio)
	{
		printf("Warning: Failed to load audio test file: chimes.raw\n");
		printf("         The audio encoding tests will be skipped.\n");
	}

	// Make sure the API version of Revel we're compiling against matches the
	// header files!  This is terribly important!
	if (REVEL_API_VERSION != Revel_GetApiVersion())
	{
		printf("ERROR: Revel version mismatch!\n");
		printf("Headers: version %06x, API version %d\n", REVEL_VERSION,
			REVEL_API_VERSION);
		printf("Library: version %06x, API version %d\n", Revel_GetVersion(),
			Revel_GetApiVersion());
		exit(1);
	}

	// Create an encoder
	revError = Revel_CreateEncoder(&encoderHandle);
	if (revError != REVEL_ERR_NONE)
	{
		printf("Revel Error while creating encoder: %d\n", revError);
		exit(1);
	}

	// Set up the encoding parameters.  ALWAYS call Revel_InitializeParams()
	// before filling in your application's parameters, to ensure that all
	// fields (especially ones that you may not know about) are initialized
	// to safe values.

	Revel_InitializeParams(&revParams);
	revParams.width = width;
	revParams.height = height;
	revParams.frameRate = 25.0f;
	revParams.quality = 1.0f;
	revParams.codec = REVEL_CD_XVID;

	revParams.hasAudio = hasAudio ? 1 : 0;
	revParams.audioChannels = audioChannels;
	revParams.audioRate = audioRate;
	revParams.audioBits = audioBits;
	revParams.audioSampleFormat = audioFormat;

	// Initiate encoding
	revError = Revel_EncodeStart(encoderHandle, filename, &revParams);
	if (revError != REVEL_ERR_NONE)
	{
		printf("Revel Error while starting encoding: %d\n", revError);
		exit(1);
	}

	// Draw and encode each frame.

	frame.width = width;
	frame.height = height;
	frame.bytesPerPixel = 4;
	frame.pixelFormat = REVEL_PF_RGBA;
	frame.pixels = new int[width*height];
}

std::queue<unsigned char*> rQueue;
boost::mutex io_mutex2;


//Remove frames from the buffer and record it to a file
void processQueue(){
	if(rQueue.empty()){
		return;
	}
	boost::unique_lock<boost::mutex> lock(io_mutex2);

	printf("Size of the queue is: %i\n",rQueue.size());

	//Flip the image upside down
	for(int i = height - 1; i >= 0; i--){
//		printf("hey %i\n", i);
		//memcpy(frame.pixels + (height - i - 1)*width*4, queue.dequeFrame() + i*width*4, width*4);
		memcpy(frame.pixels + (height - i - 1)*width*4, rQueue.front() + i*width*4, width*4);
	}

	//free(rQueue.front());
	rQueue.pop();
	//lock.unlock();

	int frameSize;
	revError = Revel_EncodeFrame(encoderHandle, &frame, &frameSize);
	if (revError != REVEL_ERR_NONE)
	{
		printf("Revel Error while writing frame: %d\n", revError);
		exit(1);
	}
	printf("Frame %d of %d: %d bytes\n", numFrames+1, numFrames, frameSize);
	numFrames++;
//	io_mutex2.unlock();
}

void recordFrame(unsigned char *data){
	boost::unique_lock<boost::mutex> lock(io_mutex2);
//	printf("data: %x\n", data[300]);
	rQueue.push(data);
//	io_mutex2.unlock();
//	free(data);
}

void endRecord(){
//		while(!rQueue.empty()){
//			processQueue();
//		}
	if(numFrames > 0){
		// Encode the audio track.  NOTE that each call to Revel_EncodeAudio()
		// *appends* the new audio data onto the existing audio track.  There is
		// no synchronization between the audio and video tracks!  If you want
		// the audio to start on frame 60, you need to manually insert 60 frames
		// worth of silence at the beginning of your audio track!
		//
		// To demonstrate this, we'll encode the audio buffer twice. Note that
		// the two chimes play immediately when the movie starts, one after the
		// other, even though we're encoding them "after" all the video frames.


		int totalAudioBytes = 0;
		revError = Revel_EncodeAudio(encoderHandle, audioBuffer, audioBufferSize,
				&totalAudioBytes);
		revError = Revel_EncodeAudio(encoderHandle, audioBuffer, audioBufferSize,
				&totalAudioBytes);
		if (revError != REVEL_ERR_NONE)
		{
			printf("Revel Error while writing audio: %d\n", revError);
			exit(1);
		}
		printf("Encoded %d bytes of audio\n", totalAudioBytes);


		// Finalize encoding.  If this step is skipped, the output movie will be
		// unviewable!
		int totalSize;
		revError = Revel_EncodeEnd(encoderHandle, &totalSize);
		if (revError != REVEL_ERR_NONE)
		{
			printf("Revel Error while ending encoding: %d\n", revError);
			exit(1);
		}
		printf("%s written: %dx%d, %d frames, %d bytes\n", filename, width, height,
				numFrames, totalSize);

		// Final cleanup.
		//			Revel_DestroyEncoder(encoderHandle);
		//			if (audioBuffer != NULL)
		//				delete [] audioBuffer;
		//			delete [] (int*)frame.pixels;
	}
}
