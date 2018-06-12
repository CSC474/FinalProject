// For reading in audio using OpenAL and ALUT - Samantha Gunzl

#include "Audio.h"

using std::list;

void Audio::initAL() {
    audioDevice = alcOpenDevice(NULL); // Request default audio device
    errorCode = alcGetError(audioDevice);
    audioContext = alcCreateContext(audioDevice,NULL); // Create the audio context
    alcMakeContextCurrent(audioContext);
    errorCode = alcGetError(audioDevice);
    // Request the default capture device with a half-second buffer
    inputDevice = alcCaptureOpenDevice(NULL,FREQ,AL_FORMAT_MONO16,FREQ/2);
    errorCode = alcGetError(inputDevice);
    alcCaptureStart(inputDevice); // Begin capturing
    errorCode = alcGetError(inputDevice);
    
    alGenBuffers(16,&helloBuffer[0]); // Create some buffer-objects
    errorCode = alGetError();
    
    // Queue our buffers onto an STL list
    for (int ii=0;ii<16;++ii) {
        bufferQueue.push_back(helloBuffer[ii]);
    }
    
    alGenSources (1, &helloSource[0]); // Create a sound source
    errorCode = alGetError();
}

//make it return a short buffer
void Audio::readAudio() {
    alGetSourcei(helloSource[0],AL_BUFFERS_PROCESSED,&availBuffers);
    if (availBuffers>0) {
        alSourceUnqueueBuffers(helloSource[0],availBuffers,buffHolder);
        for (int ii=0;ii<availBuffers;++ii) {
            // Push the recovered buffers back on the queue
            bufferQueue.push_back(buffHolder[ii]);
        }
    }
    
    // Poll for captured audio
    alcGetIntegerv(inputDevice,ALC_CAPTURE_SAMPLES,1,&samplesIn);
    if (samplesIn>CAP_SIZE) {
        // Grab the sound
        alcCaptureSamples(inputDevice,buffer,CAP_SIZE);
        
        //***** Process/filter captured data here *****//
        
        // Stuff the captured data in a buffer-object ---- plays sound back
        if (!bufferQueue.empty()) { // We just drop the data if no buffers are available
            myBuff = bufferQueue.front(); bufferQueue.pop_front();
            alBufferData(myBuff,AL_FORMAT_MONO16,buffer,CAP_SIZE*sizeof(short),FREQ);
            
            // Queue the buffer
            alSourceQueueBuffers(helloSource[0],1,&myBuff);
            
            // Restart the source if needed
            ALint sState=0;
            alGetSourcei(helloSource[0],AL_SOURCE_STATE,&sState);
            
        }
    }
}

void Audio::cleanAL() {
    // Stop capture
    alcCaptureStop(inputDevice);
    alcCaptureCloseDevice(inputDevice);
    
    // Stop the sources
    alSourceStopv(1,&helloSource[0]);
    for (int ii=0;ii<1;++ii) {
        alSourcei(helloSource[ii],AL_BUFFER,0);
    }
    
    // Clean-up
    alDeleteSources(1,&helloSource[0]);
    alDeleteBuffers(16,&helloBuffer[0]);
    errorCode = alGetError();
    alcMakeContextCurrent(NULL);
    errorCode = alGetError();
    alcDestroyContext(audioContext);
    alcCloseDevice(audioDevice);
}



