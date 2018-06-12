//  Audio.h

#ifndef Audio_h
#define Audio_h

#include "OpenAL/OpenAL.h"
#include "OpenAL/al.h"
#include "OpenAL/alc.h"
#include "AL/alut.h"
#include <list>

#define FREQ 22050   // Sample rate
#define CAP_SIZE 2048 // How much to capture at a time (affects latency)

using std::list;

class Audio {
    
public:
    void initAL();
    void cleanAL();
    void readAudio();
    short buffer[FREQ*2]; // A buffer to hold captured audio
    
private:
    ALuint helloBuffer[16], helloSource[1];
    ALint availBuffers=0; // Buffers to be recovered
    ALCint samplesIn=0;  // How many samples are captured
    ALuint myBuff; // The buffer we're using
    ALuint buffHolder[16]; // An array to hold catch the unqueued buffers
    list<ALuint> bufferQueue; // A quick and dirty queue of buffer objects
    ALCdevice* inputDevice;
    ALCcontext* audioContext;
    ALenum errorCode=0;
    ALCdevice* audioDevice;
};


#endif /* Audio_h */
