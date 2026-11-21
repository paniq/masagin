/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#include "msg.hpp"
#include "verb.hpp"
#include "base/system.hpp"
#include "entity.hpp"
#include "model.hpp"
#include "base/graphics.hpp"
#include "fader.hpp"
#include "base/sound.hpp"
#include "tools.hpp"
#include "vorbisplayer.hpp"
#include "shaders.hpp"
#undef HEADER_SHADERS_ASC
#include "util/shaders.hpp"
#include "mesh.hpp"
#include "util/painter.hpp"
#include "util/image.hpp"
#include "constants.hpp"
#include "ipptool.hpp"

sViewport *CurrentViewport = sNULL;
sMaterialEnv *CurrentEnv = sNULL;

sU32 Mesh2DVertexFormatDesc[] = { sVF_POSITION,/*sVF_NORMAL,*/sVF_COLOR0,sVF_UV0,0 };
struct sVertexFormatHandle *sMasagin::PathVertexFormat = sNULL;

sF32 sAngleDiff(sF32 alpha, sF32 beta)
{
  sF32 a=sFrac(alpha/sPI2F); 
  sF32 b=sFrac(beta/sPI2F);
  sF32 r=b-a;
  if (r<-0.5f) r+=1;
  if (r>0.5f) r-=1;
  return r * sPI2F;
}

sAutoCam::sAutoCam(sMasagin *owner)
{
  Owner = owner;
  Reset();
}

void sAutoCam::Reset()
{
  Tilt = 0.0f;
  Zoom = 1.0f;
  Angle = 0.0f;
  Pos.Init(0.0f, 0.5f);
}

void sAutoCam::SetTarget(sF32 zoom, sF32 angle, sVector2 pos)
{
  Zoom = zoom;
  Angle = angle;
  Pos = pos;
}

sF32 sAutoCam::GetZoomMagnitude()
{
  if (Zoom == 0.0f)
    return 0.0f;
  return sLog(Zoom) / sLog(2.0f);
}

sF32 sAutoCam::GetPositionMagnitude()
{
  sF32 length = Pos.Length();
  if (!length)
    return 0.0f;
  return sLog(length) / sLog(2.0f);
}

sF32 sAutoCam::GetMagnitudeRange()
{
  return sAbs(GetZoomMagnitude() - GetPositionMagnitude());
}

void sAutoCam::ResetStep()
{
}

void sAutoCam::Step()
{  
  //sDPrintF(L"angle=%.2f, zoom=%.2f, x=%.2f, y=%.2f\n", CurrentAngle, CurrentZoom, CurrentPos.x, CurrentPos.y);
}

void sAutoCam::Get(sMatrix34 &mtx, sF32 zoom, sF32 angle, sVector2 pos, sF32 z)
{
  sMatrix34 skew;
  sF32 xrot = sCos(angle);
  sF32 yrot = sSin(angle);

  sF32 s = 0.05f;
  skew.RotateAxis(sVector30(xrot, yrot, 0.0f), s * sPI2F * Tilt);

  sMatrix34 skewtrans;
  skewtrans.l.y -= xrot * s * 7.07f * zoom * zoom * 0.5f * Tilt;
  skewtrans.l.x += yrot * s * 7.07f * zoom * zoom * 0.5f * Tilt;

  sMatrix34 trans;
  trans.l.Init(pos.x, pos.y, 0.0f);
  sMatrix34 rot;
  rot.RotateAxis(sVector30(0.0f, 0.0f, 1.0f), -angle);
  sMatrix34 scale;
  if (z != 0.0f)
    zoom = 1.0f / zoom;
  scale.Scale(zoom, zoom, 1.0f);
  mtx.Init();
  mtx = skew * skewtrans * scale * rot * trans;
  mtx.l.z = z;
}

void sAutoCam::GetCurrent(sMatrix34 &mtx, sF32 z)
{
  Get(mtx, Zoom, Angle, Pos, z);
}

void sAutoCam::Get(sViewport &view)
{
  view.Camera.Init();
  GetCurrent(view.Camera, -0.5f);
  view.SetZoom(1.0f);
}

/****************************************************************************/

class sTimeSourceVorbis : public sTimeSource
{
public:
  sBool Seekable;
  sBool Playing;
  bVorbisPlayer *Player;
  sString<256> Path;

  void SetSeekable(sBool seekable)
  {
    if (Seekable == seekable)
      return;
    Seekable = seekable;
    if (!Seekable)
      return;
    sBool playing = Playing;
    sF32 t = sMax(Player->GetPosition(),0.0f);
    Player->Exit();
    sDelete(Player);
    Player = new bVorbisPlayer(Seekable);
    Playing = sFALSE;
    Player->Init(Path);
    Player->Seek(t);
    if (playing)
      Play();
  }

  sTimeSourceVorbis(const sChar *path)
  {
    Seekable = sGetShellSwitch(L"seekable");
    Path = path;
    Player = new bVorbisPlayer(Seekable);
    Playing = sFALSE;
    Player->Init(Path);
  }

  virtual ~sTimeSourceVorbis()
  { 
    Player->Exit(); 
    sDelete(Player);
  }
  
  virtual void Play()
  {
    Player->Play();
    Playing = sTRUE;
  }
  virtual sBool IsPlaying()
  { 
    return Playing;
  }
  virtual void Stop()
  {
    Player->Pause();
    Playing = sFALSE;
  }
  virtual void SetTime(sF32 t)
  {
    t = sMax(t,0.0f);
    if (t != 0.0f)
    {
      SetSeekable(sTRUE);
    }
    Player->Seek(t);
  }
  virtual sF32 GetTime()
  { 
    return Player->GetPosition(); 
  }

  virtual sF32 GetLength()
  {
    return Player->GetLength();
  }
};

/****************************************************************************/

class sTimeSourceSystem : public sTimeSource
{
public:
  sInt Time;
  sF32 Position;
  sBool Playing;

  sTimeSourceSystem()
  {
    Playing = sFALSE;
    Time = 0.0f;
    Position = 0.0f;
  }
  virtual void Play()
  {
    Playing = sTRUE;
    Time = sGetTime();
  }
  virtual sBool IsPlaying()
  { 
    return Playing;
  }
  virtual void Stop()
  {
    Playing = sFALSE;
  }
  virtual void SetTime(sF32 t)
  {
    Position = sMax(t, 0.0f);
  }
  virtual sF32 GetTime()
  {
    if (Playing)
    {
      sInt time = sGetTime();
      sInt delta = time - Time;
      Position += (sF32)delta / 1000.0f;
      Time = time;
    }
    return Position; 
  }
};

