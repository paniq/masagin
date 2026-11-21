/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/

#ifndef HEADER_ALTONA_UTIL_CAMERA
#define HEADER_ALTONA_UTIL_CAMERA

#ifndef __GNUC__
#pragma once
#endif

#include "base/types.hpp"
#include "base/math.hpp"
#include "base/graphics.hpp"
#include "base/system.hpp"

/****************************************************************************/

class sCamera
{
  sInt LastTime;
public:
  sCamera();
  virtual ~sCamera();
  virtual void OnFrame(sInt delta,sViewport &vp)=0;
  virtual void OnKey(sU32 key);
  virtual void OnDrag(const sMouseData &md);

  void Handle(sViewport &vp);
};

/****************************************************************************/

class sTumbleCamera : public sCamera
{
  sInt Time;
public:
  sTumbleCamera();

  sF32 Dist;
  sVector30 Speed;
  void OnFrame(sInt delta,sViewport &vp);
};

/****************************************************************************/

class sOrbitCamera : public sCamera
{
  sVector30 Angles;
  sF32 Dist;
public:
  sOrbitCamera();
  void OnFrame(sInt delta,sViewport &vp);
  void OnDrag(const sMouseData &md);
};


/****************************************************************************/

class sFreeCamera : public sCamera
{
  sInt KeyMask;
  sF32 Moving;

  void PrintPos();
public:
  sFreeCamera();
  void OnFrame(sInt delta,sViewport &vp);
  void OnKey(sU32 key);
  void OnDrag(const sMouseData &md);

  void Stop();

  sF32 Speed;
  sF32 LookSpeed;
  sF32 Damping;

  sVector31 Pos;
  sF32 DirLook,DirPitch,DirRoll;
  sVector30 SpeedV;
};

/****************************************************************************/

class sCameraXSI : public sCamera
{
  enum XSICamMode
  {
    XSI_DOLLY           = 0x0001,
    XSI_ORBIT           = 0x0002,
    XSI_TRANS_CAMPLANE  = 0x0004,
    XSI_SELECT          = 0x0008,
  };
  
  sInt KeyMaskCurrent;
  sInt KeyMaskSticky;
  

  sF32 DollyStep;
  sBool DollyActive;


  public:
    sCameraXSI();
    void OnFrame(sInt delta, sViewport &vp);
    void OnKey(sU32 key);
    void OnDrag(const sMouseData &md);
    void Init();

    
    sVector31 Target;
    sVector30 EulerRot;
    sVector30 Scale;
    sF32      Distance;
    sBool     StickyMode;
};

/****************************************************************************/

// HEADER_ALTONA_UTIL_CAMERA
#endif
