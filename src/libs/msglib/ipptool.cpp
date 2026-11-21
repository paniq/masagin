/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) by Dierk Ohlerich                                    ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#include "base/graphics.hpp"
#include "base/system.hpp"
#include "ipptool.hpp"
#include "util/shaders.hpp"
#include "ippmtrl.hpp"

/****************************************************************************/

#define MAXTARGETS sCOUNTOF(sIppTool::Targets)

sBool sIppToolMtrlCache::operator== (const sIppToolMtrlCache &a) const
{
  if(BlendColor!=a.BlendColor) return 0;
  if(BlendAlpha!=a.BlendAlpha) return 0;
  if(BlendFactor!=a.BlendFactor) return 0;
  if(Flags!=a.Flags) return 0;
  if(Specials!=a.Specials) return 0;
  if(TexCount!=a.TexCount) return 0;
  return 1;
}

void sIppToolBlit::Init()
{
  Flags = sMTRL_CULLOFF|sMTRL_ZOFF;
  Specials = 0;
  BlendColor = sMB_OFF;
  BlendAlpha = sMB_SAMEASCOLOR;
  BlendFactor = 0xffffffff;
  Dest = 0;
  Source0 = 0;
  Source1 = 0;
  Source2 = 0;
  Source3 = 0;
  ColorAdd.Init(0,0,0,0);
  ColorMul.Init(0,0,0,0);
  Color0.Init(1,1,1,1);
  Color1.Init(1,1,1,1);
  SourceFlipped = 0;
}

sIppTool::sIppTool()
{
  for(sInt i=0;i<MAXTARGETS;i++)
    Targets[i] = 0;
  Geo = new sGeometry;
  Geo->Init(sGF_QUADLIST,sVertexFormatDouble);

  SpreadMtrl = new sIppToolMaterial;
  SpreadMtrl->Specials = sIPS_SPREAD8;
  SpreadMtrl->Texture[0] = (sTexture2D *) (void *)1;
  SpreadMtrl->TFlags[0] = sMTF_LEVEL2|sMTF_BORDER;
  SpreadMtrl->Prepare(sVertexFormatDouble);
}

sIppTool::~sIppTool()
{
  sIppToolMtrlCache *cache;
  delete Geo;
  delete SpreadMtrl;
  for(sInt i=0;i<MAXTARGETS;i++)
    delete Targets[i];
  sFORALL(Materials,cache)
    delete cache->Mtrl;
}

sMaterial *sIppTool::FindMaterial(const sIppToolBlit &blit)
{
  sIppToolMtrlCache cache;

  cache.Flags = blit.Flags;
  cache.Specials = blit.Specials;
  cache.TexCount = 0;
  if(blit.Source0)
  {
    cache.TexCount++;
    if(blit.Source1) 
      cache.TexCount++;
  }
  cache.BlendColor = blit.BlendColor;
  cache.BlendAlpha = blit.BlendAlpha;
  cache.BlendFactor = blit.BlendFactor;

  sIppToolMtrlCache *found;
  found  = sFind(Materials,cache);
  if(!found)
  {
    sIppToolMaterial *mtrl = new sIppToolMaterial;
    mtrl->Flags = cache.Flags;
    mtrl->Specials = cache.Specials;
    mtrl->BlendColor = cache.BlendColor;
    mtrl->BlendAlpha = cache.BlendAlpha;
    mtrl->BlendFactor = cache.BlendFactor;
    mtrl->Texture[0] = blit.Source0;
    mtrl->Texture[1] = blit.Source1;
    mtrl->Texture[2] = blit.Source2;
    mtrl->Texture[3] = blit.Source3;
    if(cache.Specials & sIPS_POINTFILTER)
    {
      mtrl->TFlags[0] = sMTF_LEVEL0|sMTF_CLAMP;
      mtrl->TFlags[1] = sMTF_LEVEL0|sMTF_CLAMP;
    }
    else
    {
      if((cache.Specials & 15)==sIPS_SPREAD8)
      {
        mtrl->TFlags[0] = sMTF_LEVEL2|sMTF_MIRROR;
        mtrl->TFlags[1] = sMTF_LEVEL2|sMTF_MIRROR;
      }
      else if((cache.Specials & 15)==sIPS_SPREAD8FAR)
      {
        mtrl->TFlags[0] = sMTF_LEVEL2|sMTF_MIRROR;
        mtrl->TFlags[1] = sMTF_LEVEL2|sMTF_MIRROR;
      }
      else
      {
        mtrl->TFlags[0] = sMTF_LEVEL2|sMTF_CLAMP;
        mtrl->TFlags[1] = sMTF_LEVEL2|sMTF_CLAMP;
      }
    }
    mtrl->Prepare(sVertexFormatDouble);
    cache.Mtrl = mtrl;
    Materials.AddTail(cache);
    found = &cache;
  }
  return found->Mtrl;
}

