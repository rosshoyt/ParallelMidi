/*
 ==============================================================================
 Testing MIDI-Based scan/reduce algorithms
 ==============================================================================
 */

#include <JuceHeader.h>
#include "NoteHistoScan.h"

const char * MIDI_FILE_REL_PATH = "/source/book1-prelude01.mid";
//==============================================================================

/**
 * Method that reads a midi file from the provided full path.
 * @throws std::exception if file doesn't exist or MIDI file could not be read
 */
static MidiFile readMidiFile (const String &relativePath)
{
    File fileToRead(relativePath);
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

/**
 * Method that reads the NoteOn messages in a midi file into a single list
 */
std::vector<const MidiMessage*> getNoteList(MidiFile& midiFile, bool debug = false){
    
    midiFile.convertTimestampTicksToSeconds();
    
    // single list to store notes from all tracks
    std::vector<const MidiMessage*> data;
    
    // sequentially read each track's NoteOn events into data list
    int nTracks = midiFile.getNumTracks();
    MidiMessageSequence* tracks = new MidiMessageSequence[nTracks];
    for (int t = 0; t < nTracks; ++t) {
        if(debug) DBG("Scanning track #" + std::to_string(t));
        tracks[t] = *midiFile.getTrack(t);
        
        // sequentially read each note eventin track
        int i = 0;
        int nEvents = tracks[t].getNumEvents();
        while (i < nEvents) {
            MidiMessage* mm = &tracks[t].getEventPointer(i)->message;
            if (mm->isNoteOn()) {
                data.push_back(mm);
                if(debug)
                    DBG(String("Track #" + std::to_string(t)) + " @" + std::to_string(mm->getTimeStamp()) + " Note(" + std::to_string(mm->getNoteNumber()) + ")");
            }
            ++i;
        }
    }
    delete[] tracks;
    return data;
}

/**
 * Static method that returns the current project directory
 * @return String full path of the current project directory
 */
static String getProjectFullPath(bool debug = false)
{
    // get current project directory
    File dir = File::getCurrentWorkingDirectory();
    for(int i = 0; i < 4; i++) dir = dir.getParentDirectory();
    String fullPath = dir.getFullPathName();
    if(debug) DBG("Project full path - " + fullPath);
    return fullPath;
}

int main (int argc, char* argv[])
{
    String fullPath = getProjectFullPath() + MIDI_FILE_REL_PATH;
    try{
        MidiFile midiFile = readMidiFile(fullPath);
        auto data = getNoteList(midiFile);
        // TODO Sequential scan of data
        // ...
    } catch(...){
        DBG("MIDI File was not read");
        return 1;
    }
    return 0;
}

