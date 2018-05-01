/**
 * PulseAudio output module for  Digiplay
 * @author Jonty Sewell <jontysewell@gmail.com>
 * @date May-2018
 */

#include "Output.h"

#include <iostream>
#include <string.h>
#include <unistd.h>
using namespace std;

#include <pulse/simple.h>

#include "OutputDspPulseAudio.h"

namespace Audio {

    /**
     * Dynamic loader entry point
     */
    extern "C" {
        Output * OUTPUT_SO_ENTRY(const char *device) {
            string deviceName = device;
            return new OutputDspPulseAudio(deviceName);
        }
    };

    OutputDspPulseAudio::OutputDspPulseAudio(string channel) {
        ss.format = PA_SAMPLE_S16LE;
        ss.channels = 2;
        ss.rate = 44100;
        s = pa_simple_new(
            NULL,               // Use the default server.
            channel.c_str(),    // Our application's name.
            PA_STREAM_PLAYBACK, 
            NULL,               // Use the default device.
            "Music",            // Description of our stream.
            &ss,                // Our sample format.
            NULL,               // Use default channel map
            NULL,               // Use default buffering attributes.
            NULL                // Ignore error code.
        );
    }

    OutputDspPulseAudio::~OutputDspPulseAudio() {
        pa_simple_drain(s, 0);
        pa_simple_free(s);
    }

    void OutputDspPulseAudio::receiveMessage(PORT inPort, MESSAGE message) {
        if (inPort != IN0) {
            cout << "OutputDspPulseAudio::receive: only use IN0 on a DSP device" << endl;
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

    void OutputDspPulseAudio::onPatch(PORT localPort) {

    }

    void OutputDspPulseAudio::onUnpatch(PORT localPort) {

    }

    void OutputDspPulseAudio::threadExecute() {
       if (!connectedDevice(IN0))
            cout << "CONNECTED DEVICE IS NULL" << endl;

        AudioPacket *buffer = new AudioPacket(PACKET_SAMPLES);
        SAMPLEVAL *d = buffer->getData();
        
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

            C->getAudio(buffer);
            pa_simple_write(s, d, buffer->getSize()*2, 0);
        }
        delete buffer;
    }
}
