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
