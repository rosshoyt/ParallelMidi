/*
 ==============================================================================
 Testing MIDI-Based scan/reduce algorithms
 ==============================================================================
 */

#include <JuceHeader.h>
#include "NoteHistoScan.h"

const char * MIDI_FILE_REL_PATH = "/source/book1-prelude01.mid";
const char * PROJECT_JUCER_FILENAME_FLAG = "Final-Project-ParAlgDev.jucer";

//==============================================================================

/**
 * Static method that returns the current project directory
 * @return String full path of the current project directory
 */
static String getProjectFullPath(bool debug = false)
{
    // get current project directory
    File dir = File::getCurrentWorkingDirectory();
    //while(dir.g)
    //for (int i = 0; i < 4; i++) {
    Array<File> matchedFiles;
    while (matchedFiles.size() == 0){
        dir = dir.getParentDirectory();
        matchedFiles = dir.findChildFiles(2, false, PROJECT_JUCER_FILENAME_FLAG);
    } 

    String fullPath = dir.getFullPathName();
    if(debug) DBG("Project full path - " + fullPath);
    return fullPath;
}

/**
 * Method that reads a midi file from the provided full path.
 * @throws std::exception if file doesn't exist or MIDI file could not be read
 */
static MidiFile readInMidiFile(const String& relativePath)
{
    File fileToRead(relativePath);
    if (fileToRead.existsAsFile())
    {
        MidiFile midiFile;
        if (std::unique_ptr<FileInputStream> inputStream{ fileToRead.createInputStream() })
        {
            if (midiFile.readFrom(*inputStream.get()))
                return midiFile;
        }
    }
    throw new std::exception;  // file doesn't exist
}

/**
 * Method that reads the NoteOn messages in a midi file into a single list
 */
std::vector<const MidiMessage*> getNoteList(MidiFile& midiFile, bool debug = false) {
    // TODO check if midifile is valid
    // prepare file
    midiFile.convertTimestampTicksToSeconds();

    // list to store note events from all tracks
    std::vector<const MidiMessage*> data;

    // read tracks 
    for (int t = 0; t < midiFile.getNumTracks(); ++t) {
        if (debug) DBG("Scanning track #" + std::to_string(t));
        auto track = *midiFile.getTrack(t);
        // copy track note events
        for(int i = 0; i < track.getNumEvents(); ++i){
            MidiMessage* mm = &track.getEventPointer(i)->message;
            if (mm->isNoteOn()) {
                data.push_back(mm);
                if (debug)
                    DBG(String("Track #" + std::to_string(t)) + " @" + std::to_string(mm->getTimeStamp()) + " Note(" + std::to_string(mm->getNoteNumber()) + ")");
            }
        }
    }
    return data;
}

int main (int argc, char* argv[])
{
    auto fullPath = getProjectFullPath() + MIDI_FILE_REL_PATH;
    try{
        auto midiFile = readInMidiFile(fullPath);
        auto data = getNoteList(midiFile);
        // TODO Sequential scan of data
        // ...
        

    } catch(...){
        DBG("MIDI File was not read");
        return 1;
    }
    return 0;
}

