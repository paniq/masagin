/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#ifndef FILE_MSGLIB_SHAPE_HPP
#define FILE_MSGLIB_SHAPE_HPP

#include "base/types.hpp"
#include "base/math.hpp"

/****************************************************************************/

enum {
  MSGCLASS_ANCHOR = 0,
  MSGCLASS_ROOT = 1,
  MSGCLASS_ROUTE = 2,
};

class sShape2D
{
public:
  sMatrix34 Matrix;
  sInt MsgClass;
  sStaticArray<sVector2> Points;
  sStaticArray<sF32> Lengths;
  sShape2D();
  ~sShape2D();
  void Serialize(sReader &s);
  void GetPoint(sF32 pos, sVector2 &pt);
  void Draw();
};

/****************************************************************************/

#endif // FILE_MSGLIB_SHAPE_HPP

