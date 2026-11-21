/****************************************************************************/
/*  sIppToolMaterial host side (motion blur and glare). The VS block (mvp)   */
/*  and the PS block (4 colours) follow each other in push constants,        */
/*  64 + 64 = 128 bytes. The permutations msglib uses, Normal and Spread8,   */
/*  are implemented in shaders/ipp.slang.                                    */
/****************************************************************************/
#ifndef HEADER_MSGLIB_IPPMTRL
#define HEADER_MSGLIB_IPPMTRL

#include "base/types.hpp"
#include "base/math.hpp"
#include "base/graphics.hpp"
#include "util/shaders.hpp"

struct sIppToolMaterialVSPara
{
  static const sInt RegStart = 0;
  static const sInt RegCount = 4;
  static const sInt Slot = sCBUFFER_VS|0;
  sMatrix44 mvp;
};

struct sIppToolMaterialPSPara
{
  static const sInt RegStart = 12;
  static const sInt RegCount = 4;
  static const sInt Slot = sCBUFFER_VS|1;
  sVector4 ColorAdd;
  sVector4 Color0;
  sVector4 Color1;
  sVector4 ColorMul;
};

class sIppToolMaterial : public sMaterial
{
public:
  sInt Specials;
  sIppToolMaterial() { Specials = 0; NameId = sVKM_IPP; }
  virtual void SelectShaders(sVertexFormatHandle *format)
  {
    // permutation word: bits 0..3 = texture present, bits 8.. = special mode
    sU32 n = 0;
    if(Texture[0]) n |= 1;
    if(Texture[1]) n |= 2;
    if(Texture[2]) n |= 4;
    if(Texture[3]) n |= 8;
    n |= (sU32)(Specials & 15) << 8;
    sVkSetMaterialShaders(this,sVKM_IPP,n);
  }
  void Set(sCBufferBase *a,sCBufferBase *b) { sCBufferBase *A[2] = {a,b}; sMaterial::Set(A,2,0); }
};

#endif