/****************************************************************************/

void sIppTool::AddTargetAbs(sInt id,sInt xs,sInt ys,sInt format)
{
  sVERIFY(id>=1 && id<32);
  sVERIFY(Targets[id]==0);
  Targets[id] = new sTexture2D;
  Targets[id]->Init(xs,ys,format|sTEX_2D|sTEX_RENDERTARGET|sTEX_NOMIPMAPS);
}

void sIppTool::AddTargetRel(sInt id,sInt xshift,sInt yshift,sInt format)
{
  sInt xs,ys;
  sGetScreenSize(xs,ys);
  AddTargetAbs(id,xs>>xshift,ys>>yshift,format);
}

sTexture2D *sIppTool::GetTarget(sInt id)
{
  if(id==0) return 0;
  sVERIFY(id>=1 && id<32);
  sVERIFY(Targets[id]);
  return Targets[id];
}

void sIppTool::Grab(sInt id)
{
  sGrabScreen(GetTarget(id));
//  sDPrintF(L"[ipp] grab %d\n",id);
}

void sIppTool::InitBlit(sIppToolBlit &blit,sInt d,sInt s0,sInt s1,sInt s2, sInt s3)
{
  blit.Init();
  blit.Dest = GetTarget(d);
  blit.Source0 = GetTarget(s0);
  blit.Source1 = GetTarget(s1);
  blit.Source2 = GetTarget(s2);
  blit.Source3 = GetTarget(s3);
#if sRENDERER==sRENDER_PS3GL
  blit.SourceFlipped |= 3;
#endif
//  sDPrintF(L"[ipp] %d %d -> %d (flip %d)\n",s0,s1,d,blit.SourceFlipped);
}

/****************************************************************************/

void sIppTool::Prepare()
{
}

