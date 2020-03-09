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
#include "FileUtils.h"
#include "NoteHistoScan.h"

// the raw data components of the scan
typedef MidiMessage * NoteOn;
typedef MidiMessage * NoteOff;
// A note is made up of a noteOn and its corresponding noteOff..
// Assumes complete midi file with matching noteOns/noteOffs
struct Note 
{ 
    const NoteOn noteOn; 
    const NoteOff noteOff; 
};
// The container for all the midi notes in a midi file or collection of 
// midi files
typedef std::multimap<double, Note> NoteMultiMap;
// a single frame in the midi heatmap list, with an array and timestamp
struct HeatmapFrame 
{ 
    HeatmapFrame() : additions(), subtractions()
    {
        heatMap = new int[SIZE];
    }
    HeatmapFrame(double timestamp) : HeatmapFrame() {
        this->timestamp = timestamp;
    }
    HeatmapFrame(double timestamp, const int* heatMap) : HeatmapFrame(timestamp) {
        this->heatMap = heatMap;
    }
    double timestamp; 
    const int* heatMap; 
    const int SIZE = 1 << 7; 
    std::list<int> additions, subtractions;
};
// the final scan result which is used to animate the heatmap
typedef std::list<HeatmapFrame> HeatmapList;

/**
 * Reads the NoteOn messages from a midi file into a single list
 * @return notes in midi File
 */
static const NoteMultiMap getNoteMap(MidiFile& midiFile) 
{
    midiFile.convertTimestampTicksToSeconds();
    NoteMultiMap data;
    // Read tracks
    for (int t = 0; t < midiFile.getNumTracks(); ++t) {
        auto track = *midiFile.getTrack(t);
        // Copy track note events
        for(int i = 0; i < track.getNumEvents(); ++i){
            MidiMessage * midiMessage = &track.getEventPointer(i)->message;
            if (midiMessage->isNoteOn()) {
                MidiMessage* noteOff = &track.getEventPointer(track.getIndexOfMatchingKeyUp(i))->message;
                data.insert( { midiMessage->getTimeStamp(), { midiMessage, noteOff } }) ;
            }
        }
    }
    return data;
}

/**
 * TODO move method to a Map class
 */
static std::list<Note> getNotesAtTimeStamp(double timestamp, const NoteMultiMap& noteMap)
{
    DBG("Getting Notes at timestamp " + std::to_string(timestamp));
    auto itr1 = noteMap.lower_bound(timestamp), itr2 = noteMap.upper_bound(timestamp);
    std::list<Note> notes;
    while (itr1 != itr2)
    {
        if (itr1->first == timestamp)
            notes.push_back(itr1->second);
        itr1++;
    }
    return notes;
}

/**
 * Creates a NoteHeatMap from a NoteMap.
 * TODO assumes MidiFile doesn't start with a NoteOff
 */
static HeatmapList * scanNoteMap(const NoteMultiMap & inputNoteMap)
{
    HeatmapList * noteHeatMap;
    
    NoteMultiMap pendingNoteOffMap; // holds noteOffs that will also create heatmaps when they update
    // go through each note in order by timestamp. can be multiple notes per timestamp
    auto iter = inputNoteMap.begin();
    while (iter != inputNoteMap.end())
    {
        auto timestamp = iter->first; // time off the current noteOn event we're looking at        
        
        // Create heatmap for this new timestamp
        HeatmapFrame frame(timestamp);
        int* heatMap = new int[128];
        // Add all noteOns at the current timestamp into the heatmap
        for (Note n : getNotesAtTimeStamp(timestamp, inputNoteMap))
        {
            int midiNoteNumber = n.noteOn->getNoteNumber();
            ++heatMap[midiNoteNumber];
            frame.additions.push_back(midiNoteNumber);
            // add the note to noteOffs map to eventually remove them
            pendingNoteOffMap.insert( { n.noteOff->getTimeStamp(), n });
        }
        
        // Remove any noteOffs that need to be removed at this timestamp
        for (Note n : getNotesAtTimeStamp(timestamp, pendingNoteOffMap))
        {
            int midiNoteNumber = n.noteOn->getNoteNumber();
            frame.subtractions.push_back(midiNoteNumber);
            --heatMap[midiNoteNumber];
        }
        pendingNoteOffMap.erase(timestamp);

        // Add heatMapFrame to HeatmapList output list
        //noteHeatMap.push_back( { timestamp, heatMap });
        frame.heatMap = heatMap; //TODO remove
        noteHeatMap->push_back(frame);

        /* Advance to next noteOn timestamp */
        iter++;
        // get timestamp 
        double nextNoteOnTimestamp = iter->first;

        // Create heatmaps for the pending note release timestamp
        // which happen before next noteOn
        // TODO bug fix -relies on the fact that no noteOff messages come before noteOns in MidiFile
        auto noteOffIter = pendingNoteOffMap.begin();
        double noteOffTimestamp = noteOffIter->first;
        while (nextNoteOnTimestamp > noteOffTimestamp)
        {
            // create new heatmap by copying previous values
            HeatmapFrame newFrame(noteOffTimestamp);
            int * newHeatMap = new int[128];
            for (int i = 0; i < 128; i++) newHeatMap[i] = heatMap[i];
            
            // remove all notes at timestamp in heatmap
            auto notes = getNotesAtTimeStamp(noteOffTimestamp, pendingNoteOffMap);
            for (Note n : notes)
            {
                int midiNoteNumber = n.noteOn->getNoteNumber();
                frame.subtractions.push_back(midiNoteNumber);
                newHeatMap[midiNoteNumber]--;
            }
            pendingNoteOffMap.erase(noteOffTimestamp);

            noteHeatMap->push_back( { timestamp, newHeatMap } );

            noteOffTimestamp = noteOffIter++->first;
        }


    }
    return noteHeatMap;

}


