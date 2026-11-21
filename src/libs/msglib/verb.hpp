/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#ifndef FILE_MSGLIB_VERB_HPP
#define FILE_MSGLIB_VERB_HPP

#include "base/types.hpp"
#include "base/math.hpp"
#include "funcvalue.hpp"

/****************************************************************************/

enum {
  OP_SCALE = 0,
  OP_TRANSLATE = 1,
  OP_ROTATE = 2,
  OP_TRANSLATE_ROUTE = 3,
  OP_ROTATE_ROUTE = 4,
  OP_LOGSCALE = 5,
  OP_SKEW = 6,
  OP_PROJECT = 7,

  OP_MUL_MATRIX = 10,
  OP_IDENTITY_MATRIX = 11,
  OP_STORE_MATRIX = 12,
  OP_INVERT_MATRIX = 13,
  OP_LOAD_MATRIX = 14,

  OP_SET_GLOBAL = 15,

  OP_NOP = 255,
  OP_COUNT,
};

enum {
  OP_FLAG_LOOP = (1<<0),
  OP_FLAG_CLAMP = (1<<1),
  OP_FLAG_UNIFORM = (1<<2),
};


enum {
  GLOBAL_BGCOLOR = 0,
};

#define sMASAGIN_PRECALC 0

/****************************************************************************/

class sOpFactory
{
public:
  class sMasagin *Owner;
  sInt Index;
  sInt Index2;
  sInt Flags;
  sInt Type;
  sFuncValue Time[2];
  sFuncValue Range[2][4];
  class sFaderFactory *Fader;

  sOpFactory(sMasagin *owner);
  void Serialize(class sReader &s);  
  class sOp *New(class sEntity *entity);
};

/****************************************************************************/

class sOp
{
public:
  class sMasagin *Owner;
  sInt Index;
  sInt Index2;
  sInt Flags;
  sInt Type;
  sF32 Time[2];
  sVector4 Range[2];
  class sFader *Fader;

  sOp(sOpFactory &factory);
  virtual ~sOp();
  sBool GetPosition(sF32 &x, sF32 time);
  virtual sBool GetValue(sVector4 &v, sF32 time);
  virtual void GetMatrix(sMatrix44 *mtx, sF32 time);
  virtual void GetLocalMatrix(sVector4 &v, sMatrix44 &mtx, sF32 time) {};
};

/****************************************************************************/

class sSetGlobalOp : public sOp
{
public:
  sSetGlobalOp(sOpFactory &factory,class sEntity* entity) : sOp(factory) {}
  virtual void GetMatrix(sMatrix44 *mtx, sF32 time);
};

/****************************************************************************/

class sMulMatrixOp : public sOp
{
public:
  sMulMatrixOp(sOpFactory &factory) : sOp(factory) {}
  virtual void GetMatrix(sMatrix44 *mtx, sF32 time);
};

/****************************************************************************/

class sIdentityMatrixOp : public sOp
{
public:
  sIdentityMatrixOp(sOpFactory &factory) : sOp(factory) {}
  virtual void GetMatrix(sMatrix44 *mtx, sF32 time);
};

/****************************************************************************/

class sInvertMatrixOp : public sOp
{
public:
  sInvertMatrixOp(sOpFactory &factory) : sOp(factory) {}
  virtual void GetMatrix(sMatrix44 *mtx, sF32 time);
};

/****************************************************************************/

class sStoreMatrixOp : public sOp
{
public:
  sStoreMatrixOp(sOpFactory &factory) : sOp(factory) {}
  virtual void GetMatrix(sMatrix44 *mtx, sF32 time);
};

/****************************************************************************/

class sLoadMatrixOp : public sOp
{
public:
  sLoadMatrixOp(sOpFactory &factory) : sOp(factory) {}
  virtual void GetMatrix(sMatrix44 *mtx, sF32 time);
};

/****************************************************************************/

class sScaleOp : public sOp
{
public:
  sScaleOp(sOpFactory &factory) : sOp(factory) {}
  virtual void GetLocalMatrix(sVector4 &v, sMatrix44 &mtx, sF32 time);
};

/****************************************************************************/

class sLogScaleOp : public sScaleOp
{
public:
  sLogScaleOp(sOpFactory &factory) : sScaleOp(factory) {}
  virtual sBool GetValue(sVector4 &v, sF32 time);
};

/****************************************************************************/

class sTranslateOp : public sOp
{
public:
  sTranslateOp(sOpFactory &factory) : sOp(factory) {}
  virtual void GetLocalMatrix(sVector4 &v, sMatrix44 &mtx, sF32 time);
};

/****************************************************************************/

class sRotateOp : public sOp
{
public:
  sMatrix34 M;
  sRotateOp(sOpFactory &factory) : sOp(factory) {}
  virtual void GetLocalMatrix(sVector4 &v, sMatrix44 &mtx, sF32 time);
};

/****************************************************************************/

class sSkewOp : public sOp
{
public:
  sMatrix34 M;
  sSkewOp(sOpFactory &factory) : sOp(factory) {}
  virtual void GetLocalMatrix(sVector4 &v, sMatrix44 &mtx, sF32 time);
};

/****************************************************************************/

class sProjectOp : public sOp
{
public:
  sProjectOp(sOpFactory &factory) : sOp(factory) {}
  virtual void GetLocalMatrix(sVector4 &v, sMatrix44 &mtx, sF32 time);
};

/****************************************************************************/

class sRouteOp : public sOp
{
public:
  class sShape2D *Route;
  sRouteOp(sOpFactory &factory,sEntity *entity);
};

/****************************************************************************/

class sRouteRotateOp : public sRouteOp
{
public:
  sRouteRotateOp(sOpFactory &factory,sEntity *entity) : sRouteOp(factory,entity) {}
  virtual void GetLocalMatrix(sVector4 &v, sMatrix44 &mtx, sF32 time);
};

/****************************************************************************/

class sRouteTranslateOp : public sRouteOp
{
public:
  sRouteTranslateOp(sOpFactory &factory,sEntity *entity) : sRouteOp(factory,entity) {}
  virtual void GetLocalMatrix(sVector4 &v, sMatrix44 &mtx, sF32 time);
};

/****************************************************************************/

class sVerbFactory
{
public:
  class sMasagin *Owner;
  sInt Flags;
  sStackArray<class sOpFactory*, 32> Ops;

  sVerbFactory(sMasagin *owner);
  ~sVerbFactory();
  void Serialize(class sReader &s);  
  class sVerb *New(sEntity *entity);
};

/****************************************************************************/

class sVerb
{
public:
  class sMasagin *Owner;
  sInt Flags;
  class sMsgMaterial *Material;
  sStackArray<class sOp*, 32> Ops;
#if sMASAGIN_PRECALC
  sStaticArray<sMatrix44> Frames;
#endif

  sVerb(sVerbFactory &factory,sEntity *entity);
  ~sVerb();
  sF32 GetDuration();
  sF32 GetStartTime();
  sF32 GetEndTime();
  void GetMatrix(sMatrix44 *mtx, sF32 time);
};

/****************************************************************************/

#endif // FILE_MSGLIB_VERB_HPP

