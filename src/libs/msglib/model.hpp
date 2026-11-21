/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#ifndef FILE_MSGLIB_MODEL_HPP
#define FILE_MSGLIB_MODEL_HPP

#include "base/types.hpp"
#include "base/math.hpp"

/****************************************************************************/

class sGroup2D
{
public:
  class sMasagin *Owner;
  class sModel2D *Model;
  sString<64> Id;
  sFRect Bounds;
  sVector31 Points[4];


  sStaticArray<class sMesh2D*> Meshes;
  sStaticArray<class sShape2D*> Shapes;

  sStackArray<class sShape2D*, 32> Anchors;
  sStackArray<class sShape2D*, 16> Routes;
  class sShape2D *Root;

  sGroup2D(sMasagin *owner, sModel2D *model=sNULL);
  virtual ~sGroup2D() {};
  sBool IsUsingAlpha();
  void CalculateBounds();
  void Serialize(sReader &reader);
  void AddShape(class sShape2D *shape);
  void GetBoundingBox(sFRect &rect, const sMatrix34 &mtx);
  void GetUpVector(sVector2 &up, const sMatrix34 &mtx);
  void Draw(class sEntity *entity, const sMatrix34 &mtx, const sMatrix44 &colormtx, const sMatrix44 &uvmtx);
};

class sModel2D : public sGroup2D
{
public:
  sStaticArray<class sGroup2D*> Groups;

  sModel2D(sMasagin *owner);
  ~sModel2D();
  void Serialize(sReader &reader);
};

/****************************************************************************/

#endif // FILE_MSGLIB_MODEL_HPP

