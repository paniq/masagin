/****************************************************************************/
/*  sSimpleMaterial host side: debug lines and sPainter text. The push       */
/*  constants carry mvp only, 64 bytes; the shader is shaders/simple.slang.  */
/****************************************************************************/
#ifndef HEADER_ALTONA_UTIL_SHADERS
#define HEADER_ALTONA_UTIL_SHADERS

#include "base/types.hpp"
#include "base/math.hpp"
#include "base/graphics.hpp"

// backend material kinds, carried in sMaterial::NameId (see graphics_vk.cpp)
enum sVkMaterialKind
{
  sVKM_NONE   = 0,
  sVKM_SIMPLE = 0x56410001,
  sVKM_MSG    = 0x56410002,
  sVKM_MSGGROW= 0x56410003,
  sVKM_IPP    = 0x56410004,
};
// called from SelectShaders(): tells the backend which shader set and which
// permutation bits (see the .slang specialization constants) a material uses
void sVkSetMaterialShaders(sMaterial *mtrl, sInt kind, sU32 permutation);

struct sSimpleMaterialPara
{
  static const sInt RegStart = 0;
  static const sInt RegCount = 4;
  static const sInt Slot = sCBUFFER_VS|0;
  sMatrix44 mvp;                              // push constants, offset 0, 64 bytes
  void Set() { sGetMatrix(mvp,sGM_MODELSCREEN); }
};

struct sSimpleMaterialEnvPara                  // lighting is not used by the demo
{
  static const sInt RegStart = 0;
  static const sInt RegCount = 4;
  static const sInt Slot = sCBUFFER_VS|0;
  sMatrix44 mvp;
  void Set() { sGetMatrix(mvp,sGM_MODELSCREEN); }
  void Set(sMaterialEnv &env) { sGetMatrix(mvp,sGM_MODELSCREEN); }
};

enum sSimpleMaterialPerm                       // bits of the permutation word
{
  sSMP_COLOR = 1,
  sSMP_UV0   = 2,
  sSMP_T0MUL = 4,
};

class sSimpleMaterial : public sMaterial
{
public:
  sBool AddTex1;
  sSimpleMaterial() { AddTex1 = 0; NameId = sVKM_SIMPLE; }
  virtual void SelectShaders(sVertexFormatHandle *format)
  {
    sU32 n = 0;
    if(format->GetAvailMask() & (1<<sVF_COLOR0)) n |= sSMP_COLOR;
    if(format->GetAvailMask() & (1<<sVF_UV0))    n |= sSMP_UV0;
    if(Texture[0])                               n |= sSMP_T0MUL;
    sVkSetMaterialShaders(this,sVKM_SIMPLE,n);
  }
  void Set(sMaterialEnv *env,sInt variant=0)
  {
    sCBuffer<sSimpleMaterialEnvPara> cb;
    sCBufferBase *ptr = &cb;
    if(env) cb.Data->Set(*env); else cb.Data->Set();
    sMaterial::Set(&ptr,1,variant);
  }
  void Set(sCBufferBase *cb,sInt variant=0) { sMaterial::Set(&cb,1,variant); }
};

#endif