/****************************************************************************/

sLoadingTimeSource::sLoadingTimeSource()
{
}

sLoadingTimeSource::~sLoadingTimeSource()
{
}

void sLoadingTimeSource::Play()
{
}

sBool sLoadingTimeSource::IsPlaying()
{
  return sTRUE;
}

void sLoadingTimeSource::Stop()
{
}

void sLoadingTimeSource::SetTime(sF32 t)
{
  
}

sF32 sLoadingTimeSource::GetTime()
{
  if (!Progress->TotalLoadingUnits)
    return 0.0f;
  return sClamp(((sF32)Progress->CurrentLoadingUnit * 100.0f) / Progress->TotalLoadingUnits, 0.1f, 100.0f);
}

sF32 sLoadingTimeSource::GetLength()
{
  return 100.0f;
}

/****************************************************************************/

sMasagin::sMasagin()
{
#if sRELEASE
  DebugView = sFALSE;
#else
  DebugView = sTRUE;
#endif
  sScreenMode sm;
  sGetScreenMode(sm);
  MotionBlur = MSG_MB_16_I;
  FSAA = sTRUE;
  Slow = sTRUE;
  RenderSerialize = sFALSE;
  Bloom = sTRUE;
  BloomRadius = 1.0f;
  BloomStrength = 1.0f;
  LoopBegin = -1.0f;
  LoopEnd = -1.0f;
  //Aspect = 16.0f/10.0f; // most pc's have this, bigscreen will be cut down a bit
  Aspect = 16.0f/9.0f; // optimal bigscreen format, black bars for regular pcs
  //Aspect = 1.85f; // widescreen
  //Aspect = 2.35f; // cinerama
  if (!PathVertexFormat)
    PathVertexFormat = sCreateVertexFormat(Mesh2DVertexFormatDesc);
  Init();
  //Frames.HintSize((100*(1<<20)) / sizeof(sMsgFrame)); // 100mb for frames
}

sMasagin::~sMasagin()
{
  Cleanup();
}

void sMasagin::Init()
{
  sScopeLock lock(&Lock);
  TotalLoadingUnits = 0;
  CurrentLoadingUnit = 0;
  FramesRendered = 0; 
  Painter = new sPainter;
  LastWriteTime = 0;
  Root = sNULL;

  BPM = 120.0f;

  Time = 0.0f;
  CamPrecog = 0.0f;

  BackgroundColor = 0xff000000;
  TimeSource = sNULL;
  Cam = new sAutoCam(this);

  LineMaterial = new sSimpleMaterial;
  LineMaterial->Flags = sMTRL_ZOFF;
  LineMaterial->Prepare(sVertexFormatBasic);
  LineGeometry = new sGeometry;
  LineGeometry->Init(sGF_LINELIST,sVertexFormatBasic);
  QuadGeometry = new sGeometry;
  QuadGeometry->Init(sGF_QUADLIST,sVertexFormatBasic);

  sClear(Env);
  Env.AmbientColor  = ~0;
  Env.Fix();

  sScreenMode sm;
  sGetScreenMode(sm);

  sU32 rez = 256;
  AvgFPS = 0.0f;

  PathTexture = new sTexture2D(rez,rez,sTEX_ARGB8888|sTEX_2D,5);

  sImage *img = new sImage(rez,rez);
  sU32 *p = img->Data;
  for (sU32 y = 0; y < rez; y++)
  {
    for (sU32 x = 0; x < rez; x++)
    {
      if ((x < (rez/2)) && (y >= (rez/2)))
        *p++ = ~0;
      else
        *p++ = 0x00ffffff;
    }
  }

  sImageData *id = new sImageData(img,sTEX_ARGB8888|sTEX_2D);
  PathTexture->LoadAllMipmaps(id->Data);

  sDelete(id);
  sDelete(img);

  ZClearMaterial = CreateMaterial(VERB_FLAG_ZWRITE);

  Ipp = new sIppTool();
  Ipp->AddTargetRel(1, 0, 0);
  Ipp->AddTargetRel(2, 0, 0);
  Ipp->AddTargetRel(3, 0, 0);
  Ipp->AddTargetRel(4, 0, 0);
  Ipp->AddTargetRel(5, 0, 0);
  Ipp->AddTargetRel(6, 0, 0);
  Ipp->AddTargetRel(7, 0, 0);
  Ipp->AddTargetRel(8, 0, 0);
  Ipp->AddTargetRel(9, 1, 1);
  Ipp->AddTargetRel(10, 1, 1);
}

sMsgMaterial *sMasagin::CreateMaterial(sInt verbmask)
{
  sScopeLock lock(&Lock);
  sMsgMaterial *mat = sNULL;
  sFORALL(PathMaterials, mat)
  {
    if (mat->VerbFlags == verbmask)
      return mat;
  }
  mat = sNULL;
  if (verbmask & VERB_FLAG_TEXTURE)
  {
    mat = new sMsgMaterialGrowable;
    mat->Texture[0] = PathTexture;
    mat->TFlags[0] = sMTF_LEVEL2|sMTF_CLAMP;
  }
  else
    mat = new sMsgMaterial;
  mat->VerbFlags = verbmask;
  mat->Flags = 0;
  sInt maskflags = 0;
  if (verbmask & VERB_FLAG_COLOR)
  {
    mat->Flags |= sMTRL_VC_COLOR0;
  }
  else
  {
    maskflags = sMTRL_MSK_RGBA;
  }
  if (verbmask & VERB_FLAG_ZREAD)
    mat->Flags |= sMTRL_ZREAD;
  if (verbmask & VERB_FLAG_ZWRITE)
    mat->Flags |= sMTRL_ZWRITE;
  sInt both = VERB_FLAG_ADDCOLOR|VERB_FLAG_SUBCOLOR;
  if ((verbmask & both) == both)
  {
    if (verbmask & VERB_FLAG_ALPHA)
      mat->BlendColor = sMBS_1 | sMBO_SUB | sMBD_1;
    else
      mat->BlendColor = sMBS_1 | sMBO_SUB | sMBD_1;
  }
  else if (verbmask & VERB_FLAG_ADDCOLOR)
  {
    if (verbmask & VERB_FLAG_ALPHA)
      mat->BlendColor = sMBS_SA | sMBO_ADD | sMBD_1;
    else
      mat->BlendColor = sMB_ADD;
  }
  else if (verbmask & VERB_FLAG_SUBCOLOR)
  {
    if (verbmask & VERB_FLAG_ALPHA)
      mat->BlendColor = sMBS_SA | sMBO_SUBR | sMBD_1;
    else
      mat->BlendColor = sMB_SUB;
  }
  else if (verbmask & VERB_FLAG_MULCOLOR)
  {
    //if (verbmask & VERB_FLAG_ALPHA)
    //  mat->BlendColor = sMBS_SC | sMBO_MIN | sMBD_1;
    //else
    mat->BlendColor = sMB_MUL;
  }
  else if (verbmask & VERB_FLAG_ALPHA)
  {
    mat->BlendColor = sMB_ALPHA;
    if ((verbmask & VERB_FLAG_TEXTURE) && (verbmask & VERB_FLAG_ZWRITE))
    {
      mat->FuncFlags[sMFT_ALPHA] = sMFF_GREATEREQUAL;
      mat->AlphaRef = 80;
      maskflags = sMTRL_MSK_RED|sMTRL_MSK_GREEN|sMTRL_MSK_BLUE;
    }
  }
  mat->Flags |= maskflags;
  mat->FuncFlags[sMFT_DEPTH] = sMFF_LESSEQUAL;
  mat->Prepare(PathVertexFormat);
  PathMaterials.AddTail(mat);
  return mat;
}

