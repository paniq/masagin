/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#ifndef FILE_MSGLIB_CONSTANTS_HPP
#define FILE_MSGLIB_CONSTANTS_HPP

#include "base/types.hpp"

/****************************************************************************/

enum {
  MSG_MBSLOTS1 = 4, // motion blur render target slots (level 1)
  MSG_MBSLOTS2 = 4, // motion blur render target slots (level 2)
  MSG_MBSR = MSG_MBSLOTS1 * MSG_MBSLOTS2, // motion blur sampling rate
  MSG_FPS = 60 * MSG_MBSR * 2,

  MSG_MB_NONE = 0, // no motion blur
  MSG_MB_16 = 1, // motion blur with 16 steps, non-interleaved
  MSG_MB_16_I = 2, // motion blur with 16 steps, interleaved (= 32 steps)
  MSG_MB_8 = 3, // motion blur with 8 steps, non-interleaved
  MSG_MB_8_I = 4, // motion blur with 8 steps, interleaved (= 16 steps)
};

extern const sF32 MSG_SPF;

#define PAKFILE L"masagin.pak"
#define TXTFILE L"contents.txt"

enum {
  VERB_FLAG_COLOR = (1<<0),
  VERB_FLAG_ZWRITE = (1<<1),
  VERB_FLAG_ZREAD = (1<<2),
  VERB_FLAG_ZCLEAR0 = (1<<3),
  VERB_FLAG_ZCLEAR1 = (1<<4),
  VERB_FLAG_ALPHA = (1<<5),
  VERB_FLAG_TEXTURE = (1<<6),
  VERB_FLAG_ADDCOLOR = (1<<7),
  VERB_FLAG_SUBCOLOR = (1<<8),
  VERB_FLAG_MULCOLOR = (1<<9),
};

/****************************************************************************/

#endif // FILE_MSGLIB_CONSTANTS_HPP