void sIppTool::Blit(const sIppToolBlit &blit,sInt flags)
{
  sVertexDouble *vp;
  sCBuffer<sIppToolMaterialVSPara> vspara;
  sCBuffer<sIppToolMaterialPSPara> pspara;
  sMaterial *mtrl;

  vspara.Modify();
  pspara.Modify();

  sSetRendertarget(0,blit.Dest,0,sCLEAR_NONE);
  View.Orthogonal = sVO_SCREEN;
  View.SetTargetCurrent();
  View.Prepare();
  View.Set();
  sGetMatrix(vspara.Data->mvp,sGM_MODELSCREEN);
  sClear(*pspara.Data);
  pspara.Data->ColorAdd = blit.ColorAdd;
  pspara.Data->ColorMul = blit.ColorMul;
  pspara.Data->Color0 = blit.Color0;
  pspara.Data->Color1 = blit.Color1;

  mtrl = FindMaterial(blit);
  mtrl->Texture[0] = blit.Source0;
  mtrl->Texture[1] = blit.Source1;
  mtrl->Texture[2] = blit.Source2;
  mtrl->Texture[3] = blit.Source3;

  sF32 u,v;
  sInt xs,ys;
  // Vulkan samples texel centres exactly, so the half-texel blit offsets are off by
  // default; define MSG_VK_BLIT_TEXEL_OFFSET to sample half a texel off-centre instead.
#ifndef MSG_VK_BLIT_TEXEL_OFFSET
  u = 0;
  v = 0;
#else
  if(flags & sIPB_NOOFFSET)
  {
    u = 0;
    v = 0;
  }
  else
  {
    sGetRendertargetSize(xs,ys);
    if (flags & sIPB_OFFSETODD)
    {
      // 0.375 seems to be the most consistant on nvidia cards (no visible difference)
      u = 0.375f/xs; 
      v = 0.375f/ys;
    }
    else
    {
      u = (-0.5f)/xs;
      v = (-0.5f)/ys;
    }
  }
#endif

  Geo->BeginLoadVB(4,sGD_STREAM,(void **)&vp);
  sInt flip = blit.SourceFlipped;
  switch(flip)
  {
  case 0:
    vp[0].Init(0,0,0,0,0+u,0+v,0+u,0+v);
    vp[1].Init(1,0,0,0,1+u,0+v,1+u,0+v);
    vp[2].Init(1,1,0,0,1+u,1+v,1+u,1+v);
    vp[3].Init(0,1,0,0,0+u,1+v,0+u,1+v);
    break;
  case 1:
    vp[0].Init(0,0,0,0,0+u,1-(0+v),0+u,0+v);
    vp[1].Init(1,0,0,0,1+u,1-(0+v),1+u,0+v);
    vp[2].Init(1,1,0,0,1+u,1-(1+v),1+u,1+v);
    vp[3].Init(0,1,0,0,0+u,1-(1+v),0+u,1+v);
    break;
  case 2:
    vp[0].Init(0,0,0,0,0+u,0+v,0+u,1-(0+v));
    vp[1].Init(1,0,0,0,1+u,0+v,1+u,1-(0+v));
    vp[2].Init(1,1,0,0,1+u,1+v,1+u,1-(1+v));
    vp[3].Init(0,1,0,0,0+u,1+v,0+u,1-(1+v));
    break;
  case 3:
    vp[0].Init(0,0,0,0,0+u,1-(0+v),0+u,1-(0+v));
    vp[1].Init(1,0,0,0,1+u,1-(0+v),1+u,1-(0+v));
    vp[2].Init(1,1,0,0,1+u,1-(1+v),1+u,1-(1+v));
    vp[3].Init(0,1,0,0,0+u,1-(1+v),0+u,1-(1+v));
    break;
  }
  Geo->EndLoadVB();
  mtrl->Set(&vspara,&pspara);
  Geo->Draw();
}

void sIppTool::BlitSpread8(const sIppToolBlit &blit,sF32 u,sF32 v)
{
  sVertexDouble *vp;
  sCBuffer<sIppToolMaterialVSPara> vspara;
  sCBuffer<sIppToolMaterialPSPara> pspara;
  sMaterial *mtrl;

  sSetRendertarget(0,blit.Dest,0,sCLEAR_NONE);
  View.Orthogonal = sVO_SCREEN;
  View.SetTargetCurrent();
  View.Prepare();
  View.Set();
  sGetMatrix(vspara.Data->mvp,sGM_MODELSCREEN);
  sClear(*pspara.Data);
  pspara.Data->ColorAdd = blit.ColorAdd;
  //pspara.Data->ColorMul = blit.ColorMul;
  pspara.Data->Color0 = blit.Color0;
  pspara.Data->Color1 = blit.Color1;

  // TODO: use sFindMaterial(blit,BLAFLAGS_SPREAD8)

  mtrl = FindMaterial(blit);
  mtrl->Texture[0] = blit.Source0;

  sF32 f = -2.652f;
  // The tap offset f above centres the blur on its source: with the taps placed raw the
  // blur sits up-left of the picture it blurs (measured: -3.9px on a 640x400 screen),
  // and shifting the sampling by one destination texel brings it back to within 0.03px.
  // Set MSG_VK_SPREAD_OFFSET to 0 for the raw placement.
#ifndef MSG_VK_SPREAD_OFFSET
#define MSG_VK_SPREAD_OFFSET 1.0f
#endif
  sInt dxs,dys;
  sGetRendertargetSize(dxs,dys);
  sF32 du = u*f - MSG_VK_SPREAD_OFFSET/dxs;
  sF32 dv = v*f - MSG_VK_SPREAD_OFFSET/dys;
  Geo->BeginLoadVB(4,sGD_STREAM,(void **)&vp);
#if sRENDERER==sRENDER_PS3GL
  vp[0].Init(0,0,0,0,0+du,1-(0+dv),u,-v);
  vp[1].Init(1,0,0,0,1+du,1-(0+dv),u,-v);
  vp[2].Init(1,1,0,0,1+du,1-(1+dv),u,-v);
  vp[3].Init(0,1,0,0,0+du,1-(1+dv),u,-v);
#else
  vp[0].Init(0,0,0,0,0+du,0+dv,u,v);
  vp[1].Init(1,0,0,0,1+du,0+dv,u,v);
  vp[2].Init(1,1,0,0,1+du,1+dv,u,v);
  vp[3].Init(0,1,0,0,0+du,1+dv,u,v);
#endif
  Geo->EndLoadVB();
  mtrl->Set(&vspara,&pspara);
  Geo->Draw();
}

