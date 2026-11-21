/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/

#ifndef HEADER_ALTONA_UTIL_PAINTER
#define HEADER_ALTONA_UTIL_PAINTER

#ifndef __GNUC__
#pragma once
#endif


#include "base/types.hpp"
#include "base/system.hpp"

/****************************************************************************/

class sBasicPainter
{
#if !sCONFIG_QUADRICS
  sInt Alloc;
  sInt Used;
  struct sVertexSingle *Vertices;
#endif

  class sGeometry *Geo;
  class sGeometry *GeoX;
  class sMaterial *Mtrl;
  class sMaterialEnv *Env;
  class sTexture2D *Tex;
  class sViewport *View;

public:
  // management
  sBasicPainter();
  ~sBasicPainter();
  void SetTarget();
  void SetTarget(const sRect &target);
  void Begin();
  void End();
  void Clip(const sFRect &r);
  void ClipOff();

  // shapes
  void Box(const sFRect &r,const sU32 *col);
  void Line(sF32 x0,sF32 y0,sF32 x1,sF32 y1,sU32 c0,sU32 c1,sBool skiplastpixel=0);

  // fonts. select unique fontid as you like
  void RegisterFont(sInt fontid,const sChar *name,sInt height,sInt style);
  void Print(sInt fontid,sF32 x,sF32 y,sU32 col,const sChar *text,sInt len=-1,sF32 zoom=1.0f);
  sF32 GetWidth(sInt fontid,const sChar *text,sInt len=-1);
  sF32 GetHeight(sInt fontid);

  // debbuging textures
  void PaintTexture(sTexture2D* tex, sU32 col=0xffffffff, sInt xs=-1, sInt ys=-1, sInt xo=0, sInt yo=0, sBool noalpha=sTRUE);
};

/****************************************************************************/

class sPainter : public sBasicPainter
{
  sInt FontId;
  sU32 TextColor;
  sF32 Zoom;
  sString<1024> classbuf;
  void Print0(sF32 x,sF32 y,const sChar *txt);
public:
  sPainter();
  void Box(const sFRect &r,sU32 col);
  void Box(sF32 x0,sF32 y0,sF32 x1,sF32 y1,sU32 col);

  void SetPrint(sInt fontid,sU32 col,sF32 zoom);
  sPRINTING2(PrintF,sFormatStringBuffer buf=sFormatStringBase(classbuf,format);buf,Print0(arg1,arg2,classbuf);,sInt,sInt);
};

/****************************************************************************/

// HEADER_ALTONA_UTIL_PAINTER
#endif
