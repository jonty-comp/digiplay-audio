#include "Output.h"

namespace Audio {
    /**
     * Local sub-class of Output to implement PulseAudio interface
     */
    class OutputDspPulseAudio : public Output {
        public:
            OutputDspPulseAudio(string device);
            virtual ~OutputDspPulseAudio();

            virtual void receiveMessage(PORT inPort, MESSAGE message);
            virtual void onPatch(PORT localPort);
            virtual void onUnpatch(PORT localPort);
            virtual void threadExecute();

        private:
            pa_simple *s;
            pa_sample_spec ss;

            enum STATE audioState;
    };
};
