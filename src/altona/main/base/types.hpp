/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/

#ifndef HEADER_ALTONA_BASE_TYPES
#define HEADER_ALTONA_BASE_TYPES

#ifndef __GNUC__
#pragma once
#endif

#include <math.h>

/****************************************************************************/
/***                                                                      ***/
/***   Configuration checking                                             ***/
/***                                                                      ***/
/****************************************************************************/

// configuration enumerations.

// sPLATFORM = one of...

#define sPLAT_WINDOWS   1
#define sPLAT_LINUX     2
#define sPLAT_PS3       3
#define sPLAT_XBOX360   4
#define sPLAT_PS2       5
#define sPLAT_WII       6
#define sPLAT_APPLE     7         // not implemented

// sRENDERER = one of

#define sRENDER_BLANK     0       // blank screen, no rendering
#define sRENDER_DX9       1       // dx9 rendering
#define sRENDER_OGL2      2       // >=opengl 2.0 rendering
#define sRENDER_DX10      3       // >=dx10 rendering (vista required)
#define sRENDER_PS3GL     4       // ps3 psgl 
#define sRENDER_PS2       5
#define sRENDER_WII       6       // wii GX
#define sRENDER_XBOX360   7
  
/****************************************************************************/

// include the installation dependent configuration file
// it lives in the engine tree at altona/altona_config.hpp.

#include "../altona_config.hpp"

#ifndef sCONFIG_CODEROOT_WINDOWS
#define sCONFIG_CODEROOT_WINDOWS sCONFIG_CODEROOT
#endif

#ifndef sCONFIG_CODEROOT_LINUX
#define sCONFIG_CODEROOT_LINUX sCONFIG_CODEROOT
#endif

#ifndef sCONFIG_CODEROOT
#if sCONFIG_SYSTEM_LINUX
#define sCONFIG_CODEROOT sCONFIG_CODEROOT_LINUX
#else
#define sCONFIG_CODEROOT sCONFIG_CODEROOT_WINDOWS
#endif
#endif

/****************************************************************************/

#ifdef sCONFIG_BUILD_DEBUG
#undef sCONFIG_BUILD_DEBUG
#define sCONFIG_BUILD_DEBUG 1
#else
#define sCONFIG_BUILD_DEBUG 0
#endif

#ifdef sCONFIG_BUILD_DEBUGFAST
#undef sCONFIG_BUILD_DEBUGFAST
#define sCONFIG_BUILD_DEBUGFAST 1
#else
#define sCONFIG_BUILD_DEBUGFAST 0
#endif

#ifdef sCONFIG_BUILD_RELEASE
#undef sCONFIG_BUILD_RELEASE
#define sCONFIG_BUILD_RELEASE 1
#else
#define sCONFIG_BUILD_RELEASE 0
#endif

#ifdef sCONFIG_BUILD_STRIPPED
#undef sCONFIG_BUILD_STRIPPED
#define sCONFIG_BUILD_STRIPPED 1
#else
#define sCONFIG_BUILD_STRIPPED 0
#endif


#ifdef sCONFIG_OPTION_XSI
#undef sCONFIG_OPTION_XSI
#define sCONFIG_OPTION_XSI 1
#else
#define sCONFIG_OPTION_XSI 0
#endif

#ifdef sCONFIG_OPTION_NOSYSTEM
#undef sCONFIG_OPTION_NOSYSTEM
#define sCONFIG_OPTION_NOSYSTEM 1
#else
#define sCONFIG_OPTION_NOSYSTEM 0
#endif

#ifdef sCONFIG_OPTION_SHELL
#undef sCONFIG_OPTION_SHELL
#define sCONFIG_OPTION_SHELL 1
#else
#define sCONFIG_OPTION_SHELL 0
#endif

#ifdef sCONFIG_OPTION_GAMEPAD
#undef sCONFIG_OPTION_GAMEPAD
#define sCONFIG_OPTION_GAMEPAD 1
#else
#define sCONFIG_OPTION_GAMEPAD 0
#endif


#ifdef sCONFIG_RENDER_BLANK
#undef sCONFIG_RENDER_BLANK
#define sCONFIG_RENDER_BLANK 1
#define sRENDERER sRENDER_BLANK
#define sCONFIG_QUADRICS 0
#else
#define sCONFIG_RENDER_BLANK 0
#endif

#ifdef sCONFIG_RENDER_DX9
#undef sCONFIG_RENDER_DX9
#define sCONFIG_RENDER_DX9 1
#define sRENDERER sRENDER_DX9
#define sCONFIG_QUADRICS 0
#else
#define sCONFIG_RENDER_DX9 0
#endif

#ifdef sCONFIG_RENDER_OGL2
#undef sCONFIG_RENDER_OGL2
#define sCONFIG_RENDER_OGL2 1
#define sRENDERER sRENDER_OGL2
#define sCONFIG_QUADRICS 0
#else
#define sCONFIG_RENDER_OGL2 0
#endif

#ifdef sCONFIG_RENDER_PS3GL
#undef sCONFIG_RENDER_PS3GL
#define sCONFIG_RENDER_PS3GL 1
#define sRENDERER sRENDER_PS3GL
#define sCONFIG_QUADRICS 0
#else
#define sCONFIG_RENDER_PS3GL 0
#endif

#ifdef sCONFIG_RENDER_PS2
#undef sCONFIG_RENDER_PS2
#define sCONFIG_RENDER_PS2 1
#define sRENDERER sRENDER_PS2
#define sCONFIG_QUADRICS 1
#else
#define sCONFIG_RENDER_PS2 0
#endif

#ifdef sCONFIG_RENDER_WII
#undef sCONFIG_RENDER_WII
#define sCONFIG_RENDER_WII 1
#define sRENDERER sRENDER_WII
#define sCONFIG_QUADRICS 0
#else
#define sCONFIG_RENDER_WII 0
#endif

#ifdef sCONFIG_RENDER_XBOX360
#undef sCONFIG_RENDER_XBOX360
#define sCONFIG_RENDER_XBOX360 1
#define sRENDERER sRENDER_XBOX360
#define sCONFIG_QUADRICS 0
#else
#define sCONFIG_RENDER_XBOX360 0
#endif

// find out used platform
#ifdef sCONFIG_SYSTEM_PS2
#undef sCONFIG_SYSTEM_PS2
#define sCONFIG_SYSTEM_PS2 1
#define sPLATFORM sPLAT_PS2
#else
#ifdef sCONFIG_SYSTEM_WII
#undef sCONFIG_SYSTEM_WII
#define sCONFIG_SYSTEM_WII 1
#define sPLATFORM sPLAT_WII
#else
#if _XBOX
#define sCONFIG_SYSTEM_XBOX360 1
#define sPLATFORM sPLAT_XBOX360
#else
#if defined(_WIN32) && !defined(SN_TARGET_PS3)
#define sCONFIG_SYSTEM_WINDOWS 1
#define sPLATFORM sPLAT_WINDOWS
#else
#if SN_TARGET_PS3
#define sCONFIG_SYSTEM_PS3 1
#define sPLATFORM sPLAT_PS3
#else
#define sCONFIG_SYSTEM_LINUX 1
#define sPLATFORM sPLAT_LINUX
#endif
#endif
#endif
#endif
#endif

// fill out rest of platforms
#ifndef sCONFIG_SYSTEM_PS2
#define sCONFIG_SYSTEM_PS2 0
#endif
#ifndef sCONFIG_SYSTEM_WII
#define sCONFIG_SYSTEM_WII 0
#endif
#ifndef sCONFIG_SYSTEM_XBOX360
#define sCONFIG_SYSTEM_XBOX360 0
#endif
#ifndef sCONFIG_SYSTEM_WINDOWS
#define sCONFIG_SYSTEM_WINDOWS 0
#endif
#ifndef sCONFIG_SYSTEM_PS3
#define sCONFIG_SYSTEM_PS3 0
#endif
#ifndef sCONFIG_SYSTEM_LINUX
#define sCONFIG_SYSTEM_LINUX 0
#endif

/****************************************************************************/
/****************************************************************************/

#ifdef _MSC_VER                              // microsoft C++

// configuration switching

#define sCONFIG_COMPILER_MSC      1         // compiled with Microsoft Visuacl C++
#define sCONFIG_COMPILER_GCC      0         // compiled with Gnu C++
#define sCONFIG_COMPILER_CWCC     0         // compiled with CodeWarrior C++
#define sCONFIG_32BIT             1         // Pointers are 32 bit
#define sCONFIG_64BIT             0         // Pointers are 64 bit
#define sCONFIG_ASM_MSC_IA32      1         // use 80386 instruction set with MSC syntax
#define sCONFIG_ASM_87            1         // when using IA32, use 8087 fpu
#define sCONFIG_ASM_SSE           0         // when using IA32, use SSE fpu

// type switching

#define sCONFIG_NATIVEINT         int _w64                // sDInt: an int of the same size as a pointer
#define sCONFIG_INT64             __int64                 // sS64, sU64: a 64 bit int
#define sCONFIG_UNICODETYPE       wchar_t                 // sChar: an unicode character
#define sCONFIG_SIZET             unsigned int
#define sINLINE                   __forceinline           // use this to inline
#define sNORETURN                 __declspec(noreturn)    // use this for dead end funtions
#define sDEBUGBREAK               __debugbreak();
#define sCOMPILEERROR(x)          y y y y x 
#define sCDECL                    __cdecl
#define sOBSOLETE                 __declspec(deprecated)
#define sALIGNED(type, name, al)  __declspec( align (al) ) type name
#define sUNUSED                                           // possibly unused variable

// compiler dependent settings

#pragma warning (disable : 4244) 
void __debugbreak();
#pragma intrinsic (__debugbreak)
// variations

#if sCONFIG_OPTION_XSI
#undef sCONFIG_UNICODETYPE
#define sCONFIG_UNICODETYPE       unsigned short 
#endif

#ifdef _WIN64
#undef sCONFIG_NATIVEINT
#undef sCONFIG_INT64
#undef sCONFIG_SIZET
#undef sCONFIG_32BIT
#undef sCONFIG_64BIT
#undef sCONFIG_ASM_MSC_IA32
#undef sCONFIG_ASM_87
#undef sCONFIG_ASM_SSE
#define sCONFIG_NATIVEINT         __int64 _w64
#define sCONFIG_INT64             __int64 _w64
#define sCONFIG_SIZET             unsigned __int64
#define sCONFIG_32BIT             0
#define sCONFIG_64BIT             1
#define sCONFIG_ASM_MSC_IA32      0
#define sCONFIG_ASM_87            0
#define sCONFIG_ASM_SSE           0
#endif

#define sCONFIG_ALLOCA(a)         _alloca(a)
extern "C" void *_alloca(sCONFIG_SIZET size);
#pragma intrinsic (_alloca)

// done

#endif

/****************************************************************************/

// don't comment options here, that would duplicate.

#if defined(__GNUC__) || defined(__GCC__)                           // GNU C++

// configuration switching

#define sCONFIG_COMPILER_MSC      0
#define sCONFIG_COMPILER_GCC      1
#define sCONFIG_COMPILER_CWCC     0
#define sCONFIG_32BIT             1
#define sCONFIG_64BIT             0

// type switching

#define sCONFIG_NATIVEINT         int
#define sCONFIG_INT64             long long
#define sCONFIG_UNICODETYPE       wchar_t
#define sCONFIG_SIZET             unsigned int
#if defined(__x86_64__) || defined(__aarch64__)
#undef sCONFIG_NATIVEINT
#undef sCONFIG_INT64
#undef sCONFIG_SIZET
#undef sCONFIG_32BIT
#undef sCONFIG_64BIT
#define sCONFIG_NATIVEINT         long
#define sCONFIG_INT64             long
#define sCONFIG_SIZET             unsigned long
#define sCONFIG_32BIT             0
#define sCONFIG_64BIT             1
#endif
#define sINLINE                   inline
#define sNORETURN
#define sDEBUGBREAK               {};
#define sCOMPILEERROR(x)          y y y y x 
#define sCDECL
//#define sOBSOLETE                  __attribute__ ((deprecated))
#define sOBSOLETE                 
#define sALIGNED(type, name, al)  type name __attribute__ ((aligned (al)))
#define sCONFIG_ALLOCA(a)         __builtin_alloca(a)
#define sUNUSED                   __attribute__((unused))    // possibly unused variable

// compiler dependent settings

void __debugbreak();

// done

#endif

#ifdef __CWCC__                             // CodeWarrior C++

// configuration switching

#define sCONFIG_COMPILER_MSC      0
#define sCONFIG_COMPILER_GCC      0
#define sCONFIG_COMPILER_CWCC     1
#define sCONFIG_32BIT             1
#define sCONFIG_64BIT             0

// type switching

#define sCONFIG_NATIVEINT         int
#define sCONFIG_INT64             long long
#define sCONFIG_UNICODETYPE       wchar_t
#define sCONFIG_SIZET             unsigned long
#define sINLINE                   __inline__
#define sNORETURN
#define sDEBUGBREAK               {};
#define sCOMPILEERROR(x)          y y y y x 
#define sCDECL
#define sOBSOLETE                 
#define sALIGNED(type, name, al)  type name __attribute__ ((aligned (al)))
#define sCONFIG_ALLOCA(a)         __alloca(a)
#define sUNUSED                                           // possibly unused variable

// compiler dependent settings

void __debugbreak();

// done

#endif

// platform dependent settings

#if sCONFIG_SYSTEM_WINDOWS
#define D3D_DEBUG_INFO 1
#define WINVER 0x0501           // require windows xp!
#define _WIN32_WINNT 0x0501
#define DIRECTINPUT_VERSION 0x0800
#undef UNICODE
#undef _UNICODE
#define UNICODE
#define _UNICODE

#define _INC_MALLOC
#include "xmmintrin.h"
#undef _INC_MALLOC
#define sCONFIG_SSE               __m128
#define sSTDCALL                  __stdcall
#endif

#if sCONFIG_SYSTEM_XBOX360
#include "vectorintrinsics.h"
#define sSTDCALL
#define sCONFIG_SSE               __vector4
#undef sCONFIG_ASM_MSC_IA32
#define sCONFIG_ASM_MSC_IA32      0
#endif

#if sCONFIG_SYSTEM_LINUX
#define sSTDCALL
#define sCONFIG_SSE               void
#endif

#if sCONFIG_SYSTEM_PS2
#define sSTDCALL
typedef unsigned int sCONFIG_SSE __attribute__ ((mode (TI))) __attribute__ ((aligned (16)));
#endif

#if sCONFIG_SYSTEM_PS3
#define sSTDCALL
#define sCONFIG_SSE               float __attribute__ ((vector_size(16)))
#endif

#if sCONFIG_SYSTEM_WII
#define sSTDCALL
#define sCONFIG_SSE               void
#endif

#if sCONFIG_SYSTEM_PS3 || sCONFIG_SYSTEM_XBOX360 || sCONFIG_SYSTEM_WII
#define sCONFIG_LE 0
#define sCONFIG_BE 1
#else
#define sCONFIG_LE 1
#define sCONFIG_BE 0
#endif

#if sCONFIG_SYSTEM_PS3 || sCONFIG_SYSTEM_XBOX360 || sCONFIG_SYSTEM_WII || sCONFIG_SYSTEM_PS2
#define sCONFIG_UNALIGNEDCRASH 1
#else
#define sCONFIG_UNALIGNEDCRASH 0
#endif


/****************************************************************************/

// use these macros instead of the sCONFIG_xxx_xxx ones.

#define sDEBUG        1                     // include sVERIFY
#define sINTRO        0                     // code size optimisations 
#define sRELEASE      (sCONFIG_BUILD_RELEASE||sCONFIG_BUILD_STRIPPED)      // ommit time consuming checks
#define sSTRIPPED     sCONFIG_BUILD_STRIPPED // no cheats, no debugs.
#define sCOMMANDLINE  sCONFIG_OPTION_SHELL  // commandline build - don't open window

/****************************************************************************/
/***                                                                      ***/
/***   Basic Types and Functions                                          ***/
/***                                                                      ***/
/****************************************************************************/

