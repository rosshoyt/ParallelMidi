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

//==============================================================================
/*
*/
class NoteMapComponent    : public Component, public Thread
{
public:

    void animate() {
        startThread(10);
    }
    
    NoteMapComponent() : Thread("Heatmap Display Thread"), rectangles()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        
        hitMap = new int[nDisplayBoxes];
        colours = new Colour[nDisplayBoxes];
        for (int i = 0; i < nDisplayBoxes; ++i) {
            colours[i] = Colours::darkblue;
        }
    }

    ~NoteMapComponent()
    {
        delete[] hitMap;
        delete[] colours;
    }

    void paint (Graphics& g) override
    {
        
        for (int i = 0; i < rectangles.size(); ++i)
        {
            g.setColour(colours[i]);
            g.fillRect(rectangles[i]);
        }
        g.fillRect(getBounds());
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
    void run() {
        //SortedSet<int> channelNoteMap;
        //bool sustainOn = false;
        
        //for(int i = 0; i < noteMap.size() && !threadShouldExit(); ++i)
        auto iter = noteMap.begin();
        while(iter != noteMap.end() && !threadShouldExit())
        {
            auto timestamp = iter->first;
            DBG("Timestamp = " + std::to_string(timestamp));
            auto itr1 = noteMap.lower_bound(timestamp);
            auto itr2 = noteMap.upper_bound(timestamp);
            while (itr1 != itr2)
            {
                if (itr1->first == timestamp) {
                    // hitMap[itr1->second.noteOn->getNoteNumber()]++;
                    int pitch = itr1->second.noteOn->getNoteNumber() % 12;
                    colours[pitch] = colours[pitch].brighter();
                }
                itr1++;
            }
            
            // advance iterator to next timestamp key
            auto nextMsg = iter++;
            double waitTimeSecs = 1000 / playbackRate * (iter->first - timestamp);

            DBG("Waittime(s): " + std::to_string(waitTimeSecs));
            //if (waitTimeSecs != 0) 
            wait((int)waitTimeSecs);
            
            
            
            /*if (currMsg->isNoteOn())
            {
                DBG("NoteOn " + std::to_string(currMsg->getNoteNumber()));*/
            //    keyboardState.noteOn(1, currMsg->getNoteNumber(), currMsg->getFloatVelocity());
            //    if (useSustainPedalMessages && sustainOn) channelNoteMap.add(currMsg->getNoteNumber());
            //}
            //else if (currMsg->isNoteOff())
            //{
            //    DBG("NoteOff " + std::to_string(currMsg->getNoteNumber()));
            //    //                if(useSustainPedalMessages && sustainOn)
            //    //                    DBG("Note " + std::to_string(currMsg->getNoteNumber()) + " RELEASED BUT SUSTAINED");
            //    //                else
            //    if (!useSustainPedalMessages || !sustainOn)
            //        keyboardState.noteOff(1, currMsg->getNoteNumber(), currMsg->getFloatVelocity());
            //}
            //else if (currMsg->isSustainPedalOn())
            //{
            //    DBG("---SUSTAIN PEDAL ON---");
            //    sustainOn = true;
            //}
            //else if (currMsg->isSustainPedalOff())
            //{
            //    DBG("---SUSTAIN PEDAL OFF---");
            //    sustainOn = false;
            //    for (int i = 0; i < channelNoteMap.size(); ++i)
            //    {
            //        int relNote = channelNoteMap.getUnchecked(i);
            //        DBG(std::to_string(i + 1) + ". RELEASING PITCH" + std::to_string(relNote));
            //        keyboardState.noteOff(1, relNote, 64);
            //    }
            //}
            

        }
        DBG("Closing Play() thread");
    }

    void setNoteMap(NoteMap noteMap) {
        this->noteMap = noteMap;
        //findMostHits(); // TODO implement findMostHits using reduction or other parallel technique
    }
    //void findMostHits();
private:
    //int maxHits; //TODO 


    double playbackRate = 5.0;
    int nDisplayBoxes = 12;
    NoteMap noteMap;
    Colour* colours;
    //Array<std::pair<Rectangle<int>, Colour>> rectangles;
    Array<Rectangle<int>, CriticalSection, 12> rectangles;
    int * hitMap;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoteMapComponent)
};
