/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#include "mesh.hpp"
#include "shaders.hpp"
#include "base/system.hpp"
#include "base/graphics.hpp"
#include "msg.hpp"
#include "tools.hpp"
#include "entity.hpp"
#include "constants.hpp"
#include "verb.hpp"

/****************************************************************************/

sMesh2D::sMesh2D(sMasagin *owner)
{
  Owner = owner;
  Geometry = sNULL;
  Index = 0;
  UseAlpha = sFALSE;
}

sMesh2D::~sMesh2D()
{
  sDelete(Geometry);
}

void sMesh2D::Serialize(sReader &s)
{
  Geometry = new sGeometry();
  Geometry->Init(sGF_TRILIST|sGF_INDEX16,sMasagin::PathVertexFormat);

  s.Check();
  //sMatrix44 m;
  //file->Read(&m, sizeof(m));
  sF32 r,g,b,a;
  s | r;
  s | g;
  s | b;
  s | a;
  Color.Init(r,g,b,a);
  if (a < 1.0f)
    UseAlpha = sTRUE;
  RGBColor = Color;
  sRgbToHsl(Color.x, Color.y, Color.z, Color.x, Color.y, Color.z);
  sU32 vxcount;
  s.U32(vxcount);
  sVERIFY(vxcount < 65535);
  Vertices.HintSize(vxcount);
  Z = (sF32)-Index / 4096.0f;
  for (sU32 j = 0; j < vxcount; ++j)
  {
    if (!(j % 4096))
      s.Check();
    sVector31 v;
    s | v;
    sVERIFY((v.x >= 0.0f) && (v.x <= 1.0f));
    sVERIFY((v.y >= 0.0f) && (v.y <= 1.0f));
    sVERIFY((v.z >= 0.0f) && (v.z <= 1.0f));
    Vertices.AddTail(v);
  }
  //sU32 ncount;
  //s.U32(ncount);
  //sVERIFY(ncount == vxcount);
  //Normals.HintSize(ncount);
  //s.Check();
  //for (sU32 j = 0; j < ncount; ++j)
  //{
  //  sVector2 v;
  //  sS16 x,y;
  //  s.S16(x);
  //  s.S16(y);
  //  v.Init(sF32(x) / 32767.0f, sF32(y) / 32767.0f);
  //  sVERIFY((v.Length() == 0.0f) || ((v.Length() > 0.9f) && (v.Length() < 1.1f)));
  //  Normals.AddTail(v);
  //}
  s.Check();
  sU32 tccount;
  s.U32(tccount);
  sVERIFY(tccount == vxcount);
  sStaticArray<sVector2> texcoords;
  texcoords.HintSize(tccount);
  for (sU32 j = 0; j < tccount; ++j)
  {
    if (!(j % 4096))
      s.Check();
    sVector2 v;
    sU16 x,y;
    s.U16(x); s.U16(y);
    v.Init(sF32(x) / 65535.0f, sF32(y) / 65535.0f);
    texcoords.AddTail(v);
  }

  sU32 ccount;
  s.U32(ccount);
  sStaticArray<sU32> colors;
  if (ccount)
  {
    Color.Init(1.0f,1.0f,1.0f,1.0f);
    sVERIFY(ccount == vxcount);
    colors.HintSize(ccount);
    for (sU32 j = 0; j < ccount; ++j)
    {
      sU32 color;
      s.U32(color);
      colors.AddTail(color);
    }
  }

  sU32 facecount;
  s.U32(facecount);
  sVERIFY(facecount < 65535); 

  sVertexMesh2D *vp;
  sU16 *ip;
  {
    sScopeLock lock(&Owner->Lock);
    Geometry->BeginLoad(sMasagin::PathVertexFormat,sGF_TRILIST|sGF_INDEX16,sGD_STATIC,vxcount,facecount*3,(void **)&vp,(void **)&ip);
    for (sU32 j = 0; j < vxcount; ++j)
    {
      if (j == 0)
        Bounds.Init(Vertices[j].x, Vertices[j].y, Vertices[j].x, Vertices[j].y);
      else
      {
        Bounds.x0 = sMin(Bounds.x0, Vertices[j].x);
        Bounds.y0 = sMin(Bounds.y0, Vertices[j].y);
        Bounds.x1 = sMax(Bounds.x1, Vertices[j].x);
        Bounds.y1 = sMax(Bounds.y1, Vertices[j].y);
      }
      sU32 color = ~0; //0xff000000 | (sU32)(texcoords[j].x * 255)<<8 | (sU32)(texcoords[j].y * 255);
      if (ccount)
        color = colors[j];
      if ((color >> 24) != 0xff)
        UseAlpha = sTRUE;
      vp[j].Init(Vertices[j].x, Vertices[j].y, Vertices[j].z, color, texcoords[j].x, texcoords[j].y);
    }

    s.Check();

    for (sU32 j = 0; j < facecount; ++j)
    {
      if (!(j % 4096))
        s.Check();
      sS16 f[3];
      s.S16(f[0]);
      s.S16(f[1]);
      s.S16(f[2]);
      *ip++ = f[0];
      *ip++ = f[1];
      *ip++ = f[2];
    }

    Geometry->EndLoad();
  }
}

void sMesh2D::Draw(sEntity *entity, const sMatrix34 &mtx, const sMatrix44 &colormtx, const sMatrix44 &uvmtx)
{
  if (!entity || !entity->Verb || !entity->Verb->Material)
    return;
  sInt verbflags = entity->Verb->Flags;

  sCBuffer<sMsgMaterialEnvPara> cb;  
  if (verbflags & (VERB_FLAG_COLOR|VERB_FLAG_ALPHA))
  {
    sVector4 color = Color * colormtx;
    color.y = sClamp(color.y, 0.0f, 1.0f);
    color.z = sClamp(color.z, 0.0f, 1.0f);
    color.w = sClamp(color.w, 0.0f, 1.0f);
    sHslToRgb(color.x, color.y, color.z, color.x, color.y, color.z);
    cb.Data->color = color;//RGBColor;
    if ((color.w == 0.0f) && (verbflags & VERB_FLAG_ALPHA))
      return;
  }
  if (verbflags & VERB_FLAG_TEXTURE)
  {
    sVector4 grow(1.0f,1.0f,1.0f,1.0f);
    grow = grow * uvmtx;
    cb.Data->grow = grow;
  }
  cb.Modify();
  cb.Data->Set(*CurrentEnv);
  entity->Verb->Material->Set(&cb);
  Geometry->Draw();
}

/****************************************************************************/