typedef unsigned char             sU8;      // for packed arrays
typedef unsigned short            sU16;     // for packed arrays
typedef unsigned int              sU32;     // for packed arrays and bitfields
typedef unsigned sCONFIG_INT64    sU64;     // use as needed
typedef signed char               sS8;      // for packed arrays
typedef short                     sS16;     // for packed arrays
typedef int                       sS32;     // for packed arrays
typedef signed sCONFIG_INT64      sS64;     // use as needed
typedef float                     sF32;     // basic floatingpoint
typedef double                    sF64;     // use as needed
typedef int                       sInt;     // use this most!
typedef unsigned sCONFIG_NATIVEINT sPtr;    // type for pointer
typedef signed sCONFIG_NATIVEINT  sDInt;    // type for pointer diff
typedef sCONFIG_UNICODETYPE       sChar;    // type for strings (UNICODE!)
typedef sS64                      sSize;    // size of a file = something possibly larger than memory

typedef char                      sChar8;   // only use where the API requires ascii
typedef int                       sBool;    // use for boolean function results

typedef sCONFIG_SSE               sSSE ;   // 4 x sF32

/****************************************************************************/

#define sVARARG(p,i) (((sInt *)(p))[(i)+1])               // don't use varargs at all if possible
#define sVARARGF(p,i) (*(sF64*)&(((sInt *)(p))[(i)+1]))
#define sALLOCSTACK(type,count)  ((type *) sCONFIG_ALLOCA(count*sizeof(type)))   // this has to be a macro because is MUST be inlined!
#define sALLOCALIGNEDSTACK(type,count,ba)  ((type *) (sDInt(sCONFIG_ALLOCA(count*sizeof(type)+ba-1))&~(ba-1)))   // this has to be a macro because is MUST be inlined!
#define sMAKE2(a,b) (((b)<<16)|(a))
#define sMAKE4(a,b,c,d) (((d)<<24)|((c)<<16)|((b)<<8)|(a))
#define sCOUNTOF(x) sDInt(sizeof(x)/sizeof(*(x)))    // #elements of array, usefull for unicode strings

#if sCONFIG_COMPILER_CWCC
#define sALIGNOF(T) ((sizeof(T)<4)?4:sizeof(T))
#else
#define sALIGNOF __alignof
#endif

#define sTXT2(y) L ## y
#define sTXT(x) sTXT2(x)              // make long string from short string, like sTXT(__FILE__)

#if sPLATFORM==sPLAT_WINDOWS || sPLATFORM==sPLAT_LINUX || sPLATFORM==sPLAT_APPLE
#define sMAXPATH    4096              // use this for path-strings. 
#else
#define sMAXPATH  512
#endif

/****************************************************************************/

#if sCONFIG_COMPILER_GCC
  #if sPLATFORM==sPLAT_PS3 || sPLATFORM==sPLAT_PS2
    #define sOFFSET(t,m) (((sDInt)(&((t*)64)->m))-64)
  #else
    #define sOFFSET(TYPE, MEMBER)					\
    (__offsetof__ (reinterpret_cast <sDInt>			\
                 (&reinterpret_cast <const volatile char &>	\
                  (static_cast<TYPE *> (0)->MEMBER))))
  #endif
#else
  #define sOFFSET(t,m) (((sDInt)(&((t*)64)->m))-64)
#endif

#define sOFFSETO(o,m) ((sDInt)(((sU8 *)(&(o)->m))-((sU8 *)(o))))    // offset from object
#define sOFFSETOMP(o,m) ((sDInt)(((sU8 *)(&((o)->*(m))))-((sU8 *)(o))))  // offset from object, member pointer

// experimental!
// in the presence of virtual functions, i can't cast pointer to member
// variables in a relyable way. unless multiple inheritence is used, this 
// hack should work on all current platforms
//
// mp = sMemberPtr<membertype>(sOFFSET(classtype,member));
template<typename ElementType> struct sMemberPtr
{
  sDInt Offset;
  sMemberPtr() { Offset=0; }
  sMemberPtr(sDInt o) { Offset=o; }
  typedef ElementType Type;
  ElementType &Ref(void *obj) { return *(ElementType *)(((sU8 *)obj)+Offset); }
};
// abstract version: cast as you like
struct sVoidMemberPtr
{
  sDInt Offset;
  sVoidMemberPtr() { Offset=0; }
  sVoidMemberPtr(sDInt o) { Offset=o; }
  template<typename ElementType> ElementType &Ref(void *obj) { return *(ElementType *)(((sU8 *)obj)+Offset); }
};


template<typename MemberType,typename ClassType> 
sMemberPtr<MemberType> sMakeMemberPtr(MemberType ClassType::*,sDInt o) 
{ return sMemberPtr<MemberType>(o); }

#define sMEMBERPTR(classtype,membername) sMakeMemberPtr(&classtype::membername,sOFFSET(classtype,membername))

/****************************************************************************/

#define sTRUE   (!0)
#define sFALSE  0
#define sNULL   0
#define sPI     3.1415926535897932384626433832795
#define sPI2    6.28318530717958647692528676655901
#define sPIF    3.1415926535897932384626433832795f
#define sPI2F   6.28318530717958647692528676655901f
#define sSQRT2  1.4142135623730950488016887242097
#define sSQRT2F 1.4142135623730950488016887242097f

/****************************************************************************/

template <class Type> sINLINE Type sMin(Type a,Type b)              {return (a<b) ? a : b;}
template <class Type> sINLINE Type sMax(Type a,Type b)              {return (a>b) ? a : b;}
template <class Type> sINLINE Type sSign(Type a)                    {return (a==0) ? Type(0) : (a>0) ? Type(1) : Type(-1);}
template <class Type> sINLINE Type sClamp(Type a,Type min,Type max) {return (a>=max) ? max : (a<=min) ? min : a;}
template <class Type> sINLINE void sSwap(Type &a,Type &b)           {Type s; s=a; a=b; b=s;}
template <class Type> sINLINE Type sAlign(Type a,sInt b)            {return (Type)((((sDInt)a)+b-1)&(~(b-1)));}

template <class Type> sINLINE Type sSquare(Type a)                  {return a*a;}
sInt sFindLowerPower(sInt x);
sInt sFindHigherPower(sInt x);
inline sBool sIsPower2(sInt x)                                      {return (x&(x-1)) == 0;}
inline sInt sDivDown(sInt a,sInt b)                                 {return a>0?a/b:-(-a/b);}

template <class Type> sINLINE void sDelete(Type &a)                 {if(a) delete a; a=0;}
template <class Type> sINLINE void sDeleteArray(Type &a)            {if(a) delete[] a; a=0;}
template <class Type> sINLINE void sRelease(Type &a)                {if(a) a->Release(); a=0;}
template <class Type> sINLINE void sAddRef(Type &a)                 {if(a) a->AddRef();}

template <class Type> sINLINE Type sFade(sF32 f,Type a,Type b)      {return a+(b-a)*f;}


// the endian API will change - do not use yet!
// it also needs inline assembly
// LE assumes IA32 processor, where unaligned loads/stores do not crash

inline sU32 sSwapEndian(sU32 a)                                     {return ((a&0xff000000)>>24)|((a&0x00ff0000)>>8)|((a&0x0000ff00)<<8)|((a&0x000000ff)<<24); }
inline sU16 sSwapEndian(sU16 a)                                     {return ((a&0xff00)>>8)|((a&0x00ff)<<8); }
inline sU64 sSwapEndian(sU64 a)                                     {return sU64(sSwapEndian(sU32(a)))<<32 | sU64(sSwapEndian(sU32(a>>32))); }
inline void sSwapEndianI(sU32 &a)                                   {a= ((a&0xff000000)>>24)|((a&0x00ff0000)>>8)|((a&0x0000ff00)<<8)|((a&0x000000ff)<<24); }
inline void sSwapEndianI(sU16 &a)                                   {a= ((a&0xff00)>>8)|((a&0x00ff)<<8); }
inline void sSwapEndianI(sU64 &a)                                   {a= sU64(sSwapEndian(sU32(a)))<<32 | sU64(sSwapEndian(sU32(a>>32))); }
inline sInt sSwapEndian(sInt a)                                     {sU32 tmp = *(sU32*)&a; sSwapEndianI(tmp); a = *(sInt*)&tmp; return a;}
inline sF32 sSwapEndian(sF32 a)                                     {sU32 tmp = *(sU32*)&a; sSwapEndianI(tmp); a = *(sF32*)&tmp; return a;}
#if sCONFIG_BE
inline sU32 sSwapIfBE(sU32 a)                                       {return ((a&0xff000000)>>24)|((a&0x00ff0000)>>8)|((a&0x0000ff00)<<8)|((a&0x000000ff)<<24); }
inline sU32 sSwapIfLE(sU32 a)                                       {return a;}
inline sU16 sSwapIfBE(sU16 a)                                       {return ((a&0xff00)>>8)|((a&0x00ff)<<8); }
inline sU16 sSwapIfLE(sU16 a)                                       {return a;}
inline sU64 sSwapIfBE(sU64 a)                                       {return sU64(sSwapEndian(sU32(a)))<<32 | sU64(sSwapEndian(sU32(a>>32))); }
inline sU64 sSwapIfLE(sU64 a)                                       {return a;}
inline sInt sSwapIfBE(sInt a)                                       {return sSwapEndian(a);}
inline sInt sSwapIfLE(sInt a)                                       {return a;}
inline sF32 sSwapIfBE(sF32 a)                                       {return sSwapEndian(a);}
inline sF32 sSwapIfLE(sF32 a)                                       {return a;}
inline void sUnalignedLittleEndianStore16(sU8 *p,sU16 v)            {p[0]=v; p[1]=v>>8;}
inline void sUnalignedLittleEndianStore32(sU8 *p,sU32 v)            {p[0]=v; p[1]=v>>8; p[2]=v>>16; p[3]=v>>24;}
inline void sUnalignedLittleEndianStore64(sU8 *p,sU64 v)            {p[0]=v; p[1]=v>>8; p[2]=v>>16; p[3]=v>>24; p[0]=v>>32; p[1]=v>>40; p[2]=v>>48; p[3]=v>>56;}
inline void sUnalignedLittleEndianLoad16(const sU8 *p,sU16 &v)      {v = p[0]|(p[1]<<8);}
inline void sUnalignedLittleEndianLoad32(const sU8 *p,sU32 &v)      {v = p[0]|(p[1]<<8)|(p[2]<<16)|(p[3]<<24);}
inline void sUnalignedLittleEndianLoad64(const sU8 *p,sU64 &v)      {v = p[0]|(p[1]<<8)|(p[2]<<16)|(p[3]<<24)|(sU64(p[3])<<32)|(sU64(p[3])<<40)|(sU64(p[3])<<48)|(sU64(p[3])<<56);}
#else
inline sU32 sSwapIfBE(sU32 a)                                       {return a; }
inline sU32 sSwapIfLE(sU32 a)                                       {return ((a&0xff000000)>>24)|((a&0x00ff0000)>>8)|((a&0x0000ff00)<<8)|((a&0x000000ff)<<24); }
inline sU16 sSwapIfBE(sU16 a)                                       {return a;}
inline sU16 sSwapIfLE(sU16 a)                                       {return ((a&0xff00)>>8)|((a&0x00ff)<<8); }
inline sU64 sSwapIfBE(sU64 a)                                       {return a;}
inline sU64 sSwapIfLE(sU64 a)                                       {return sU64(sSwapEndian(sU32(a)))<<32 | sU64(sSwapEndian(sU32(a>>32))); }
inline sInt sSwapIfBE(sInt a)                                       {return a;}
inline sInt sSwapIfLE(sInt a)                                       {return sSwapEndian(a);}
inline sF32 sSwapIfBE(sF32 a)                                       {return a;}
inline sF32 sSwapIfLE(sF32 a)                                       {return sSwapEndian(a);}
inline void sUnalignedLittleEndianStore16(sU8 *p,sU16 v)            {*(sU16 *)p = v;}
inline void sUnalignedLittleEndianStore32(sU8 *p,sU32 v)            {*(sU32 *)p = v;}
inline void sUnalignedLittleEndianLoad16(const sU8 *p,sU16 &v)      {v = *(sU16 *)p;}
inline void sUnalignedLittleEndianLoad32(const sU8 *p,sU32 &v)      {v = *(sU32 *)p;}
#if sCONFIG_UNALIGNEDCRASH
inline void sUnalignedLittleEndianStore64(sU8 *p,sU64 v)            {((sU32 *)p)[0]=v; ((sU32 *)p)[1]=v>>32;}
inline void sUnalignedLittleEndianLoad64(const sU8 *p,sU64 &v)      { sU32 a=((sU32*)p)[0],b=((sU32*)p)[1]; v = a+(sU64(b)<<32); }
#else
inline void sUnalignedLittleEndianStore64(sU8 *p,sU64 v)            {*(sU64 *)p = v;}
inline void sUnalignedLittleEndianLoad64(const sU8 *p,sU64 &v)      {v = *(sU64 *)p;}
#endif
#endif


// raw cast
template <typename T, sInt Bytes> struct sSwapHelper;
template <typename T0, typename T1> struct sRawCastHelper
{
  sRawCastHelper(T1 v1):V1(v1) { }
  union
  {
    T0 V0;
    T1 V1;
  };
};
template <typename T0, typename T1> T0 raw_cast(T1 v1) { sRawCastHelper<T0,T1> tmp(v1); return tmp.V0; }

// template endian swapper
// if this work on all compilers explicit swap permutations can be removed
template <typename T, sInt S> struct sSwapEndianHelper;
template <typename T> struct sSwapEndianHelper<T,1> { static T Swap(T a) { return a; } };
template <typename T> struct sSwapEndianHelper<T,2> { static T Swap(T a) { return raw_cast<T>(sSwapEndian(raw_cast<sU16>(a))); } };
template <typename T> struct sSwapEndianHelper<T,4> { static T Swap(T a) { return raw_cast<T>(sSwapEndian(raw_cast<sU32>(a))); } };
template <typename T> struct sSwapEndianHelper<T,8> { static T Swap(T a) { return raw_cast<T>(sSwapEndian(raw_cast<sU64>(a))); } };

#if sCONFIG_BE
template <typename T> T sSwapIfLE(T a) { return a; }
template <typename T> T sSwapIfBE(T a) { return sSwapEndianHelper<T,sizeof(T)>::Swap(a); }
#else
template <typename T> T sSwapIfLE(T a) { return sSwapEndianHelper<T,sizeof(T)>::Swap(a); }
template <typename T> T sSwapIfBE(T a) { return a; }
#endif

/****************************************************************************/
/***                                                                      ***/
/***   Arithmetic Functions                                               ***/
/***                                                                      ***/
/****************************************************************************/

// int

sInt sMulDiv(sInt a,sInt b,sInt c);
sInt sMulShift(sInt var_a,sInt var_b);
sInt sDivShift(sInt var_a,sInt var_b);

// float fiddling

sF32 sAbs(sF32);                            // specialization for float
sF32 sMinF(sF32,sF32);                      // specialization for float
sF32 sMaxF(sF32,sF32);                      // specialization for float
void sDivMod(sF32 over,sF32 under,sF32 &div,sF32 &mod); // |over| = div*under+mod
sF32 sMod(sF32 over,sF32 under);
void sSetFloat();                           // set floating point unit into default precision/rounding
void sFrac(sF32 val,sF32 &frac,sF32 &full); // val = frac+full; 0>=frac>1; full is int
void sFrac(sF32 val,sF32 &frac,sInt &full);
sF32 sFrac(sF32);
sF32 sRoundDown(sF32);                      // round towards minus infinity
sF32 sRoundUp(sF32);                        // round towards plus infinity
sF32 sRoundZero(sF32);                      // round towards zero
sF32 sRoundNear(sF32);                      // round towards nearest integer, in case of tie round to nearest even integer
sInt sRoundDownInt(sF32);
sInt sRoundUpInt(sF32);
sInt sRoundZeroInt(sF32);
sInt sRoundNearInt(sF32);
sF32 sSelectEQ(sF32 a,sF32 b,sF32 t,sF32 f);// if (a==b) then x=t else x=f;
sF32 sSelectNE(sF32 a,sF32 b,sF32 t,sF32 f);// if (a!=b) then x=t else x=f;
sF32 sSelectGT(sF32 a,sF32 b,sF32 t,sF32 f);// if (a>=b) then x=t else x=f;
sF32 sSelectGE(sF32 a,sF32 b,sF32 t,sF32 f);// if (a> b) then x=t else x=f;
sF32 sSelectLT(sF32 a,sF32 b,sF32 t,sF32 f);// if (a<=b) then x=t else x=f;
sF32 sSelectLE(sF32 a,sF32 b,sF32 t,sF32 f);// if (a< b) then x=t else x=f;
sF32 sSelectEQ(sF32 a,sF32 b,sF32 t);       // if (a==b) then x=t else x=0;
sF32 sSelectNE(sF32 a,sF32 b,sF32 t);       // if (a!=b) then x=t else x=0;
sF32 sSelectGT(sF32 a,sF32 b,sF32 t);       // if (a>=b) then x=t else x=0;
sF32 sSelectGE(sF32 a,sF32 b,sF32 t);       // if (a> b) then x=t else x=0;
sF32 sSelectLT(sF32 a,sF32 b,sF32 t);       // if (a<=b) then x=t else x=0;
sF32 sSelectLE(sF32 a,sF32 b,sF32 t);       // if (a< b) then x=t else x=0;