void sMasagin::Cleanup()
{
  sScopeLock lock(&Lock);
  sDelete(Ipp);
  sDelete(Painter);
  sDelete(TimeSource);
  sDelete(Cam);
  sDeleteAll(Verbs);
  sDeleteAll(Models);
  sDeleteAll(Entities);
  sDeleteAll(Faders);
  sDelete(LineMaterial);
  sDelete(LineGeometry);
  sDelete(QuadGeometry);
  
  sDeleteAll(PathMaterials);
  sDelete(PathTexture);
}

void sMasagin::PaintBoundingBoxes()
{
  sEntity *entity;
  sFORALL(Entities, entity)
  {
    if (entity->IsAlive() && entity->Model)
    {
      sMatrix34 mtx(entity->Matrix[MTX_MODEL]);
      sMesh2D *mesh;
      sFORALL(entity->Model->Meshes, mesh)
      {
        PaintRect(mesh->Bounds, mtx, 0xffff00ff, 0xff00ff00);
      }
    }
  }
}

void sMasagin::PaintNormals()
{
  //const sF32 l = 0.005f;

  //sEntity *entity;
  //sFORALL(Entities, entity)
  //{
  //  if (entity->IsAlive() && entity->Model)
  //  {
  //    sMatrix34 mtx(entity->Matrix[MTX_MODEL]);
  //    sMesh2D *mesh;
  //    sFORALL(entity->Model->Meshes, mesh)
  //    {
  //      sVector31 *v;
  //      sFORALL(mesh->Vertices, v)
  //      {
  //        sVector2 n = mesh->Normals[_i];
  //        sVector31 v1(v->x, v->y, v->z);
  //        if (v->z > 0.5f)
  //          continue;
  //        sVector31 v2(v->x + n.x*l, v->y + n.y*l, v->z);
  //        v1 *= mtx;
  //        v2 *= mtx;
  //        Line(v1,v2,  0x40ff00ff, 0xff800000);
  //      }
  //    }
  //  }
  //}
}

sBool sMasagin::LoadScript(const sChar *path)
{
  ScriptPath = path;
  sBool result = sFALSE;
  sInt pathpos = sFindLastChar(path, '\\');
  if (pathpos == -1)
    pathpos = sFindLastChar(path, '/');
  if (pathpos != -1)
  {
    sString<128> folder = path;
    folder[pathpos] = '\0';
    sChangeDir(folder);
    // the folder is the working directory now, so the script is named without it
    ScriptPath = path + pathpos + 1;
  }  
  if (!sCmpStringI(&ScriptPath[ScriptPath.Count()-4], L".bsm"))
  {
    TimeSource = new sTimeSourceSystem();
    sModel2D *model = new sModel2D(this);
    sLoadObject(ScriptPath, model);
    Models.AddTail(model);
    Root = NewEntity(sNULL);
    Root->Model = model;
    Root->Focus = 999999.0f;
    Root->Lifespan = 0.0f;
    result = sTRUE;
  }
  else
  {
    result = sLoadObject(ScriptPath, this);
  }
  if (result)
    LastWriteTime = GetLastWriteTime();
  //PrerunScene();
  return result;
}

void sMasagin::PrerunScene()
{
  sEntity *entity;
  sF32 maxtime = 0.0f;
  sFORALL(Entities, entity)
  {
    maxtime = sMax(maxtime, entity->Delay + entity->Lifespan);
  }
  sInt steps = 60;
  sF32 length = sInt(maxtime+0.5f) * steps;
  sDPrintF(L"prerunning scene (%.2f/%i frames) ", maxtime, length);
  sInt delta = 0;
  sInt time = 0;  
  while (time < length)
  {
    RefreshEntities();
    sEntity *entity;
    sFORALL(Entities, entity)
    {
      StepEntity(entity);
    }
    time++;
    delta++;
    Time = (sF32)time / MSG_FPS;
    if (delta == MSG_FPS)
    {
       delta = 0;
       sDPrintF(L".");
    }
  }
  sDPrintF(L"OK\n");
  Time = 0.0f;
}

sU64 sMasagin::GetLastWriteTime()
{
  sDirEntry entry;
  if (sGetFileInfo(ScriptPath, &entry))
  {
    return entry.LastWriteTime;
  }
  else
  {
    sDPrintF(L"ERROR: couldn't retrieve info for '%s'.\n", ScriptPath);
  }
  return LastWriteTime;
}

sBool sMasagin::DocumentChanged()
{
  return (LastWriteTime != GetLastWriteTime())?sTRUE:sFALSE;
}

void sMasagin::Reload()
{
  sF32 lb = -1.0f;
  sF32 le = -1.0f;
  sF32 pos = 0.0f;
  if (TimeSource)
    pos = TimeSource->GetTime();
  sString<256> path = ScriptPath;
  Reset();  
  LoadScript(ScriptPath);
  Play();
  if (TimeSource)
  {
    TimeSource->SetTime(pos);
  }
}

void sMasagin::Reset()
{
  Cleanup();
  Init();
}

