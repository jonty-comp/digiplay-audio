#include <iostream>
#include <unistd.h>
using namespace std;

#include "Audio.h"
#include "ffmpeg/InputFile.h"
#include "OutputDsp.h"
#include "ProcessMixer.h"
using namespace Audio;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: audioplay <file> <device>" << endl;
    }
	// Create input sources and load
	InputFile* filereader1 = new InputFile();
    ProcessMixer* mixer = new ProcessMixer();
	// Create a DSP output
	OutputDsp* player = new OutputDsp(argv[2]);
	
	// Connect everything up
    filereader1->load(string(argv[1]),0,441000000);
	filereader1->patch(OUT0,mixer,IN0);
    mixer->patch(OUT0,player,IN0);
    // Load track, and play it

	filereader1->play();
	sleep(1000);
}