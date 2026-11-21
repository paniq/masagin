/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#include "model.hpp"
#include "mesh.hpp"
#include "shape.hpp"
#include "base/system.hpp"
#include "tools.hpp"

/****************************************************************************/

sGroup2D::sGroup2D(sMasagin *owner, sModel2D *model)
{
  Owner = owner;
  Root = sNULL;
  Model = model;
}

void sGroup2D::Serialize(sReader &s)
{
  sReadMSGString(Id, s);
  sInt meshcount;
  s | meshcount;
  Meshes.HintSize(meshcount);
  for (sInt i = 0; i < meshcount; ++i)
  {
    sInt index;
    s | index;
    Meshes.AddTail(Model->Meshes[index]);
  }
  sInt shapecount;
  s | shapecount;
  Shapes.HintSize(shapecount);
  for (sInt i = 0; i < shapecount; ++i)
  {
    sInt index;
    s | index;
    AddShape(Model->Shapes[index]);
  }
}

sBool sGroup2D::IsUsingAlpha()
{
  sMesh2D *mesh;
  sFORALL(Meshes, mesh)
  {
    if (mesh->UseAlpha)
      return sTRUE;
  }
  return sFALSE;  
}

void sGroup2D::CalculateBounds()
{
  sMesh2D *mesh;
  sFORALL(Meshes, mesh)
  {
    Bounds.Add(mesh->Bounds);
  }
  Points[0].Init(Bounds.x0, Bounds.y0, 0.0f);
  Points[1].Init(Bounds.x1, Bounds.y0, 0.0f);
  Points[2].Init(Bounds.x1, Bounds.y1, 0.0f);
  Points[3].Init(Bounds.x0, Bounds.y1, 0.0f);
}

void sGroup2D::AddShape(class sShape2D *shape)
{
  if (shape->MsgClass == MSGCLASS_ANCHOR)
    Anchors.AddTail(shape);
  else if (shape->MsgClass == MSGCLASS_ROOT)
    Root = shape;
  else if (shape->MsgClass == MSGCLASS_ROUTE)
    Routes.AddTail(shape);
  Shapes.AddTail(shape);
}

void sGroup2D::GetBoundingBox(sFRect &rect, const sMatrix34 &mtx)
{  
  CalculateBounds();
  for (sInt i = 0; i != 4; ++i)
  {
    sVector31 p = Points[i] * mtx;
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

void sGroup2D::GetUpVector(sVector2 &up, const sMatrix34 &mtx)
{
  sVector31 a = sVector31(0.0f, 0.0f, 0.0f) * mtx;
  sVector31 b = sVector31(0.0f, 1.0f, 0.0f) * mtx;
  sVector30 p = b - a;
  up.x = p.x;
  up.y = p.y;
}

void sGroup2D::Draw(sEntity *entity, const sMatrix34 &mtx, const sMatrix44 &colormtx, const sMatrix44 &uvmtx)
{
  sMesh2D *mesh;
  sFORALL(Meshes, mesh)
  {
    mesh->Draw(entity,mtx,colormtx,uvmtx);
  }
}

/****************************************************************************/

sModel2D::sModel2D(sMasagin *owner)
: sGroup2D(owner)
{
}

sModel2D::~sModel2D()
{
  sDeleteAll(Meshes);
  sDeleteAll(Shapes);
  sDeleteAll(Groups);
}

void sModel2D::Serialize(sReader &s)
{
  sF32 sizex, sizey;
  s.F32(sizex);
  s.F32(sizey);
  sF32 vx,vy,vw,vh;
  s.F32(vx); s.F32(vy); s.F32(vw); s.F32(vh);
  sU32 meshcount;
  s.U32(meshcount);
  sVERIFY(meshcount < 1024);
  Meshes.HintSize(meshcount);
  for (sU32 i = 0; i < meshcount; ++i)
  {
    sMesh2D *mesh = new sMesh2D(Owner);
    mesh->Index = i;
    mesh->Serialize(s);
    Meshes.AddTail(mesh);
  } 
  s.Check();
  sU32 shapecount;
  s.U32(shapecount);
  sVERIFY(shapecount < 256);
  Shapes.HintSize(shapecount);
  for (sU32 i = 0; i < shapecount; ++i)
  {
    sShape2D *shape = new sShape2D();
    shape->Serialize(s);
    AddShape(shape);
  } 
  sU32 groupcount;
  s | groupcount;
  sVERIFY(groupcount < 256);
  Groups.HintSize(groupcount);
  for (sU32 i = 0; i < groupcount; ++i)
  {
    sGroup2D *group = new sGroup2D(Owner,this);
    group->Serialize(s);
    Groups.AddTail(group);
  }
}


/****************************************************************************/

