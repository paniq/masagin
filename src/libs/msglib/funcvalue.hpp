/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#ifndef FILE_MSGLIB_FUNCVALUE_HPP
#define FILE_MSGLIB_FUNCVALUE_HPP

#include "base/types.hpp"

/****************************************************************************/

enum {
  FVALUE_FLOAT = 0, // arg0 = value
  FVALUE_RANDOM = 1, // arg0 = seed, arg1 = from, arg2 = to
};

class sFuncValue
{
public:
  static sRandom Random;
  sInt Type;
  sF32 Args[3];

  sFuncValue();
  void Serialize(class sReader &s);
  sF32 Get();
};

void sReadFuncValue(sFuncValue &target, class sReader &s);
void sReadFuncValue(sF32 &target, class sReader &s);
void sReadFuncValue(class sVector31 &target, class sReader &s);

/****************************************************************************/

#endif // FILE_MSGLIB_FUNCVALUE_HPP