// non-trivial float

sF32 sSqrt(sF32);
sF32 sRSqrt(sF32);
sF32 sLog(sF32);
sF32 sLog2(sF32);
sF32 sLog10(sF32);
sF32 sExp(sF32);
sF32 sPow(sF32,sF32);

sF32 sSin(sF32);
sF32 sCos(sF32);
sF32 sTan(sF32);
void sSinCos(sF32,sF32 &s,sF32 &c);
sF32 sASin(sF32);
sF32 sACos(sF32);
sF32 sATan(sF32);
sF32 sATan2(sF32 over,sF32 under);

// non-trivial float, fast

sF32 sFSqrt(sF32);
sF32 sFRSqrt(sF32);
sF32 sFLog(sF32);
sF32 sFLog2(sF32);
sF32 sFLog10(sF32);
sF32 sFExp(sF32);
sF32 sFPow(sF32,sF32);

sF32 sFSin(sF32);
sF32 sFCos(sF32);
sF32 sFTan(sF32);
void sFSinCos(sF32,sF32 &s,sF32 &c);
sF32 sFASin(sF32);
sF32 sFACos(sF32);
sF32 sFATan(sF32);
sF32 sFATan2(sF32 over,sF32 under);

// some obsolete variants. obsolence not yet confirmed...

sINLINE sF32 /*sOBSOLETE*/ sFMod(sF32 a,sF32 b) { return sMod(a,b); }
sINLINE sF32 /*sOBSOLETE*/ sFFloor(sF32 a) { return sRoundDown(a); }
sINLINE sF32 /*sOBSOLETE*/ sFCeil(sF32 a) { return sRoundUp(a); }
sINLINE sF32 /*sOBSOLETE*/ sFAbs(sF32 a) { return sAbs(a); }
sINLINE sF32 /*sOBSOLETE*/ sFInvSqrt(sF32 a) { return sRSqrt(a); }

/****************************************************************************/
/***                                                                      ***/
/***   Strings and Memory                                                 ***/
/***                                                                      ***/
/****************************************************************************/

// memory

void sSetMem(void *dd,sInt s,sInt c);
void sCopyMem(void *dd,const void *ss,sInt c);
sInt sCmpMem(const void *dd,const void *ss,sInt c);
template <class Type> void sClear(Type &p) { sSetMem(&p,0,sizeof(p)); }

// strings

struct sStringDesc                // this is always used as CONST, otherwise it won't work correctly.
{
  sChar *Buffer;
  sInt Size;
  sStringDesc(sChar *b,sInt s) : Buffer(b),Size(s) {}
  sStringDesc(const sStringDesc &s) : Buffer(s.Buffer),Size(s.Size) {}
  sStringDesc& operator=(const sStringDesc &s) { Buffer=s.Buffer; Size=s.Size; return*this;}
};

void sCopyString(sChar *d,const sChar *s,sInt size);
inline void sCopyString(const sStringDesc &d, const sChar *s) { sCopyString(d.Buffer,s,d.Size); }

// use only if API functions don't support unicode
void sCopyString(sChar8* d, const sChar *s,sInt c);
void sCopyString(sChar* d, const sChar8* s,sInt c);
inline void sCopyString(const sStringDesc &d, const sChar8 *s) { sCopyString(d.Buffer,s,d.Size); }
void sCopyString(sChar8* d, const sChar8* s,sInt c);

sChar sUpperChar(sChar c); // returns the uppercase character for c
sChar sLowerChar(sChar c); // returns the lowercase character for c
sChar *sMakeUpper(sChar *s); // converts s in place to uppercase and returns s
sChar *sMakeLower(sChar *s); // converts s in place to lowercase and returns s

sINLINE sInt sGetStringLen(const sChar *s)    { sInt i; for(i=0;s[i];i++); return i; }
void sAppendString(sChar *d,const sChar *s,sInt size);
inline void sAppendString(const sStringDesc &d,const sChar *s) { sAppendString(d.Buffer,s,d.Size); }
void sAppendString2(sChar *d,const sChar *s,sInt size,sInt len);
void sAppendPath(sChar *d,const sChar *s,sInt size);          // append strings, have exactly one '/' between them, handle absolute paths
inline void sAppendPath(const sStringDesc &d,const sChar *s) { sAppendPath(d.Buffer,s,d.Size); }
sInt sCmpString(const sChar *a,const sChar *b);               // normal string compare
sInt sCmpStringI(const sChar *a,const sChar *b);              // case insensitive 
sInt sCmpStringP(const sChar *a,const sChar *b);              // filename paths ('/'=='\', case insensitive)
sInt sCmpStringLen(const sChar *a,const sChar *b,sInt len);   // first characters match
sInt sCmpStringILen(const sChar *a,const sChar *b,sInt len);
sInt sCmpStringPLen(const sChar *a,const sChar *b,sInt len);
const sChar *sFindFileWithoutPath(const sChar *a);
sBool sIsAbsolutePath(const sChar *a);
void sExtractPath(const sChar *a, const sStringDesc &path);
void sExtractPath(const sChar *a, const sStringDesc &path, const sStringDesc &filename);
void sMakeRelativePath(const sStringDesc &path, const sChar *relTo); // make first path relative to second one if possible

sInt sFindString(const sChar *f, const sChar *s); // returns index of first occurrence of s in f or -1
sInt sFindLastChar(const sChar *f,sInt c);
sInt sFindFirstChar(const sChar *f,sInt c);
sInt sFindNthChar(const sChar *f,sInt c, sInt n);
const sChar *sFindFileExtension(const sChar *a);              // "ab.cd.ef" -> "ef"
sBool sExtractFileExtension(const sStringDesc &d, const sChar *a, sInt nr=0);  // "ab.cd.ef" -nr==0-> "ef", "ab.cd.ef" -nr==1-> "cd"
sChar *sFindFileExtension(sChar *a);
sBool sCheckFileExtension(const sChar *name,const sChar *ext); // example: ext = "mp.txt"
sBool sMatchWildcard(const sChar *wild,const sChar *text,sBool casesensitive=sFALSE);    // one recursion per '*'
sInt sCountChar(const sChar *str,sChar c);                    // useful as sCountChar(text,'\n');
sInt sFindChoice(const sChar *str,const sChar *choices);
sBool sFindFlag(const sChar *str,const sChar *choices,sInt &mask_,sInt &value_);
//sChar *sMakeChoice(sChar * choice, const sChar * choices, sInt index);
sInt sCountChoice(const sChar *choices);
sBool sCheckSuffix(const sChar *string,const sChar *suffix);  // ("1234","34") -> true

void sReadString(sU32 *&data,sChar *buffer,sInt size);
inline void sReadString(sU32 *&data,const sStringDesc &desc) { sReadString(data,desc.Buffer,desc.Size); }
void sWriteString(sU32 *&data,const sChar *buffer);

inline sBool sIsDigit(sInt i) { return (i>='0' && i<='9'); }
inline sBool sIsLetter(sInt i) { return (i>='a' && i<='z') || (i>='A' && i<='Z') || i=='_'; }
inline sBool sIsSpace(sInt i) { return i==' ' || i=='\t' || i=='\r' || i=='\n'; }
inline sBool sIsHex(sInt i) { return (i>='a' && i<='f') || (i>='A' && i<='F') || (i>='0' && i<='9'); }
sBool sIsName(const sChar *);   // c conventions: letter followed by letter or digit
sU32 sHashString(const sChar *string);
sU32 sHashString(const sChar *string,sInt len);

sBool sScanInt(const sChar *&scan,sInt &val);
sBool sScanFloat(const sChar *&scan,sF32 &val);
sBool sScanHex(const sChar *&scan,sInt &val);

void sMakeChoice(const sStringDesc &,const sChar *choices,sInt index);


// get a stringdesc which will append to given string
sStringDesc sGetAppendDesc(const sStringDesc &sd);

/****************************************************************************/

sBool sFormatString(const sStringDesc &,const sChar *format,const sChar **fp); // return sFALSE if text is truncated!

/****************************************************************************/

struct sFormatStringInfo      // parses %n.mf, like %12.6f 
{
  sInt Format;                // f, f
  sInt Field;                 // n, 12
  sInt Fraction;              // m, 6 (-1 if not specified)
  sInt Minus;                 // starts with negative sign -> left aligned
  sInt Null;                  // starts with null character -> pad with zero
};

struct sFormatStringBuffer
{
  sChar *Dest;
  sChar *End;
  const sChar *Format;

  void Fill();
  void GetInfo(sFormatStringInfo &);
  void Add(const sFormatStringInfo &info,const sChar *buffer,sBool sign);
  void Print(const sChar *str);
  void PrintInt(const sFormatStringInfo &info,sInt v);
  void PrintInt(const sFormatStringInfo &info,sS64 v);
  void PrintFloat(const sFormatStringInfo &info,sF64 v);
};

sFormatStringBuffer sFormatStringBase(const sStringDesc &buffer,const sChar *format);
sFormatStringBuffer& operator% (sFormatStringBuffer &,sInt);
sFormatStringBuffer& operator% (sFormatStringBuffer &,sU32);
sFormatStringBuffer& operator% (sFormatStringBuffer &,sU64);
sFormatStringBuffer& operator% (sFormatStringBuffer &,sS64);
//inline sFormatStringBuffer& operator% (sFormatStringBuffer & buf,sU32 v)     { return buf%(sInt)v; }
sFormatStringBuffer& operator% (sFormatStringBuffer &,sF32);
sFormatStringBuffer& operator% (sFormatStringBuffer &,sF64);
sFormatStringBuffer& operator% (sFormatStringBuffer &,const sChar *);

// we want to get rid of the variable arg thing and have a typesafe solution
// three examples:
//   sDPrintF(format,...)
//   sSPrintF(buffer,format,...)
//   sTextBuffer::PrintF(format,...)
// this has to be extended to templates (up to ten variable args):
//   template<typename A> sDPrintF(format,A a);
//   template<typename A,typename B> sDPrintF(format,A a,B b);
//   template<typename A,typename B,typename C> sDPrintF(format,A a,B b,C c);
// we don't want to type this everytime, especially since the template syntax is so lengthy
// that's what the sPRINTING macros are for:
//   sDPrintF(format,...) -> sPRINTING0
//   sSPrintF(buffer,format,...) -> sPRINTING1
//   sTextBuffer::PrintF(format,...) -> sPRINTING0
// how do we implement that?
//   #define sPRINTING0(Symbol,PreImpl,PostImpl)
//   #define sPRINTING1(Symbol,PreImpl,PostImpl,ArgType1)
//   #define sPRINTING2(Symbol,PreImpl,PostImpl,ArgType1,ArgType2)

#define sPRINTING0(Symbol,PreImpl,PostImpl) \
template<typename A,typename B,typename C,typename D,typename E,typename F,typename G,typename H,typename I,typename J> inline void Symbol(const sChar *format,A a,B b,C c,D d,E e,F f,G g,H h,I i,J j) { PreImpl%a%b%c%d%e%f%g%h%i%j; PostImpl } \
template<typename A,typename B,typename C,typename D,typename E,typename F,typename G,typename H,typename I           > inline void Symbol(const sChar *format,A a,B b,C c,D d,E e,F f,G g,H h,I i    ) { PreImpl%a%b%c%d%e%f%g%h%i  ; PostImpl } \
template<typename A,typename B,typename C,typename D,typename E,typename F,typename G,typename H                      > inline void Symbol(const sChar *format,A a,B b,C c,D d,E e,F f,G g,H h        ) { PreImpl%a%b%c%d%e%f%g%h    ; PostImpl } \
template<typename A,typename B,typename C,typename D,typename E,typename F,typename G                                 > inline void Symbol(const sChar *format,A a,B b,C c,D d,E e,F f,G g            ) { PreImpl%a%b%c%d%e%f%g      ; PostImpl } \
template<typename A,typename B,typename C,typename D,typename E,typename F                                            > inline void Symbol(const sChar *format,A a,B b,C c,D d,E e,F f                ) { PreImpl%a%b%c%d%e%f        ; PostImpl } \
template<typename A,typename B,typename C,typename D,typename E                                                       > inline void Symbol(const sChar *format,A a,B b,C c,D d,E e                    ) { PreImpl%a%b%c%d%e          ; PostImpl } \
template<typename A,typename B,typename C,typename D                                                                  > inline void Symbol(const sChar *format,A a,B b,C c,D d                        ) { PreImpl%a%b%c%d            ; PostImpl } \
template<typename A,typename B,typename C                                                                             > inline void Symbol(const sChar *format,A a,B b,C c                            ) { PreImpl%a%b%c              ; PostImpl } \
template<typename A,typename B                                                                                        > inline void Symbol(const sChar *format,A a,B b                                ) { PreImpl%a%b                ; PostImpl } \
template<typename A                                                                                                   > inline void Symbol(const sChar *format,A a                                    ) { PreImpl%a                  ; PostImpl } \
/*                                                                                                                   */ inline void Symbol(const sChar *format                                        ) { PreImpl%0                  ; PostImpl }

#define sPRINTING1(Symbol,PreImpl,PostImpl,ArgType1) \
template<typename A,typename B,typename C,typename D,typename E,typename F,typename G,typename H,typename I,typename J> inline void Symbol(ArgType1 arg1,const sChar *format,A a,B b,C c,D d,E e,F f,G g,H h,I i,J j) { PreImpl%a%b%c%d%e%f%g%h%i%j; PostImpl } \
template<typename A,typename B,typename C,typename D,typename E,typename F,typename G,typename H,typename I           > inline void Symbol(ArgType1 arg1,const sChar *format,A a,B b,C c,D d,E e,F f,G g,H h,I i    ) { PreImpl%a%b%c%d%e%f%g%h%i  ; PostImpl } \
template<typename A,typename B,typename C,typename D,typename E,typename F,typename G,typename H                      > inline void Symbol(ArgType1 arg1,const sChar *format,A a,B b,C c,D d,E e,F f,G g,H h        ) { PreImpl%a%b%c%d%e%f%g%h    ; PostImpl } \
template<typename A,typename B,typename C,typename D,typename E,typename F,typename G                                 > inline void Symbol(ArgType1 arg1,const sChar *format,A a,B b,C c,D d,E e,F f,G g            ) { PreImpl%a%b%c%d%e%f%g      ; PostImpl } \
template<typename A,typename B,typename C,typename D,typename E,typename F                                            > inline void Symbol(ArgType1 arg1,const sChar *format,A a,B b,C c,D d,E e,F f                ) { PreImpl%a%b%c%d%e%f        ; PostImpl } \
template<typename A,typename B,typename C,typename D,typename E                                                       > inline void Symbol(ArgType1 arg1,const sChar *format,A a,B b,C c,D d,E e                    ) { PreImpl%a%b%c%d%e          ; PostImpl } \
template<typename A,typename B,typename C,typename D                                                                  > inline void Symbol(ArgType1 arg1,const sChar *format,A a,B b,C c,D d                        ) { PreImpl%a%b%c%d            ; PostImpl } \
template<typename A,typename B,typename C                                                                             > inline void Symbol(ArgType1 arg1,const sChar *format,A a,B b,C c                            ) { PreImpl%a%b%c              ; PostImpl } \
template<typename A,typename B                                                                                        > inline void Symbol(ArgType1 arg1,const sChar *format,A a,B b                                ) { PreImpl%a%b                ; PostImpl } \
template<typename A                                                                                                   > inline void Symbol(ArgType1 arg1,const sChar *format,A a                                    ) { PreImpl%a                  ; PostImpl } \
/*                                                                                                                   */ inline void Symbol(ArgType1 arg1,const sChar *format                                        ) { PreImpl%0                 ; PostImpl }

