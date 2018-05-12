#include <vector>
#include <algorithm>
#include <unistd.h>

#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>


#include "Counter.h"
#include "CircularCache.h"
#include "InputFile.h"
using Audio::InputFile;
/**
 * @param   cacheSize           Size of cache to use.
 */
InputFile::InputFile(unsigned int cacheSize) :
        Input(cacheSize) {
    cache_size = cacheSize;

    int retval = threadStart();
    if (retval != 0) {
        cout << "ERROR: Error creating thread. Error code: " << retval << endl;
        throw -1;
    }
    allCached = false;
    sample_rate = 44100;

    // initialize all muxers, demuxers and protocols for libavformat
    // (does nothing if called twice during the course of one program execution)
    av_register_all();
}


/**
 * Destroy implementation and clean up.
 */
InputFile::~InputFile() {
    av_frame_free(&frame);
    swr_free(&swr);
    avcodec_close(codec);
    avformat_free_context(format);
}

/**
 * Loads an audio file. The format is determined from the filename extension.
 */
void InputFile::load(string filename, long start_smpl, long end_smpl) {
    if (filename == "") return;

    // If we're not stopped, change to a stopped state
    if (state != STATE_STOP) {
        // If all the audio fits into cache, no need to restart caching
        if (!allCached) stopCaching();
        
        // Change to stopped state.
        state = STATE_STOP;
        send(OUT0,STOP);
        updateStates(STATE_STOP);
    }
 
    // get format from audio file
    format = avformat_alloc_context();
    if (avformat_open_input(&format, filename.c_str(), NULL, NULL) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", filename.c_str());
        throw -1;
    }
    if (avformat_find_stream_info(format, NULL) < 0) {
        fprintf(stderr, "Could not retrieve stream info from file '%s'\n", filename.c_str());
        throw -1;
    }
 
    // Find the index of the first audio stream
    int stream_index =- 1;
    for (uint i=0; i<format->nb_streams; i++) {
        if (format->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            stream_index = i;
            break;
        }
    }
    if (stream_index == -1) {
        fprintf(stderr, "Could not retrieve audio stream from file '%s'\n", filename.c_str());
        throw -1;
    }
    stream = format->streams[stream_index];
 
    // find & open codec
    codec = stream->codec;
    if (avcodec_open2(codec, avcodec_find_decoder(codec->codec_id), NULL) < 0) {
        fprintf(stderr, "Failed to open decoder for stream #%u in file '%s'\n", stream_index, filename.c_str());
        throw -1;
    }
 
    // prepare resampler
    swr = swr_alloc();
    av_opt_set_int(swr, "in_channel_count",  codec->channels, 0);
    av_opt_set_int(swr, "out_channel_count", 1, 0);
    av_opt_set_int(swr, "in_channel_layout",  codec->channel_layout, 0);
    av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
    av_opt_set_int(swr, "in_sample_rate", codec->sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate", sample_rate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt",  codec->sample_fmt, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_DBL,  0);
    swr_init(swr);
    if (!swr_is_initialized(swr)) {
        fprintf(stderr, "Resampler has not been properly initialized\n");
        throw -1;
    }

    // Initialise position variables, counters and reset cache
    f_filename = filename;
    f_start_byte = start_smpl * 4;
    f_end_byte = end_smpl * 4;
    f_length_byte = f_end_byte - f_start_byte*4; //f_end_byte - f_start_byte;

    // If we're just resetting after a stop, try to seek to beginning of cache
    try {
        if (mCache->size() > f_length_byte) { 
            mCache->seek(-(f_pos_byte - f_start_byte));
        }
        else {
            throw -1;
        }
    }
    catch (int e) {
        // Otherwise clear the cache and start again
        mCache->clear();
    }
    f_pos_byte = f_start_byte;
    
    updateCounters(0);
    updateTotals(f_length_byte / 4);

    try {
        if (!allCached) startCaching();
    }
    catch (int e) {
        throw -1;
    }

    // Flag true if the whole track fits into cache
    // Can then ensure we only cache the audio once.
    allCached = (f_length_byte <= mCache->size()) ? true : false;
}

bool InputFile::isLoaded() {
    return false;
}

void InputFile::threadExecute() {
    while (!threadTestKill()) {

        // Wait until told to start caching a file
        if (!threadReceive(START)) {
            // purge any STOP commands while stopped
            threadReceive(STOP);
            usleep(10000);
            continue;
        }

        // Set caching state as active
        cacheStateLock.lock();
        cacheState = CACHE_STATE_ACTIVE;
        cacheStateLock.unlock();

        loaded = true;

        /**************************************************
         * CACHE AUDIO FILE
         **************************************************/
        while ( !threadTestKill() &&            // Thread not terminated
                !threadReceive(STOP)) {         // Not told to stop

            // Handle seek requests first
            // TODO implement seek
            if (threadReceive(SEEK)) {
                mCache->clear();
                
            }

            // Sleep if cache is full
            // TODO figure out how to tell when the cache is full
            if (mCache->free() < 1 * 2) {
                usleep(10000);
                continue;
            }

            // prepare to read data
            av_init_packet(&packet);
            
            frame = av_frame_alloc();
            if (!frame) {
                fprintf(stderr, "Error allocating the frame\n");
                throw -1;
            }
        
            if(av_read_frame(format, &packet) != 0) {
                // EOF or some other error
                usleep(10000);
            }
            int gotFrame;
            if (avcodec_decode_audio4(codec, frame, &gotFrame, &packet) < 0) {
                break;
            }
            if (!gotFrame) {
                continue;
            }
            // resample frames
            char* buffer;
            av_samples_alloc((uint8_t**) &buffer, NULL, 1, frame->nb_samples, AV_SAMPLE_FMT_DBL, 0);
            int frame_count = swr_convert(swr, (uint8_t**) &buffer, frame->nb_samples, (const uint8_t**) frame->data, frame->nb_samples);

            mCache->write(frame_count, buffer);
        }

        // Close the file and delete
        loaded = false;

        // Set cache state to inactive
        cacheStateLock.lock();
        cacheState = CACHE_STATE_INACTIVE;
        cacheStateLock.unlock();
    }
}
