/*
  ==============================================================================

    MainComponent.h
    Created: 7 Mar 2020 1:34:53pm
    Author:  Ross Hoyt

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "FileUtils.h"
#include "MidiUtils.h"
#include "NoteMapComponent.h"

const char * MIDI_FILE_REL_PATH = "/source/book1-prelude01.mid";
const char * PROJECT_JUCER_FILENAME_FLAG = "Final-Project-ParAlgDev.jucer";

//==============================================================================
/*
* Main Application Window component.
* Manages sub-components and executes main program MidiFile Scan logic
*/
class MainComponent    : public Component
{
public:
    /**
     * Constructor which adds child components, intiates settings & runs main application logic
     */ 
    MainComponent() : noteMapComponent()
    {
        
        HeatmapList * heatMaps;
        try {
            auto midiFile = readInMidiFile(getProjectFullPath(PROJECT_JUCER_FILENAME_FLAG) + MIDI_FILE_REL_PATH);

            auto noteMap = getNoteMap(midiFile);
            heatMaps = scanNoteMap(noteMap);
                                   
        } catch (...) {
            DBG("Problem reading file");
        }
        
        addAndMakeVisible(noteMapComponent);
        setSize(600, 400);
        noteMapComponent.setNoteHeatMap(heatMaps);
        noteMapComponent.animate();
    }

    ~MainComponent()
    {
        
    }

    void paint (Graphics& g) override
    {


        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));   // clear the background

        g.setColour (Colours::grey);
        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

        g.setColour (Colours::white);
        g.setFont (14.0f);
        g.drawText ("MainComponent", getLocalBounds(),
                    Justification::centred, true);   // draw some placeholder text
    }
    /**
     * Method that sets the bounds of child components this component contains.
     */
    void resized() override
    {
        noteMapComponent.setBounds(getBounds());
    }

private:

    NoteMapComponent noteMapComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
