/*
 ==============================================================================
 Testing MIDI File scan/reduce algorithms
 ==============================================================================
 */

#include <JuceHeader.h>
#include "GeneralScanRecursive.h"

const char * MIDI_FILE_REL_PATH = "/source/book1-prelude01.mid";
//==============================================================================

/**
 * Method that reads a midi file from the provided full path.
 * @throws std::exception if file doesn't exist or MIDI file could not be read
 */
MidiFile readMidiFile (const String &fullPath)
{
    File fileToRead(fullPath);
    if (fileToRead.existsAsFile())
    {
        MidiFile midiFile;
        if (std::unique_ptr<FileInputStream> inputStream { fileToRead.createInputStream() })
        {
            if(midiFile.readFrom(*inputStream.get()))
                return midiFile;
        }
    }
    throw new std::exception;  // file doesn't exist
}

int main (int argc, char* argv[])
{
    // get current project directory
    File dir = File::getCurrentWorkingDirectory();
    for(int i = 0; i < 4; i++) dir = dir.getParentDirectory();
    String fullPath = dir.getFullPathName();
    DBG("Current working dir = " + fullPath);
    // append relative Midi file location to project dir
    fullPath.append(MIDI_FILE_REL_PATH, 50);
    
    // read MIDI file
    try{
        MidiFile midiFile = readMidiFile(fullPath);
        
        midiFile.convertTimestampTicksToSeconds();
        
        // single list to store notes from all tracks
        std::vector<const MidiMessage*> data;
        
        // sequentially read each track's NoteOn events into data list
        int nTracks = midiFile.getNumTracks();
        MidiMessageSequence* tracks = new MidiMessageSequence[nTracks];
        for (int t = 0; t < nTracks; ++t) {
            DBG("Scanning track #" + std::to_string(t));
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
    } catch(...){
        return 1; // MIDI FILE was not read
    }
}

