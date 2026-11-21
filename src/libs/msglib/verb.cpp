/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#include "verb.hpp"
#include "base/system.hpp"
#include "fader.hpp"
#include "msg.hpp"
#include "funcvalue.hpp"
#include "entity.hpp"
#include "shape.hpp"
#include "model.hpp"
#include "tools.hpp"
#include "shaders.hpp"

/****************************************************************************/

sOpFactory::sOpFactory(sMasagin *owner)
{
  Owner = owner;
  Fader = sNULL;
}

void sOpFactory::Serialize(class sReader &s)
{
  s.Check();
  s | Type;
  sVERIFY(Type < OP_COUNT);
  s | Index;
  s | Index2;
  sReadFuncValue(Time[0], s);
  sReadFuncValue(Time[1], s);
  sInt count;
  s | count;
  sVERIFY(count == 4);
  sReadFuncValue(Range[0][0], s);
  sReadFuncValue(Range[0][1], s);
  sReadFuncValue(Range[0][2], s);
  sReadFuncValue(Range[0][3], s);
  s | count;
  sVERIFY(count == 4);
  sReadFuncValue(Range[1][0], s);
  sReadFuncValue(Range[1][1], s);
  sReadFuncValue(Range[1][2], s);
  sReadFuncValue(Range[1][3], s);
  s | Flags;
  sInt faderindex;
  s | faderindex;
  if (faderindex != -1)
    Fader = Owner->Faders[faderindex];
}

class sOp *sOpFactory::New(sEntity *entity)
{
  switch (Type)
  {
    case OP_SCALE: return new sScaleOp(*this);
    case OP_LOGSCALE: return new sLogScaleOp(*this);
    case OP_TRANSLATE: return new sTranslateOp(*this);
    case OP_ROTATE: return new sRotateOp(*this);
    case OP_SKEW: return new sSkewOp(*this);
    case OP_PROJECT: return new sProjectOp(*this);
    case OP_MUL_MATRIX: return new sMulMatrixOp(*this);
    case OP_STORE_MATRIX: return new sStoreMatrixOp(*this);
    case OP_IDENTITY_MATRIX: return new sIdentityMatrixOp(*this);
    case OP_INVERT_MATRIX: return new sInvertMatrixOp(*this);
    case OP_LOAD_MATRIX: return new sLoadMatrixOp(*this);
    case OP_TRANSLATE_ROUTE: return new sRouteTranslateOp(*this,entity);
    case OP_ROTATE_ROUTE: return new sRouteRotateOp(*this,entity);
    case OP_SET_GLOBAL: return new sSetGlobalOp(*this,entity);
  }
  sVERIFYFALSE;
  return sNULL;
}

/****************************************************************************/

sVerbFactory::sVerbFactory(sMasagin *owner)
{
  Flags = 0;
  Owner = owner;
}

sVerbFactory::~sVerbFactory()
{
  sDeleteAll(Ops);
}

void sVerbFactory::Serialize(class sReader &s)
{
  s | Flags;
  sU32 opcount;
  s | opcount;
  for (sU32 i = 0; i < opcount; ++i)
  {
    sOpFactory *op = new sOpFactory(Owner);
    op->Serialize(s);
    Ops.AddTail(op);
  }
}

class sVerb *sVerbFactory::New(sEntity *entity)
{
  return new sVerb(*this,entity);
}

/****************************************************************************/

sOp::sOp(sOpFactory &factory)
{
  Index = factory.Index;
  Index2 = factory.Index2;
  Fader = sNULL;
  Owner = factory.Owner;
  Type = factory.Type;
  Time[0] = factory.Time[0].Get();
  Time[1] = factory.Time[1].Get();
  Range[0].x = factory.Range[0][0].Get();
  Range[0].y = factory.Range[0][1].Get();
  Range[0].z = factory.Range[0][2].Get();
  Range[0].w = factory.Range[0][3].Get();
  Range[1].x = factory.Range[1][0].Get();
  Range[1].y = factory.Range[1][1].Get();
  Range[1].z = factory.Range[1][2].Get();
  Range[1].w = factory.Range[1][3].Get();
  Flags = factory.Flags;
  if (factory.Fader)
    Fader = factory.Fader->New();
  if (Fader && Fader->GetDuration())
  {
    if (Time[1] == Time[0])
      Time[1] = Time[0] + Fader->GetDuration();
  }   
}

sOp::~sOp()
{
  sDelete(Fader);
}

