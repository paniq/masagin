/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#ifndef FILE_MSGLIB_MSG_HPP
#define FILE_MSGLIB_MSG_HPP

#include "base/types.hpp"
#include "base/math.hpp"
#include "base/graphics.hpp"
#include "base/system.hpp"

extern class sViewport *CurrentViewport;
extern class sMaterialEnv *CurrentEnv;

/****************************************************************************/

class sAutoCam
{
public:
  class sMasagin *Owner;

  sF32 Zoom;
  sF32 Angle;
  sVector2 Pos;
  sF32 Tilt;

  sAutoCam(sMasagin *owner);
  void Reset();
  void SetTarget(sF32 zoom, sF32 angle, sVector2 pos);
  void Get(sMatrix34 &mtx, sF32 zoom, sF32 angle, sVector2 pos, sF32 z);
  void GetCurrent(sMatrix34 &mtx, sF32 z);
  void Get(sViewport &view);
  void ResetStep();
  void Step();
  sF32 GetZoomMagnitude();
  sF32 GetPositionMagnitude();
  sF32 GetMagnitudeRange();

};

class sTimeSource
{
public:
  virtual ~sTimeSource() {}
  virtual void Play() {}
  virtual sBool IsPlaying() { return sFALSE; }
  virtual void Stop() {}
  virtual void SetTime(sF32 t) {}
  virtual sF32 GetTime() { return 0.0f; }
  virtual sF32 GetLength() { return 3600.0f; }
};

class sLoadingTimeSource : public sTimeSource
{
public:
  sMasagin *Progress;

  sLoadingTimeSource();
  virtual ~sLoadingTimeSource();
  virtual void Play();
  virtual sBool IsPlaying();
  virtual void Stop();
  virtual void SetTime(sF32 t);
  virtual sF32 GetTime();
  virtual sF32 GetLength();
};

enum {
  // frametype
  MSG_FT_CAMERA,
  MSG_FT_ENTITY,
  MSG_FT_BGCOLOR,
};

class sMsgFrame
{
  // The members of this class would not fit into one union under C++98, and the
  // player never instantiates it, so they are simply laid out next to each other.
  sF32 Timestamp;
  sF32 Zoom;
  sF32 Angle;
  sF32 Pos[2];
  sF32 Tilt;
  class sEntity *Entity;
  sF32 Matrix[16];
  sU32 Color;
};

class sMasagin
{
protected:
  void Init();
  void Cleanup();
public:
  sThreadLock Lock;
  sInt PingTime;
  sInt TotalLoadingUnits;
  sInt CurrentLoadingUnit;
  class sSimpleMaterial *LineMaterial;
  class sGeometry *LineGeometry;
  class sGeometry *QuadGeometry;
  sString<256> ScriptPath;
  sU64 LastWriteTime;
  sMatrix44 Identity;

  class sIppTool *Ipp;

  sViewport RTView;
  sViewport View;
  sMaterialEnv Env;
  class sPainter *Painter;
  sTiming Timer;
  sBool DebugView;

  // stats
  sInt ClipCount; // how many entities have been clipped

  class sMsgMaterial *ZClearMaterial;
  sStackArray<class sMsgMaterial *, 128> PathMaterials;

  class sTexture2D *PathTexture;
  static struct sVertexFormatHandle *PathVertexFormat;

  sInt MotionBlur;
  sBool FSAA;
  sBool Slow;
  sBool RenderSerialize;
  sBool Bloom;
  sInt FramesRendered;
  
  sStackArray<class sVerbFactory*, 8192> Verbs;
  sStackArray<class sModel2D*, 256> Models;
  sStackArray<class sEntity*, 8192> Entities;
  sStackArray<class sFaderFactory*, 256> Faders;

  class sEntity *Root;
  sAutoCam *Cam;
  sF32 CamPrecog;

  sF32 Time;
  sVector2 Up;
  sString<256> MusicPath;
  sF32 BPM;
  sTimeSource *TimeSource;

