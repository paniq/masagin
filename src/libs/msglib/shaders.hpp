/****************************************************************************/
/*  sMsgMaterial / sMsgMaterialGrowable host side. The constant block below  */
/*  is the push-constant layout of shaders/msg.slang: mvp, color and grow,   */
/*  96 bytes. libs/msglib/shaders.asc describes the same block in asc        */
/*  syntax; its unused matrix 'mv' is not part of that layout.               */
/****************************************************************************/
#ifndef HEADER_MSGLIB_SHADERS
#define HEADER_MSGLIB_SHADERS

#include "base/types.hpp"
#include "base/math.hpp"
#include "base/graphics.hpp"
#include "util/shaders.hpp"

struct sMsgMaterialEnvPara
{
  static const sInt RegStart = 0;
  static const sInt RegCount = 6;
  static const sInt Slot = sCBUFFER_VS|0;
  sMatrix44 mvp;                              // offset  0
  sVector4  color;                            // offset 64  material color
  sVector4  grow;                             // offset 80  grow position and scale
  void Set()
  {
    sGetMatrix(mvp,sGM_MODELSCREEN);
    color.Init(1,1,1,1);
    grow.Init(1.0,1.0,1.0,1.0);
  }
  void Set(sMaterialEnv &env)
  {
    sGetMatrix(mvp,sGM_MODELSCREEN);
  }
};

class sMsgMaterial : public sMaterial
{
public:
  sInt VerbFlags;
  sMsgMaterial() { VerbFlags = 0; NameId = sVKM_MSG; }
  virtual void SelectShaders(sVertexFormatHandle *format) { sVkSetMaterialShaders(this,sVKM_MSG,0); }
  void Set(sCBufferBase *cb,sInt variant=0) { sMaterial::Set(&cb,1,variant); }
};

class sMsgMaterialGrowable : public sMsgMaterial
{
public:
  sMsgMaterialGrowable() { NameId = sVKM_MSGGROW; }
  virtual void SelectShaders(sVertexFormatHandle *format) { sVkSetMaterialShaders(this,sVKM_MSGGROW,0); }
};

#endif
