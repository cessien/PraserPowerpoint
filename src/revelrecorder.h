/*
 * revelrecorder.h
 *
 *  Created on: Mar 6, 2012
 *      Author: charles
 */
#include <revel.h>

void LoadAudio(bool *hasAudio, int *audioBits, int *audioChannels,
               int *audioFormat, int *audioRate, char **audioBuffer,
               int *audioBufferSize);

void initRecord();

void processQueue();

void recordFrame(unsigned char * data);

void endRecord();
