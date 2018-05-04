#include "OutputFile.h"
using Audio::OutputFile;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <dlfcn.h>

OutputFile::OutputFile(string filename) {
    this->filename = filename;
    unsigned int len = filename.size();
    string libName = "";

    #ifdef _WIN32
        const string libExt = ".dll";
    #elif __APPLE__
        const string libExt = ".dylib";
    #elif __linux__
        const string libExt = ".so";
    #endif

    if (filename.substr(len-4,4) == "flac") {
        libName = "libdpsaudio-flac";
    }
    else if (filename.substr(len-3,3) == "mp3") {
        libName = "libdpsaudio-mp3";
    }
    else {
        libName = "libdpsaudio-raw";
    }
    
    dlHandle = dlopen((libName+libExt).c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!dlHandle) {
        cerr << "Module Error in " << libName << ": " << dlerror() << endl;
        throw 0;
    }

    OutputFileSO_Entry entry = (OutputFileSO_Entry)dlsym(dlHandle, OUTPUT_SO_SYM);
    if (!entry) {
        cerr << "No entry point in module: " << libName << endl;
        dlclose(dlHandle);
        throw 0;
    }
    cout << "Connecting..." << endl;
    pImpl = entry(filename, this);
    if (!pImpl) {
        cerr << "Module failed to create input file object." << endl;
        dlclose(dlHandle);
        throw 0;
    }
}

OutputFile::~OutputFile() {
    if (pImpl) {
        cout << "Unloading..." << endl;
        delete pImpl;
    }
    if (dlHandle) {
        dlclose(dlHandle);
    }
}

void OutputFile::receiveMessage(PORT inPort, MESSAGE message) {
    if (pImpl) {
        pImpl->receiveMessage(inPort, message);
    }
}

void OutputFile::onPatch(PORT localPort) {
    if (pImpl) {
        pImpl->onPatch(localPort);
    }
}

void OutputFile::onUnpatch(PORT localPort) {
    if (pImpl) {
        pImpl->onUnpatch(localPort);
    }
}
