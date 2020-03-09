/*
  ==============================================================================

    NoteMapComponent.h
    Created: 7 Mar 2020 2:15:52pm
    Author:  Ross Hoyt

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MidiUtils.h"
#include <chrono>
#include <ctime>

//==============================================================================
/*
*/
class NoteMapComponent    : public Component
{
public:

    bool animate() {
        if (noteMap != nullptr)
        {
            currentFrame = noteMap->begin();
            prevFrame = currentFrame;
            animating = true;
            return animating;
        }
        else return animating;
    }

    NoteMapComponent() : rectangles(), animating(false)
    {
        noteMap = nullptr;
        
        colours = new Colour[nDisplayBoxes];
        for (int i = 0; i < nDisplayBoxes; ++i)
            colours[i] = Colours::darkblue;
        
    }

    ~NoteMapComponent()
    {
        delete[] colours;
    }
    
    std::list<HeatmapFrame>::iterator currentFrame, prevFrame;
    std::chrono::time_point<std::chrono::system_clock> startTime;
    double timeElapsed = 0.0;
    void paint (Graphics& g) override
    {
        if (animating) 
        {
            // initialize startTime
            if(timeElapsed == 0.0) startTime = std::chrono::system_clock::now();
            
            auto msPassed = std::chrono::system_clock::now() - startTime;

            if (msPassed.count() >= currentFrame->timestamp)
            {

                for (auto noteNumber : currentFrame->additions)
                {
                    int bucket = noteNumber % 12;
                    colours[bucket] = colours[bucket].brighter();
                }
                for (auto noteNumber : currentFrame->subtractions)
                {
                    int bucket = noteNumber % 12;
                    colours[bucket] = colours[bucket].darker();
                }
            }
    
            for (int i = 0; i < rectangles.size(); ++i)
            {
                g.setColour(colours[i]);
                g.fillRect(rectangles[i]);
            }
        }
        else 
        {
      
            g.drawText("not animating", getBounds(), Justification::centred);
        }
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..
        int count = 0;
        int rectWidth = getWidth() / nDisplayBoxes;

        for (int i = 0; i < rectangles.size(); ++i)
            rectangles[i].setBounds(count * rectWidth, 0, rectWidth, getHeight());
    }
    

    void setNoteHeatMap(HeatmapList *noteMap) 
    {
        this->noteMap = noteMap;
        //findMostHits(); // TODO implement findMostHits using reduction or other parallel technique
    }
    //void findMostHits();
private:
    //int maxHits; //TODO 

    bool animating;
    double playbackRate = 5.0;
    int nDisplayBoxes = 12;
    HeatmapList * noteMap;
    Colour * colours;
    //Array<std::pair<Rectangle<int>, Colour>> rectangles;
    Array<Rectangle<int>, CriticalSection, 12> rectangles;

    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoteMapComponent)
};