void sMasagin::PaintCam()
{
  sMatrix34 mtx;
  Cam->GetCurrent(mtx, 0.0f);
  PaintRect(sFRect(-0.5f,-0.5f,0.5f,0.5f), mtx, 0xff0000ff, 0xffff0000);
}

void sMasagin::PaintRect(const sFRect &rect, sU32 color, sMaterial *mat)
{
  sVector31 v1(rect.x0, rect.y0, 0.0f);
  sVector31 v2(rect.x1, rect.y0, 0.0f);
  sVector31 v3(rect.x1, rect.y1, 0.0f);
  sVector31 v4(rect.x0, rect.y1, 0.0f);
  SolidQuad(v1,v2,v3,v4,color,mat);
}

void sMasagin::PaintRect(const sFRect &rect, sMatrix34 &mtx, sU32 bottom, sU32 color)
{
  sVector31 v1(rect.x0, rect.y0, 0.0f);
  sVector31 v2(rect.x1, rect.y0, 0.0f);
  sVector31 v3(rect.x1, rect.y1, 0.0f);
  sVector31 v4(rect.x0, rect.y1, 0.0f);
  v1 *= mtx;
  v2 *= mtx;
  v3 *= mtx;
  v4 *= mtx;
  Quad(v1,v2,v3,v4,bottom,color,color,color);
}

void sMasagin::SolidQuad(sVector31 v1, sVector31 v2, sVector31 v3, sVector31 v4, sU32 color, sMaterial *mat)
{
  sVertexBasic *vp;

  if (!mat)
  {
    sCBuffer<sSimpleMaterialPara> cb; cb.Data->Set();
    LineMaterial->Set(&cb);
  }

  QuadGeometry->BeginLoadVB(4,sGD_STREAM,(void **)&vp);
  vp->Init(v1.x, v1.y, v1.z, color); vp++;
  vp->Init(v2.x, v2.y, v2.z, color); vp++;
  vp->Init(v3.x, v3.y, v3.z, color); vp++;
  vp->Init(v4.x, v4.y, v4.z, color); vp++;
  QuadGeometry->EndLoadVB();
  QuadGeometry->Draw();
}

void sMasagin::Quad(sVector31 v1, sVector31 v2, sVector31 v3, sVector31 v4, sU32 color1, sU32 color2, sU32 color3, sU32 color4)
{
  if (!color2)
    color2 = color1;
  if (!color3)
    color3 = color2;
  if (!color4)
    color4 = color3;
  sVertexBasic v[8];
  v[0].Init(v1.x, v1.y, v1.z, color1);
  v[1].Init(v2.x, v2.y, v2.z, color1);
  v[2].Init(v2.x, v2.y, v2.z, color2);
  v[3].Init(v3.x, v3.y, v3.z, color2);
  v[4].Init(v3.x, v3.y, v3.z, color3);
  v[5].Init(v4.x, v4.y, v4.z, color3);
  v[6].Init(v4.x, v4.y, v4.z, color4);
  v[7].Init(v1.x, v1.y, v1.z, color4);
  Lines(v, 4);
}

void sMasagin::Line(sVector31 v1, sVector31 v2, sU32 color, sU32 color2)
{
  if (!color2)
    color2 = color;
  sVertexBasic v[2];
  v[0].Init(v1.x, v1.y, v1.z, color);
  v[1].Init(v2.x, v2.y, v2.z, color2);
  Lines(v, 1);
}

void sMasagin::Lines(sVertexBasic *src,sInt linecount)
{
  sVertexBasic *vp;

  sCBuffer<sSimpleMaterialPara> cb; cb.Data->Set();
  LineMaterial->Set(&cb);

  LineGeometry->BeginLoadVB(linecount*2,sGD_STREAM,(void **)&vp);
  sCopyMem(vp,src,sizeof(sVertexBasic)*linecount*2);
  LineGeometry->EndLoadVB();
  LineGeometry->Draw();
}

sEntity *sMasagin::NewEntity(sEntity *after)
{
  sEntity *ent = new sEntity(this);
  if (!after)
    Entities.AddTail(ent);
  else
    Entities.AddAfter(ent, Entities.IndexOf(after));
  return ent;
}

void sMasagin::Play()
{
  Cam->Reset();
  TimeSource->SetTime(0.0f);
  TimeSource->Play();
}

void sMasagin::ToggleMotionBlur()
{
  MotionBlur = (MotionBlur+1)%5;
}

void sMasagin::ToggleLoop()
{
  sF32 lb,le;
  lb = LoopBegin;
  le = LoopEnd;
  if (lb >= 0.0f)
  {
    if (le < 0)
    {
      le = sMin(TimeSource->GetTime(), TimeSource->GetLength());
    }
    else
    {
      lb = -1.0f;
      le = -1.0f;
    }
  }
  else
  {
    lb = sMax(TimeSource->GetTime(), 0.0f);
  }
  LoopBegin = lb;
  LoopEnd = le;
}

void sMasagin::TogglePause()
{
  if (TimeSource->IsPlaying())
  {
    TimeSource->Stop();
  }
  else
  {
    TimeSource->Play();
  }
}

void sMasagin::MoveTimeFwd()
{
  sF32 delta = TimeSource->IsPlaying()?5:0.017f;
  TimeSource->SetTime(TimeSource->GetTime() + delta);
}

void sMasagin::MoveTimeBack()
{
  sF32 delta = TimeSource->IsPlaying()?-5:-0.017f;
  TimeSource->SetTime(TimeSource->GetTime() + delta);
}

void sMasagin::Stop()
{
  TimeSource->Stop();
}

sF32 sMasagin::GetRealTime()
{
  return TimeSource->GetTime();
}

sF32 sMasagin::GetTime()
{
  return Time;
}

void sMasagin::MBGrab(sInt index)
{
  Ipp->Grab(index);
}

void sMasagin::MBBlit(sInt d, sInt s0, sInt s1, sInt s2, sInt s3)
{
  sIppToolBlit blit;
  blit.Init();
  sF32 c = 2;
  if (s2)
    c++;
  if (s3)
    c++;
  c = 1.0f/c;
  blit.Color0.Init(c,c,c,1.0f);
  blit.Color1 = blit.Color0;
  blit.Dest = Ipp->GetTarget(d);
  blit.Source0 = Ipp->GetTarget(s0);
  blit.Source1 = Ipp->GetTarget(s1);
  blit.Source2 = Ipp->GetTarget(s2);
  blit.Source3 = Ipp->GetTarget(s3);
  Ipp->Blit(blit, sIPB_OFFSETODD);
}