/****************************************************************************/

void sIppToolGlarePara::Init()
{
  SubColor = 0x808080;
  ImageColor = 0xc0c0c0;
  GlareColor = 0xffffff;
  Amplify = 1.5f;
  Radius0 = 3.0f;
  Radius1 = 12.0f;
  Source0 = 1;
  Source1 = 2;
  Source2 = 3;
}

void sIppTool::Glare(const sIppToolGlarePara &para)
{
  sIppToolBlit blit;
  sInt xs,ys;
  sGetScreenSize(xs,ys);
  sF32 ixs = 1.0f/xs;
  sF32 iys = 1.0f/ys;
  const sF32 f = 0.125f;
  const sF32 g0 =   1 / 128.0f;
  const sF32 g1 =   7 / 128.0f;
  const sF32 g2 =  21 / 128.0f;
  const sF32 g3 =  35 / 128.0f;
  sF32 a = para.Amplify;

  // subtract

  InitBlit(blit,para.Source1,para.Source0);
  blit.ColorAdd.InitColor(para.SubColor);
  blit.ColorAdd *= -1;
  //blit.Color0.Init(para.Amplify,para.Amplify,para.Amplify,para.Amplify);
  //blit.Specials = sIPS_GRAY;
  Blit(blit, sIPB_OFFSETODD);

  // first stage

  if(para.Radius0>0)
  {
    InitBlit(blit,para.Source2,para.Source1);
    blit.Specials = sIPS_SPREAD8;
    blit.Color0.Init(g0*a,g1*a,g2*a,g3*a);
    blit.Color1.Init(g3*a,g2*a,g1*a,g0*a);
    BlitSpread8(blit,0.5f*ixs,para.Radius0*iys);

    InitBlit(blit,para.Source1,para.Source2);
    blit.Specials = sIPS_SPREAD8;
    blit.Color0.Init(g0*a,g1*a,g2*a,g3*a);
    blit.Color1.Init(g3*a,g2*a,g1*a,g0*a);
    BlitSpread8(blit,para.Radius0*ixs,0.5f*iys);
  }

  // second stage

  if(para.Radius1>0.0f)
  {
    InitBlit(blit,para.Source2,para.Source1);
    blit.Specials = sIPS_SPREAD8;
    blit.Color0.Init(f,f,f,f);
    blit.Color1.Init(f,f,f,f);
    BlitSpread8(blit,para.Radius1*ixs,-0.5f*iys);

    InitBlit(blit,para.Source1,para.Source2);
    blit.Specials = sIPS_SPREAD8;
    blit.Color0.Init(f,f,f,f);
    blit.Color1.Init(f,f,f,f);
    BlitSpread8(blit,-0.5f*ixs,para.Radius1*iys);
  }

  // combine

  InitBlit(blit,para.Source0,para.Source1);
  blit.Specials = sIPS_POINTFILTER;
  blit.BlendColor = sMBS_1 | sMBO_ADD | sMBD_F;
  blit.Color0.InitColor(para.GlareColor);
  blit.BlendFactor = para.ImageColor;
  Blit(blit, sIPB_OFFSETODD);
}

/****************************************************************************/

