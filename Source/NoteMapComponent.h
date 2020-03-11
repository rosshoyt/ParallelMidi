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
#include <iterator>

//==============================================================================
/*
*/
class NoteMapComponent    : public Component
{
public:

    void animate() 
    {
        if (!animating)
        {
            resized();
            animating = true;
            currentFrame = noteMap->begin();
            startTime = std::chrono::system_clock::now();
        }      
    }

    bool isAnimating()
    {
        return animating;
    }

    NoteMapComponent() //: rectangles()
    {
        animating = false;
        noteMap = nullptr;
        nDisplayBoxes = 12;
       

        colours = new Colour[nDisplayBoxes];
        rectangles = new Rectangle<int>[nDisplayBoxes];
        for (int i = 0; i < nDisplayBoxes; ++i)
        {
            //rectangles[i];
            colours[i] = Colours::darkblue;
        
        }
        updateRectanglePositions();
        
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
            
            auto msPassed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime);
            DBG("MSPassed = " + msPassed.count());
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
            
            for (int i = 0; i < nDisplayBoxes; ++i)
            {
                g.setColour(colours[i]);
                g.fillRect(rectangles[i]);
            }
            ++currentFrame;
            if (currentFrame == noteMap->end())
                animating == false;
        }
        else 
        {
      
            g.drawText("not animating", getBounds(), Justification::centred);
        }
    }

    void resized() override
    {
        updateRectanglePositions();
        
    }
    

    void setNoteHeatMap(HeatmapList *heatmapList) 
    {
        this->noteMap = heatmapList;
        DBG("Set NoteHeatMap List. Frames to animate: " + std::to_string(noteMap->size()));
        //findMostHits(); // TODO implement findMostHits using reduction or other parallel technique
    }
    //void findMostHits();
private:
    //int maxHits; //TODO 
    void updateRectanglePositions()
    {
        DBG("Updating Rectangle Positions");
        
        
        
        
        auto rectTemplate(getBounds());
        int rectWidth = rectTemplate.getWidth() / nDisplayBoxes;
        rectTemplate.setWidth(rectWidth);
        for (int i = 0; i < nDisplayBoxes; ++i)
        {
            //auto rectBounds = bounds.removeFromLeft(width);

            rectangles[i].setBounds(i * rectWidth, rectTemplate.getY(), rectTemplate.getWidth(), rectTemplate.getHeight());
            //auto thisRect = rectangles[i];
            //DBG(thisRect.get)
        }
    }
    
    bool animating;
    double playbackRate = 5.0;
    int nDisplayBoxes;
    HeatmapList * noteMap;
    Colour * colours;
    Rectangle<int> * rectangles;
    //Array<std::pair<Rectangle<int>, Colour>> rectangles;
    //Array<Rectangle<int>> rectangles;

    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoteMapComponent)
};
