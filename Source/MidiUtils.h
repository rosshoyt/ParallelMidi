/*
  ==============================================================================

    MidiUtils.h
    Created: 7 Mar 2020 1:35:55pm
    Author:  Ross Hoyt

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <iterator>
#include <algorithm>
#include <list>
#include "FileUtils.h"
#include "NoteHistoScan.h"

// program constants
static const int NUM_MIDI_NOTES = 128;
static const int NUM_NOTES_OCTAVE = 12;

// the raw data components of the scan
typedef MidiMessage NoteOn;
typedef MidiMessage NoteOff;

// A note is made up of a noteOn and its corresponding noteOff..
// Assumes complete midi file with matching noteOns/noteOffs
struct Note 
{ 
    Note(NoteOn & noteOn, NoteOff & noteOff)
    {
        this->noteOn = noteOn;
        this->noteOff = noteOff;
    }

    NoteOn noteOn; 
    NoteOff noteOff; 
    /**
     * Copy constructor for Note
     */
    Note(const Note& n) : noteOn(n.noteOn), noteOff(n.noteOff) {}
    
    // TODO implement operator equals, + etc.
};

// The container for all the midi notes in a midi file or collection of 
// midi files organized by timestamp key
typedef std::multimap<double, Note> NoteMap;

// A single frame in the midi heatmap list
struct HeatmapFrame 
{ 
    HeatmapFrame(double timestamp) : additions(), subtractions(), timestamp(timestamp) {}

    void addNoteEvent(int noteNumber, bool noteOn) {
        if (noteOn) additions.push_back(noteNumber);
        else subtractions.push_back(noteNumber);
    }

    double timestamp; 
    std::list<int> additions, subtractions;
};

// Final result of the scan which is used to animate the heatmap in order
typedef std::list<HeatmapFrame> HeatmapList;

/**
 * Reads the NoteOn messages from a midi file into a single list
 * @return notes in midi File
 */
static NoteMap getNoteMap(MidiFile& midiFile)
{
    midiFile.convertTimestampTicksToSeconds();
    NoteMap data;
    // Read tracks
    for (int t = 0; t < midiFile.getNumTracks(); ++t) {
        auto track = *midiFile.getTrack(t);
        // Copy track note events
        for(int i = 0; i < track.getNumEvents(); ++i){
            MidiMessage midiMessage = track.getEventPointer(i)->message;
            if (midiMessage.isNoteOn()) {
                MidiMessage noteOff = track.getEventPointer(track.getIndexOfMatchingKeyUp(i))->message;
                Note note( midiMessage, noteOff);
                //DBG("Adding note with NoteOn = " + std::to_string(note.noteOn->getTimeStamp())
                //    + " and NoteOff = " + std::to_string(note.noteOff->getTimeStamp()));

                data.insert( { note.noteOn.getTimeStamp(), note }) ;
            }
        }
    }
    for (auto & iter : data)
    {
        DBG("DataMap - Note #" + std::to_string(iter.second.noteOn.getNoteNumber()) + " @ map key: " + std::to_string(iter.first) + " NoteOn = " 
            + std::to_string(iter.second.noteOn.getTimeStamp()) + " and NoteOff = " + std::to_string(iter.second.noteOff.getTimeStamp()));
    }
    return data;
}

/**
 * Utility method that retrieves the notes at a given timestamp from a NoteMap
 * TODO move method to NoteMap class
 */
static std::list<Note> getNotesAtTimeStamp(double timestamp, const NoteMap& noteMap)
{
    //DBG("Getting Notes at timestamp " + std::to_string(timestamp));
    auto & itr1 = noteMap.lower_bound(timestamp), & itr2 = noteMap.upper_bound(timestamp);
    std::list<Note> notes;
    while (itr1 != itr2)
    {
        if (itr1->first == timestamp)
            notes.push_back(itr1->second);
        itr1++;
    }
    DBG("Found " + std::to_string(notes.size()) + " notes");
    return notes;
}

/**
 * Creates a NoteHeatMap from a NoteMap.
 * TODO assumes MidiFile doesn't start with a NoteOff
 */
