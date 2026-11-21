/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#include "fader.hpp"
#include "spring.hpp"
#include "base/system.hpp"
#include "funcvalue.hpp"
#include "constants.hpp"
#include "tools.hpp"
#include "msg.hpp"

sFaderFactory::sFaderFactory(sMasagin *owner)
{
  Owner = owner;
  Type = 0;
  Fader = sNULL;
}

void sFaderFactory::Serialize(class sReader &s)
{
  s | Type;
  sInt fader;
  s | fader;
  if (fader != -1)
    Fader = Owner->Faders[fader];
  sReadFuncValue(SpringConstant,s);
  sReadFuncValue(SpringMass,s);
  sReadFuncValue(SpringFriction,s);
  sReadFuncValue(Phase,s);
  sReadFuncValue(Cycles,s);
  sReadFuncValue(Exp1,s);
  sReadFuncValue(Exp2,s);
}

class sFader *sFaderFactory::New()
{
  switch(Type)
  {
    case FADER_SPRING: return new sSpringFader(*this);
    case FADER_SINE: return new sSineFader(*this);
    case FADER_TRI: return new sTriFader(*this);
    case FADER_SQUARE: return new sSquareFader(*this);
    case FADER_POLYNOME: return new sPolynomeFader(*this);
    case FADER_LOG: return new sLogFader(*this);
    default: break;
  }
  return sNULL;
}

/****************************************************************************/

sF32 sFader::Translate(sF32 x)
{
  if (Fader)
    x = Fader->Translate(x);
  return Get(x);
}

sFader::sFader(sFaderFactory &factory)
{
  Fader = sNULL;
  if (factory.Fader)
    Fader = factory.Fader->New();
}

sFader::~sFader()
{
  sDelete(Fader);
}

/****************************************************************************/

sFrameFader::sFrameFader(sFaderFactory &factory)
: sFader(factory)
{
  
}

sF32 sFrameFader::GetDuration()
{
  return (sF32)Frames.GetCount() * MSG_SPF;
}

sF32 sFrameFader::Get(sF32 x)
{
  x = sClamp(x,0.0f,1.0f);
  return Frames[sInt(x * (Frames.GetCount()-1) + 0.5f)];
}

/****************************************************************************/

sSpringFader::sSpringFader(sFaderFactory &factory)
: sFrameFader(factory)
{
  Spring.Constant = factory.SpringConstant.Get();
  Spring.Mass = factory.SpringMass.Get();
  Spring.Friction = factory.SpringFriction.Get();
  // sample spring
  Spring.Reset(0);
  Spring.Length = 1.0f;
  sF32 length = Spring.MeasureFalloffTime();
  sInt framecount = sInt(length / MSG_SPF + 0.5f);
  Spring.Reset(0);
  Spring.Length = 1.0f;
  Frames.HintSize(framecount);
  while (framecount--)
  {
    Frames.AddTail(Spring.GetPosition());
    Spring.Iterate();
    factory.Owner->LoadStep();
  }
}

/****************************************************************************/

sSineFader::sSineFader(sFaderFactory &factory) : sFader(factory)
{
  Phase = factory.Phase.Get();
  Cycles = factory.Cycles.Get();
}

sF32 sSineFader::Get(sF32 x)
{
  return (sFSin(x * Cycles * sPI2F + Phase)+1.0f)*0.5f;
}

/****************************************************************************/

sTriFader::sTriFader(sFaderFactory &factory) : sFader(factory)
{
  Phase = factory.Phase.Get();
  Cycles = factory.Cycles.Get();
}

sF32 sTriFader::Get(sF32 x)
{
  x = sAbsMod(x * Cycles + Phase, 1.0f);
  if (x < 0.5f)
    return x * 2.0f;
  else
    return 1.0f - (x - 0.5f) * 2.0f;
}

/****************************************************************************/

sSquareFader::sSquareFader(sFaderFactory &factory) : sFader(factory)
{
  Phase = factory.Phase.Get();
  Cycles = factory.Cycles.Get();
}

sF32 sSquareFader::Get(sF32 x)
{
  x = sAbsMod(x * Cycles + Phase, 1.0f);
  if (x < 0.5f)
    return 0.0f;
  else
    return 1.0f;
}

/****************************************************************************/

sF32 sPolynomeFader::TranslateNormalExp(sF32 x)
{
  x = sClamp(x, -1.0f, 1.0f);
  if (x >= 0.0f)
    return x;
  else
    return 1.0f / -x;
}

sPolynomeFader::sPolynomeFader(sFaderFactory &factory) : sFader(factory)
{
  Exp1 = TranslateNormalExp(factory.Exp1.Get());
  Exp2 = TranslateNormalExp(factory.Exp2.Get());
}

sF32 sPolynomeFader::Get(sF32 x)
{
  return sFPow(x, Exp1) * (1.0f - x) + (1.0f - sFPow(1.0f - x,Exp2))*x;
}

/****************************************************************************/

sF32 sLogFader::Translate(sF32 x)
{
  return sLog(x+0.0001f);
}

sLogFader::sLogFader(sFaderFactory &factory) : sFader(factory)
{
  sF32 e1 = factory.Exp1.Get();
  sF32 e2 = factory.Exp2.Get();
  Offset = e1;
  Scale = 1.0f / (e2 - e1);
  Exp1 = Translate(e1);
  Exp2 = Translate(e2);
}

sF32 sLogFader::Get(sF32 x)
{
  return (sExp(Exp1 * (1.0f - x) + Exp2 * x) - Offset) * Scale;
}

/****************************************************************************/