#define sPRINTING2(Symbol,PreImpl,PostImpl,ArgType1,ArgType2) \
template<typename A,typename B,typename C,typename D,typename E,typename F,typename G,typename H,typename I,typename J> inline void Symbol(ArgType1 arg1,ArgType2 arg2,const sChar *format,A a,B b,C c,D d,E e,F f,G g,H h,I i,J j) { PreImpl%a%b%c%d%e%f%g%h%i%j; PostImpl } \
template<typename A,typename B,typename C,typename D,typename E,typename F,typename G,typename H,typename I           > inline void Symbol(ArgType1 arg1,ArgType2 arg2,const sChar *format,A a,B b,C c,D d,E e,F f,G g,H h,I i    ) { PreImpl%a%b%c%d%e%f%g%h%i  ; PostImpl } \
template<typename A,typename B,typename C,typename D,typename E,typename F,typename G,typename H                      > inline void Symbol(ArgType1 arg1,ArgType2 arg2,const sChar *format,A a,B b,C c,D d,E e,F f,G g,H h        ) { PreImpl%a%b%c%d%e%f%g%h    ; PostImpl } \
template<typename A,typename B,typename C,typename D,typename E,typename F,typename G                                 > inline void Symbol(ArgType1 arg1,ArgType2 arg2,const sChar *format,A a,B b,C c,D d,E e,F f,G g            ) { PreImpl%a%b%c%d%e%f%g      ; PostImpl } \
template<typename A,typename B,typename C,typename D,typename E,typename F                                            > inline void Symbol(ArgType1 arg1,ArgType2 arg2,const sChar *format,A a,B b,C c,D d,E e,F f                ) { PreImpl%a%b%c%d%e%f        ; PostImpl } \
template<typename A,typename B,typename C,typename D,typename E                                                       > inline void Symbol(ArgType1 arg1,ArgType2 arg2,const sChar *format,A a,B b,C c,D d,E e                    ) { PreImpl%a%b%c%d%e          ; PostImpl } \
template<typename A,typename B,typename C,typename D                                                                  > inline void Symbol(ArgType1 arg1,ArgType2 arg2,const sChar *format,A a,B b,C c,D d                        ) { PreImpl%a%b%c%d            ; PostImpl } \
template<typename A,typename B,typename C                                                                             > inline void Symbol(ArgType1 arg1,ArgType2 arg2,const sChar *format,A a,B b,C c                            ) { PreImpl%a%b%c              ; PostImpl } \
template<typename A,typename B                                                                                        > inline void Symbol(ArgType1 arg1,ArgType2 arg2,const sChar *format,A a,B b                                ) { PreImpl%a%b                ; PostImpl } \
template<typename A                                                                                                   > inline void Symbol(ArgType1 arg1,ArgType2 arg2,const sChar *format,A a                                    ) { PreImpl%a                  ; PostImpl } \
/*                                                                                                                   */ inline void Symbol(ArgType1 arg1,ArgType2 arg2,const sChar *format                                        ) { PreImpl%0                  ; PostImpl }

/****************************************************************************/

template<int size> struct sString
{
protected:
  sChar Buffer[size];
public:
  sString()                    { Buffer[0] = 0; }
  ~sString()                    { Buffer[0] = 1; }
  sString(const sChar *s)      { if (s) sCopyString(Buffer,s,size); else Buffer[0]=0; }
  sString(const sString &s)    { sCopyString(Buffer,s.Buffer,size); }
  sString &operator=(const sChar *s) { if (s) sCopyString(Buffer,s,size); else Buffer[0]=0; return *this; }
  operator sChar*()            { return Buffer; }
  operator const sChar*() const { return Buffer; }
  operator sStringDesc()       { return sStringDesc(Buffer,size); }
  sBool operator==(const sChar *s) const { return sCmpString(Buffer,s)==0; }
  sBool operator==(const sString &s) const { return sCmpString(Buffer,s.Buffer)==0; }
  sBool operator!=(const sChar *s) const { return sCmpString(Buffer,s)!=0; }
  sBool operator!=(const sString &s) const { return sCmpString(Buffer,s.Buffer)!=0; }
  sBool operator>(const sChar *s) const { return sCmpString(Buffer,s)>0; }
  sBool operator>(const sString &s) const { return sCmpString(Buffer,s.Buffer)>0; }
  sBool operator<(const sChar *s) const { return sCmpString(Buffer,s)<0; }
  sBool operator<(const sString &s) const { return sCmpString(Buffer,s.Buffer)<0; }
  const sChar &operator[](sInt i) const { return Buffer[i]; }
  sChar &operator[](sInt i)    { return Buffer[i]; }
  sInt Count() const           { return sGetStringLen(Buffer); }
  sBool IsEmpty() const        { return Buffer[0]==0; }
  sInt Size() const            { return size; }

  void Add(const sChar *a)     { sAppendString(Buffer,a,size); }
  void Add(const sChar *a,sInt len) { sAppendString2(Buffer,a,size,len); }
  void Add(const sChar *a,const sChar *b) { sAppendString(Buffer,a,b,size); }
  void AddPath(const sChar *a) { sAppendPath(Buffer,a,size); }

  sPRINTING0(PrintF,sFormatStringBuffer buf=sFormatStringBase(sStringDesc(Buffer,size),format);buf,;);
};


/****************************************************************************/
/***                                                                      ***/
/***   Debugging                                                          ***/
/***                                                                      ***/
/****************************************************************************/

sNORETURN void sVerifyFalse(const sChar *file,sInt line);

void sDPrint(const sChar *text);
void sPrint(const sChar *text);
extern void (*sNetPrint)(const sChar *text);        // sDPrintF will be mirrored here
extern void (*sRedirectStdOut)(const sChar *text);  // sPrintF will be send here instead
void sDPrintMark(); // will insert a line if things have been sDPrinted beforehand

sPRINTING1(sSPrintF,sFormatStringBuffer buf=sFormatStringBase(arg1,format);buf,;,const sStringDesc &)


// singlethreaded implementation
extern sString<1024> sXDPrintFBuffer;   // buffer for singlethreaded applications
sPRINTING0(sDPrintF_ST,sFormatStringBuffer buf=sFormatStringBase(sXDPrintFBuffer,format);buf,sDPrint(sXDPrintFBuffer);)
sPRINTING0(sPrintF_ST,sFormatStringBuffer buf=sFormatStringBase(sXDPrintFBuffer,format);buf,sPrint(sXDPrintFBuffer);)


// multithreaded implementation

// this sThread-stuff should be located in system.hpp but it can't because of the templates
struct sThreadContext
{
  sU32 Flags;                       // some flags for the thread

  sString<1024> PrintBuffer;        // the buffer for the threadsave print-functions

  sString<64>   ThreadName;         // a name for the thread

  void *UserData;                   // free for your own business

  sInt MemSegId;                    // currently active memory segment id

  void Init();
};
class sThread        *sGetCurrentThread();
struct sThreadContext &sGetCurrentThreadContext();
sPRINTING0(sDPrintF_MT,sString<1024> &sXDPrintFBufferT = sGetCurrentThreadContext().PrintBuffer; sFormatStringBuffer buf=sFormatStringBase(sXDPrintFBufferT,format);buf,sDPrint(sXDPrintFBufferT);)
sPRINTING0(sPrintF_MT,sString<1024> &sXDPrintFBufferT = sGetCurrentThreadContext().PrintBuffer; sFormatStringBuffer buf=sFormatStringBase(sXDPrintFBufferT,format);buf,sPrint(sXDPrintFBufferT);)


// choose the sDPrintF - method
#if sCONFIG_OPTION_XSI || sCONFIG_SYSTEM_PS3 || sCONFIG_SYSTEM_PS2 || sCONFIG_SYSTEM_XBOX360     // XSI plugins are singlesthreaded!
#define sDPrintF sDPrintF_ST
#define sPrintF  sPrintF_ST
#else
#define sDPrintF sDPrintF_MT
#define sPrintF  sPrintF_MT
#endif

sNORETURN void sCDECL sFatalImpl(const sChar *str);
sPRINTING0(sFatal,sFormatStringBuffer buf=sFormatStringBase(sXDPrintFBuffer,format);buf,sFatalImpl(sXDPrintFBuffer););



#if sDEBUG

#if sRELEASE
#define sVERIFY(x) {if(!(x))sVerifyFalse(sTXT(__FILE__),__LINE__);}
#define sVERIFYFALSE {sVerifyFalse(sTXT(__FILE__),__LINE__);}
#else
#define sVERIFY(x) {if(!(x)){ sDEBUGBREAK ;sVerifyFalse(sTXT(__FILE__),__LINE__);}}
#define sVERIFYFALSE { sDEBUGBREAK ;sVerifyFalse(sTXT(__FILE__),__LINE__);}
#endif

#else
#define sVERIFY(x) {;}
#define sVERIFYFALSE {;}
#endif

// placement new 
inline void *operator new(sCONFIG_SIZET, void *ptr) { return ptr; }
inline void *operator new[](sCONFIG_SIZET, void *ptr) { return ptr; }

// own new/delete using altona memory manager
void *sCDECL operator new(sCONFIG_SIZET);
void *sCDECL operator new[](sCONFIG_SIZET);
void sCDECL operator delete(void* p);
void sCDECL operator delete[](void* p);

#if !sRELEASE && !sCONFIG_SYSTEM_PS3
void * sCDECL operator new(sCONFIG_SIZET,const char*,int);
void * sCDECL operator new[](sCONFIG_SIZET,const char*,int);
#define sDEFINE_NEW new(__FILE__,__LINE__)
#else
#define sDEFINE_NEW new
#endif
#define new sDEFINE_NEW

#if sCONFIG_COMPILER_MSC && !sCONFIG_OPTION_NOSYSTEM && sPLATFORM!=sPLAT_XBOX360
#define _MFC_OVERRIDES_NEW
inline void sCDECL operator delete(void *p, const char *, int s) { ::operator delete(p); }
inline void sCDECL operator delete[](void *p, const char *, int s) { ::operator delete[](p); }
void sCheckMemory(const char* file, int line);
void sBreakOnAllocation(sInt n);
#define sCHECKMEMORY() sCheckMemory(__FILE__,__LINE__)
#else
#define sCHECKMEMORY()
#endif

#if defined(sTRACEON)
#define sTRACE() sDPrintF(L"%s(%d):Trace\n",sTXT(__FILE__),__LINE__);
#else
#define sTRACE() ;
#endif


#if sSTRIPPED
extern sInt sOBSOLETE There_Are_Still_Open_Todo_Items_In_This_Build;
#define sTODO_L(s) There_Are_Still_Open_Todo_Items_In_This_Build=__LINE__;
#define sTODO_G3(line) static void sTODO_HACK_FUNCTION ## line () { There_Are_Still_Open_Todo_Items_In_This_Build=0; }
#define sTODO_G2(line) sTODO_G3(line)
#define sTODO_G(s) sTODO_G2(__LINE__)
#else
#define sTODO_L(s) 
#define sTODO_G(s) 
#endif

// helpers for managing not implemented stuff, useful for porting
#if sCONFIG_COMPILER_MSC
#define sTOSTRING8_I(x) #x
#define sTOSTRING8(x) sTOSTRING8_I(x)

#define sNOT_IMPLEMENTED()    \
  __pragma (message(__FILE__"("sTOSTRING8(__LINE__)"): TODO implement "__FUNCTION__))    \
  sDPrintF(sTXT(__FILE__)L"(%d): TODO implement "sTXT(__FUNCTION__)L"\n",__LINE__);

#define sNOT_IMPLEMENTED_FAIL()    \
  __pragma (message(__FILE__"("sTOSTRING8(__LINE__)"): TODO implement "__FUNCTION__))    \
  sVERIFY(0);
#define sTODO(msg)  \
  __pragma (message(__FILE__"("sTOSTRING8(__LINE__)"): TODO "msg))

#else
#define sNOT_IMPLEMENTED()  \
  sDPrintF(sTXT(__FILE__)L"(%d): TODO implement "sTXT(__FUNCTION__)L"\n",__LINE__);
#define sNOT_IMPLEMENTED_FAIL()    \
  sDPrintF(sTXT(__FILE__)L"(%d): TODO implement "sTXT(__FUNCTION__)L"\n",__LINE__);   \
  sVERIFY(0);
#define sTODO(msg)
#endif


/****************************************************************************/
/***                                                                      ***/
/***   Small Structures                                                   ***/
/***                                                                      ***/
/****************************************************************************/

template <class Type> struct sBaseRect
{
  Type x0,y0,x1,y1;
  sBaseRect() { x0=y0=x1=y1=0; }
  sBaseRect(Type XS,Type YS) { x0=0;y0=0;x1=XS;y1=YS; }
  sBaseRect(Type X0,Type Y0,Type X1,Type Y1) { x0=X0;y0=Y0;x1=X1;y1=Y1; }
  sBaseRect(const sBaseRect &r) { *this=r; }
  template <typename T2> explicit sBaseRect(const sBaseRect<T2> &r) { x0 = Type(r.x0); y0 = Type(r.y0); x1 = Type(r.x1); y1 = Type(r.y1); }

  void Init() { x0=y0=x1=y1=0; }
  void Init(Type XS,Type YS) { x0=0;y0=0;x1=XS;y1=YS; }
  void Init(Type X0,Type Y0,Type X1,Type Y1) { x0=X0;y0=Y0;x1=X1;y1=Y1; }

  Type SizeX() const { return x1-x0; }
  Type SizeY() const { return y1-y0; }
  Type CenterX() const { return (x0+x1)/2; }
  Type CenterY() const { return (y0+y1)/2; }
  sBool IsInside(const sBaseRect &r) const { return (r.x0<x1 && r.x1>x0 && r.y0<y1 && r.y1>y0); }   // this is inside r
  sBool IsEmpty() const { return x0==x1 || y0==y1; }

  void Extend(Type e) { x0-=e;y0-=e;x1+=e;y1+=e; }
  void Extend(const sBaseRect &r) { x0-=r.x0;y0-=r.y0;x1+=r.x1;y1+=r.y1; }
  void Add(const sBaseRect &a,const sBaseRect &b) { if(b.IsEmpty()) { *this=a; } else if(a.IsEmpty()) { *this=b; } else { x0=sMin(a.x0,b.x0); y0=sMin(a.y0,b.y0); x1=sMax(a.x1,b.x1); y1=sMax(a.y1,b.y1); } } 
  void Add(const sBaseRect &a)                    { if(  IsEmpty()) { *this=a; } else if(!a.IsEmpty())                  { x0=sMin(a.x0,  x0); y0=sMin(a.y0,  y0); x1=sMax(a.x1,  x1); y1=sMax(a.y1,  y1); } } 
  void And(const sBaseRect &a,const sBaseRect &b) { x0=sMax(a.x0,b.x0); y0=sMax(a.y0,b.y0); x1=sMin(a.x1,b.x1); y1=sMin(a.y1,b.y1); if(x0>=x1||y0>=y1) Init(); } 
  void And(const sBaseRect &a)                    { x0=sMax(a.x0,  x0); y0=sMax(a.y0,  y0); x1=sMin(a.x1,  x1); y1=sMin(a.y1,  y1); if(x0>=x1||y0>=y1) Init(); } 
  void Move(Type x, Type y)                       { x0+=x; x1+=x; y0+=y; y1+=y; }
  void Scale(Type s)                              { x0*=s; x1*=s; y0*=s; y1*=s; }
  void Scale(Type x, Type y)                      { x0*=x; x1*=x; y0*=y; y1*=y; }
  void ScaleCenter(sF32 x, sF32 y)                { Type sx = SizeX()/2; Type sy = SizeY()/2; Type cx = CenterX(); Type cy = CenterY(); x0 = Type(cx-sx*x); y0 = Type(cy-sy*y); x1 = Type(cx+sx*x); y1 = Type(cy+sy*y); }
  void ScaleCenter(sF32 s)                        { ScaleCenter(s,s); }
  void MoveCenter(Type cx, Type cy)               { Type sx = SizeX()/2; Type sy = SizeY()/2; x0=cx-sx;x1=cx+sx;y0=cy-sy;y1=cy+sy; }
  void MoveCenterX(Type cx)                       { Type sx = SizeX()/2; x0=cx-sx;x1=cx+sx; }
  void MoveCenterY(Type cy)                       { Type sy = SizeY()/2; y0=cy-sy;y1=cy+sy; }
  void SetSize(Type sx, Type sy)                  { x1 = x0 + sx; y1 = y0 + sy; }
  
  int operator !=(const sBaseRect<Type> &r) const { return x0!=r.x0 || x1!=r.x1 || y0!=r.y0 || y1!=r.y1; }
  int operator ==(const sBaseRect<Type> &r) const { return x0==r.x0 && x1==r.x1 && y0==r.y0 && y1==r.y1; }
  sBaseRect<Type> & operator = (const sBaseRect<Type> &r) { x0=r.x0; x1=r.x1; y0=r.y0; y1=r.y1; return *this; }
  sBool Hit(Type x,Type y) const { return x>=x0 && x<x1 && y>=y0 && y<y1; }
  void Sort() { if(x0>x1) sSwap(x0,x1); if(y0>y1) sSwap(y0,y1); }
  void Inset(Type o) { InsetX(o); InsetY(o); } 
  void Inset(const sBaseRect &r) { x0+=r.x0;y0+=r.y0;x1-=r.x1;y1-=r.y1; }
  void InsetX(Type o) { Type mx=sMin(o,SizeX()/2); x0+=mx; x1-=mx; } 
  void InsetY(Type o) { Type my=sMin(o,SizeY()/2); y0+=my; y1-=my; }

  void SwapX() { sSwap(x0, x1); }
  void SwapY() { sSwap(y0, y1); }
  void Swap() { SwapX(); SwapY(); }
  void Fix() { if (x1 < x0) SwapX(); if (y1 < y0) SwapY(); }

  void Blend(const sBaseRect &l, const sBaseRect &r, sF32 n) { sF32 m=1-n; x0 = l.x0 * m + r.x0 * n; x1 = l.x1 * m + r.x1 * n; y0 = l.y0 * m + r.y0 * n; y1 = l.y1 * m + r.y1 * n; }
};

