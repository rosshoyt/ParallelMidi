/*
  ==============================================================================

    FileUtils.h
    Created: 6 Mar 2020 3:51:52pm
    Author:  Ross Hoyt

  ==============================================================================
*/

#pragma once

/**
 * Static method that returns the current project directory
 * @return String full path of the current project directory
 */
static String getProjectFullPath(const char * jucerFilename, bool debug = false)
{
    // get current project directory
    File dir = File::getCurrentWorkingDirectory();
    //while(dir.g)
    //for (int i = 0; i < 4; i++) {
    Array<File> matchedFiles;
    while (matchedFiles.size() == 0){
        dir = dir.getParentDirectory();
        matchedFiles = dir.findChildFiles(2, false, jucerFilename);
    } 

    String fullPath = dir.getFullPathName();
    if(debug) DBG("Project full path - " + fullPath);
    return fullPath;
}

/**
 * Method that reads a midi file from the provided full path.
 * @throws std::exception if file doesn't exist or MIDI file could not be read
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