sBool sOp::GetPosition(sF32 &x, sF32 time)
{
  sF32 range = Time[1] - Time[0];
  if (range == 0.0f)
    x = Time[0];
  else
  {
    x = (time - Time[0]) / range;
    if (Flags & OP_FLAG_CLAMP)
      x = sMax(x, 0.0f);
    else if (x < 0.0f)
      return sFALSE;
    if (Flags & OP_FLAG_LOOP)
      x = sMod(x, 1.0f);
    else if (Flags & OP_FLAG_CLAMP)
      x = sMin(x, 1.0f);
    else if (x >= 1.0f)
      return sFALSE;
  }

  if (Fader)
  {
    x = Fader->Translate(x);
  }
  return sTRUE;
}

sBool sOp::GetValue(sVector4 &v, sF32 time)
{
  sF32 x;
  if (!GetPosition(x, time))
    return sFALSE;
  v = Range[0] + (Range[1] - Range[0]) * x;
  if (Flags & OP_FLAG_UNIFORM)
  {
    v.x *= v.z;
    v.y *= v.z;
    v.z = 1.0f;
  }
  return sTRUE;
}

void sOp::GetMatrix(sMatrix44 *mtx, sF32 time)
{
  sVector4 v;
  if (!GetValue(v,time))
    return;
  sMatrix44 m;
  GetLocalMatrix(v, m,time);
  mtx[Index] *= m;
}

/****************************************************************************/

void sSetGlobalOp::GetMatrix(sMatrix44 *mtx, sF32 time)
{
  sF32 x;
  if (!GetPosition(x, time))
    return;
  switch(Index)
  {
  case GLOBAL_BGCOLOR:
    {
      sVector4 color = sVector4(0.0f, 0.0f, 0.0f, 1.0f) * mtx[Index2];
      color.y = sClamp(color.y, 0.0f, 1.0f);
      color.z = sClamp(color.z, 0.0f, 1.0f);
      color.w = sClamp(color.w, 0.0f, 1.0f);
      sHslToRgb(color.x, color.y, color.z, color.x, color.y, color.z);
      Owner->BackgroundColor = color.GetColor();
    } break;
  }
}

/****************************************************************************/

void sMulMatrixOp::GetMatrix(sMatrix44 *mtx, sF32 time)
{
  mtx[Index2] *= mtx[Index];
}

/****************************************************************************/

void sIdentityMatrixOp::GetMatrix(sMatrix44 *mtx, sF32 time)
{
  mtx[Index].Init();
}

/****************************************************************************/

void sInvertMatrixOp::GetMatrix(sMatrix44 *mtx, sF32 time)
{
  mtx[Index].Invert();
}

/****************************************************************************/

void sStoreMatrixOp::GetMatrix(sMatrix44 *mtx, sF32 time)
{
  mtx[Index] = mtx[Index2];
}

/****************************************************************************/

void sLoadMatrixOp::GetMatrix(sMatrix44 *mtx, sF32 time)
{
  mtx[Index2] = mtx[Index];
}

/****************************************************************************/

void sScaleOp::GetLocalMatrix(sVector4 &v, sMatrix44 &mtx, sF32 time)
{
  mtx.Scale(v.x, v.y, v.z);
  mtx.l.w *= v.w;
}

/****************************************************************************/

sBool sLogScaleOp::GetValue(sVector4 &v, sF32 time)
{
  sF32 x;
  if (!GetPosition(x, time))
    return sFALSE;
  sF32 y = (1.0f - x);
  for (sInt i = 0; i < 4; ++i)
  {
    v[i] = sExp(sLog(Range[0][i]+0.000001f)*y + sLog(Range[1][i]+0.000001f) * x);
  }
  if (Flags & OP_FLAG_UNIFORM)
  {
    v.x *= v.z;
    v.y *= v.z;
    v.z = 1.0f;
  }
  return sTRUE;
}

/****************************************************************************/

void sTranslateOp::GetLocalMatrix(sVector4 &v, sMatrix44 &mtx, sF32 time)
{
  mtx.l.x += v.x;
  mtx.l.y += v.y;
  mtx.l.z += v.z;
  mtx.l.w += v.w;
}

/****************************************************************************/

void sRotateOp::GetLocalMatrix(sVector4 &v, sMatrix44 &mtx, sF32 time)
{
  M.EulerXYZ(v.x, v.y, v.z);
  mtx *= M;
}


/****************************************************************************/

void sSkewOp::GetLocalMatrix(sVector4 &v, sMatrix44 &mtx, sF32 time)
{
  M.i.y = sTan(v.y);
  M.j.x = sTan(v.x);
  mtx *= M;
}

/****************************************************************************/

