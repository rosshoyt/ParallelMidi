/*
 ==============================================================================
 Testing MIDI-Based scan/reduce algorithms
 ==============================================================================
 */

#include <JuceHeader.h>
#include "FileUtils.h"
#include "NoteHistoScan.h"
const char * MIDI_FILE_REL_PATH = "/source/book1-prelude01.mid";
const char * PROJECT_JUCER_FILENAME_FLAG = "Final-Project-ParAlgDev.jucer";

typedef struct Note {
    const MidiMessage * noteOn, noteOff;
    Note(const MidiMessage* noteOn, const MidiMessage* noteOff) {
        noteOn = noteOn;
        noteOff = noteOff;
    }
};
//=============================================================================

/**
 * Method that reads the NoteOn messages in a midi file into a single list
 */
std::list<Note> getNoteList(MidiFile& midiFile, bool debug = false) {
    
    midiFile.convertTimestampTicksToSeconds();
    std::list<Note> data; // list to store note events from all tracks
    // Read tracks
    for (int t = 0; t < midiFile.getNumTracks(); ++t) {
        if (debug) DBG("Scanning track #" + std::to_string(t));
        auto track = *midiFile.getTrack(t);
        // copy track note events
        for(int i = 0; i < track.getNumEvents(); ++i){
            MidiMessage * midiMessage = &track.getEventPointer(i)->message;
            if (midiMessage->isNoteOn()) {
                MidiMessage* noteOff = &track.getEventPointer(track.getIndexOfMatchingKeyUp(i))->message;
                data.push_back(Note(midiMessage, noteOff));
                if (debug) DBG(midiMessage->getDescription() + ", " + noteOff->getDescription());
            }
        }
    }
    return data;
}


//std::list<NoteHisto> sortNoteList(std::list<Note> noteList) {}

int main (int argc, char* argv[])
{
    auto fullPath = getProjectFullPath(PROJECT_JUCER_FILENAME_FLAG) + MIDI_FILE_REL_PATH;
    try{
        auto data = getNoteList(readInMidiFile(fullPath), true);
        // TODO Sequential scan of data
        // ...

        for (auto noteOn : data) {
            
        }
        

    } catch(...){
        DBG("MIDI File was not read");
        return 1;
    }
    return 0;
}

