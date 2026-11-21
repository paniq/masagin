/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/
/***                                                                      ***/
/***   This file contains classes for working with images                 ***/
/***                                                                      ***/
/***   The sImageData class is used to store images that are ready for    ***/
/***   usage as texture.                                                  ***/
/***   - all texture formats are supported                                ***/
/***   - no functions for manipulating are supported                      ***/
/***   - you can only load and save the altona file format                ***/
/***   - support for mipmaps and cubemaps                                 ***/
/***                                                                      ***/
/***   The sImage class provides functionality to work with images        ***/
/***   - only one pixel format is supported: ARGB888                      ***/
/***   - it's simple to work with the images                              ***/
/***   - you can convert to sImageData for a specific texture             ***/
/***   - you can load and save different file formats                     ***/
/***                                                                      ***/
/****************************************************************************/

#ifndef HEADER_ALTONA_UTIL_IMAGE
#define HEADER_ALTONA_UTIL_IMAGE

#ifndef __GNUC__
#pragma once
#endif


#include "base/types.hpp"
#include "base/graphics.hpp"
class sImage;
class sTextureBase;

extern sU64 sTotalImageDataMem;  // total memory in sImageData bitmaps

/****************************************************************************/

class sImageData
{
  sInt DataSize;                  // size for all data, including cubemaps
  sInt CubeFaceSize;
public:
  sInt Format;                    // sTEX_XXX format
  sInt SizeX;                     // Width in pixels
  sInt SizeY;                     // Height in pixels
  sInt SizeZ;                     // for volumemaps. 0 otherwise
  sInt Mipmaps;                   // number of mipmaps
  sInt BitsPerPixel;              // for size calculation
  sU8 *Data;                      // the Data itself
  sInt PaletteCount;              // in color entries (32 bit units)
  sU32 *Palette;                  // if a palette is required, here it is!
  sInt NameId;                    // Texture serialization nameid

  sImageData();
  sImageData(sImage *,sInt flagsandformat=sTEX_2D|sTEX_ARGB8888);
  ~sImageData();

//  void Init(sInt format,sInt xs,sInt ys,sInt mipmaps=1);    // obsolete
  void Init2(sInt format,sInt mipmaps,sInt xs,sInt ys,sInt zs); // new version, with volume maps
  void Copy(const sImageData *source);
  void Swap(sImageData *img);
  template <class streamer> void Serialize_(streamer &s);
  void Serialize(sWriter &s);
  void Serialize(sReader &s);

  class sTextureBase *CreateTexture() const;
  void UpdateTexture(sTextureBase *) const; // this will recreate the texture if extends don't fit.

  void ConvertFrom(const sImage *);
  void ConvertTo(sImage *) const;
  void ConvertFromCube(sImage **imgptr);

  sInt Size()const { return DataSize; }
};

void sGenerateMipmaps(sImageData *img);
sTextureBase *sStreamImageAsTexture(sReader &s);

/****************************************************************************/

struct sFontMapFontDesc
{
  sString<256> Name;      // name of font to search by the fontsystem of the OS
  sU32         Size;      // fontsize in pixel
  sS32         YOffset;   // offset in y-direction (positive values shift the font upwards)
  sBool        Italic;    // font in italic mode
  sBool        Bold;      // font in bold mode
};

class sFontMapInputParameter
{
  public:
    sStaticArray<sFontMapFontDesc> FontsToSearchIn;
    sU32         AntiAliasingFactor;
    sBool        Mipmaps;   // font has mitmaps
    sBool        Hinting;   // font has hinting enabled
    sU32         Outline;   // size of outline in pixels
    sU32         Blur;      // size of blur kernel in pixels
    const sChar *Letters;   // the letters to be contained by the fontmap
};


// there are two units for pixels
// bitmap-pixels: 
//   without multisampling factor
// multi-pixels: 
//   with multisampling multi-pixels = bitmap-pixels * Alias

struct sFontMap
{
  sU16 X,Y;                       ///< position in bitmap in bitmap-pixels
  sS16 Pre;                       ///< kerning before character in multi-pixels
  sS16 Cell;                      ///< size of bitmap-cell in bitmap-pixels
  sS16 Post;                      ///< kerning after character in multi-pixels
  sS16 Advance;                   ///< total advance in multi-pixels
  sU16 Letter;                    ///< 16-bit unicode of character
  sU16 Pad;
};


class sImage
{
public:
  sBool LoadBMP(const sU8 *data,sInt size);
  sBool LoadPIC(const sU8 *data,sInt size);
  sBool LoadTGA(const sChar *name);
  sBool LoadJPG(const sU8 *data,sInt size);
  sBool LoadPNG(const sU8 *data,sInt size);

  sBool SaveBMP(const sChar *name);
  sBool SavePIC(const sChar *name);
  sBool SaveTGA(const sChar *name);
public:
/*
  enum IChannel     // IChannel functions removed because rarely needed 
  {                 // and not really better than shifting & masking by hand.
    ChBLUE=0,       // delete this when nobody complains...
    ChGREEN,
    ChRED,
    ChALPHA,
  };
*/
  sInt SizeX;
  sInt SizeY;
  sU32 *Data;

  sImage();
  sImage(sInt xs,sInt ys);
  ~sImage();
  void Init(sInt xs,sInt ys);
  template <class streamer> void Serialize_(streamer &s);
  void Serialize(sWriter &);
  void Serialize(sReader &);