void sMasagin::MB16x2(sInt frame)
{
  // 15 blits
  switch(frame)
  {
    case 0x0: MBGrab(1); break;
    case 0x1: MBGrab(2); MBBlit(3, 1, 2); break;
    case 0x2: MBGrab(1); break;
    case 0x3: MBGrab(2); MBBlit(4, 1, 2); MBBlit(5, 3, 4); break;
    case 0x4: MBGrab(1); break;
    case 0x5: MBGrab(2); MBBlit(3, 1, 2); break;
    case 0x6: MBGrab(1); break;
    case 0x7: MBGrab(2); MBBlit(4, 1, 2); MBBlit(6, 3, 4); MBBlit(7, 5, 6); break;
    case 0x8: MBGrab(1); break;
    case 0x9: MBGrab(2); MBBlit(3, 1, 2); break;
    case 0xa: MBGrab(1); break;
    case 0xb: MBGrab(2); MBBlit(4, 1, 2); MBBlit(5, 3, 4); break;
    case 0xc: MBGrab(1); break;
    case 0xd: MBGrab(2); MBBlit(3, 1, 2); break;
    case 0xe: MBGrab(1); break;
    case 0xf: MBGrab(2); MBBlit(4, 1, 2); MBBlit(6, 3, 4); MBBlit(8, 5, 6); MBBlit(0, 7, 8); break;
  }
}

void sMasagin::MB8x2(sInt frame)
{
  // 7 blits
  switch(frame)
  {
    case 0x0: MBGrab(1); break;
    case 0x1: MBGrab(2); MBBlit(3, 1, 2); break;
    case 0x2: MBGrab(1); break;
    case 0x3: MBGrab(2); MBBlit(4, 1, 2); MBBlit(5, 3, 4); break;
    case 0x4: MBGrab(1); break;
    case 0x5: MBGrab(2); MBBlit(3, 1, 2); break;
    case 0x6: MBGrab(1); break;
    case 0x7: MBGrab(2); MBBlit(4, 1, 2); MBBlit(6, 3, 4); MBBlit(0, 5, 6); break;
  }
}

void sMasagin::MB16x4(sInt frame)
{
  // 5 blits
  switch(frame)
  {
    case 0x0: MBGrab(1); break;
    case 0x1: MBGrab(2); break;
    case 0x2: MBGrab(3); break;
    case 0x3: MBGrab(4); MBBlit(5, 1, 2, 3, 4); break;
    case 0x4: MBGrab(1); break;
    case 0x5: MBGrab(2); break;
    case 0x6: MBGrab(3); break;
    case 0x7: MBGrab(4); MBBlit(6, 1, 2, 3, 4); break;
    case 0x8: MBGrab(1); break;
    case 0x9: MBGrab(2); break;
    case 0xa: MBGrab(3); break;
    case 0xb: MBGrab(4); MBBlit(7, 1, 2, 3, 4); break;
    case 0xc: MBGrab(1); break;
    case 0xd: MBGrab(2); break;
    case 0xe: MBGrab(3); break;
    case 0xf: MBGrab(4); MBBlit(8, 1, 2, 3, 4); MBBlit(0, 5, 6, 7, 8); break;
  }
}

void sMasagin::MB8x4(sInt frame)
{
  // 3 blits
  switch(frame)
  {
    case 0x0: MBGrab(1); break;
    case 0x1: MBGrab(2); break;
    case 0x2: MBGrab(3); break;
    case 0x3: MBGrab(4); MBBlit(5, 1, 2, 3, 4); break;
    case 0x4: MBGrab(1); break;
    case 0x5: MBGrab(2); break;
    case 0x6: MBGrab(3); break;
    case 0x7: MBGrab(4); MBBlit(6, 1, 2, 3, 4); MBBlit(0, 5, 6); break;
  }
}

void sMasagin::Draw(sInt frame)
{
  // set rendertarget
  sSetRendertarget(0,sCLEAR_ALL,BackgroundColor);

  if (frame < 0)
    return;

  sF32 realaspect = View.TargetAspect;
  View.TargetAspect = 1.0f;
  Cam->Get(View);
  View.ZoomX *= 1.0f/Aspect;
  View.ZoomY *= realaspect * 1.0f/Aspect;
  View.TargetAspect = realaspect;

  sF32 rvx = 0;
  sF32 rvy = 0;
  if (MotionBlur && FSAA)
  {
    sF32 vx=0;
    sF32 vy=0;
    sF32 ox = 1.0f/4.0f; // 1/6 or 1/4 is both good.
    vx = ((sF32)(frame % 4)-1.5f)*ox;
    sF32 oy = ox;
    if ((MotionBlur == MSG_MB_8)||(MotionBlur == MSG_MB_8_I))
    {
      oy = 1.0f/2.0f;
      vy = ((sF32)(frame / 4)-0.5f)*oy;
    }
    else
    {
      vy = ((sF32)(frame / 4)-1.5f)*oy;
    }
    sF32 a = sPIF * 0.16667f; // turn sampling kernel by 30°
    //if (FramesRendered%2)
    //  a = sPIF * 0.25f;
    sF32 s,c;
    sFSinCos(a, s,c);
    rvx = c*vx - s*vy;
    rvy = s*vx + c*vy;
  }
  //if (FramesRendered%2)
  //  rvy += 0.5f;
  View.CenterX = 0.5f + rvx / (sF32)View.TargetSizeX;
  View.CenterY = 0.5f + rvy / (sF32)View.TargetSizeY;

  View.Prepare();
  View.Set();

  RefreshEntities();
  sEntity *entity;
  sFORALL(Entities, entity)
  {
    DrawEntity(entity);
  }

  if (MotionBlur)
  {
    const sInt tu = 4; // texture units
    sInt steps = 16;
    if ((MotionBlur == MSG_MB_8)||(MotionBlur == MSG_MB_8_I))
      steps = 8;
    if (tu == 2)
    {
      if (steps == 16)
        MB16x2(frame);
      else if (steps == 8)
        MB8x2(frame);
    }
    else if (tu == 4)
    {
      if (steps == 16)
        MB16x4(frame);
      else if (steps == 8)
        MB8x4(frame);
    }
  }
}