  sU32 BackgroundColor;

  sF32 LoopBegin;
  sF32 LoopEnd;

  sF32 Aspect;
  sF32 AvgFPS;

  sMasagin();
  ~sMasagin();
  sMsgMaterial *CreateMaterial(sInt verbmask);
  void Reset();
  sBool LoadScript(const sChar *path);
  sU64 GetLastWriteTime();
  sBool DocumentChanged();
  void Reload();
  void PrerunScene();
  sEntity *NewEntity(sEntity *after);
  void ResetStep();
  void Step();
  void StepRange(sF32 t1, sF32 t2);
  void RefreshEntities();
  void MoveEntityBack(sEntity *entity);
  void DrawEntity(sEntity *entity);
  void StepEntity(sEntity *entity);
  void UpdateCam();  

  void Serialize(class sReader &s);
  void SetTimeSource(sTimeSource *timesource);
  void TogglePause();
  sF32 BloomRadius;     // multiplier for the bloom blur radius (1.0 = as authored)
  sF32 BloomStrength;   // multiplier for the bloom intensity (1.0 = as authored)
  void ToggleLoop();
  void ToggleMotionBlur();
  void MoveTimeFwd();
  void MoveTimeBack();
  void Play();
  void Stop();
  sF32 GetTime();
  sF32 GetRealTime();
  void Draw(sInt frame);
  void MBGrab(sInt index);
  void MBBlit(sInt d, sInt s0, sInt s1, sInt s2=0, sInt s3=0);
  void MB16x2(sInt frame);
  void MB8x2(sInt frame);
  void MB16x4(sInt frame);
  void MB8x4(sInt frame);
  void GetViewplanePoint(sVector31 &v, sF32 x, sF32 y, sF32 z);
  void ZClear(sF32 z);
  void ZClear0();
  void ZClear1();
  void Paint(sViewport *view=sNULL);

  void LoadStep();

  void DrawRect(const sFRect &rc);
  void DrawScreenBorders();
  void Lines(struct sVertexBasic *src,sInt linecount);
  void Line(sVector31 v1, sVector31 v2, sU32 color, sU32 color2=0);
  void SolidQuad(sVector31 v1, sVector31 v2, sVector31 v3, sVector31 v4, sU32 color, sMaterial *mat=sNULL);
  void Quad(sVector31 v1, sVector31 v2, sVector31 v3, sVector31 v4, sU32 color1, sU32 color2, sU32 color3, sU32 color4);
  void PaintRect(const sFRect &rect, sMatrix34 &mtx, sU32 bottom, sU32 color);
  void PaintRect(const sFRect &rect, sU32 color, sMaterial *mat=sNULL);
  void PaintCam();
  void PaintBoundingBoxes();
  void PaintNormals();  
  void CheckForNormalization();
  void NormalizeCamPosition();
  void NormalizeCamScale(); 
};

struct sVertexMesh2D              // 36 bytes
{
  sF32 px,py,pz;                  // 3 pos
  //sF32 nx,ny,nz;                  // 3 normal
  sU32 c0;                        // 1 color
  sF32 u0,v0;                     // 2 uv's

  void Init(sF32 PX,sF32 PY,sF32 PZ,/*sF32 NX, sF32 NY, sF32 NZ,*/sU32 C0,sF32 U0,sF32 V0) volatile
  { px=PX; py=PY; pz=PZ; /*nx=NX; ny=NY; nz=NZ; */c0=sSWAPVC(C0); u0=U0; v0=V0; }
  void Init(const sVector31 &p,/*const sVector31 &n, */sU32 C0,sF32 U0,sF32 V0) volatile
  { px=p.x; py=p.y; pz=p.z; /*nx=n.x;ny=n.y;nz=n.z; */c0=sSWAPVC(C0); u0=U0; v0=V0; }

  static sVertexFormatHandle* VertexFormat() {return sMasagin::PathVertexFormat;}
};

/****************************************************************************/

#endif // FILE_MSGLIB_MSG_HPP

