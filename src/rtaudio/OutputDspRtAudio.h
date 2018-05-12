#include "Output.h"

namespace Audio {
    /**
     * Local sub-class of Output to implement RtAudio interface
     */
    class OutputDspRtAudio : public Output {
        public:
            OutputDspRtAudio(string device);
            virtual ~OutputDspRtAudio();

            virtual void receiveMessage(PORT inPort, MESSAGE message);
            virtual void onPatch(PORT localPort);
            virtual void onUnpatch(PORT localPort);
            virtual void threadExecute();

        private:
            RtAudio dac;
            RtAudio::StreamParameters parameters;
            SAMPLEVAL *dbuf;
            unsigned int bufferFrames;
            volatile int bufReady;

            static int callback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                double streamTime, RtAudioStreamStatus status, void *userData);
            int process(SAMPLEVAL *buffer, unsigned int nBufferFrames);

            enum STATE audioState;
            pthread_cond_t *sync;            
    };
};
