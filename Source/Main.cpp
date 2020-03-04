/*
  ==============================================================================
    Testing MIDI File scan/reduce algorithms 
  ==============================================================================
*/

#include <JuceHeader.h>
#include "GeneralScanRecursive.h"

const char * MIDI_FILE_PATH = "C:\\Users\\Ross Hoyt\\SU\\WQ2020\\CPSC5600\\Final-Project-ParAlgDev\\Source\\book1-prelude01.mid";
//==============================================================================
int main (int argc, char* argv[])
{
    MidiFile midiFile;
    File file(MIDI_FILE_PATH);
    midiFile.readFrom(FileInputStream(file));
    DBG(midiFile.getLastTimestamp());
    return 0;
}
