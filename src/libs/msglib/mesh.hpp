/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#ifndef FILE_MESHVIEW_MESH_HPP
#define FILE_MESHVIEW_MESH_HPP

#include "base/types.hpp"
#include "base/math.hpp"

/****************************************************************************/

template<typename T> 
class sCBuffer;

class sMesh2D
{
public:
  class sMasagin *Owner;
  class sGeometry *Geometry;
  sBool UseAlpha;
  sVector4 Color;
  sVector4 RGBColor;
  sF32 Z;
  sInt Index;
  sFRect Bounds;
  sStaticArray<sVector31> Vertices;
  //sStaticArray<sVector2> Normals;

  sMesh2D(sMasagin *owner);
  ~sMesh2D();
  void Serialize(class sReader &s);
  void Draw(class sEntity *entity, const sMatrix34 &mtx, const sMatrix44 &colormtx, const sMatrix44 &uvmtx);
};

/****************************************************************************/

#endif // FILE_MESHVIEW_MESH_HPP

