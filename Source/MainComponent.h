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
class MainComponent    : public Component, public Button::Listener
{
public:
    /**
     * Constructor which adds child components, intiates settings & runs main application logic
     */ 
    MainComponent() : noteMapComponent(), startButton("Start"), animationStatus()
    {
        
        HeatmapList * heatMaps;
        try {
            auto midiFile = readInMidiFile(getProjectFullPath(PROJECT_JUCER_FILENAME_FLAG) + MIDI_FILE_REL_PATH);
            auto noteMap = getNoteMap(midiFile);
            heatMaps = scanNoteMap(noteMap);
                                   
        } catch (...) {
            DBG("Problem reading file");
        }
        
        addAndMakeVisible(startButton);
        startButton.addListener(this);
        
        addAndMakeVisible(animationStatus);
        animationStatus.setFont(Font(16.0f, Font::bold));
        animationStatus.setText("Not Animating", dontSendNotification);
        animationStatus.setColour(Label::textColourId, Colours::lightgreen);
        
        addAndMakeVisible(noteMapComponent);
        noteMapComponent.setNoteHeatMap(heatMaps);
           
        setSize(600, 400);
    }

    ~MainComponent()
    {
        
    }


    void paint (Graphics& g) override
    {

    }
    /**
     * Method that sets the bounds of child components this component contains.
     */
    void resized() override
    {
        auto rect = getBounds();
        auto top = rect.removeFromTop(30);
        noteMapComponent.setBounds(rect);
        startButton.setBounds(top.removeFromLeft(top.getWidth() / 2));
        animationStatus.setBounds(top);
        
    }

private:
    void buttonClicked(Button* b)
    {
        if (b == &startButton)
        {
            DBG("Start Button presed");
            noteMapComponent.animate();
            animationStatus.setText(noteMapComponent.isAnimating() ? "Animating!" : "Not Animating", dontSendNotification);

        }
    } 

    NoteMapComponent noteMapComponent;
    TextButton startButton;
    Label animationStatus;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
