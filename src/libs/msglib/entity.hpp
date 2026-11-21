/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#ifndef FILE_MSGLIB_ENTITY_HPP
#define FILE_MSGLIB_ENTITY_HPP

#include "base/types.hpp"
#include "base/math.hpp"
#include "base/graphics.hpp"
#include "constants.hpp"

/****************************************************************************/

enum {
  MTX_MODEL = 0,
  MTX_ROOT = 1,
  MTX_ANCHOR = 2,
  MTX_COLOR = 3,
  MTX_TEXTURE = 4,
  MTX_CAMERA = 5,
  MTX_MODELCAM = 6,
  MTX_AUX0 = 7,
  MTX_AUX1 = 8,
  MTX_PARENT_MODEL = 9,
  MTX_PARENT_ROOT = 10,
  MTX_PARENT_ANCHOR = 11,
  MTX_PARENT_COLOR = 12,
  MTX_PARENT_TEXTURE = 13,
  MTX_PARENT_CAMERA = 14,
  MTX_PARENT_MODELCAM = 15,
  MTX_PARENT_AUX0 = 16,
  MTX_PARENT_AUX1 = 17,
  MTX_COUNT = 18,
};

class sEntity
{
public:
  sString<128> Source;
  class sMasagin *Owner;

  sMatrix44 Matrix[MTX_COUNT];
  class sGroup2D *Model;
  class sVerb *Verb;
  //sStackArray<sEntity *, 32> Entities;
  sEntity *Parent;
  sBool Done;
  sBool Back;
  sInt Anchor;
  sF32 Delay;
  sF32 Focus;
  sF32 Lifespan;
  sInt StepCount;

  sFRect BoundingBox;
  sFRect ModelBoundingBox;
  sVector4 Camera; // x = bounding box weight, y = angle weight, z = tilt weight, w = ?

  sEntity(class sMasagin *owner);
  ~sEntity();
  sBool IsAlive(sF32 time);
  sBool IsAlive();
  sF32 GetBBWeight(); // bounding box weight
  sF32 GetUpWeight(); // up vector weight
  sF32 GetTiltWeight(); // tilt weight
  sBool IsFocusTime();
  sMatrix44 &GetCurrentMatrix();
  void PushMatrix(sMatrix44 &mtx);
  void Step();
  void StepTime(sF32 time, sMatrix44 *mtx);
  sBool UpdateBoundingBox(const sMatrix34 &mtx);
  void CalcBoundingBox(sFRect &rect, const sMatrix34 &mtx);
  void GetUpVector(sVector2 &up);
  void Draw();
  void Serialize(sReader &s);
};

/****************************************************************************/

#endif // FILE_MSGLIB_ENTITY_HPP

 