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
#include "msglib/msg.hpp"

/****************************************************************************/

enum {
  LL_LOADING,
  LL_LOAD_END,
  LL_PLAYING,
};

class MyApp : public sApp
{
public:
  sMasagin *Masagin;
  sMasagin *Loader;
  sInt LoadLevel;
  sThread *LoaderThread;
  sString<256> ScriptPath;

  MyApp();
  ~MyApp();
  sBool HasPackfile();
  sBool HasLogfile();
  void Init();
  void Reload();
  void OnPaint3D();
  void OnPrepareFrame();
  void OnInput(const sInputEvent &ie);
};

/****************************************************************************/

#endif // FILE_CUBE_MAIN_HPP
