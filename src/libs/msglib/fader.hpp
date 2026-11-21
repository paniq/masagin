/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#ifndef FILE_MSGLIB_FADER_HPP
#define FILE_MSGLIB_FADER_HPP

#include "base/types.hpp"
#include "spring.hpp"
#include "funcvalue.hpp"

/****************************************************************************/

enum {
  FADER_SPRING = 0,
  FADER_SINE = 1,
  FADER_TRI = 2,
  FADER_SQUARE = 3,
  FADER_POLYNOME = 4,
  FADER_LOG = 5,
};

class sFaderFactory
{
public:
  class sMasagin *Owner;
  sInt Type;
  sFaderFactory *Fader;
  sFuncValue SpringConstant;
  sFuncValue SpringMass;
  sFuncValue SpringFriction;
  sFuncValue Phase;
  sFuncValue Cycles;
  sFuncValue Exp1;
  sFuncValue Exp2;

  sFaderFactory(sMasagin *owner);
  void Serialize(class sReader &s);  
  class sFader *New();
};

/****************************************************************************/

class sFader
{
public:
  sFader *Fader;
  virtual sF32 GetDuration() { return 0.0f; }
  virtual sF32 Get(sF32 x) { return 0.0f; }
  sF32 Translate(sF32 x);
  sFader(sFaderFactory &factory);
  virtual ~sFader();
};

/****************************************************************************/

class sFrameFader : public sFader
{
protected:
public:
  sStaticArray<sF32> Frames; // 60fps
  sFrameFader(sFaderFactory &factory);
  virtual sF32 GetDuration();
  virtual sF32 Get(sF32 x);
};

/****************************************************************************/

class sSpringFader : public sFrameFader
{
public:
  sSpring Spring;
  sSpringFader(sFaderFactory &factory);
};

/****************************************************************************/

class sSineFader : public sFader
{
public:
  sF32 Phase;
  sF32 Cycles;
  sSineFader(sFaderFactory &factory);
  virtual sF32 Get(sF32 x);
};
/****************************************************************************/

class sTriFader : public sFader
{
public:
  sF32 Phase;
  sF32 Cycles;
  sTriFader(sFaderFactory &factory);
  virtual sF32 Get(sF32 x);
};

/****************************************************************************/

class sSquareFader : public sFader
{
public:
  sF32 Phase;
  sF32 Cycles;
  sSquareFader(sFaderFactory &factory);
  virtual sF32 Get(sF32 x);
};

/****************************************************************************/

class sPolynomeFader : public sFader
{
public:
  sF32 Exp1;
  sF32 Exp2;

  sF32 TranslateNormalExp(sF32 x);

  sPolynomeFader(sFaderFactory &factory);
  virtual sF32 Get(sF32 x);
};

/****************************************************************************/

class sLogFader : public sFader
{
public:
  sF32 Exp1;
  sF32 Exp2;
  sF32 Scale;
  sF32 Offset;

  sF32 Translate(sF32 x);

  sLogFader(sFaderFactory &factory);
  virtual sF32 Get(sF32 x);
};

/****************************************************************************/

#endif // FILE_MSGLIB_FADER_HPP

