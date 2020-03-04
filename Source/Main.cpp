/*
  ==============================================================================
    Testing MIDI File scan/reduce algorithms 
  ==============================================================================
*/

#include <JuceHeader.h>
#include "GeneralScanRecursive.h"

// TODO make midi file path portable 
const char * MIDI_FILE_PATH_WIN = "C:\\Users\\Ross Hoyt\\SU\\WQ2020\\CPSC5600\\Final-Project-ParAlgDev\\Source\\book1-prelude01.mid";
//==============================================================================

int main (int argc, char* argv[])
{
    // read midi file
    MidiFile midiFile;
    File file(MIDI_FILE_PATH_WIN);
    midiFile.readFrom(FileInputStream(file));
    midiFile.convertTimestampTicksToSeconds();
    
    // single list to store notes from all tracks
    std::vector<const MidiMessage*> data;
    
    // sequentially read each track's NoteOn events into data list 
    int nTracks = midiFile.getNumTracks();  
    MidiMessageSequence* tracks = new MidiMessageSequence[nTracks];
    for (int t = 0; t < nTracks; ++t) {
        DBG("Scanning track #" + t);
        tracks[t] = *midiFile.getTrack(t);

        // sequentially read each note eventin track
        int i = 0;
        int nEvents = tracks[t].getNumEvents();
        while (i < nEvents) {
            MidiMessage* mm = &tracks[t].getEventPointer(i)->message;
            if (mm->isNoteOn()) {
                data.push_back(mm);
                DBG(String("Track #" + std::to_string(t)) + " @" + std::to_string(mm->getTimeStamp()) + " Note(" + std::to_string(mm->getNoteNumber()) + ")");
            }
            ++i;
        }
    }
    delete[] tracks;
    return 0;
}