typedef sBaseRect<sInt> sRect;
typedef sBaseRect<sF32> sFRect;

sFormatStringBuffer& operator% (sFormatStringBuffer &f, const sRect &r);
sFormatStringBuffer& operator% (sFormatStringBuffer &f, const sFRect &r);

/****************************************************************************/

struct sRandom                          // unreliable and fast random
{
private:
  sU32 kern;
public:
  void Init();
  void Seed(sInt seed);
  sInt Int(sInt max);
  sInt Int16();
  sU32 Int32();
  sF32 Float(sF32 max);
};

/****************************************************************************/

class sTiming
{
  sInt TotalTime;
  sInt LastTime;
  sInt FirstFrame;
  sInt Slices;
  sInt TimeSlice;
  sInt Delta;
  sInt Jitter;
  sInt Timestamp;

  sInt History[64];                         // buffer for building average time.
  sInt HistoryIndex;                        // round-robin pointer to next sample
  sInt HistoryCount;                        // how many samples have been taken?
  sInt HistorySkip;                         // don't measure the first few frames
public:
  sTiming();                                // initialize structures
  void SetTimeSlice(sInt ts=10);            // set duration of timeslice
  sInt GetTime() const { return TotalTime; }     // get time of frame, in ms
  sInt GetDelta() const { return Delta; }        // get time delta from last frame
  sInt GetSlices() const { return Slices; }      // get number of slices for this frame
  sInt GetJitter() const { return Jitter; }      // get remaining milliseconds in next slice
  sF32 GetAverageDelta() const;                   // averaged milliseconds of last 64 frames
  

  void OnFrame(sInt time);                  // call this every frame
  
  void StartMeasure(sInt time);             // alternative timing interface
  void StopMeasure(sInt time);

  // timestamp methods
  void SetTimestamp(sInt time) { Timestamp = time; } 
  sInt GetTimestamp()          { return Timestamp; } 
  sInt GetTimestampDelta(sInt time) { return time-Timestamp; }
};

/****************************************************************************/

sU32 sChecksumAdler32(const sU8 *data,sInt size);
sU32 sChecksumCRC32(const sU8 *data,sInt size);
sU32 sChecksumRandomByte(const sU8 *data,sInt size);

void sChecksumAdler32Begin();
void sChecksumAdler32Add(const sU8 *data,sInt size);
sU32 sChecksumAdler32End();


struct sChecksumMD5
{
private:
  inline void f(sU32 &a,sU32 b,sU32 c,sU32 d,sInt k,sInt s,sU32 t,sU32 *data) { a += (d^(b&(c^d))) + data[k] + t;  a = ((a<<s)|(a>>(32-s))) + b; }
  inline void g(sU32 &a,sU32 b,sU32 c,sU32 d,sInt k,sInt s,sU32 t,sU32 *data) { a += (c^(d&(b^c))) + data[k] + t;  a = ((a<<s)|(a>>(32-s))) + b; }
  inline void h(sU32 &a,sU32 b,sU32 c,sU32 d,sInt k,sInt s,sU32 t,sU32 *data) { a += (b^c^d)       + data[k] + t;  a = ((a<<s)|(a>>(32-s))) + b; }
  inline void i(sU32 &a,sU32 b,sU32 c,sU32 d,sInt k,sInt s,sU32 t,sU32 *data) { a += (c^(b|~d))    + data[k] + t;  a = ((a<<s)|(a>>(32-s))) + b; }
  void Block(const sU8 *data);
public:
  sU32 Hash[4];

  void CalcBegin();
  sInt CalcAdd(const sU8 *data, sInt size);   // calculate 64 byte blocks, returns consumed bytes
  void CalcEnd(const sU8 *data, sInt size, sInt sizeall);

  void Calc(const sU8 *data,sInt size);
  sBool Check(const sU8 *data,sInt size);
  sBool operator== (const sChecksumMD5 &)const;
  sBool operator!= (const sChecksumMD5 &md5)const    { return !(*this==md5); }
};

sFormatStringBuffer& operator% (sFormatStringBuffer &f, const sChecksumMD5 &md5);

/****************************************************************************/

sU32 sMulColor(sU32 a,sU32 b);              // a*b
sU32 sAddColor(sU32 a,sU32 b);              // a+b
sU32 sMadColor(sU32 mul1,sU32 mul2,sU32 add); // mul1*mul2+add
sU32 sFadeColor(sInt fade,sU32 a,sU32 b);   // fade=0..0x10000

sU32 sColorFade (sU32 a, sU32 b, sF32 f);   // fade=0.0f..1.0f
sU32 sAlphaFade (sU32 c, sF32 f);           // fade=0.0f..1.0f
sF32 sScaleFade(sF32 a, sF32 b, sF32 f);    // fade=0.0f..1.0f

/****************************************************************************/
/***                                                                      ***/
/***   Memory Management                                                  ***/
/***                                                                      ***/
/****************************************************************************/
/***                                                                      ***/
/***   when you call sInitMem()...                                        ***/
/***   - positive "mem": specify total segment memory (for pc debugs)     ***/
/***   - negative "mem": specify legacy-heap left amount (for consoles)   ***/
/***     on devkits, the surplus memory will not be used!                 ***/
/***   - for pc release builds: don't use segments! without calling       ***/
/***     sInitMem(), all other calls (except sALlocMem()/sFreeMem())      ***/
/***     will be ignored and return reasonable values                     ***/
/***                                                                      ***/
/***   - per default no top/down segments exist                           ***/
/***   - create top/down segment with sPush???MemSegment/sPop???MemSegment***/
/***   - new/delete access bottom segment                                 ***/
/***   - you need to flush whole segment to redeem memory                 ***/
/***   - you can start a new segment, for controlled flushing             ***/
/***                                                                      ***/
/***   - segments id's range from 1 to 15 inclusive, you should define    ***/
/***     global constants for the id's you choose                         ***/
/***   - use segment id sMEMSEG_HEAP to access the legacy-heap. there     ***/
/***     should always be a few 100kb's free for os-usage                 ***/
/***   - use segment id's sMEMSEG_TOP and sMEMSEG_BOTTOM in rare cases    ***/
/***                                                                      ***/
/***   - new/delete matches are monitored in debug builds                 ***/
/***                                                                      ***/
/***   - elements using the legacy heap:                                  ***/
/***     - by default all allocations before sSystemInit and              ***/
/***       sSetConstructorMemSegment to another segment                   ***/
/***     - managed sVertexFormats always                                  ***/
/***                                                                      ***/
/****************************************************************************/

enum sMemConstants
{
  sMEMSEG_HEAP = -1,              // legacy heap
  sMEMSEG_TOP = -2,               // current top segment
  sMEMSEG_BOTTOM = -3,            // current bottom segment
  sMEMSEG_CONSTRUCT = -4,         // current constructor segment
  sMEMSEG_INVALID = 0,            // invalid segment id
  sMEMSEG_DYNBUFFER = 1,          // quadbuffer segment for dynamic graphics

  // memory debugging can slow things down a lot, with lines and match beeing the cheapest.
  sMEMDEBUG_LINES = 1,            // track allocs and print file and line numbers when the free was forgotten.
  sMEMDEBUG_MATCH = 2,            // track allocs and match frees. assert when flushing a segment with allocs alive
  sMEMDEBUG_MEMWALL = 4,          // clear memory before alloc and after free. add some bytes to check overflows.
  sMEMDEBUG_PROTECT = 8,          // use virtual memory to protect flushed segments from read and write access

  // mem system config
  sMEMSEG_MAX = 16,               // maximum number of segments
  sMEMSEG_DBGINFOMAX = 16384,     // maximum number of debug info entries (must be <= 65535)

  // alloc flags
  sMEMFLAG_NORMAL = 0,            // normal cached memory
  sMEMFLAG_GRAPHICS = 1,          // sequential write only access, good for graphics cards.
};

void sInitMem(sInt mem, sInt dynmem, sInt debugflags);      // initalize memsegment system
void sPushTopMemSegment(sInt seg, sCONFIG_SIZET minsize=0, sInt align=16);            // start a new segment from top and reserves up to minsize memory
void sPushBottomMemSegment(sInt seg, sCONFIG_SIZET minsize=0, sInt align=16);         // start a new segment from bottom and reserves up to minsize memory
void sPopMemSegment(sInt seg);                // pop top or bottom segment
void sFlushMemSegment(sInt seg);              // flush segment, without popping it
void sSetConstructorMemSegment(sInt seg);     // deligate new/delete to memsegment or legacy-heap
void *sAllocMem(sCONFIG_SIZET size,sInt flags,sInt align,sInt segid, const sChar8 *file=0, sInt line=-1); // alloc memory from memsegment or legacy-heap
void sFreeMem(void *);                        // debug-builds carefully count alloc/free matches

sInt sGetBottomMemSegment();                  // current bottom segment (starting at low adresses)
sInt sGetTopMemSegment();                     // current top segment (starting at high addresses)
sDInt sGetMemSegmentUsed(sInt seg);           // memory used in segment
sDInt sGetMemSegmentFree(sInt seg);           // current free memory in segment
sInt sGetMemTotalFree();                      // free memory between top and bottom pointer
sInt sGetConstructorMemSegment();             // get current ConstructorMemSegment (to restore if tempoary changed)

// double buffered dynamic buffers for dynamic gfx data
void *sAllocDynBuffer(sCONFIG_SIZET size, sInt align);
void sInitDynBuffer(sCONFIG_SIZET size);
void sExitDynBuffer();
void sFlipDynBuffer();

// helper switch to specific segment for current scope
class sScopeMemSeg
{
  sInt Seg;
public:
  sScopeMemSeg(sInt seg)  { Seg=sGetConstructorMemSegment(); sSetConstructorMemSegment(seg); }
  ~sScopeMemSeg()         { sSetConstructorMemSegment(Seg); }
};

#if !sRELEASE
#define sDEFINE_ALLOC(size,flags,align,segid) sAllocMem(size,flags,align,segid,__FILE__,__LINE__)
#else
#define sDEFINE_ALLOC(size,flags,align,segid) sAllocMem(size,flags,align,segid)
#endif
#define sAllocMem(size,flags,align,segid) sDEFINE_ALLOC(size,flags,align,segid)

/****************************************************************************/

/****************************************************************************/
/***                                                                      ***/
/***   Application Interface                                              ***/
/***                                                                      ***/
/****************************************************************************/

// this should be in system.hpp!

enum sInputEventDevice
{
  sIED_KEYBOARD = 1,
  sIED_MOUSE = 2,
  sIED_JOYPAD = 3,
  sIED_MAX,
};

struct sInputEvent
{
  sInt DeviceType;                          // sIED_???
  sInt DeviceId;                            // 0..n
  sU32 Key;                                 // Key | Qualifiers
  sInt Timestamp;                           // ms accurate timestamp - good for 50 days - only use differences!
  sInt MouseX,MouseY;                       // position of mouse cursor when event was created
};

class sApp
{
public:
  virtual ~sApp();

  virtual void OnInit();                   // to init stuff which cannot be done by contructor
  
  virtual void OnInput(const sInputEvent &ie);
  virtual void OnPaint2D(const sRect &client,const sRect &update);
  virtual void OnPrepareFrame();           // is called just before paint3d but not between renderbegin and renderend
  virtual void OnPaint3D();

  virtual void OnEvent(sInt id); 
//  virtual void OnMouse(sInt buttons,sInt x,sInt y);   // obsolete
//  virtual void OnKey(sU32 key);     // obsolete
};

void sMain();
void sSetApp(sApp *);
sApp *sGetApp();

// events not tested!

enum sAppEvents
{
  sAE_TIMER = 1,
  sAE_TRIGGER,
  sAE_EXIT,

  sAE_USER = 0x1000,
};

void sSetTimerEvent(sInt time,sBool loop = 1);
void sTriggerEvent(sInt event=sAE_TRIGGER);

// command line parsing
void sParseCmdLine(const sChar *cmd,sBool skipcmd=1);
sBool sAddShellParameter(const sChar *opt, const sChar *val);

// ommit the '-', like sGetShellSwitch(L"x");
const sChar *sGetShellParameter(const sChar *opt,sInt n);
sInt sGetShellParameterInt(const sChar *opt,sInt n,sInt def);
sBool sGetShellSwitch(const sChar *opt);

// for short and long option, like sGetShellSwitch(L"n",L"-name","dierk2);
const sChar *sGetShellString(const sChar *optshort,const sChar *optlong,const sChar *def=0);
sInt sGetShellInt(const sChar *optshort,const sChar *optlong,sInt def=0);

/****************************************************************************/
/****************************************************************************/
/***                                                                      ***/
/***   Containers with explicitly static memory management                ***/
/***                                                                      ***/
/****************************************************************************/
/****************************************************************************/


/****************************************************************************/
/***                                                                      ***/
/***   sStaticArray                                                       ***/
/***                                                                      ***/
/****************************************************************************/
/***                                                                      ***/
/***   Base of all Arrays. You state the size ONCE and never change it.   ***/
/***   No Dynamic allocation occours.                                     ***/
/***                                                                      ***/
/***   To set the Size, use HintSize() or AddMany(). HintSize() will      ***/
/***   allocate memory, but the Array is still empty, you will need to    ***/
/***   use AddXXX() to put something into. AddMany() will add multiple    ***/
/***   items, and if this is your first call you can allocate and fill    ***/
/***   the Array at the same time                                         ***/
/***                                                                      ***/
/***   sArray in types2.hpp is the same, just with dynamic reallocation.  ***/
/***                                                                      ***/
/****************************************************************************/

// used to get the base type of a pointer type,
// which can then be used like this:
//
// sStaticArray<T>::ElementType *element;
// sFORALL(Array, element) {}

template<typename T>
struct sRemovePtrType { typedef T Type; };
template<typename T>
struct sRemovePtrType<T*> { typedef T Type; };
template<typename T>
struct sRemovePtrType<T**> { typedef T Type; };

