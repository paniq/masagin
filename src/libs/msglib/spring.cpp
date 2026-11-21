/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#include "spring.hpp"
#include "constants.hpp"

/****************************************************************************/

sSpring::sSpring()
{ 
  Friction = 0.7f;
  Mass = 1.0f;
  Constant = 1000.0f;
  Length = 1.0f;

  Reset();
}

void sSpring::Reset(sF32 position)
{
  Time = 0.0f;
  Speed = 0.0f;
  Position = position;
}

sF32 sSpring::GetPosition()
{
  return Position;
}

sBool sSpring::IsMoving()
{
  return ((sAbs(Speed) > 0.001f)||(sAbs(Length - Position) > 0.001f));
}

void sSpring::Iterate()
{
  if (!IsMoving())
  {
    Speed = 0.0f;
    return;
  }
  Speed *= sPow(Friction, 60.0f * MSG_SPF);
  Speed += ((Constant * (Length - Position)) / Mass) * MSG_SPF;
  Position += Speed * MSG_SPF;
}

void sSpring::Step(sF32 delta)
{
  Time += delta;
  while (Time > MSG_SPF)
  {
    Time -= MSG_SPF;
    Iterate();
  }
}

sF32 sSpring::MeasureFalloffTime()
{
  sF32 maxtime = 60.0f;
  sSpring spring(*this);
  sF32 time = 0.0f;
  while (spring.IsMoving())
  {
    spring.Iterate();
    time += MSG_SPF;
    if (time > maxtime)
      return time;
  }
  return time;
}

/****************************************************************************/

