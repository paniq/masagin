/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/

#ifndef FILE_BASE_SOUND_HPP
#define FILE_BASE_SOUND_HPP

#include "base/types.hpp"

/****************************************************************************/

typedef void (*sSoundHandler)(sS16 *data,sInt count);      // 1 count = left+right = 4 bytes

// sound out

sBool sSetSoundHandler(sInt freq,sSoundHandler,sInt latency);
void sClearSoundHandler();
sInt sGetCurrentSample();
void sClearSoundBuffer();

void sSoundHandlerNull(sS16 *data,sInt count);
void sSoundHandlerTest(sS16 *data,sInt count);

// sound in

sBool sSetSoundInHandler(sInt freq,sSoundHandler,sInt latency);
void sSoundInput();
sInt sGetCurrentInSample();

void sLockSound();
void sUnlockSound();

/****************************************************************************/

#endif
