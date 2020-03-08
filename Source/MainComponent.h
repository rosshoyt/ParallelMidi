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
*/
class MainComponent    : public Component
{
public:
    MainComponent() : noteMapComponent()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        NoteMap noteMap;
        try {
            noteMap = getNoteMap(readInMidiFile(getProjectFullPath(PROJECT_JUCER_FILENAME_FLAG) + MIDI_FILE_REL_PATH), true);
        }
        catch (...) {
            DBG("Problem reading file");
        }
        
        noteMapComponent.setNoteMap(noteMap);
        addAndMakeVisible(noteMapComponent);
        setSize(600, 400);
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

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..

    }

private:

    NoteMapComponent noteMapComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
