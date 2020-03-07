/*
  ==============================================================================

    MidiUtils.h
    Created: 7 Mar 2020 1:35:55pm
    Author:  Ross Hoyt

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FileUtils.h"
#include "NoteHistoScan.h"


typedef struct Note {
    const MidiMessage * noteOn, noteOff;
    Note(const MidiMessage* noteOn, const MidiMessage* noteOff) {
        noteOn = noteOn;
        noteOff = noteOff;
    }
};

typedef std::multimap<double, Note> NoteMap;

//=============================================================================
/**
 * Method that reads a midi file from the provided full path.
 * @throws exception if file doesn't exist or MIDI file could not be read
 */
static MidiFile readInMidiFile(const String& path)
{
    File fileToRead(path);
    if (fileToRead.existsAsFile())
    {
        if (std::unique_ptr<FileInputStream> inputStream{ fileToRead.createInputStream() })
        {
            MidiFile midiFile;
            // read midi file and create matching note off messages (if not provided in file)
            if (midiFile.readFrom(*inputStream.get()), true) return midiFile;
        }
        throw "Error reading file";
    }
    throw "File doesn't exist at provided path: " + path;
}

/**
 * Reads the NoteOn messages from a midi file into a single list
 * @return notes in midi File
 */
static NoteMap getNoteMap(MidiFile& midiFile, bool debug = false) {
    midiFile.convertTimestampTicksToSeconds();
    NoteMap data;
    // Read tracks
    for (int t = 0; t < midiFile.getNumTracks(); ++t) {
        if (debug) DBG("Scanning track #" + std::to_string(t));
        auto track = *midiFile.getTrack(t);
        // copy track note events
        for(int i = 0; i < track.getNumEvents(); ++i){
            MidiMessage * midiMessage = &track.getEventPointer(i)->message;
            if (midiMessage->isNoteOn()) {
                MidiMessage* noteOff = &track.getEventPointer(track.getIndexOfMatchingKeyUp(i))->message;
                data.insert( { midiMessage->getTimeStamp(), Note(midiMessage, noteOff) } );
                if (debug) DBG(midiMessage->getDescription() + ", " + noteOff->getDescription());
            }
        }
    }
    return data;
}