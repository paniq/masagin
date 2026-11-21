/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#include "entity.hpp"
#include "model.hpp"
#include "msg.hpp"
#include "shape.hpp"
#include "verb.hpp"
#include "tools.hpp"
#include "shaders.hpp"

/****************************************************************************/

sEntity::sEntity(class sMasagin *owner)
{
  Done = sFALSE;
  Owner = owner;
  Model = sNULL;
  Verb = sNULL;
  Parent = sNULL;
  Anchor = -1;
  Back = sFALSE;
  Delay = 0.0f;
  Focus = 0.0f;
  Lifespan = 0.0f;
  StepCount = 0;
}

sEntity::~sEntity()
{
  sDelete(Verb);
}

void sEntity::Step()
{
  StepTime(Owner->GetTime() - Delay, Matrix);
}

void sEntity::StepTime(sF32 time, sMatrix44 *mtx)
{
  if (IsAlive(time))
  {    
    if (Parent)
    {
      if (Parent->IsAlive(time))
      {
        for (sInt i = MTX_PARENT_MODEL; i != MTX_COUNT; ++i)
        {
          mtx[i] = Parent->Matrix[i-MTX_PARENT_MODEL];
        }
      }
    }
    StepCount++;
    mtx[MTX_MODEL].Init();
    mtx[MTX_MODELCAM].Init();
    mtx[MTX_COLOR].Init();
    mtx[MTX_TEXTURE].Init();
    mtx[MTX_CAMERA].Init();
    if (Verb)
    {
      Verb->GetMatrix(mtx, time);
    }
    else
    {
      mtx[MTX_MODEL] = mtx[MTX_ROOT] * mtx[MTX_ANCHOR] * mtx[MTX_PARENT_MODEL];
      mtx[MTX_MODELCAM] = mtx[MTX_MODEL];
    }
    sVector4 cam(0.0f, 0.0f, 0.0f, 1.0f);
    if (time < Focus)
    {
      // very simple and stupid blend
      sF32 x = time / Focus;
      if (x < 0.25f)
        x = x / 0.25f;
      else if (x < 0.75f)
        x = 1.0f;
      else
        x = (1.0f - x) / 0.25f;
      x = x*x;
      cam.x = x;
      cam.y = 0.0f;
    }
    Camera = cam * mtx[MTX_CAMERA];
  }
}

void sEntity::GetUpVector(sVector2 &up)
{
  if (GetUpWeight() <= 0.0f)
    return;
  sMatrix34 mtx(Matrix[MTX_MODELCAM]);
  if (Model)
    Model->GetUpVector(up, mtx);
  else
  {
    sVector31 a = sVector31(0.0f, 0.0f, 0.0f) * mtx;
    sVector31 b = sVector31(0.0f, 1.0f, 0.0f) * mtx;
    sVector30 p = b - a;
    up.x = p.x;
    up.y = p.y;
  }
}

void sEntity::CalcBoundingBox(sFRect &rect, const sMatrix34 &mtx)
{
  if (Model)
  {
    Model->GetBoundingBox(rect, mtx);
  }
  else
  {
    sVector31 points[4];
    points[0].Init(0.0f, 0.0f, 0.0f);
    points[1].Init(1.0f, 0.0f, 0.0f);
    points[2].Init(1.0f, 1.0f, 0.0f);
    points[3].Init(0.0f, 1.0f, 0.0f);
    for (sInt i = 0; i != 4; ++i)
    {
      sVector31 p = points[i] * mtx;
      if (!i)
      {
        rect.Init(p.x, p.y, p.x, p.y);
      }
      else
      {
        rect.x0 = sMin(rect.x0, p.x);
        rect.y0 = sMin(rect.y0, p.y);
        rect.x1 = sMax(rect.x1, p.x);
        rect.y1 = sMax(rect.y1, p.y);
      }
    }
  }
}

sBool sEntity::UpdateBoundingBox(const sMatrix34 &mtx)
{
  CalcBoundingBox(BoundingBox, sMatrix34(Matrix[MTX_MODELCAM]) * mtx);
  return sTRUE;
}

void sEntity::Draw()
{
  if (!IsAlive())
    return;
  if (Verb)
  {
    if (Verb->Flags & VERB_FLAG_ZCLEAR0)
      Owner->ZClear0();
    if (Verb->Flags & VERB_FLAG_ZCLEAR1)
      Owner->ZClear1();
  }
  if (Model)
  {
    sVERIFY(CurrentViewport);
    sVERIFY(CurrentEnv);
    CalcBoundingBox(ModelBoundingBox, sMatrix34(Matrix[MTX_MODEL]));
    CurrentViewport->UpdateModelMatrix(sMatrix34(Matrix[MTX_MODEL]));
    CurrentViewport->Set();    
    Model->Draw(this, sMatrix34(Matrix[MTX_MODEL]), Matrix[MTX_COLOR], Matrix[MTX_TEXTURE]);
  }
}

sBool sEntity::IsAlive(sF32 time)
{
  if (time < 0)
    return sFALSE;
  if (Lifespan && (time >= Lifespan))
    return sFALSE;
  return sTRUE;
}

sBool sEntity::IsAlive()
{
  return IsAlive(Owner->GetTime() - Delay);
}

sF32 sEntity::GetBBWeight()
{
  return sClamp(Camera.x, 0.0f, 1.0f);
}

sF32 sEntity::GetUpWeight()
{
  return sClamp(Camera.y, 0.0f, 1.0f);
}

sF32 sEntity::GetTiltWeight()
{
  return sClamp(Camera.z, 0.0f, 1.0f);
}

sBool sEntity::IsFocusTime()
{
  sF32 time = Owner->GetTime() - Delay;
  if (time < 0)
    return sFALSE;
  if (time >= Focus)
    return sFALSE;
  return sTRUE;
}

void sEntity::Serialize(sReader &s)
{
  Owner->LoadStep();
  s.Check();

  sReadMSGString(Source, s);

  s.Check();

  sInt modelindex;
  s | modelindex;
  sModel2D *model = sNULL;
  if (modelindex >= 0)
    model = Owner->Models[modelindex];
  Model = model;
  sString<64> groupid;
  sReadMSGString(groupid, s);
  if (!groupid.IsEmpty() && model)
  {
    sGroup2D *group;
    sFORALL(model->Groups, group)
    {
      if (group->Id == groupid)
      {
        Model = group;
        break;
      }
    }
  }
  sInt verbindex;
  s | verbindex;
  if (verbindex >= 0)
    Verb = Owner->Verbs[verbindex]->New(this);
  s | Anchor;
  s | Back;
  if (Back)
    Owner->MoveEntityBack(this);
  if (Model && Model->Root)
  {
    Matrix[MTX_ROOT] = Model->Root->Matrix;
  }
  
  sFuncValue delay,focus,lifespan;
  delay.Serialize(s);
  focus.Serialize(s);
  lifespan.Serialize(s);
  Delay = delay.Get();
  Focus = focus.Get();
  Lifespan = lifespan.Get();

  sInt childcount;
  s | childcount;
  for (sInt i = 0; i < childcount; ++i)
  {
    sEntity *ent = Owner->NewEntity(sNULL);
    ent->Parent = this;
    ent->Serialize(s);
    if (Model && Model->Anchors.GetCount())
    {
      sInt anchor = ((ent->Anchor == -1)?i:ent->Anchor) % Model->Anchors.GetCount();
      ent->Matrix[MTX_ANCHOR] = Model->Anchors[anchor]->Matrix;
    }
  }
}

/****************************************************************************/