void sMasagin::DrawScreenBorders()
{
  RTView.Orthogonal = sVO_PIXELS;
  RTView.SetTargetCurrent();
  RTView.Prepare();
  RTView.Set();

  sInt xs,ys;
  sGetScreenSize(xs,ys);

  sFRect rcscreen(0,0,xs,ys);
  sFRect rcsafe(rcscreen);

  sF32 aspect = sGetScreenAspect();// rcscreen.SizeX() / rcscreen.SizeY();

  if (aspect < Aspect) // traditional screen resolution
  {
    rcsafe.ScaleCenter(1.0f, aspect / Aspect);
  }
  else
  {
    return;
  }

  // paint safe area
  // left
  PaintRect(sFRect(rcscreen.x0, rcscreen.y0, rcsafe.x0, rcscreen.y1), 0xff000000);
  // top
  PaintRect(sFRect(rcsafe.x0, rcscreen.y0, rcsafe.x1, rcsafe.y0), 0xff000000);
  // right
  PaintRect(sFRect(rcsafe.x1, rcscreen.y0, rcscreen.x1, rcscreen.y1), 0xff000000);
  // bottom
  PaintRect(sFRect(rcsafe.x0, rcsafe.y1, rcsafe.x1, rcscreen.y1), 0xff000000);  
}

void sMasagin::GetViewplanePoint(sVector31 &v, sF32 x, sF32 y, sF32 z)
{
  sRay ray;
  View.MakeRay(x,y,ray);
  v = ray.Start + ray.Dir*((z-ray.Start.z) / ray.Dir.z);
}

void sMasagin::ZClear(sF32 z)
{
  View.UpdateModelMatrix(sMatrix34());
  View.Set();

  sVector31 v[4];

  sFRect rc(2.1f,2.1f);
  rc.ScaleCenter(2,2);
  rc.MoveCenter(0.0f, 0.0f);

  GetViewplanePoint(v[3], rc.x0, rc.y0, z);
  GetViewplanePoint(v[2], rc.x1, rc.y0, z);
  GetViewplanePoint(v[1], rc.x1, rc.y1, z);
  GetViewplanePoint(v[0], rc.x0, rc.y1, z);

  sCBuffer<sMsgMaterialEnvPara> cb;
  cb.Modify();
  cb.Data->Set();
  ZClearMaterial->Set(&cb);
  SolidQuad(v[0], v[1], v[2], v[3], 0xff00ff00, ZClearMaterial);
}

void sMasagin::ZClear0()
{
  ZClear(0.1f);
}

void sMasagin::ZClear1()
{
  ZClear(-0.1f);
}

void sMasagin::LoadStep()
{
  CurrentLoadingUnit++;
}

void sMasagin::Paint(sViewport *view)
{
  // reset stats
  ClipCount = 0;

  // get timing
  Timer.OnFrame(sGetTime());
  sInt time = Timer.GetTime();

  sTiming Timing;
  Timing.OnFrame(sGetTime());

  if (!view)
    view = &View;
  else
    View = *view;
  CurrentViewport = view;
  CurrentEnv = &Env;

  Timing.OnFrame(sGetTime());
  sInt time1 = Timing.GetDelta();

  view->SetTargetCurrent();
  Step();

  Timing.OnFrame(sGetTime());
  sInt time2 = Timing.GetDelta();
  AvgFPS = 1000.0f/Timer.GetAverageDelta();

  // set rendertarget
  sSetRendertarget(0,sCLEAR_NONE);

  if (Bloom)
  {
    sIppToolGlarePara para;
    para.Init();
    para.Source1 = 9;
    para.Source2 = 10;
    para.SubColor = 0xff000000;
    para.ImageColor = 0xffc0c0c0;   // image weight 0.75 + glare weight 0.25 == 1.0, so flat colours stay put
    sU32 gl = sClamp(sInt(0x40*BloomStrength+0.5f),0,255);
    para.GlareColor = 0xff000000 | (gl*0x010101);
    if (BloomStrength!=1.0f)    // keep image weight + glare weight == 1 for any strength (else flat white turns grey)
    {
      sU32 ic = sClamp(0xff-sInt(gl)+1,0,255);
      para.ImageColor = 0xff000000 | (ic*0x010101);
    }
    para.Radius0 = 3.0f*BloomRadius;
    para.Radius1 = 0.0f;
    para.Amplify = 1.0f;
    Ipp->Grab(1);
    Ipp->Glare(para);
    sIppToolBlit blit;
    Ipp->InitBlit(blit, 0, 1);
    Ipp->Blit(blit, sIPB_OFFSETODD);
  }

  DrawScreenBorders();

  if (DebugView)
  {
    sStackArray<sString<128>, 80> textarray;

    // debug output
    Painter->SetTarget();

    sString<64> loopinfo;
    if (LoopBegin < 0.0f)
      loopinfo.PrintF(L"Off");
    else if (LoopEnd < 0.0f)
      loopinfo.PrintF(L"%6.2fb -> ?", BPM*LoopBegin/60.0f);
    else
      loopinfo.PrintF(L"%6.2fb -> %6.2fb", BPM*LoopBegin/60.0f, BPM*LoopEnd/60.0f);

    sString<128> text;  
    sF32 t = GetTime();
    text.PrintF(L"%5.2ffps %i:%02i:%02i %6.2fb LOOP %s",
      AvgFPS,((sInt)(t))/60, (sInt)t % 60, (sInt)(t*100.0f) % 100, BPM*t/60.0f,
      loopinfo);
    textarray.AddTail(text);
    
    text.PrintF(L"Cam P:%f,%f / A:%f\xb0 / Z: 10^%f", 
      Cam->Pos.x, Cam->Pos.y, 180.0f * Cam->Angle / sPIF, sLog(Cam->Zoom) / sLog(10.0f));
    textarray.AddTail(text);

    sGraphicsStats stats;
    sGetGraphicsStats(stats);

    sS64 mem = sGetUsedMainMemory();
    const sChar *mems = L"used";
    if(mem<0)
    {
      mem = -mem;
      mems = L"free";
    }

    text.PrintF(L"main %K %s / vidmem %K used | V:%i / P:%i / B:%i", (sInt)mem, mems, (sInt)stats.StaticTextureMem, stats.Vertices, stats.Primitives, stats.Batches);
    textarray.AddTail(text);

    sInt ei = -1;
    sInt active = 0;
    sF32 bbw = 0.0f;
    sF32 upw = 0.0f;
    sF32 tilt = 0.0f;
    sF32 mintime = 99999999999999.0f;
    sInt flags = 0;
    sMatrix44 mtx;
    mtx.Init();
    sEntity *entity;
    sFORALL(Entities, entity)
    {
      if (entity->IsAlive() && entity->Parent)
      {
        flags |= entity->Verb->Flags;
        mintime = sMin(entity->Delay, mintime);
        active++;
        ei = _i;
        mtx = entity->Matrix[MTX_MODEL];
        bbw += entity->GetBBWeight();
        upw += entity->GetUpWeight();
        tilt += entity->GetTiltWeight();
      }
    }

    text.PrintF(L"%i/%i Entities [%6.2fb oldest] / BB: %f / UP: %f / T:%f", 
      active, Entities.GetCount(), BPM*mintime/60.0f, bbw, upw, tilt);
    textarray.AddTail(text); 
    sString<128> flagtext;
    if (flags & VERB_FLAG_ZCLEAR0)
      flagtext.Add(L"|ZCLEAR0");
    if (flags & VERB_FLAG_ZCLEAR1)
      flagtext.Add(L"|ZCLEAR1");
    if (flags & VERB_FLAG_ZREAD)
      flagtext.Add(L"|ZREAD");
    if (flags & VERB_FLAG_ZWRITE)
      flagtext.Add(L"|ZWRITE");
    textarray.AddTail(flagtext);

    if (!TimeSource->IsPlaying())
    {
      for (sInt i = Entities.GetCount()-1; i >= 0; i--)
      {
        if (textarray.GetCount() == textarray.GetSize())
          break;
        sEntity *entity = Entities[i];
        //if (i < (Entities.GetCount()-20))
        //  break;
        if (entity->IsAlive() && entity->Parent)
        {
          textarray.AddTail(entity->Source);
        }
      }
    }

    Painter->Begin();
    sInt i = 0;
    sString<128> *s;
    sFORALL(textarray, s)
    {
      Painter->SetPrint(0,0xff000000,1);
      Painter->PrintF(10,i+11,L"%s",*s);
      Painter->PrintF(10,i+9,L"%s",*s);
      Painter->PrintF(11,i+10,L"%s",*s);
      Painter->PrintF(9,i+10,L"%s",*s);
      Painter->SetPrint(0,~0,1);
      Painter->PrintF(10,i+10,L"%s",*s);
      i += 10;
    }
    Painter->End();
  }
  CheckForNormalization();
  FramesRendered++;
}