void sProjectOp::GetLocalMatrix(sVector4 &v, sMatrix44 &mtx, sF32 time)
{  
  sMatrix44 mat;
  mat.k.x = 0.2; // x += z*0.5
  mat.k.y = 0.1; // y += z*0.5
  mat.k.z = 0;   // z = 0

  mtx *= mat;
}

/****************************************************************************/

sRouteOp::sRouteOp(sOpFactory &factory,sEntity *entity) : sOp(factory)
{
  Route = sNULL;
  sGroup2D *model = sNULL;
  if (entity && entity->Parent && entity->Parent->Model)
    model = entity->Parent->Model;
  if (model && model->Routes.GetCount())
    Route = model->Routes[Index2 % model->Routes.GetCount()];
}

/****************************************************************************/

void sRouteRotateOp::GetLocalMatrix(sVector4 &v, sMatrix44 &mtx, sF32 time)
{
  if (!Route)
    return;
  sF32 span1 = v.x;
  sF32 span2 = v.y;
  if (sAbs(span2 - span1) < 0.0001f)
  {
    span2 = span1 + 0.0001f;
  }
  sVector2 s,t;
  Route->GetPoint(v.z-span1, s);
  Route->GetPoint(v.z+span2, t);
  sF32 a = sATan2(t.y - s.y, t.x - s.x);// - sPI*0.5f;
  sMatrix34 m;
  m.EulerXYZ(0.0f, 0.0f, a);
  mtx *= m;
}

/****************************************************************************/

void sRouteTranslateOp::GetLocalMatrix(sVector4 &v, sMatrix44 &mtx, sF32 time)
{
  if (!Route)
    return;
  sVector2 pt;
  Route->GetPoint(v.z, pt);
  mtx.l.x += pt.x;
  mtx.l.y += pt.y;
}

/****************************************************************************/

sVerb::sVerb(sVerbFactory &factory, sEntity *entity)
{
  Owner = factory.Owner;
  Flags = factory.Flags;
  sBool useTexture = sFALSE;
  sBool useAlpha = sFALSE;
  sOpFactory *opfactory;
  sFORALL(factory.Ops, opfactory)
  {
    sOp *op = opfactory->New(entity);
    if (op)
    {
      switch(op->Type)
      {
        case OP_SCALE:
        case OP_TRANSLATE:
        case OP_ROTATE:
        case OP_TRANSLATE_ROUTE:
        case OP_ROTATE_ROUTE:
        case OP_MUL_MATRIX:
        case OP_IDENTITY_MATRIX:
        case OP_STORE_MATRIX:
        case OP_INVERT_MATRIX:
        case OP_LOAD_MATRIX:
          {
            if ((op->Index == MTX_TEXTURE)||(op->Index2 == MTX_TEXTURE))
              useTexture = sTRUE;
            if ((op->Index == MTX_COLOR)||(op->Index2 == MTX_COLOR))
              useAlpha = sTRUE;
          } break;
        default: break;
      }
      Ops.AddTail(op);
    }
  }
  if (!Flags) // automatic
  {
    if (entity->Model)
    {
      sGroup2D *model = entity->Model;
      Flags = VERB_FLAG_COLOR;
      if (useAlpha || model->IsUsingAlpha())
        Flags |= VERB_FLAG_ALPHA;
      if (useTexture)
      {
        Flags |= VERB_FLAG_TEXTURE | VERB_FLAG_ALPHA;
      }
    }
  }
  Material = Owner->CreateMaterial(Flags);
}

sVerb::~sVerb()
{
  sDeleteAll(Ops);
}

sF32 sVerb::GetDuration()
{
  return GetEndTime() - GetStartTime();
}

sF32 sVerb::GetEndTime()
{
  sF32 maxtime = 0.0f;
  sOp *op;
  sFORALL(Ops, op)
  {
    maxtime = sMax(maxtime, op->Time[1]);
  }
  return maxtime;
}

sF32 sVerb::GetStartTime()
{
  sF32 start = 999999.0f;
  sOp *op;
  sFORALL(Ops, op)
  {
    start = sMin(start, op->Time[0]);
  }
  return start;
}

void sVerb::GetMatrix(sMatrix44 *mtx, sF32 time)
{
#if sMASAGIN_PRECALC
  time = sClamp(time, 0.0f, (sF32)(Frames.GetCount()-1)/60.0f);
  mtx = Frames[(sInt)(time * 60.0f)];
#else
  sOp *op;
  sFORALL(Ops, op)
  {
    op->GetMatrix(mtx, time);
  }
#endif
}

/****************************************************************************/