template <class Type> class sStaticArray
{
protected:

  Type *Data;
  sInt Used;
  sInt Alloc;

  virtual void ReAlloc(sInt max)          { if(max>Alloc) { sVERIFY(!Data); Data = new Type[max]; Alloc=max; } }
  virtual void Grow(sInt add)             { 
                                            sBool overflow = Used+add>Alloc;
#if !sRELEASE
                                            if (overflow)
                                            {
                                              sString<128> s;
                                              s.PrintF(L"Overflow! Alloc: %d Used: %d Added: %d\n",Alloc, Used, add);
                                              sFatal(s);
                                            }
#endif

                                            sVERIFY(!overflow); 
                                          } 

public:
  typedef Type BaseType;
  typedef typename sRemovePtrType<Type>::Type ElementType;
  typedef sInt Iterator;

  explicit sStaticArray(sInt max)       { Data = 0; Used = 0; Alloc = 0; ReAlloc(max); }
  sStaticArray()                        { Data = 0; Used = 0; Alloc = 0; }
  sStaticArray(Type *data, sInt max) { Data = data; Used = 0; Alloc = max; }
  virtual ~sStaticArray()               { delete[] Data; }
  void Copy(const sStaticArray &a)      { Resize(a.GetCount()); for(sInt i=0;i<a.GetCount();i++) Data[i] = a.Data[i]; }
  sStaticArray(const sStaticArray &a)   { Data = 0; Copy(a); }
  sStaticArray &operator=(const sStaticArray &a)  { if (this != &a) Copy(a); return *this; }
  // reset deallocates memory and resets container,
  // use Clear instead if you want to reuse your container afterwards
  virtual void Reset()                  { sDeleteArray(Data); Used = 0; Alloc = 0;}

  sInt GetCount() const                 { return Used; }
  sInt GetSize() const                  { return Alloc; }
  sBool IsEmpty() const                 { return Used==0; }
  sBool IsFull() const                  { return Used==Alloc; }
  void HintSize(sInt max)               { ReAlloc(max); }
  Type *Resize(sInt count)              { if (count>Alloc) ReAlloc(count); Used = count; return Data; }
  void Clear()                          { Used = 0; }

//  void Add(const Type &e)               { sVERIFY(&e<Data || &e>=Data+Alloc); Grow(1); Data[Used++]=e; }
//  Type *Add()                           { Grow(1); return &Data[Used++]; }
  void Add(const sStaticArray<Type> &a) { Grow(a.Used); for(sInt i=0;i<a.Used;i++) Data[Used++]=a.Data[i]; }
  Type *AddMany(sInt count)             { Grow(count); Type *r=Data+Used; Used+=count; return r; }
  void AddBefore(const Type &e,sInt p)  { sVERIFY(&e<Data || &e>=Data+Alloc); sVERIFY(p>=0 && p<=Used); Grow(1); for(sInt i=Used;i>p;i--) Data[i]=Data[i-1]; Data[p]=e; Used++; }
  void AddAfter(const Type &e,sInt p)   { sVERIFY(&e<Data || &e>=Data+Alloc); AddBefore(e,p+1); }
  void AddHead(const Type &e)           { sVERIFY(&e<Data || &e>=Data+Alloc); AddBefore(e,0); }
  void AddTail(const Type &e)           { sVERIFY(&e<Data || &e>=Data+Alloc); Grow(1); Data[Used++]=e; }

  Type &GetTail() const                 { sVERIFY(!IsEmpty()); return Data[Used-1]; }

  void RemAt(sInt p)                    { sVERIFY(p>=0 && p<Used); Data[p] = Data[--Used]; }
  void RemAtOrder(sInt p)               { sVERIFY(p>=0 && p<Used); Used--; while(p<Used) {Data[p]=Data[p+1]; p++;} }
  Type RemTail()                        { sVERIFY(Used>0); Used--; return Data[Used]; }
  sBool Rem(const Type e)               { sBool result = sFALSE; for(sInt i=0;i<Used;) if(Data[i]==e) { result = sTRUE; RemAt(i); } else i++; return result; }
  sBool RemOrder(const Type e)          { sBool result = sFALSE; for(sInt i=0;i<Used;) if(Data[i]==e) { result = sTRUE; RemAtOrder(i); } else i++; return result; }

  const Type &operator[](sInt p) const  { sVERIFY(p>=0 && p<Used); return Data[p]; }
  Type &operator[](sInt p)              { sVERIFY(p>=0 && p<Used); return Data[p]; }

  void Swap(sStaticArray &a)            { sSwap(Data,a.Data); sSwap(Used,a.Used); sSwap(Alloc,a.Alloc); }
  void Swap(sInt i,sInt j)              { sSwap(Data[i],Data[j]); }

  sInt IndexOf(const Type e)            { for(sInt i = 0; i < GetCount(); i++) if (Data[i] == e) return i; return -1; }
};

template <class T> class sGetIterator   // used anywhere?
{
public:
  typedef sInt It;
};

/****************************************************************************/

template <class Type,int Max_> class sStackArray : public sStaticArray<Type>
{
protected:

  void ReAlloc(sInt max)                { sVERIFY(max<=Max_); }
  void Grow(sInt add)                   { 
                                            sBool overflow = sStaticArray<Type>::Used+add>Max_;
#if !sRELEASE
                                            if (overflow)
                                            {
                                              sString<128> s;
                                              s.PrintF(L"Overflow! Alloc: %d Used: %d Added: %d\n",sStaticArray<Type>::Alloc,sStaticArray<Type>::Used, add);
                                              sFatal(s);
                                            }
#endif

                                            sVERIFY(!overflow); 
                                        } 
  Type Storage[Max_];
public:                                 // das ganze rumgescope ist für den PS3-compiler...
  typedef typename sRemovePtrType<Type>::Type ElementType;
  sStackArray()                         { sStaticArray<Type>::Data = Storage; sStaticArray<Type>::Used = 0; sStaticArray<Type>::Alloc = Max_; }
  sStackArray(sInt initcount)           { sStaticArray<Type>::Data = Storage; sStaticArray<Type>::Used = 0; sStaticArray<Type>::Alloc = Max_; sStaticArray<Type>::AddMany(initcount); }
  ~sStackArray()                        { sStaticArray<Type>::Data = 0; }
  void Reset()                          { sStaticArray<Type>::Used = 0; sStaticArray<Type>::Alloc = Max_; }
};


/****************************************************************************/
/***                                                                      ***/
/***   FORALL macro                                                       ***/
/***                                                                      ***/
/****************************************************************************/

// adapter functions

template <template <typename> class ArrayType,class BaseType> 
BaseType *sGetPtr(ArrayType<BaseType *> &a,sInt i)
{
  return a[i];
}

template <template <typename> class ArrayType,class BaseType> 
BaseType *sGetPtr(ArrayType<BaseType> &a,sInt i)
{
  return &a[i];
}

template <template <typename> class ArrayType,class BaseType> 
const BaseType *sGetPtr(const ArrayType<BaseType *> &a,sInt i)
{
  return a[i];
}

template <template <typename> class ArrayType,class BaseType> 
const BaseType *sGetPtr(const ArrayType<BaseType> &a,sInt i)
{
  return &a[i];
}

// GCC needs these versions for sStackArray<class type,int max>

template <template <typename,int> class ArrayType,class BaseType,int count> 
BaseType *sGetPtr(ArrayType<BaseType *,count> &a,sInt i)
{
  return a[i];
}

template <template <typename,int> class ArrayType,class BaseType,int count> 
BaseType *sGetPtr(ArrayType<BaseType,count> &a,sInt i)
{
  return &a[i];
}

template <template <typename,int> class ArrayType,class BaseType,int count> 
const BaseType *sGetPtr(const ArrayType<BaseType *,count> &a,sInt i)
{
  return a[i];
}

template <template <typename,int> class ArrayType,class BaseType,int count> 
const BaseType *sGetPtr(const ArrayType<BaseType,count> &a,sInt i)
{
  return &a[i];
}

// the forall macro itself

#define sFORALL(l,e) for(sInt _i=0;_i<(l).GetCount()?((e)=sGetPtr((l),_i)),1:0;_i++)
#define sFORALLREVERSE(l,e) for(sInt _i=(l).GetCount()-1;_i>=0?((e)=sGetPtr((l),_i)),1:0;_i--)

/****************************************************************************/
/***                                                                      ***/
/***   Sorting                                                            ***/
/***                                                                      ***/
/****************************************************************************/

template <template <class> class ArrayType, class BaseType> 
void sSortUp(ArrayType<BaseType> &a)
{
  sInt max = a.GetCount();
  for(sInt i=0;i<max-1;i++)
    for(sInt j=i+1;j<max;j++)
      if(a[i] > a[j])
        sSwap(a[i],a[j]);
}

template <template <class> class ArrayType, class BaseType> 
void sSortDown(ArrayType<BaseType> &a)
{
  sInt max = a.GetCount();
  for(sInt i=0;i<max-1;i++)
    for(sInt j=i+1;j<max;j++)
      if(a[i] < a[j])
        sSwap(a[i],a[j]);
}

template <template <class> class ArrayType, class BaseType, class CmpType> 
void sCmpSortUp(ArrayType<BaseType> &a, sInt (*cmp) (CmpType, CmpType))
{
  sInt max = a.GetCount();
  for(sInt i=0;i<max-1;i++)
    for(sInt j=i+1;j<max;j++)
      if(cmp(a[i], a[j]) > 0)
        sSwap(a[i],a[j]);
}

template <template <class> class ArrayType, class BaseType, class CmpType> 
void sCmpSortDown(ArrayType<BaseType> &a, sInt (*cmp) (CmpType, CmpType))
{
  sInt max = a.GetCount();
  for(sInt i=0;i<max-1;i++)
    for(sInt j=i+1;j<max;j++)
      if(cmp(a[i], a[j]) < 0)
        sSwap(a[i],a[j]);
}

template <class ArrayType,class BaseType,class MemberType> 
void sSortUp(ArrayType &a,MemberType BaseType::*o)
{
  sInt max = a.GetCount();
  for(sInt i=0;i<max-1;i++)
    for(sInt j=i+1;j<max;j++)
      if(sGetPtr(a,i)->*o > sGetPtr(a,j)->*o)
        sSwap(a[i],a[j]);
}

template <class ArrayType,class BaseType,class MemberType> 
void sSortDown(ArrayType &a,MemberType BaseType::*o)
{
  sInt max = a.GetCount();
  for(sInt i=0;i<max-1;i++)
    for(sInt j=i+1;j<max;j++)
      if(sGetPtr(a,i)->*o < sGetPtr(a,j)->*o)
        sSwap(a[i],a[j]);
}


template <class ArrayType> 
void sHeapSortUp(ArrayType &a)
{
  sInt count=a.GetCount();

  // pass 1: heapify (sift up)
  for (sInt i=1; i<count; i++)
  {
    sInt child=i;
    while (child>0)
    {
      sInt root=(child-1)/2;
      if (a[root]<a[child])
      {
        sSwap(a[root],a[child]);
        child=root;
      }
      else
        break;
    }
  }

  // pass 2: sort (sift down)
  while (--count>0)
  {
    sSwap(a[0],a[count]);
    sInt root=0;
    sInt child;
    while ((child=root*2+1)<count)
    {
      if (child<count-1 && a[child]<a[child+1])
        child++;
      if (a[root]<a[child])
      {
        sSwap(a[root],a[child]);
        root=child;
      }
      else
        break;
    }
  }
}



template <class ArrayType,class BaseType,class MemberType> 
void sHeapSortUp(ArrayType &a,MemberType BaseType::*o)
{
  sInt count=a.GetCount();

  // pass 1: heapify (sift up)
  for (sInt i=1; i<count; i++)
  {
    sInt child=i;
    while (child>0)
    {
      sInt root=(child-1)/2;
      if (sGetPtr(a,root)->*o < sGetPtr(a,child)->*o)
      {
        sSwap(a[root],a[child]);
        child=root;
      }
      else
        break;
    }
  }

  // pass 2: sort (sift down)
  while (--count>0)
  {    
    sSwap(a[0],a[count]);
    sInt root=0;
    sInt child;
    while ((child=root*2+1)<count)
    {
      if (child<count-1 && sGetPtr(a,child)->*o < sGetPtr(a,child+1)->*o)
        child++;
      if (sGetPtr(a,root)->*o<sGetPtr(a,child)->*o)
      {
        sSwap(a[root],a[child]);
        root=child;
      }
      else
        break;
    }
  }
}

// one should use some kind of function template or somethig...

//template <class Type> struct sCompareGreater { static sBool f(const Type a,const Type b) { return a>b; } };
//template <class Type> struct sCompareLesser  { static sBool f(const Type a,const Type b) { return a<b; } };

/****************************************************************************/
/***                                                                      ***/
/***   Remove and Delete by boolean test                                  ***/
/***                                                                      ***/
/****************************************************************************/

template <class ArrayType,class BaseType,class MemberType> 
void sRemTrue(ArrayType &a,MemberType BaseType::*o)
{
  for(sInt i=0;i<a.GetCount();)
  {
    if(sGetPtr(a,i)->*o)
      a.RemAt(i);
    else
      i++;
  }
}

template <class ArrayType,class BaseType,class MemberType> 
void sRemFalse(ArrayType &a,MemberType BaseType::*o)
{
  for(sInt i=0;i<a.GetCount();)
  {
    if(!(sGetPtr(a,i)->*o))
      a.RemAt(i);
    else
      i++;
  }
}

template <class ArrayType,class BaseType,class MemberType> 
void sRemOrderFalse(ArrayType &a,MemberType BaseType::*o)
{
  sInt max = a.GetCount();
  sInt used = 0;
  for(sInt i=0;i<max;i++)
    if(sGetPtr(a,i)->*o)
      a[used++] = a[i];
  a.Resize(used);
}

template <class ArrayType,class BaseType,class MemberType> 
void sRemEqual(ArrayType &a,MemberType BaseType::*o,const MemberType &e)
{
  for(sInt i=0;i<a.GetCount();)
  {
    if(sGetPtr(a,i)->*o == e)
      a.RemAt(i);
    else
      i++;
  }
}

template <class ArrayType,class BaseType,class MemberType> 
void sDeleteTrue(ArrayType &a,MemberType BaseType::*o)
{
  for(sInt i=0;i<a.GetCount();)
  {
    if(sGetPtr(a,i)->*o)
    {
      delete sGetPtr(a,i);
      a.RemAt(i);
    }
    else
    {
      i++;
    }
  }
}

template <class ArrayType,class BaseType,class MemberType> 
void sDeleteFalse(ArrayType &a,MemberType BaseType::*o)
{
  for(sInt i=0;i<a.GetCount();)
  {
    if(!sGetPtr(a,i)->*o)
    {
      delete sGetPtr(a,i);
      a.RemAt(i);
    }
    else
    {
      i++;
    }
  }
}

template <template <typename> class ArrayType,class BaseType> 
void sDeleteAll(ArrayType<BaseType> &a)
{
  BaseType e;
  while(!a.IsEmpty())
  {
    e = a.RemTail();
    delete e;
  }
}

template <template <typename> class ArrayType> 
void sDeleteAll(ArrayType<void *> &a)
{
  void *e;
  while(!a.IsEmpty())
  {
    e = a.RemTail();
    delete (sU8 *)e;
  }
}

template <template <typename> class ArrayType,class BaseType> 
void sReleaseAll(ArrayType<BaseType> &a)
{
  BaseType e;
  while(!a.IsEmpty())
  {
    e = a.RemTail();
    e->Release();
  }
}

template <class BaseType,int Max_>
void sDeleteAll(sStackArray<BaseType,Max_> &a)
{
  BaseType e;
  while(!a.IsEmpty())
  {
    e = a.RemTail();
    delete e;
  }
}

template <class BaseType,int Max_>
void sReleaseAll(sStackArray<BaseType,Max_> &a)
{
  BaseType e;
  while(!a.IsEmpty())
  {
    e = a.RemTail();
    e->Release();
  }
}

template <template <typename> class ArrayType,class BaseType> 
void sAddRefAll(ArrayType<BaseType> &a)
{
  BaseType e;
  sFORALL(a,e)
    e->AddRef();
}


/****************************************************************************/
/***                                                                      ***/
/***   Finding                                                            ***/
/***                                                                      ***/
/****************************************************************************/

// const versions

template <class ArrayType,class BaseType,class MemberType,class CompareType> 
const BaseType *sFind(const ArrayType &a,MemberType BaseType::*o,const CompareType i)
{
  const BaseType *e;
  sFORALL(a,e)
    if(e->*o == i)
      return e;
  return 0;
}

template <class ArrayType,class BaseType,class MemberType,class CompareType> 
sInt sFindIndex(const ArrayType &a,MemberType BaseType::*o,const CompareType i)
{
  const BaseType *e;
  sFORALL(a,e)
    if(e->*o == i)
      return _i;
  return -1;
}

template <class ArrayType,class BaseType,class MemberType,class CompareType> 
sInt sFindNextIndex(const ArrayType &a, sInt index,MemberType BaseType::*o,const CompareType i)
{
  const BaseType *e;
  sInt _i=index+1;
  for (; _i<a.GetCount()?(e=sGetPtr((a),_i)),1:0;_i++)
    if(e->*o == i)
      return _i;
  return -1;
}


