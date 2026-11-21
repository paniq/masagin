/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) by Dierk Ohlerich                                    ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#ifndef FILE_WZ4FRLIB_IPPTOOL_HPP
#define FILE_WZ4FRLIB_IPPTOOL_HPP

#include "base/types2.hpp"
#include "base/math.hpp"
#include "base/graphics.hpp"

/****************************************************************************/

struct sIppToolBlit
{
  sTexture2D *Dest;               // 0=screen
  sTexture2D *Source0;            // 0=off
  sTexture2D *Source1;
  sTexture2D *Source2;
  sTexture2D *Source3;
  sInt BlendColor;
  sInt BlendAlpha;
  sU32 BlendFactor;
  sInt Flags;
  sInt Specials;
  sVector2 Offset0;
  sVector2 Offset1;

  sVector4 Color0;                // out_2tex = (T0*C0+T1*C1)+(T0*T1*CMul)+CAdd
  sVector4 Color1;                // out_1tex = T0*C0+CAdd
  sVector4 Color2;                
  sVector4 Color3;                
  sVector4 ColorMul;              // out_0tex = CAdd
  sVector4 ColorAdd;

  sInt SourceFlipped;

  void Init();
};

enum sIppToolSpecials
{
  sIPS_NORMAL       = 0x0000,
  sIPS_SPREAD8      = 0x0001,
  sIPS_BURNOUT      = 0x0002,
  sIPS_GRAY         = 0x0003,
  sIPS_DOF          = 0x0004,
  sIPS_SPREAD8FAR   = 0x0005,     // for filtering during DOF, ignore nearer pixels (from t1-alpha)
  sIPS_BOOST        = 0x0006,
  sIPS_POINTFILTER  = 0x0100,
};

enum sIppToolBlitFlags
{
  sIPB_NOOFFSET     = 0x0001,
  sIPB_OFFSETODD    = 0x0002,
};

struct sIppToolMtrlCache
{
  sInt BlendColor;
  sInt BlendAlpha;
  sU32 BlendFactor;
  sInt TexCount;
  sInt Flags;
  sInt Specials;
  sMaterial *Mtrl;
  sBool operator== (const sIppToolMtrlCache &) const;
};

struct sIppToolGlarePara
{
  sU32 SubColor;
  sU32 ImageColor;
  sU32 GlareColor;
  sF32 Amplify;
  sF32 Radius0;
  sF32 Radius1;
  sInt Source0;
  sInt Source1;
  sInt Source2;

  void Init();
};

class sIppTool
{
  sTexture2D *Targets[32];
  sGeometry *Geo;
  sViewport View;
  class sIppToolMaterial *SpreadMtrl;

  sArray<sIppToolMtrlCache> Materials;
  sMaterial *FindMaterial(const sIppToolBlit &blit);
public:
  sIppTool();
  ~sIppTool();

  void AddTargetAbs(sInt id,sInt xs,sInt ys,sInt format=sTEX_ARGB8888);
  void AddTargetRel(sInt id,sInt xshift,sInt yshift,sInt format=sTEX_ARGB8888);
  sTexture2D *GetTarget(sInt id);
  void Grab(sInt id);

  void InitBlit(sIppToolBlit &blit,sInt dest=0,sInt s0=0,sInt s1=0, sInt s2=0, sInt s3=0);
  void Prepare();
  void Blit(const sIppToolBlit &blit,sInt flags=0);
  void BlitSpread8(const sIppToolBlit &blit,sF32 spreadu,sF32 spreadv);

  // setups

  void Glare(const sIppToolGlarePara &para);    // reads and writes buffer 1, used buffer 2&3 for pingpong
  void Dof(sF32 maxdisti,sF32 focaldist,sF32 depth,sF32 blurrad=4,sBool debugblur=0);       // reads and writes buffer 1, used buffer 2-4 as scratch
  void Boost(sF32 r0,sF32 r1,sF32 r2,sU32 min,sU32 max);
};


/****************************************************************************/

#endif // FILE_WZ4FRLIB_IPPTOOL_HPP