static HeatmapList * scanNoteMap(NoteMap & inputNoteMap)
{
    HeatmapList * noteHeatMap = new HeatmapList();

    int dbgCounter = 1;
    
    NoteMap pendingNoteOffMap; // holds noteOffs that will also create heatmaps when they update
    // go through each note in order by timestamp. can be multiple notes per timestamp
    auto & iter = inputNoteMap.begin();
    while (iter != inputNoteMap.end())
    {
        DBG("------------LOOP #" + std::to_string(dbgCounter) + "-----------");
        auto timestamp = iter->first; // time off the current noteOn event we're looking at        
        
        // Create heatmap for this new timestamp
        HeatmapFrame frame(timestamp);
        
        // Add all noteOns at the current timestamp into the heatmap
        // First, get all the noteOns at this timestamp
        DBG("Getting noteOns at " + std::to_string(timestamp));
        auto noteOnList = getNotesAtTimeStamp(timestamp, inputNoteMap);
        for (Note & n : noteOnList)
        {
            int midiNoteNumber(n.noteOn.getNoteNumber());
            DBG("Adding noteOn " + std::to_string(midiNoteNumber) + " - Its noteOff timestamp is " + std::to_string(n.noteOff.getTimeStamp()));
            frame.addNoteEvent(midiNoteNumber, true);
            Note offTmp(n);
            // add the note to noteOffs map by its timestamp, to eventually remove it
            pendingNoteOffMap.insert( { n.noteOff.getTimeStamp(), offTmp });
        }
       
        /* for (; it != iterp   air.second; ++it) {
            if (it->second == 15) {
                mymap.erase(it);
                break;
            }
        }*/
        
        // Remove any noteOffs that need to be removed at this timestamp
        // TODO only call getNotesAtTimestamp if not empty key?
        DBG("Getting all noteOffs at " + std::to_string(timestamp));
        auto noteOffList = getNotesAtTimeStamp(timestamp, pendingNoteOffMap);
        for (Note & n : noteOffList)
        {
            int midiNoteNumber = n.noteOn.getNoteNumber();
            DBG("Adding noteOff " + std::to_string(midiNoteNumber) + " at above timestamp");
            frame.addNoteEvent(midiNoteNumber, false);
        }
        pendingNoteOffMap.erase(timestamp);

        // Add heatMapFrame to HeatmapList output list
        noteHeatMap->push_back(frame);

        /* Advance to next noteOn timestamp */
        ++iter;
        if (iter == inputNoteMap.end()) DBG("ITERATOR AT MMAP.END()");
        double nextNoteOnTimestamp = iter->first;
        DBG("NextNoteOnTimestamp = " + (timestamp == nextNoteOnTimestamp ? "PREVIOUS TIMESTAMP! ERROR": std::to_string(nextNoteOnTimestamp)));
        
        // Check if we need to create heatmaps for the pending note
        // release timestamps which happen before the next noteOn
        // TODO BUG FIX - relies on the fact that no noteOff messages come before noteOns in MidiFile
        auto noteOffIter = pendingNoteOffMap.begin();
        
        double noteOffTimestamp = noteOffIter->first;
        DBG("nextNoteOffTimestamp = " + std::to_string(noteOffTimestamp));
        while (nextNoteOnTimestamp > noteOffTimestamp || iter == inputNoteMap.end())
        {
            // create new heatmap for each unique timestamp for note releases
            HeatmapFrame newFrame(noteOffTimestamp);

            // remove all notes at timestamp in heatmap
            DBG("Getting noteOffs at " + std::to_string(noteOffTimestamp) + ", before the nextNoteOnTimestamp");
            auto notes = getNotesAtTimeStamp(noteOffTimestamp, pendingNoteOffMap);
            for (Note n : notes)
            {
                int midiNoteNumber = n.noteOn.getNoteNumber();
                frame.subtractions.push_back(midiNoteNumber);
            }
            pendingNoteOffMap.erase(noteOffTimestamp);

            noteHeatMap->push_back(newFrame);

            noteOffTimestamp = noteOffIter->first;
        }
        ++dbgCounter;
    }
    
    return noteHeatMap;
}