template <class ArrayType,class BaseType> 
const BaseType *sFind(const ArrayType &a,const BaseType &v)
{
  const BaseType *e;
  sFORALL(a,e)
    if(*e == v)
      return e;
  return 0;
}

template <class ArrayType,class BaseType> 
sBool sFindPtr(const ArrayType &a,const BaseType *v)
{
  sInt max = a.GetCount();
  for (sInt i=0; i<max; i++)
    if(a[i] == v)
      return 1;
  return 0;
}

template <class ArrayType,class BaseType> 
sInt sFindIndex(const ArrayType &a,const BaseType &v)
{
  sInt max = a.GetCount();
  for (sInt i=0; i<max; i++)
    if(a[i] == v)
      return i;
  return -1;
}

template <class ArrayType,class BaseType> 
sInt sFindIndexPtr(const ArrayType &a,const BaseType *v)
{
  // this can be more efficient implemented using pointerarithmetics
  sInt max = a.GetCount();
  for (sInt i=0; i<max; i++)
    if(&a[i] == v)
      return i;
  return -1;
}

template <class ArrayType,class BaseType,class MemberType> 
sInt sFindIndex(const ArrayType &a, MemberType BaseType::*o, const MemberType v)
{
  sInt max = a.GetCount();
  for(sInt i=0;i<max;i++)
      if(sGetPtr(a,i)->*o == v)
        return i;
  return -1;
}

template <class ArrayType,class BaseType,class MemberType> 
const BaseType *sFindTrue(const ArrayType &a,MemberType BaseType::*o)
{
  const BaseType *e;
  sFORALL(a,e)
    if(e->*o)
      return e;
  return 0;
}

template <class ArrayType,class BaseType,class MemberType> 
const BaseType *sFindFalse(const ArrayType &a,MemberType BaseType::*o)
{
  const BaseType *e;
  sFORALL(a,e)
    if(!e->*o)
      return e;
  return 0;
}

/****************************************************************************/

// non-const versions

template <class ArrayType,class BaseType,class MemberType,class CompareType> 
BaseType *sFind(ArrayType &a,MemberType BaseType::*o,const CompareType i)
{
  BaseType *e;
  sFORALL(a,e)
    if(e->*o == i)
      return e;
  return 0;
}

template <class ArrayType,class BaseType> 
BaseType *sFind(ArrayType &a,const BaseType &v)
{
  BaseType *e;
  sFORALL(a,e)
    if(*e == v)
      return e;
  return 0;
}

template <class ArrayType,class BaseType,class MemberType> 
BaseType *sFindTrue(ArrayType &a,MemberType BaseType::*o)
{
  BaseType *e;
  sFORALL(a,e)
    if(e->*o)
      return e;
  return 0;
}

template <class ArrayType,class BaseType,class MemberType> 
BaseType *sFindFalse(ArrayType &a,MemberType BaseType::*o)
{
  BaseType *e;
  sFORALL(a,e)
    if(!e->*o)
      return e;
  return 0;
}

template <class ArrayType,class BaseType,class MemberType,class CompareType> 
BaseType *sFindStringI(ArrayType &a,MemberType BaseType::*o,const CompareType i)
{
  BaseType *e;
  sFORALL(a,e)
    if(sCmpStringI(e->*o,i)==0)
      return e;
  return 0;
}

template <class ArrayType,class BaseType,class MemberType,class CompareType> 
BaseType *sFindStringP(ArrayType &a,MemberType BaseType::*o,const CompareType i)
{
  BaseType *e;
  sFORALL(a,e)
    if(sCmpStringP(e->*o,i)==0)
      return e;
  return 0;
}

/****************************************************************************/
/***                                                                      ***/
/***   Double Linked List                                                 ***/
/***                                                                      ***/
/****************************************************************************/
/***                                                                      ***/
/***   Your Element class will need to have an element of type sDNode,    ***/
/***   that's where the link pointers are stored.                         ***/
/***   You must pass the byteoffset to that node to the List. This        ***/
/***   allows an element being in multiple list, provided it has a        ***/
/***   node for every List. Example:                                      ***/
/***                                                                      ***/
/***   struct MyNode                                                      ***/
/***   {                                                                  ***/
/***     sInt Payload;                                                    ***/
/***     sDNode NodeA;                                                    ***/
/***     sDNode NodeB;                                                    ***/
/***   };                                                                 ***/
/***                                                                      ***/
/***   sDList<MyNode,&MyNode::NodeA> ListA;                               ***/
/***   sDList<MyNode,&MyNode::NodeB> ListB;                               ***/
/***                                                                      ***/
/***                                                                      ***/
/***                                                                      ***/
/***                                                                      ***/
/****************************************************************************/

class sDNode
{
public:
  sDNode *Next;
  sDNode *Prev;
  sDNode() { Next = Prev = 0; }
};

template <class Type,sDNode Type::*Offset> class sDList
{
  sDNode First;
  sDNode Last;

  const sDNode *GetNode(const Type* e) const   { return &(e->*Offset); } //{ return (sDNode *)(((sU8*)e)+Offset); }
  const Type* GetType(const sDNode *n) const   { return (const Type*) (((sU8*)n)-sOFFSETOMP((Type *)0,Offset)); }

  sDNode *GetNode(Type* e)                     { return &(e->*Offset); } // (sDNode *)(((sU8*)e)+Offset); }
  Type* GetType(sDNode *n)                     { return (Type*) (((sU8*)n)-sOFFSETOMP((Type *)0,Offset)); }

public:
  typedef typename sRemovePtrType<Type>::Type ElementType;

  void Clear()                    { First.Next=&Last; First.Prev=0; Last.Next=0; Last.Prev=&First; }
  sDList()                        { Clear(); }
  int GetCount() const            { int count=0; for (const Type* iter = GetHead(); !IsEnd(iter); iter=GetNext(iter), ++count); return count;}
  sBool IsEmpty() const           { return (First.Next==&Last); }

  const Type* GetNext(const Type* e) const            { return GetType(GetNode(e)->Next); } // { return (Type*) (((sU8*)(GetNode(e)->Next))-MO(Offset)); }
  const Type* GetPrev(const Type* e) const            { return GetType(GetNode(e)->Prev); } // { return (Type*) (((sU8*)(GetNode(e)->Prev))-MO(Offset)); }
  Type* GetNext(Type* e)                              { return GetType(GetNode(e)->Next); } // { return (Type*) (((sU8*)(GetNode(e)->Next))-MO(Offset)); }
  Type* GetPrev(Type* e)                              { return GetType(GetNode(e)->Prev); } // { return (Type*) (((sU8*)(GetNode(e)->Prev))-MO(Offset)); }
  sBool IsFirst(const Type* e) const     { return (GetNode(e)->Prev->Prev==0); }
  sBool IsLast(const Type* e) const      { return (GetNode(e)->Next->Next==0); }
  sBool IsEnd(const Type* e) const       { return (GetNode(e)->Next==0); }

  Type* GetHead()                  { return GetType(First.Next); }
  Type* GetTail()                  { return GetType(Last.Prev); }
  const Type* GetHead() const      { return GetType(First.Next); }
  const Type* GetTail() const      { return GetType(Last.Prev); }
  Type* GetSlow(sInt n)            { Type* e=GetHead(); for(;;) { if(IsEnd(e)) return 0; if(n==0) return e; e=GetNext(e); n--; } }
  sInt GetIndex(Type* e) const     { const Type* h=GetType(First.Next); for(sInt i=0;!IsEnd(h);i++, h=GetNext(h)) if (e==h) return i; return -1; }

  void AddHead(Type* e)            { GetNode(e)->Next = First.Next; GetNode(e)->Prev=&First; First.Next->Prev = GetNode(e); First.Next=GetNode(e); }
  void AddTail(Type* e)            { GetNode(e)->Prev = Last .Prev; GetNode(e)->Next=&Last ; Last .Prev->Next = GetNode(e); Last .Prev=GetNode(e); }
  void Rem(Type* e)                { GetNode(e)->Next->Prev = GetNode(e)->Prev; GetNode(e)->Prev->Next = GetNode(e)->Next; }
  Type* RemHead()                  { if(IsEmpty()) return 0; Type* e=GetHead(); Rem(e); return e; }
  Type* RemTail()                  { if(IsEmpty()) return 0; Type* e=GetTail(); Rem(e); return e; }
  void AddBefore(Type* e,Type* ref) { GetNode(e)->Prev=GetNode(ref)->Prev; GetNode(e)->Next=GetNode(ref); GetNode(ref)->Prev->Next=GetNode(e); GetNode(ref)->Prev=GetNode(e); }
  void AddAfter(Type* e,Type* ref)  { GetNode(e)->Next=GetNode(ref)->Next; GetNode(e)->Prev=GetNode(ref); GetNode(ref)->Next->Prev=GetNode(e); GetNode(ref)->Next=GetNode(e); }
};

#if sCONFIG_COMPILER_MSC
template <class Type> class sDList2
{
  sDNode First;
  sDNode Last;

  const sDNode *GetNode(const Type* e) const   { return &(e->Node); } 
  const Type* GetType(const sDNode *n) const   { return (const Type*) (((sU8*)n)-sOFFSETOMP((Type *)0,&Type::Node)); }

  sDNode *GetNode(Type* e)                     { return &(e->Node); } 
  Type* GetType(sDNode *n)                     { return (Type*) (((sU8*)n)-sOFFSETOMP((Type *)0,&Type::Node)); }

public:
  typedef typename sRemovePtrType<Type>::Type ElementType;

  void Clear()                    { First.Next=&Last; First.Prev=0; Last.Next=0; Last.Prev=&First; }
  sDList2()                       { Clear(); }
  int GetCount() const            { int count=0; for (const Type* iter = GetHead(); !IsEnd(iter); iter=GetNext(iter), ++count); return count;}
  sBool IsEmpty() const           { return (First.Next==&Last); }

  const Type* GetNext(const Type* e) const            { return GetType(GetNode(e)->Next); } // { return (Type*) (((sU8*)(GetNode(e)->Next))-MO(Offset)); }
  const Type* GetPrev(const Type* e) const            { return GetType(GetNode(e)->Prev); } // { return (Type*) (((sU8*)(GetNode(e)->Prev))-MO(Offset)); }
  Type* GetNext(Type* e)                              { return GetType(GetNode(e)->Next); } // { return (Type*) (((sU8*)(GetNode(e)->Next))-MO(Offset)); }
  Type* GetPrev(Type* e)                              { return GetType(GetNode(e)->Prev); } // { return (Type*) (((sU8*)(GetNode(e)->Prev))-MO(Offset)); }
  sBool IsFirst(const Type* e) const     { return (GetNode(e)->Prev->Prev==0); }
  sBool IsLast(const Type* e) const      { return (GetNode(e)->Next->Next==0); }
  sBool IsEnd(const Type* e) const       { return (GetNode(e)->Next==0); }

  Type* GetHead()                  { return GetType(First.Next); }
  Type* GetTail()                  { return GetType(Last.Prev); }
  const Type* GetHead() const      { return GetType(First.Next); }
  const Type* GetTail() const      { return GetType(Last.Prev); }
  Type* GetSlow(sInt n)            { Type* e=GetHead(); for(;;) { if(IsEnd(e)) return 0; if(n==0) return e; e=GetNext(e); n--; } }
  sInt GetIndex(Type* e) const     { const Type* h=GetType(First.Next); for(sInt i=0;!IsEnd(h);i++, h=GetNext(h)) if (e==h) return i; return -1; }

  void AddHead(Type* e)            { GetNode(e)->Next = First.Next; GetNode(e)->Prev=&First; First.Next->Prev = GetNode(e); First.Next=GetNode(e); }
  void AddTail(Type* e)            { GetNode(e)->Prev = Last .Prev; GetNode(e)->Next=&Last ; Last .Prev->Next = GetNode(e); Last .Prev=GetNode(e); }
  void Rem(Type* e)                { GetNode(e)->Next->Prev = GetNode(e)->Prev; GetNode(e)->Prev->Next = GetNode(e)->Next; }
  Type* RemHead()                  { if(IsEmpty()) return 0; Type* e=GetHead(); Rem(e); return e; }
  Type* RemTail()                  { if(IsEmpty()) return 0; Type* e=GetTail(); Rem(e); return e; }
  void AddBefore(Type* e,Type* ref) { GetNode(e)->Prev=GetNode(ref)->Prev; GetNode(e)->Next=GetNode(ref); GetNode(ref)->Prev->Next=GetNode(e); GetNode(ref)->Prev=GetNode(e); }
  void AddAfter(Type* e,Type* ref)  { GetNode(e)->Next=GetNode(ref)->Next; GetNode(e)->Prev=GetNode(ref); GetNode(ref)->Next->Prev=GetNode(e); GetNode(ref)->Next=GetNode(e); }
};
#endif


#define sFORALL_LIST(l,p)         for((p)=(l).GetHead();!(l).IsEnd(p);(p)=(l).GetNext(p) )


/*
// dieser code funktioniert nicht mit ps2-gcc.
// bitte keine änderungen an den conbtainern ohne absprache mit DIERK.


// an iterator class that works for both DList and StaticArray in the most efficient way

template<typename ContainerTypeT>
struct sIterator
{
  // you land here if you try to construct an iterator for an unsupported array type
};

template<typename Type>
struct sIterator< sStaticArray<Type> >
{
  typedef sStaticArray<Type> ContainerType;
  ContainerType &Container;
  sInt Index;
  sIterator(ContainerType &container) : Container(container) { Reset(); }  
  void Reset() { Index = -1; }
  Type *GetCurrent() const { sVERIFY((Index >= 0) && (Index < Container.GetCount())); return &Container[Index]; }
  operator Type &() const { return *GetCurrent(); }
  operator Type *() const { return GetCurrent(); }
  Type *operator ->() const { return GetCurrent(); }
  sBool Next() { if ((Index+1) >= Container.GetCount()) return sFALSE; Index++; return sTRUE; }
};

template<typename Type>
struct sIterator< const sStaticArray<Type> >
{
  typedef sStaticArray<Type> ContainerType;
  const ContainerType &Container;
  sInt Index;
  sIterator(const ContainerType &container) : Container(container) { Reset(); }  
  void Reset() { Index = -1; }
  const Type *GetCurrent() const { sVERIFY((Index >= 0) && (Index < Container.GetCount())); return &Container[Index]; }
  operator const Type &() const { return *GetCurrent(); }
  operator const Type *() const { return GetCurrent(); }
  const Type *operator ->() const { return GetCurrent(); }
  sBool Next() { if ((Index+1) >= Container.GetCount()) return sFALSE; Index++; return sTRUE; }
};

template<typename Type, int _Max>
struct sIterator< sStackArray<Type, _Max> > : sIterator< sStaticArray<Type> >
{
  typedef typename sIterator< sStaticArray<Type> >::ContainerType ContainerType;
  sIterator(ContainerType &container) : sIterator< sStaticArray<Type> >(container) {}
};

template<typename Type, int _Max>
struct sIterator< const sStackArray<Type, _Max> > : sIterator< const sStaticArray<Type> >
{
  typedef typename sIterator< const sStaticArray<Type> >::ContainerType ContainerType;
  sIterator(const ContainerType &container) : sIterator< const sStaticArray<Type> >(container) {}
};

template<typename Type, sDNode Type::*Offset>
struct sIterator< sDList<Type, Offset> >
{
  typedef sDList<Type, Offset> ContainerType;
  ContainerType &Container;
  Type *Current;
  sIterator(ContainerType &container) : Container(container) { Reset(); }  
  void Reset() { Current = sNULL; }
  Type *GetCurrent() const { sVERIFY(Current); return Current; }
  operator Type &() const { return *GetCurrent(); }
  operator Type *() const { return GetCurrent(); }
  Type *operator ->() const { return GetCurrent(); }
  sBool Next() { if (!Current) Current = Container.GetHead(); else Current = Container.GetNext(Current); return Container.IsEnd(Current)?sFALSE:sTRUE; }
};

template<typename Type, sDNode Type::*Offset>
struct sIterator< const sDList<Type, Offset> >
{
  typedef sDList<Type, Offset> ContainerType;
  const ContainerType &Container;
  const Type *Current;
  sIterator(const ContainerType &container) : Container(container) { Reset(); }  
  void Reset() { Current = sNULL; }
  const Type *GetCurrent() const { sVERIFY(Current); return Current; }
  operator const Type &() const { return *GetCurrent(); }
  operator const Type *() const { return GetCurrent(); }
  const Type *operator ->() const { return GetCurrent(); }
  sBool Next() { if (!Current) Current = Container.GetHead(); else Current = Container.GetNext(Current); return Container.IsEnd(Current)?sFALSE:sTRUE; }
};
//#define sFORALL_GENERIC(t,l,p) for (sIterator<t> _it(l); _it.Next() && (p = _it);)
*/
/****************************************************************************/
/***                                                                      ***/
/***   Blob                                                               ***/
/***                                                                      ***/
/****************************************************************************/
/***                                                                      ***/
/***   A blob is a refcounted binary block. There are no copy semantics,  ***/
/***   so a copy of a blob will point to the same area of memory,         ***/
/***   providing for efficient handling of binary buffers and their       ***/
/***   ownership issues.                                                  ***/
/***                                                                      ***/
/****************************************************************************/

