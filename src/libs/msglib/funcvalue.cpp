/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#include "funcvalue.hpp"
#include "base/system.hpp"
#include "base/math.hpp"

/****************************************************************************/

sRandom sFuncValue::Random;

sFuncValue::sFuncValue()
{
  Type = FVALUE_FLOAT;
  Args[0] = 0.0f;
  Args[1] = 0.0f;
  Args[2] = 0.0f;
}

void sFuncValue::Serialize(class sReader &s)
{
  s | Type;
  s | Args[0];
  s | Args[1];
  s | Args[2];
}

sF32 sFuncValue::Get()
{
  switch(Type)
  {
    case FVALUE_FLOAT:
      {
        return Args[0];
      } break;
    case FVALUE_RANDOM:
      {
        if (Args[0] > 0)
        {
          sRandom random;
          random.Seed(Args[0]);
          return Args[1] + random.Float(Args[2] - Args[1]);
        }
        else
        {
          // global randomizer
          return Args[1] + Random.Float(Args[2] - Args[1]);
        }        
      } break;
    default:
      break;
  }
  sVERIFYFALSE;
  return 0.0f;
}

void sReadFuncValue(sFuncValue &target, class sReader &s)
{
  target.Serialize(s);
}

void sReadFuncValue(sF32 &target, class sReader &s)
{
  sFuncValue fv;
  fv.Serialize(s);
  target = fv.Get();
}

void sReadFuncValue(sVector31 &target, class sReader &s)
{
  sInt count;
  s | count;
  sVERIFY(count == 3);
  sReadFuncValue(target.x, s);
  sReadFuncValue(target.y, s);
  sReadFuncValue(target.z, s);
}


/****************************************************************************/

