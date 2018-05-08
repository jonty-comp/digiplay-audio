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

    /**
     * @brief Construct a new RtAudio Output
     * 
     * @param channel The device to connect to - if a / with an int is
     *                appended then offset to that channel within the device
     */
    OutputDspRtAudio::OutputDspRtAudio(string channel) {
        // Split off a channel offset if one is supplied
        int o = channel.find("/");
        int offset;
        if (o < 0) {
            offset = 0;
        } else {
            offset = atoi(channel.substr(o+1).c_str());
            channel = channel.substr(0, o);
        }

        unsigned int devices = dac.getDeviceCount();
        RtAudio::DeviceInfo info;
        int device = -1;
        for (unsigned int i=0; i<devices; i++) {
            info = dac.getDeviceInfo(i);
            if (info.probed == true) {
                if (info.name == channel) device = i;
            }
        }

        if( device == -1 ) {
            cout << "OutputDspRtAudio: Device " << channel.c_str() << " not found." << endl;
            cout << "Possible devices: " << endl;
            for (unsigned int i=0; i<devices; i++) {
                info = dac.getDeviceInfo(i);
                if (info.probed == true) {
                    cout << i << ": " << info.name << endl;                    
                }
            }
            throw 0;
        }

        parameters.deviceId = device;
        parameters.nChannels = 2;
        parameters.firstChannel = offset;

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
        SAMPLE size = buffer->getSize();
        dbuf = new SAMPLEVAL[bufferFrames * 2];
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

            SAMPLE samples = 0;
            while (samples < bufferFrames * 2) {
                C->getAudio(buffer);
                for(SAMPLE n=0; n<size; n++) {
                    dbuf[samples+n] = d[n];
                }
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
            for(SAMPLE n=0; n<bufferFrames * 2; n++) {
                buffer[n] = dbuf[n];
            }
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
