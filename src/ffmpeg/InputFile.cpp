#include <vector>
#include <algorithm>
#include <unistd.h>

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
    av_log_set_level(AV_LOG_DEBUG);
}


/**
 * Destroy implementation and clean up.
 */
InputFile::~InputFile() {
    av_frame_free(&frame);
    avcodec_free_context(&codec);
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
        if (format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
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
    AVCodec* c = avcodec_find_decoder(stream->codecpar->codec_id);
    codec = avcodec_alloc_context3(c);
    if (avcodec_open2(codec, c, NULL) < 0) {
        fprintf(stderr, "Failed to open decoder for stream #%u in file '%s'\n", stream_index, filename.c_str());
        throw -1;
    }
    data_size = av_get_bytes_per_sample(codec->sample_fmt);

    // Initialise position variables, counters and reset cache
    f_filename = filename;
    f_start_byte = start_smpl * data_size;
    f_end_byte = end_smpl * data_size;
    if (stream->duration * data_size < (off_t)f_end_byte || f_end_byte==0) f_end_byte = stream->duration * data_size;
    f_length_byte = f_end_byte - f_start_byte * data_size;

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
    updateTotals(f_length_byte / data_size);

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


/**
 * Processes messages received from other audio components.
 * @param   inPort      Port on which message is received
 * @param   message     The message received
 */
void InputFile::receiveMessage(PORT inPort, MESSAGE message) {

}


/**
 * Perform tasks when a component connects to this component
 * @param   localPort   Port to which another component connects.
 */
void InputFile::onPatch(PORT localPort) {

}


/**
 * Perform tasks when a component disconnects from this component
 * @param   localPort   Port on which another component has disconnected
 */
void InputFile::onUnpatch(PORT localPort) {

}

void InputFile::threadExecute() {
    while (!threadTestKill()) {
        bool atEnd = false;

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
                f_pos_byte = f_seek_byte;
                // av_seek_summat
                atEnd = false;
            }

            // We pause caching when free cache drops to below twice the frame size,
            // or we have reached the end of file
            if (mCache->free() < 8192*2) {
                usleep(100);
                continue;
            }

            // prepare to read data
            av_init_packet(&packet);
            
            frame = av_frame_alloc();
            if (!frame) {
                fprintf(stderr, "Error allocating the frame\n");
                throw -1;
            }

            int ret;

            ret = av_read_frame(format, &packet);
            if (ret == AVERROR_EOF) {
                atEnd = true;
                continue;
            } else if (ret < 0) {
                fprintf(stderr, "Could not read frame\n");
                throw -1;
            }
            
            ret = avcodec_send_packet(codec, &packet);
            if (ret < 0) {
                fprintf(stderr, "Error submitting the packet to the decoder\n");
                throw -1;
            }

            ret = avcodec_receive_frame(codec, frame);
            if (ret == AVERROR(EAGAIN)) {
                // more data required to decode the frame
                continue;
            } else if (ret == AVERROR_EOF) {
                // no more data in file
                atEnd = true;
                continue;
            } else if (ret < 0) {
                fprintf(stderr, "Could not decode frame\n");
                throw -1;
            }

            mCache->write(frame->linesize[0], (char* ) frame->data);

            if (mCache->size() - mCache->free() > preCacheSize) {
                usleep(100);
            }

            av_packet_unref(&packet);
        }

        // Set cache state to inactive
        cacheStateLock.lock();
        cacheState = CACHE_STATE_INACTIVE;
        cacheStateLock.unlock();
    }
}
