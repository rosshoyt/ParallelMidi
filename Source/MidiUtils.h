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
 * Creates a NoteHeatMap from a NoteMap.
 * TODO assumes MidiFile doesn't start with a NoteOff and each NoteOn has matching NoteOff
 */
static HeatmapList * scanNoteMap(NoteMap & inputNoteMap)
{
    HeatmapList* noteHeatMap = new HeatmapList();

    NoteMap pendingNoteOffMap; // holds noteOffs that will also update or create heatmaps
    int dbgCounter = 1;

    /* 
     * Go through each noteOn in order by timestamp. can be multiple notes per timestamp
     */
    auto & iter = inputNoteMap.begin();
    while (iter != inputNoteMap.end())
    {
        DBG("------------LOOP #" + std::to_string(dbgCounter++) + "-----------\nPENDING NoteOffs: " + std::to_string(pendingNoteOffMap.size()));
        // Get time off the current noteOn event we're looking at
        auto timestamp = iter->first;         
        
        // Initialize the heatmap for the current timestamp
        HeatmapFrame frame(timestamp);
       
        /*
        * Add all noteOns at the current timestamp into the heatmap
        */
        DBG("Getting Notes that Start and End at " + std::to_string(timestamp));
        while (iter != inputNoteMap.end() && iter->first == timestamp)
        {
            int midiNoteNumber(iter->second.noteOn.getNoteNumber());
            DBG("Adding noteOn  " + std::to_string(midiNoteNumber) + " - it ends at " + std::to_string(iter->second.noteOff.getTimeStamp()));
            frame.addNoteEvent(midiNoteNumber, true);
            // Copy note to be added to pendingNoteOff mmap.
            Note off(iter->second);
            // add the note to noteOffs map by its timestamp, to eventually remove it
            pendingNoteOffMap.insert({off.noteOff.getTimeStamp(), off });
            ++iter;
        }

        /*
        * Add any note offs occuring at the current timestamp to the heatmap
        */
        auto& offIter = pendingNoteOffMap.begin();
        while (offIter->first == timestamp)
        {
            int midiNoteNumber(offIter->second.noteOn.getNoteNumber());
            DBG("Adding noteOff " + std::to_string(midiNoteNumber));
            frame.addNoteEvent(midiNoteNumber, false);
            offIter++;
        }

        int numErased = pendingNoteOffMap.erase(timestamp);
        if(numErased > 0) DBG("Erased " + std::to_string(numErased) + " notes from PendingNoteOffMap");
        
        /*
        * Add the heatMapFrame to HeatmapList 
        */
        noteHeatMap->push_back(frame);

        /* 
        * Now, check if we need to create any additional Heatmap Frames for of any noteOff event timestamps
        * which happen before the next NoteOn event, or before the end of the song.
        */
        // First get the next NoteOn timestamp, or set it to the previous timestamp if we just processed the final note.
        double nextNoteOnTimestamp = iter == inputNoteMap.end() ? timestamp : iter->first;
        
        auto offIter2 = pendingNoteOffMap.begin();
        double nextNoteOffTimestamp = offIter2->first; // assumes there will at least 1 pending noteOff
        DBG("NextNoteOffTimestamp = " + std::to_string(nextNoteOffTimestamp));
        std::list<double> noteOffTimestampsToDelete; // TODO optimize deletion logic (inside loop?)
        while (nextNoteOnTimestamp > nextNoteOffTimestamp ||
            iter == inputNoteMap.end() && offIter2 != pendingNoteOffMap.end())
        {
            // The heatmap frame for this note release timestamp
            HeatmapFrame newFrame(nextNoteOffTimestamp);

            DBG("Removing noteOffs at " + std::to_string(nextNoteOffTimestamp));
            // remove all notes at timestamp in heatmap
            while(offIter2 != pendingNoteOffMap.end() && offIter2->first == nextNoteOffTimestamp)
            {
                int midiNoteNumber = offIter2->second.noteOn.getNoteNumber(); // (noteOn/Off should have same note#)
                frame.addNoteEvent(midiNoteNumber, false);
                offIter2++;   
            }
            noteOffTimestampsToDelete.push_back(nextNoteOffTimestamp);
            // Add the frame to the list of frames
            noteHeatMap->push_back(newFrame);
            if(offIter2 != pendingNoteOffMap.end())
                nextNoteOffTimestamp = offIter2->first;
        }
        // remove any timestamps of noteOffs processed
        int delCount = 0;
        for (auto ts : noteOffTimestampsToDelete) delCount += pendingNoteOffMap.erase(ts);
        if(delCount > 0) DBG("Erased " + std::to_string(delCount) + " from PendingNoteOffMap");
    }
    return noteHeatMap;
}


