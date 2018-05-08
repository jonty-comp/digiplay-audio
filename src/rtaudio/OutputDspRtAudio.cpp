/**
 * RtAudio output module for  Digiplay
 * @author Jonty Sewell <jontysewell@gmail.com>
 * @date May-2018
 */

#include "Output.h"

#include <iostream>
#include <string.h>
#include <unistd.h>
using namespace std;

#include "rtaudio/RtAudio.h"

#include "OutputDspRtAudio.h"

namespace Audio {

    /**
     * Dynamic loader entry point
     */
    extern "C" {
        Output * OUTPUT_SO_ENTRY(const char *device) {
            string deviceName = device;
            return new OutputDspRtAudio(deviceName);
        }
    };

    OutputDspRtAudio::OutputDspRtAudio(string channel) {

        parameters.deviceId = dac.getDefaultOutputDevice();
        parameters.nChannels = 2;
        parameters.firstChannel = 0;

        unsigned int sampleRate = 44100;
        bufferFrames = PACKET_SAMPLES;

        try {
            dac.openStream( &parameters, NULL, RTAUDIO_SINT16,
                            sampleRate, &bufferFrames, callback, this);
            dac.startStream();
        }
        catch ( RtAudioError& e ) {
            e.printMessage();
            exit( 0 );
        }

        // Create buffer synchronisation object
        sync = new pthread_cond_t;
        pthread_cond_init(sync, NULL);
        bufReady = 0;
    }

    OutputDspRtAudio::~OutputDspRtAudio() {
        try {
            dac.stopStream();
        }
        catch (RtAudioError& e) {
            e.printMessage();
        }
        if ( dac.isStreamOpen() ) dac.closeStream();
    }

    void OutputDspRtAudio::receiveMessage(PORT inPort, MESSAGE message) {
        if (inPort != IN0) {
            cout << "OutputDspRtAudio::receive: only use IN0 on a DSP device" << endl;
            return;
        }
        switch (message) {
            case PLAY:
                audioState = STATE_PLAY;
                if (!isThreadActive()) threadStart();
                break;
            case PAUSE:
                audioState = STATE_PAUSE;
                break;
            case STOP:
                audioState = STATE_STOP;
                break;
            default:
                break;
        }
    }

    void OutputDspRtAudio::onPatch(PORT localPort) {

    }

    void OutputDspRtAudio::onUnpatch(PORT localPort) {

    }

    void OutputDspRtAudio::threadExecute() {
       if (!connectedDevice(IN0))
            cout << "CONNECTED DEVICE IS NULL" << endl;

        AudioPacket *buffer = new AudioPacket(PACKET_SAMPLES);
        SAMPLEVAL *d = buffer->getData();
        int size = buffer->getSize();
        dbuf = new SAMPLEVAL[bufferFrames];
        ComponentAudio *C;

        while (!threadTestKill()) {
            if (audioState == STATE_STOP) {
                usleep(10000);
                continue;
            }

            if (audioState == STATE_PAUSE) {
                usleep(1000);
                continue;
            }

            if (!(C = dynamic_cast<ComponentAudio *>(connectedDevice(IN0)))) {
                usleep(10000);
                continue;
            }

            unsigned int samples = 0;
            while (samples < bufferFrames) {
                C->getAudio(buffer);
                memcpy(dbuf+samples, d, size);
                samples = samples + size;
            }
            bufReady = 1;

            pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
            pthread_cond_wait(sync, &mutex);

        }
        delete buffer;
    }

    int OutputDspRtAudio::callback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData ) {

        SAMPLEVAL *buffer = (SAMPLEVAL *) outputBuffer;

        if ( status )
            std::cout << "XRUN" << std::endl;

        OutputDspRtAudio *me = (OutputDspRtAudio *)userData;
        return me->process(buffer, nBufferFrames);
    } 

    int OutputDspRtAudio::process(SAMPLEVAL *buffer, unsigned int nBufferFrames) {
        
        // Write interleaved audio data.
        if (bufReady && dbuf && nBufferFrames == bufferFrames) {
            memcpy(buffer, dbuf, nBufferFrames);
            bufReady = 0;
        } else {
            for(SAMPLE n=0; n<nBufferFrames; n++) {
                buffer[n] = 0;
            }
        }

        pthread_cond_signal(sync);
        return 0;
    }
}
