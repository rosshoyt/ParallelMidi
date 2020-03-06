/*
 ==============================================================================
 Testing MIDI-Based scan/reduce algorithms
 ==============================================================================
 */

#include <JuceHeader.h>
#include "FileUtils.h"

const char * MIDI_FILE_REL_PATH = "/source/book1-prelude01.mid";
const char * PROJECT_JUCER_FILENAME_FLAG = "Final-Project-ParAlgDev.jucer";

//=============================================================================

/**
 * Method that reads the NoteOn messages in a midi file into a single list
 */
std::vector<const MidiMessage*> getNoteList(MidiFile& midiFile, bool debug = false) {
    midiFile.convertTimestampTicksToSeconds();
    std::vector<const MidiMessage*> data; // list to store note events from all tracks
    // Read tracks
    for (int t = 0; t < midiFile.getNumTracks(); ++t) {
        if (debug) DBG("Scanning track #" + std::to_string(t));
        auto track = *midiFile.getTrack(t);
        // copy track note events
        for(int i = 0; i < track.getNumEvents(); ++i){
            MidiMessage* mm = &track.getEventPointer(i)->message;
            if (mm->isNoteOn()) {
                data.push_back(mm);
                if (debug)
                    DBG(String("Track #" + std::to_string(t)) + " @" + std::to_string(mm->getTimeStamp()) 
                        + " Note(" + std::to_string(mm->getNoteNumber()) + ")");
            }
        }
    }
    return data;
}

int main (int argc, char* argv[])
{
    auto fullPath = getProjectFullPath(PROJECT_JUCER_FILENAME_FLAG) + MIDI_FILE_REL_PATH;
    try{
        auto midiFile = readInMidiFile(fullPath);
        auto data = getNoteList(midiFile, true);
        // TODO Sequential scan of data
        // ...
        

    } catch(...){
        DBG("MIDI File was not read");
        return 1;
    }
    return 0;
}

