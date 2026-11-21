/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#ifndef FILE_MSGLIB_TOOLS_HPP
#define FILE_MSGLIB_TOOLS_HPP

#include "base/types.hpp"

/****************************************************************************/

class sReader;

template<int size>
void sReadMSGString(sString<size> &sd, class sReader &s)
{
  sS16 strlen;
  s.S16(strlen);
  if (!strlen)
  {
    sd = L"";
    return;
  }
  sChar8 *str = new sChar8[strlen+1];
  s.ArrayS8((sS8*)str, strlen);
  str[strlen] = '\0';
  sStringDesc sdc = sd;
  sCopyString(sdc.Buffer, str, sMin(size, strlen+1));
  sd[size-1] = '\0';
  delete[] str;
}

void sRgbToHsl(sF32 r, sF32 g, sF32 b, sF32 &h, sF32 &s, sF32 &l);
void sHslToRgb(sF32 h, sF32 s, sF32 l, sF32 &r, sF32 &g, sF32 &b);
sF32 sAbsMod(sF32 v, sF32 w);
void sFixRectAspect(sFRect &rc, sF32 w, sF32 h, sBool enlarge);

/****************************************************************************/

#endif // FILE_MSGLIB_TOOLS_HPP

