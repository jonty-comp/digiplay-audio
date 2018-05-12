#ifndef CLASS_AUDIO_INPUT_FILE
#define CLASS_AUDIO_INPUT_FILE

#include <string>
using std::string;

#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>

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

    private:
        /// Caches the audio in a separate thread
        void threadExecute();

        int sample_rate;

        AVFormatContext* format;
        AVStream* stream;
        AVCodecContext* codec;
        struct SwrContext* swr;

        AVPacket packet;
        AVFrame* frame;

        unsigned int cache_size;
        bool allCached;
};

#endif