void sMasagin::CheckForNormalization()
{
  // rescale cam and entity matrices so that they are back in the 2^-12..2^12 zoom magnitude
  // and move cam and entity matrices so the cam is in the coordinate origin. this way
  // we should be able to escape fraction truncation.

  if (sAbs(Cam->GetZoomMagnitude()) >= 6.0f)
  {
    NormalizeCamScale();
    NormalizeCamPosition();
  }

  //// IEEE fraction size is 23bit for 32bit floats.
  //// we need about 11 bits for addressing screen resolutions
  //// up to 2048 pixels, when camera scale and position 
  //// exceed a range of 12 bits, we need to normalize the camera position.
  //sF32 maxmag = 23.0f - 11.0f;
  //if (Cam->GetMagnitudeRange() >= maxmag)
  //{
  //  NormalizeCamPosition();
  //}
}

void sMasagin::NormalizeCamPosition()
{
  sDPrintF(L"Normalizing position...\n");
  // translation
  sVector2 pos = Cam->Pos * -1;
  sMatrix44 trans;
  trans.l.x += pos.x;
  trans.l.y += pos.y;
  // scale all matrices
  sEntity *entity;
  sFORALL(Entities, entity)
  {
    entity->Matrix[MTX_MODEL] *= trans;
    entity->Matrix[MTX_AUX0] *= trans;
    entity->Matrix[MTX_PARENT_MODEL] *= trans;
    entity->Matrix[MTX_PARENT_AUX0] *= trans;
  }
  // translate cam pos
  Cam->Pos += pos;
}

void sMasagin::NormalizeCamScale()
{
  sDPrintF(L"Normalizing scale...\n");
  sF32 mag = Cam->GetZoomMagnitude();
  // scaling
  sF32 magscale = sPow(2.0f, -mag);
  // scale cam pos
  Cam->Pos *= magscale;
  // scale cam zoom
  Cam->Zoom *= magscale;
  sMatrix44 scale;
  scale.Scale(magscale, magscale, 1.0f);
  // scale all matrices
  sEntity *entity;
  sFORALL(Entities, entity)
  {
    entity->Matrix[MTX_MODEL] *= scale;
    entity->Matrix[MTX_AUX0] *= scale;
    entity->Matrix[MTX_PARENT_MODEL] *= scale;
    entity->Matrix[MTX_PARENT_AUX0] *= scale;
  }
}

void sMasagin::UpdateCam()
{
  // get summed up angle
  sF32 weight = 0.0f;
  sF32 angle = 0;//sPIF * 0.5f;
  //sVector2 up;
  sEntity *entity;
  sFORALL(Entities, entity)
  {
    if (entity->IsAlive() && (entity->GetUpWeight() > 0.0f))
    {
      sVector2 v;
      entity->GetUpVector(v);
      sF32 a = sATan2(-v.x, v.y);
      if (!weight)
        angle = a;
      else
        angle += sAngleDiff(angle, a) * entity->GetUpWeight();
      weight += entity->GetUpWeight();
    }
  }

  // create rotated matrix
  sMatrix34 mtx;
  mtx.EulerXYZ(0.0f, 0.0f, -angle);

  sMatrix34 invmtx(mtx);
  invmtx.Invert34();

  weight = 0.0f;

  sVector2 pos;
  sF32 zoom = 0.0f;

  sF32 tilt = 0.0f;

  sInt counted = 0;

  // collect total weight and entities
  sFORALL(Entities, entity)
  {
    if (entity->IsAlive() && (entity->GetBBWeight() > 0.0f) && entity->UpdateBoundingBox(mtx))
    {
      weight += entity->GetBBWeight();
      zoom += sLog(sMax(entity->BoundingBox.SizeX() / Aspect, entity->BoundingBox.SizeY())) * entity->GetBBWeight();
      sVector31 center = sVector31(entity->BoundingBox.CenterX(), entity->BoundingBox.CenterY(), 0.0f) * invmtx;
      pos.x += center.x * entity->GetBBWeight();
      pos.y += center.y * entity->GetBBWeight();
      tilt += entity->GetTiltWeight();
      counted++;
    }
  }

  zoom = sExp(zoom / weight);
  pos.x = pos.x / weight;
  pos.y = pos.y / weight;
  if (counted)
    tilt = tilt / (sF32)counted;

  if (!zoom)
    zoom = 1.0f;

  Cam->Tilt = tilt;
  Cam->SetTarget(zoom, angle, pos);
  Cam->Step();
}