  sBool HasAlpha();                 // TRUE when at least one pixel is alpha!=255 -> use for DXT1/DXT5 check
  
  void Fill(sU32 color);
//  void Fill(sU32 srcBitmask,sU32 color);
  void SwapRedBlue();
  void SwapEndian();
#if sCONFIG_BE
  void SwapIfLE()       { }
  void SwapIfBE()       { SwapEndian(); }
#else
  void SwapIfLE()       { SwapEndian(); }
  void SwapIfBE()       { }
#endif

  void Checker(sU32 col0,sU32 col1,sInt maskx=8,sInt masky=8);
  void Glow(sF32 cx,sF32 cy,sF32 rx,sF32 ry,sU32 color,sF32 alpha=1.0f,sF32 power=0.5f);


  void MakeSigned();                    // convert from 0..255 unsigned to -128..127 signed
//  void ExchangeChannels(IChannel channelX, IChannel channelY);
//  void CopyChannelFrom(IChannel channelDst, IChannel channelSrc);
//  void SetChannel(IChannel channelX,sU8 val);
  void Copy(sImage *);
  void Add(sImage *);
  void Mul(sImage *);
  void Mul(sU32 color);
  void BlitFrom(sImage *src, sInt sx, sInt sy, sInt dx, sInt dy, sInt width, sInt height);
  void Outline(sInt pixelCount);
  void BlurX(sInt R);
  void BlurY(sInt R);
  void Blur(sInt R, sInt passes=3);
  void FlipXY(); // flip x/y

  sFontMap *CreateFontPage(sFontMapInputParameter &inParam, sInt &outLetterCount);
  

  sImage *Scale(sInt xs,sInt ys) const;
  void Scale(const sImage *src, sBool filter=sFALSE);
  sImage *Half() const;
  sImage *Copy() const;

  sU32 Filter(sInt x,sInt y) const;     // 24:8 pixel, requires power 2 texture!

//  sImageData *Convert(sInt tf,sInt mip=0) const;  // obsolete
//  void Convert(sImageData *);                     // obsolete

  sBool Load(const sChar *name);
  sBool Save(const sChar *name);

  void Diff(const sImage *img0, const sImage *img1);

  void CopyRenderTarget();
};

/****************************************************************************/
/*
class sImageGen
{
public:
  sInt SizeX;
  sInt SizeY;
  sU64 *Data;

  sImageGen();
  ~sImageGen();
  void Init(sInt xs,sInt ys);
  sImageGen *Copy();

  void Mul(sU32 color);
  void Add(sU32 color);
  void Sub(sU32 color);

  void Mul(sImageGen *);
  void Add(sImageGen *);
  void Sub(sImageGen *);

  void Fill(sU32 color);
  void GlowRect(sF32 centerx,sF32 centery,sF32 squarex,sF32 squarey,sF32 circlex,sF32 circley,sF32 gamma=0.5f,sU32 color=~0);
  void Text(sF32 x,sF32 y,sF32 xs,sF32 ys,sChar *text,sChar *font=L"arial",sInt flags=0,sU32 col=~0);
  void CB(sF32 contrast=1.0f,sF32 brightness=1.0f);
  void HSV(sF32 hue=0.0f,sF32 saturation=1.0f,sF32 value=1.0f);
  void Perlin(sInt startoctave=0,sInt octaves=1,sF32 falloff=1.0f,sInt mode=0,sInt seed=0);
  void Blur(sF32 x,sF32 y,sF32 amp=1.0f,sInt order=1);

  sImageGen *Scale(sInt xs,sInt ys);
};
*/
/****************************************************************************/
/***                                                                      ***/
/***   Some sImage helpers                                                ***/
/***                                                                      ***/
/****************************************************************************/

class sTexture2D *sLoadTexture2D(const sChar *name,sInt formatandflags);
class sTexture2D *sLoadTexture2D(const sImage *img,sInt formatandflags);
class sTexture2D *sLoadTexture2D(const sImageData *img);
class sTextureCube *sLoadTextureCube(const sChar *t0,const sChar *t1,const sChar *t2,const sChar *t3,const sChar *t4,const sChar *t5,sInt formatandflags);
//sImage *sReadTexImage(sU32 *&data, sU32 &texflags);
//void sReadTexImage(sU32 *&data, sU32 &texflags, sImage *img);
void sSaveRT(const sChar *filename,sTexture2D *rt=0);


sInt sDiffImage(sImage *dest, const sImage *img0, const sImage *img1, sU32 mask=0xffffffff);
void Heightmap2Normalmap(sImage *dest, const sImage *src, sF32 scale /*= 1.0f*/);

// HDR compression
inline sBool sCheckMRGB(sInt format) { format &= sTEX_FORMAT; return format==sTEX_MRGB8 || format==sTEX_MRGB16; }
void sCompressMRGB(sImageData *dst_img, sInt format, const sImageData *src_img);
void sDecompressMRGB(sImageData *dst_img, const sImageData *src_img, sF32 alpha=1.0f);
void sCompressMRGB(sImageData *img, sInt format);
void sDecompressMRGB(sImageData *img, sF32 alpha=1.0f);
void sClampToARGB8(sImageData *img);
sImageData *sConvertARGB8ToHDR(const sImageData *img, sInt format);

/****************************************************************************/

// HEADER_ALTONA_UTIL_IMAGE
#endif