class sBlob
{
  sU32 *Ptr;
  void AddRef() { if (Ptr) Ptr[1]++; }

public:

  // construction
  sBlob() { Ptr=0; }
  explicit sBlob(sInt bytes) { Ptr=0; Create(bytes); }
  sBlob(const sBlob &blob) { Ptr=blob.Ptr; AddRef(); }

  // c++ stuff
  sBlob & operator = (const sBlob &blob) { Release(); Ptr=blob.Ptr; AddRef(); return *this; }
  ~sBlob() { Release(); }

  // creation/deletion
  void Create(sInt bytes) { Release(); Ptr = new sU32[(bytes+sizeof(sU32)-1)/sizeof(sU32)+2]; Ptr[0]=bytes; Ptr[1]=1; }
  void Release() { if (Ptr && !--Ptr[1]) delete[] Ptr; Ptr=0; }

  // data retrieval
  sBool Valid() const { return Ptr!=0; }
  sInt GetSize() const { sVERIFY(Ptr); if (Ptr) return sInt(Ptr[0]); else return 0; } 
  template <typename T> T * GetPtr() { sVERIFY(Ptr); if (Ptr) return reinterpret_cast<T*>(Ptr+2); else return 0; }
  template <typename T> const T * GetPtr() const { sVERIFY(Ptr); if (Ptr) return reinterpret_cast<const T*>(Ptr+2); else return 0; }
};

// Compression algorithms

sBool sCompES(sU8 *s, sU8 *d, sInt ssize, sInt &dsize, sInt scan);
sBool sDeCompES(sU8 *s, sU8 *d, sInt ssize, sInt &dsize, sBool verify);

/****************************************************************************/
/***                                                                      ***/
/***  Hooks                                                               ***/
/***                                                                      ***/
/****************************************************************************/

// you may call hook->Add(), hook->Rem(), hook->Call() and delete hook
// even when the pointer hook is NULL.
//
// this uses a static array, assuming that there are rarely large amounts of
// hooks required. this is just to be able to put hooks in types, not types2.
// sDList would be more versatile, but sStaticArray is a lot leaner.

class sHooksBase
{
protected:
  sStaticArray<void *> *Hooks;
  void _Add(void *);
  void _Rem(void *);
public:
  sHooksBase(sInt max=16);
  ~sHooksBase();
};

class sHooks : public sHooksBase
{
  typedef void (*hfp)();
public:
  void Add(hfp hook) { _Add((void *)hook); }
  void Rem(hfp hook) { _Rem((void *)hook); }
  void Call() { if(this) { void *h; sFORALL(*Hooks,h) (*hfp(h))(); } }
};

template <typename A>
class sHooks1 : public sHooksBase
{
  typedef void (*hfp)(A a);
public:
  void Add(hfp hook) { _Add(hook); }
  void Rem(hfp hook) { _Rem(hook); }
  void Call(A a) { if(this) { void *h; sFORALL(*Hooks,h) (*hfp(h))(a); } }
};
template <typename A,typename B>
class sHooks2 : public sHooksBase
{
  typedef void (*hfp)(A a,B b);
public:
  void Add(hfp hook) { _Add(hook); }
  void Rem(hfp hook) { _Rem(hook); }
  void Call(A a,B b) { if(this) { void *h; sFORALL(*Hooks,h) (*hfp(h))(a,b); } }
};


/****************************************************************************/
/***                                                                      ***/
/***   memory debug helper                                                ***/
/***                                                                      ***/
/****************************************************************************/

#if !sRELEASE
struct sMemDebugInfo
{
private:
  struct Item
  {
    Item():Ptr(0),File(0),Size(0),Line(0xffff),Next(0xffff) {}
    sPtr Ptr;
    const sChar8 *File;
    sInt Size;
    sU16 Line;
    sU16 Next;

    void Init(sDInt ptr, sInt size, const sChar8 *file, sU16 line, sU16 next);
  };

  Item ItemPool[sMEMSEG_DBGINFOMAX];
  sInt ItemCount;
  sU16 FreeList;
public:

  sU16 Add(sU16 id, sPtr ptr, sInt size, const sChar8 *file, sU16 line);
  sU16 Free(sU16 id, sPtr ptr);
  Item *Get(sU16 id);

  void FlushDump(sU16 id);
  void StatisticDump(sU16 id=0xffff);   // choose valid id for dumping only selected segment
  sInt GetAllocSize(sU16 id);

  sMemDebugInfo();
  void Init();
  void Exit();
};

void sDumpMemUsage();     // dumps allocated memory statistics
#endif // !sRELEASE

/****************************************************************************/
/***                                                                      ***/
/***   Intrinsics MSC (Windows, XBox360                                   ***/
/***                                                                      ***/
/****************************************************************************/

#if sCONFIG_COMPILER_MSC

extern "C"
{
  int __cdecl abs(int);
  void * __cdecl memset( void *dest, int c, sCONFIG_SIZET count );
  void * __cdecl memcpy( void *dest, const void *src, sCONFIG_SIZET count );
  int __cdecl memcmp( const void *buf1, const void *buf2, sCONFIG_SIZET count );
  sCONFIG_SIZET __cdecl strlen( const char *string );
  __int64 __cdecl __emul(int a,int b);
}

#pragma intrinsic (abs,__emul)                                // int intrinsic
#pragma intrinsic (memset,memcpy,memcmp,strlen)               // memory intrinsic
#pragma intrinsic (fabs)                                      // float intrinsic

sINLINE sInt sAbs(sInt i)                                  { return abs(i); }
sINLINE void sSetMem(void *dd,sInt s,sInt c)               { memset(dd,s,c); }
sINLINE void sCopyMem(void *dd,const void *ss,sInt c)      { memcpy(dd,ss,c); }
sINLINE sInt sCmpMem(const void *dd,const void *ss,sInt c) { return (sInt)memcmp(dd,ss,c); }

#if sPLATFORM==sPLAT_XBOX360
sINLINE sInt sMulDiv(sInt a,sInt b,sInt c)      { return sS64(a)*b/c; }
sINLINE sInt sMulShift(sInt a,sInt b)           { return (sS64(a)*b)>>16; }
#else
sINLINE sInt sMulDiv(sInt a,sInt b,sInt c)      { return __emul(a,b)/c; }
sINLINE sInt sMulShift(sInt a,sInt b)           { return __emul(a,b)>>16; }
#endif
sINLINE sInt sDivShift(sInt a,sInt b)           { return (sS64(a)<<16)/b; }
sINLINE sF32 sAbs(sF32 f)                       { return fabs(f); }

#define sHASINTRINSIC_ABSI
#define sHASINTRINSIC_SETMEM
#define sHASINTRINSIC_COPYMEM
#define sHASINTRINSIC_CMPMEM
#define sHASINTRINSIC_MULDIV
#define sHASINTRINSIC_MULSHIFT
#define sHASINTRINSIC_DIVSHIFT
#define sHASINTRINSIC_ABSF


#if !sCONFIG_SYSTEM_XBOX360

#pragma intrinsic (atan,atan2,cos,exp,log,log10,sin,sqrt,tan)

sINLINE sF32 sATan(sF32 f)                      { return (sF32)atan(f); }
sINLINE sF32 sATan2(sF32 over,sF32 under)       { return (sF32)atan2(over,under); }
sINLINE sF32 sCos(sF32 f)                       { return (sF32)cos(f); }
sINLINE sF32 sExp(sF32 f)                       { return (sF32)exp(f); }
sINLINE sF32 sLog(sF32 f)                       { return (sF32)log(f); }
sINLINE sF32 sLog10(sF32 f)                     { return (sF32)log10(f); }
sINLINE sF32 sSin(sF32 f)                       { return (sF32)sin(f); }
sINLINE sF32 sSqrt(sF32 f)                      { return (sF32)sqrt(f); }
sINLINE sF32 sTan(sF32 f)                       { return (sF32)tan(f); }

sINLINE sF32 sFATan(sF32 f)                      { return (sF32)atan(f); }
sINLINE sF32 sFATan2(sF32 over,sF32 under)       { return (sF32)atan2(over,under); }
sINLINE sF32 sFCos(sF32 f)                       { return (sF32)cos(f); }
sINLINE sF32 sFExp(sF32 f)                       { return (sF32)exp(f); }
sINLINE sF32 sFLog(sF32 f)                       { return (sF32)log(f); }
sINLINE sF32 sFLog10(sF32 f)                     { return (sF32)log10(f); }
sINLINE sF32 sFSin(sF32 f)                       { return (sF32)sin(f); }
sINLINE sF32 sFSqrt(sF32 f)                      { return (sF32)sqrt(f); }
sINLINE sF32 sFTan(sF32 f)                       { return (sF32)tan(f); }


#define sHASINTRINSIC_ATAN
#define sHASINTRINSIC_ATAN2
#define sHASINTRINSIC_COS
#define sHASINTRINSIC_EXP
#define sHASINTRINSIC_LOG
#define sHASINTRINSIC_LOG10
#define sHASINTRINSIC_SIN
#define sHASINTRINSIC_SQRT
#define sHASINTRINSIC_TAN

#define sHASINTRINSIC_FATAN
#define sHASINTRINSIC_FATAN2
#define sHASINTRINSIC_FCOS
#define sHASINTRINSIC_FEXP
#define sHASINTRINSIC_FLOG
#define sHASINTRINSIC_FLOG10
#define sHASINTRINSIC_FSIN
#define sHASINTRINSIC_FSQRT
#define sHASINTRINSIC_FTAN

#endif

#endif    // compiler

/****************************************************************************/
/***                                                                      ***/
/***   Intrinsics GCC (Windows, PS2, PS3)                                 ***/
/***                                                                      ***/
/****************************************************************************/

#if sCONFIG_COMPILER_GCC              // no intrinsics: USE __builtin__

sINLINE sInt sAbs(sInt i)                                  { return __builtin_abs(i); }
sINLINE void sSetMem(void *dd,sInt s,sInt c)               { __builtin_memset(dd,s,c); }
sINLINE void sCopyMem(void *dd,const void *ss,sInt c)      { __builtin_memcpy(dd,ss,c); }
sINLINE sInt sCmpMem(const void *aa,const void *bb,sInt c) { return __builtin_memcmp(aa,bb,c); }
sINLINE sInt sMulDiv(sInt a,sInt b,sInt c)      { return sS64(a)*b/c; }
sINLINE sInt sMulShift(sInt a,sInt b)           { return (sS64(a)*b)>>16; }
sINLINE sInt sDivShift(sInt a,sInt b)           { return (sS64(a)<<16)/b; }
sINLINE sF32 sAbs(sF32 f)                       { return __builtin_fabsf(f); }

#define sHASINTRINSIC_ABSI
#define sHASINTRINSIC_SETMEM
#define sHASINTRINSIC_COPYMEM
#define sHASINTRINSIC_CMPMEM
#define sHASINTRINSIC_MULDIV
#define sHASINTRINSIC_MULSHIFT
#define sHASINTRINSIC_DIVSHIFT
#define sHASINTRINSIC_ABSF

sINLINE sF32 sCos(sF32 f)                       { return __builtin_cosf(f); }
sINLINE sF32 sSin(sF32 f)                       { return __builtin_sinf(f); }
sINLINE sF32 sSqrt(sF32 f)                      { return __builtin_sqrtf(f); }
sINLINE sF32 sFSqrt(sF32 f)                     { return __builtin_sqrtf(f); }
sINLINE sF32 sRSqrt(sF32 f)                     { return 1.0f/__builtin_sqrtf(f); }
sINLINE sF32 sFRSqrt(sF32 f)                    { return 1.0f/__builtin_sqrtf(f); }

#define sHASINTRINSIC_COS
#define sHASINTRINSIC_SIN
#define sHASINTRINSIC_SQRT
#define sHASINTRINSIC_FSQRT
#define sHASINTRINSIC_RSQRT
#define sHASINTRINSIC_FRSQRT

#if !sCONFIG_SYSTEM_PS2
sINLINE sF32 sLog(sF32 f)                       { return __builtin_logf(f); }
sINLINE sF32 sExp(sF32 f)                       { return __builtin_expf(f); }

sINLINE sF32 sFLog(sF32 f)                       { return __builtin_logf(f); }
sINLINE sF32 sFExp(sF32 f)                       { return __builtin_expf(f); }

#define sHASINTRINSIC_LOG
#define sHASINTRINSIC_EXP

#define sHASINTRINSIC_FLOG
#define sHASINTRINSIC_FEXP
#endif

#if sCONFIG_SYSTEM_PS2
#define sHASINTRINSIC_FSIN        // have special implementation
#define sHASINTRINSIC_FCOS
#define sHASINTRINSIC_FATAN2
#define sHASINTRINSIC_FLOG2
#endif

#if sCONFIG_SYSTEM_PS3
sINLINE sF32 sATan(sF32 f)                      { return __builtin_atanf(f); }
sINLINE sF32 sATan2(sF32 a,sF32 b)              { return __builtin_atan2f(a,b); }
sINLINE sF32 sLog10(sF32 f)                     { return __builtin_log10f(f); }
sINLINE sF32 sTan(sF32 f)                       { return __builtin_tanf(f); }
sINLINE sF32 sPow(sF32 a,sF32 b)                { return __builtin_powf(a,b); }
sINLINE sF32 sMod(sF32 a,sF32 b)                { return __builtin_fmodf(a,b); }
sINLINE sF32 sACos(sF32 f)                      { return __builtin_acosf(f); }
sINLINE sF32 sASin(sF32 f)                      { return __builtin_asinf(f); }
#define sHASINTRINSIC_ATAN
#define sHASINTRINSIC_ATAN2
#define sHASINTRINSIC_LOG10
#define sHASINTRINSIC_TAN
#define sHASINTRINSIC_POW
#define sHASINTRINSIC_MOD
#define sHASINTRINSIC_ACOS
#define sHASINTRINSIC_ASIN

sINLINE sF32 sFCos(sF32 f)                      { return __builtin_cosf(f); }
sINLINE sF32 sFSin(sF32 f)                      { return __builtin_sinf(f); }

#define sHASINTRINSIC_FCOS
#define sHASINTRINSIC_FSIN

#endif

#endif // compiler

/****************************************************************************/
/***                                                                      ***/
/***   Intrinsics Codewarrier (Wii)                                       ***/
/***                                                                      ***/
/****************************************************************************/

#if sCONFIG_COMPILER_CWCC 

extern "C" int memset(void *, int, int);
extern "C" int memcmp(const void *, const void*, int c);
extern "C" int memcpy(void *, const void*, int c);
sINLINE sInt sAbs(sInt i)                                  { return __abs(i); }
sINLINE void sCopyMem(void *dd,const void *ss,sInt c)      { memcpy(dd,ss,c); } // TODO: use intrinsic
sINLINE void sSetMem(void *dd,sInt s,sInt c)               { memset(dd,s,c); }
sINLINE sInt sCmpMem(const void *aa,const void *bb,sInt c) { return memcmp(aa,bb,c); }

#define sHASINTRINSIC_ABSI
#define sHASINTRINSIC_COPYMEM
#define sHASINTRINSIC_SETMEM
#define sHASINTRINSIC_CMPMEM

#if sCONFIG_SYSTEM_WII
#define sHASINTRINSIC_FSIN        // have special implementation
#define sHASINTRINSIC_FCOS
#define sHASINTRINSIC_FATAN2
#define sHASINTRINSIC_FLOG2
#define sHASINTRINSIC_RSQRT
#endif

#endif // compiler

/****************************************************************************/
/****************************************************************************/

// HEADER_ALTONA_BASE_TYPES
#endif

