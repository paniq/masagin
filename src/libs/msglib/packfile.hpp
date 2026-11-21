/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2008 by Bob Fungus (.th)                             ***/
/***   all rights reserved                                                ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#ifndef FILE_SCREENMODE_PACKFILE_HPP
#define FILE_SCREENMODE_PACKFILE_HPP

#include "base/types.hpp"
#include "base/system.hpp"

/****************************************************************************/
/***                                                                      ***/
/*** Pack File                                                            ***/
/***                                                                      ***/
/****************************************************************************/

// for the demo: add a pack file. All sCreateFile commands will access
// the packfile first. You can easily have multiple packfiles, too.

// create packfile from file
void bAddPackFile(const sChar *packfile);

// create packfile from mem (ownership stays with you, make sure mem 
// stays valid)
void bAddPackFile(const sU8 *mem, sInt size);


// for the tool: creates a packfile from a .txt with all relative path names
// of the files to pack.
void bWritePackFile(const sChar *packfilename, const sChar *deffilename);


/****************************************************************************/
/***                                                                      ***/
/*** File Logger                                                          ***/
/***                                                                      ***/
/****************************************************************************/

// adds a file logger that generates a packfile .txt from all accesses
void bAddFileLogger(const sChar *deffilename);

// call this at the end, destroys all added packfiles/loggers
void bExitFileLogger();

/****************************************************************************/

#endif // FILE_SCREENMODE_PACKFILE_HPP

