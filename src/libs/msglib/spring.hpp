/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#ifndef FILE_MSGLIB_SPRING_HPP
#define FILE_MSGLIB_SPRING_HPP

#include "base/types.hpp"

/****************************************************************************/

class sSpring 
{
protected:
  sF32 Time;
  sF32 Speed;
  sF32 Position;
public:
  sSpring();

  void Iterate();
  void Step(sF32 delta);
  void Reset(sF32 position=0.0f);
  sBool IsMoving();
  sF32 GetPosition();

  sF32 MeasureFalloffTime();

  sF32 Friction;
  sF32 Mass;
  sF32 Constant;
  sF32 Length;
};

/****************************************************************************/

#endif // FILE_MSGLIB_SPRING_HPP