void sIppTool::Dof(sF32 maxdisti,sF32 focaldist,sF32 depth,sF32 radius,sBool debugblur)
{
  sIppToolBlit blit;
  sInt xs,ys;
  sGetScreenSize(xs,ys);
  sF32 ixs = 1.0f/xs;
  sF32 iys = 1.0f/ys;
  const sF32 g0 =   9 / 510.0f *8;
  const sF32 g1 =  36 / 510.0f *8;
  const sF32 g2 =  84 / 510.0f *8;
  const sF32 g3 = 126 / 510.0f *8;

  // sample down

  InitBlit(blit,2,1);
  Blit(blit);

  // blur

  InitBlit(blit,4,2,2);
  blit.Specials = sIPS_SPREAD8FAR;
  blit.ColorAdd.w = 0.01f;
  blit.Color0.Init(g0,g1,g2,g3);
  blit.Color1.Init(g3,g2,g1,g0);
  BlitSpread8(blit,0.5*ixs,radius*iys);

  InitBlit(blit,3,4,2);
  blit.Specials = sIPS_SPREAD8FAR;
  blit.ColorAdd.w = 0.01f;
  blit.Color0.Init(g0,g1,g2,g3);
  blit.Color1.Init(g3,g2,g1,g0);
  BlitSpread8(blit,radius*ixs,-0.5*iys);

  // merge

  if(0+debugblur)
  {
    InitBlit(blit,1,3);
    Blit(blit);
  }
  else
  {
    InitBlit(blit,0,1,3);
    blit.Color1.z = depth;        // dist multiplier. zoom in and get more blurring
    blit.Color1.w = focaldist*maxdisti;       // reference depth, zMax = 16
    blit.Specials = sIPS_DOF|sIPS_POINTFILTER;
    blit.BlendColor = sMB_ALPHA;
    Blit(blit,sIPB_NOOFFSET);
    Grab(1);
  }
}

/****************************************************************************/

void sIppTool::Boost(sF32 radius0,sF32 radius1,sF32 radius2,sU32 min,sU32 max)
{
  sIppToolBlit blit;
  sInt xs,ys;
  sGetScreenSize(xs,ys);
  sF32 ixs = 1.0f/xs;
  sF32 iys = 1.0f/ys;
  const sF32 f = 0.125f;
  const sF32 g0 =   1 / 128.0f;
  const sF32 g1 =   7 / 128.0f;
  const sF32 g2 =  21 / 128.0f;
  const sF32 g3 =  35 / 128.0f;
  sF32 a = 1;

  // copy

  InitBlit(blit,2,1);
  Blit(blit);

  // first stage

  if(radius0>0)
  {
    InitBlit(blit,3,2);
    blit.Specials = sIPS_SPREAD8;
    blit.Color0.Init(g0*a,g1*a,g2*a,g3*a);
    blit.Color1.Init(g3*a,g2*a,g1*a,g0*a);
    BlitSpread8(blit,0.5f*ixs,radius0*iys);

    InitBlit(blit,2,3);
    blit.Specials = sIPS_SPREAD8;
    blit.Color0.Init(g0*a,g1*a,g2*a,g3*a);
    blit.Color1.Init(g3*a,g2*a,g1*a,g0*a);
    BlitSpread8(blit,radius0*ixs,0.5f*iys);
  }

  // second stage

  if(radius1>0)
  {
    InitBlit(blit,3,2);
    blit.Specials = sIPS_SPREAD8;
    blit.Color0.Init(f,f,f,f);
    blit.Color1.Init(f,f,f,f);
    BlitSpread8(blit,radius1*ixs,0.5f*iys);

    InitBlit(blit,2,3);
    blit.Specials = sIPS_SPREAD8;
    blit.Color0.Init(f,f,f,f);
    blit.Color1.Init(f,f,f,f);
    BlitSpread8(blit,0.5f*ixs,radius1*iys);
  }

  // third stage

  if(radius2>0)
  {
    InitBlit(blit,3,2);
    blit.Specials = sIPS_SPREAD8;
    blit.Color0.Init(f,f,f,f);
    blit.Color1.Init(f,f,f,f);
    BlitSpread8(blit,radius2*ixs,0.5f*iys);

    InitBlit(blit,2,3);
    blit.Specials = sIPS_SPREAD8;
    blit.Color0.Init(f,f,f,f);
    blit.Color1.Init(f,f,f,f);
    BlitSpread8(blit,0.5f*ixs,radius2*iys);
  }

  // combine

  InitBlit(blit,0,1,2);
  blit.Specials = sIPS_BOOST;
  blit.Color0.InitColor(min);
  blit.Color1.InitColor(max);
  Blit(blit);

  Grab(1);
}

/****************************************************************************/
