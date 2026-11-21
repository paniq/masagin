/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/

#ifndef FILE_CUBE_MAIN_HPP
#define FILE_CUBE_MAIN_HPP

#include "base/system.hpp"
#include "base/graphics.hpp"
#include "util/painter.hpp"
#include "util/shaders.hpp"

/****************************************************************************/

#include "vorbisplayer.hpp"

class MyApp : public sApp
{
  sPainter *Painter;
  sTexture2D *Tex;
  sMaterial *Mtrl;
  sGeometry *Geo;

  bVorbisPlayer Player;

  sViewport View;
  sMaterialEnv Env;

  sTiming Timer;
public:
  MyApp();
  ~MyApp();
  void OnPrepareFrame();
  void OnPaint3D();
  void OnInput(const sInputEvent &ie);
};

/****************************************************************************/

#endif // FILE_CUBE_MAIN_HPP
