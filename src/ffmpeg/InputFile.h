#ifndef CLASS_AUDIO_INPUT_FILE
#define CLASS_AUDIO_INPUT_FILE

#include <string>
using std::string;

extern "C"{
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "Input.h"

/**
 * Class to read audio from a file.
 * Uses a dynamically loaded implementation.
 */
class Audio::InputFile : public Audio::Input {
    public:
        InputFile(unsigned int cache_size = 1760000);
        ~InputFile();

        virtual void load(string filename, long start_smpl, long end_smpl);

        /// Check if a file is currently loaded
        virtual bool isLoaded();
        /// Processes messages received by this component
        virtual void receiveMessage(PORT inPort, MESSAGE message);
        /// Perform any initialisation tasks required on connection
        virtual void onPatch(PORT localPort);
        /// Perform any uninitialisation tasks required on disconnection
        virtual void onUnpatch(PORT localPort);

    private:
        /// Caches the audio in a separate thread
        void threadExecute();

        int sample_rate;
        int data_size;

        AVFormatContext* format;
        AVStream* stream;
        AVCodecContext* codec;

        AVPacket packet;
        AVFrame* frame;

        unsigned int cache_size;
        bool allCached;
};

#endif
