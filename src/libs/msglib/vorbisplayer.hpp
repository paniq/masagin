/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2008 by Bob Fungus                                   ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#ifndef FILE_OGG_VORBISPLAYER_HPP
#define FILE_OGG_VORBISPLAYER_HPP

#include "base/types.hpp"

/****************************************************************************/

// Bob Fungus presents: Bob Fungus's Bob's Vorbis Player

class bVorbisPlayer
{
public:

  explicit bVorbisPlayer(sBool seekable=sFALSE);
  ~bVorbisPlayer();

  void Init(const sChar *filename);
  void Init(void *ptr, sDInt size);
  void Exit();

  void Play();
  void Pause();
  void Seek(sF32 time); // in seconds, only when seekable==sTRUE;

  sF32 GetLength();
  void SetVolume(sF32 vol, sF32 fadeduration=0);

  sBool IsPlaying(); // returns if still playing
  sF32 GetPosition(); // returns exact position in seconds
                      // WARNING: starts with negative values because of latency!

private:

  struct VerySecret;
  VerySecret *p;

};

/****************************************************************************/

#endif // FILE_OGG_VORBISPLAYER_HPP