void sMasagin::RefreshEntities()
{
  sEntity *entity;
  sFORALL(Entities, entity)
  {
    entity->Done = sFALSE;
  }
}

void sMasagin::DrawEntity(sEntity *entity)
{
  entity->Done = sTRUE;
  entity->Draw();
}

void sMasagin::StepEntity(sEntity *entity)
{
  if (entity->Done)
    return;
  if (entity->Parent)
    StepEntity(entity->Parent);
  entity->Done = sTRUE;
  entity->Step();
}

void sMasagin::MoveEntityBack(sEntity *entity)
{
  sEntity *parent = entity->Parent;
  if (!parent)
    return;
  // move entity behind parent
  Entities.RemOrder(entity);
  Entities.AddBefore(entity, Entities.IndexOf(parent));
}

void sMasagin::ResetStep()
{
  Cam->ResetStep();
}

void sMasagin::StepRange(sF32 t1, sF32 t2)
{
  sF32 oldt = Time;
  sInt d = 0;
  sF32 t = t1;
  while (t < t2)
  {
    d++;
    Time = t;
    RefreshEntities();
    sEntity *entity;
    sFORALL(Entities, entity)
    {
      StepEntity(entity);
    }
    t += 1.0f/60.0f;
  }
  Time = oldt;
}

void sMasagin::Step()
{
  if (DebugView && (LoopBegin >= 0.0f) && ((LoopEnd > (LoopBegin+0.1f))&&(GetRealTime() >= LoopEnd)))
  {
    TimeSource->SetTime(LoopBegin);
  }
  sF32 time = GetRealTime();
  if (time < 0.0f)
  {
    Draw(-1);
    return;
  }
  sF32 oldtime = Time;
  sF32 delta = time - Time;
  if (delta >= (1.0f/60.0f))
  {
    StepRange(oldtime, time);
  }
  sF32 fps;
  if (Slow)
    fps = sMax(16.0f, AvgFPS/3.0f);
  else
    fps = sMax(24.0f, AvgFPS/2.0f);
  sF32 framerate = 1.0f/fps;
  sRandom rand;
  rand.Seed(FramesRendered);  
  sF32 timebase = time;
  sBool doublemode = (AvgFPS >= (fps*2)); // true when half images are possible
  if (MotionBlur)
  {
    sInt avg = (sInt)(AvgFPS+0.5f);
    if (Slow && (FramesRendered%2) && doublemode)
      timebase = oldtime;
    sInt framecount = MSG_MBSR;
    if ((MotionBlur == MSG_MB_8)||(MotionBlur == MSG_MB_8_I))
      framecount = 8;
    sF32 blurspf = framerate/(sF32)framecount;
    timebase += framecount*blurspf/2; 
    if (doublemode && ((MotionBlur == MSG_MB_8_I)||(MotionBlur == MSG_MB_16_I)))
    {
      switch (FramesRendered%2)
      {
        case 0: break;
        case 1: timebase -= blurspf*0.5f;
        default: break;
      }      
    }      
    for (sInt i = 0; i < framecount; ++i)
    {
      Time = timebase - ((sF32)i*blurspf);
      RefreshEntities();
      sEntity *entity;
      sFORALL(Entities, entity)
      {
        StepEntity(entity);
      }
      UpdateCam();
      Draw(i);
    }
  }
  else
  {
    Time = timebase;
    RefreshEntities();
    sEntity *entity;
    sFORALL(Entities, entity)
    {
      StepEntity(entity);
    }
    UpdateCam();
    Draw(0);
  }
  Time = time;
}

void sMasagin::SetTimeSource(sTimeSource *timesource)
{
  sDelete(TimeSource);
  TimeSource = timesource;
}

//void sMasagin::PingLoader()
//{
//  if (!RenderSerialize)
//    return;
//  if ((sGetTime() - PingTime) < 10)
//    return;
//  PingTime = sGetTime();
//  sRender3DBegin();
//  
//  if(sGetApp())
//    sGetApp()->OnPaint3D();
//  
//  sRender3DEnd();
//}

void sMasagin::Serialize(sReader &s)
{
  PingTime = sGetTime();
  TotalLoadingUnits = 4720716;

  sString<128> songpath;
  sReadMSGString(songpath, s);
  MusicPath = songpath;
  s | BPM;  
  if (!MusicPath.IsEmpty() && !sGetShellSwitch(L"nosound"))
  {
    SetTimeSource(new sTimeSourceVorbis(MusicPath));
    if (LoopEnd < 0)
    {
      LoopBegin = 0.0f;
      LoopEnd = TimeSource->GetLength() - 1.0f;
    }
  }
  else
  {
    SetTimeSource(new sTimeSourceSystem());
  }
  LoadStep();

  sVector31 bgcolor;
  s | bgcolor;
  sHslToRgb(bgcolor.x, bgcolor.y, bgcolor.z, bgcolor.x, bgcolor.y, bgcolor.z);
  BackgroundColor = bgcolor.GetColor();
  sU32 fadercount;
  s | fadercount;
  for (sU32 i = 0; i < fadercount; ++i)
  {
    sFaderFactory *fader = new sFaderFactory(this);
    fader->Serialize(s);
    Faders.AddTail(fader);
    LoadStep();
  }


  sU32 verbcount;
  s | verbcount;
  for (sU32 i = 0; i < verbcount; ++i)
  {
    sVerbFactory *verb = new sVerbFactory(this);
    verb->Serialize(s);
    Verbs.AddTail(verb);
    LoadStep();
  }

  sU32 modelcount;
  s | modelcount;
  for (sU32 i = 0; i < modelcount; ++i)
  {
    sString<128> name;
    sReadMSGString(name, s);
    name.Add(L".bsm");
    sModel2D *model = new sModel2D(this);
    //PingLoader();
    sLoadObject(name, model);
    Models.AddTail(model);
    LoadStep();
  }

  Root = NewEntity(sNULL);
  Root->Serialize(s);

  //PrerunScene();

  LoadStep();

  sDPrintF(L"Load steps counted: %i\n", CurrentLoadingUnit);
}

/****************************************************************************/
