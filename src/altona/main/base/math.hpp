/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/

#ifndef HEADER_ALTONA_BASE_MATH
#define HEADER_ALTONA_BASE_MATH

#ifndef __GNUC__
#pragma once
#endif

#include "base/types.hpp"
#include "base/serialize.hpp"

/****************************************************************************/
/****************************************************************************/
/***                                                                      ***/
/***   3d graphics primitives                                             ***/
/***                                                                      ***/
/****************************************************************************/
/****************************************************************************/

class sVector30;
class sVector31;
class sVector4;
class sMatrix34;
class sMatrix44;
class sAABBox;
class sRay;

const sF32 sEPSILON = 0.000000001f;

/****************************************************************************/
/***                                                                      ***/
/***   Vector                                                             ***/
/***                                                                      ***/
/****************************************************************************/

// may be overloading [] is good? would also work with sMatrix44 naturally.

class sVector2
{
public:
  enum { Dimensions = 2 };

  sF32 x,y;

  sVector2()                                       { x=y=0.0f; }
  sVector2(sF32 xx,sF32 yy)                        { x=xx;y=yy; }
  sVector2(const sFRect &rect)                    { x = rect.SizeX(); y = rect.SizeY(); }
  void Init(sF32 xx=0,sF32 yy=0)                  { x=xx;y=yy; }
  sVector2(const sVector2 &a)                     { x=a.x; y=a.y; }
  sVector2& operator= (const sVector2 &a)         { x=a.x; y=a.y; return *this; }

  void Fade(sF32 f,const sVector2 &a,const sVector2 &b) { x=a.x+(b.x-a.x)*f; y=a.y+(b.y-a.y)*f;; }
  void Neg()                                        { x = -x; y = -y; }

  void UnitFast()                                   { sF32 e=sRSqrt(x*x+y*y); x*=e; y*=e; }

  sF32 &operator[](sInt i)                          { return (&x)[i]; }
  const sF32 &operator[](sInt i) const              { return (&x)[i]; }

  sF32 Length() const                               { return (sF32)sFSqrt(x*x+y*y); }
  sF32 LengthSq() const                             { return x*x+y*y; }

  operator sFRect() const                           { return sFRect(x,y); }
#if sPLATFORM==sPLAT_PS2
} __attribute__ ((aligned(8)));
#else
};
#endif

class sVector31
{
public:
  enum { Dimensions = 3 };

  sF32 x,y,z;

  sVector31()                                       { x=y=z=0; }
  explicit sVector31(sF32 xyz)                      { x=y=z=xyz; }
  sVector31(sF32 xx,sF32 yy,sF32 zz)                { x=xx;y=yy;z=zz; }
  void Init(sF32 xx,sF32 yy,sF32 zz)                { x=xx;y=yy;z=zz; }
  sVector31(const sVector31 &a)                     { x=a.x; y=a.y; z=a.z; }
  explicit sVector31(const class sVector30 &a);
  explicit sVector31(const class sVector4 &a);
  sVector31& operator= (const sVector31 &a)         { x=a.x; y=a.y; z=a.z; return *this; }

  void InitColor(sU32 col);
  sU32 GetColor();
  void Fade(sF32 f,const sVector31 &a,const sVector31 &b) { x=a.x+(b.x-a.x)*f; y=a.y+(b.y-a.y)*f; z=a.z+(b.z-a.z)*f; }
  void Neg()                                        { x = -x; y = -y, z = -z; }

  sF32 &operator[](sInt i)                          { return (&x)[i]; }
  const sF32 &operator[](sInt i) const              { return (&x)[i]; }

};

class sVector30
{
public:
  enum { Dimensions = 3 };

  sF32 x,y,z;

  sVector30()                                       { x=y=z=0; }
  explicit sVector30(sF32 xyz)                      { x=y=z=xyz; }
  sVector30(sF32 xx,sF32 yy,sF32 zz)                { x=xx;y=yy;z=zz; }
  void Init(sF32 xx,sF32 yy,sF32 zz)                { x=xx;y=yy;z=zz; }
  sVector30(const sVector30 &a)                     { x=a.x; y=a.y; z=a.z; }
  sVector30& operator= (const sVector30 &a)         { x=a.x; y=a.y; z=a.z; return *this; }
  explicit sVector30(const sVector31 &a)            { x=a.x; y=a.y; z=a.z; }
  explicit sVector30(const sVector4 &a);

  void InitColor(sU32 col);
  sU32 GetColor();
  void Fade(sF32 f,const sVector30 &a,const sVector30 &b) { x=a.x+(b.x-a.x)*f; y=a.y+(b.y-a.y)*f; z=a.z+(b.z-a.z)*f; }
  sF32 Length() const                               { return (sF32)sFSqrt(x*x+y*y+z*z); }
  sF32 LengthSq() const                             { return x*x+y*y+z*z; }
  sF32 Angle(const sVector30 &a);

  void Cross(const sVector30 &a,const sVector30 &b);
  void Rotate(const sMatrix34 &m,const sVector30 &v);

  void Unit();
  void UnitFast()                                   { sF32 e=sRSqrt(x*x+y*y+z*z); x*=e; y*=e; z*=e; } 
  void Neg()                                        { x = -x; y = -y, z = -z; }
  sInt MaxAxisAbs() const                           { sF32 ax = sFAbs(x); sF32 ay = sFAbs(y); sF32 az = sFAbs(z); return (ax>ay) ? ((ax>az) ? 0 : 2) : ((ay>az) ? 1 : 2); }
  
  sF32 &operator[](sInt i)                          { return (&x)[i]; }
  const sF32 &operator[](sInt i) const              { return (&x)[i]; }
};

class sVector4
{
public:
  enum { Dimensions = 4 };

  sF32 x,y,z,w;

  sVector4()                                        { x=y=z=w=0; }
  sVector4(sF32 xx,sF32 yy,sF32 zz,sF32 ww)         { x=xx; y=yy; z=zz; w=ww; }
  void Init(sF32 xx,sF32 yy,sF32 zz,sF32 ww)        { x=xx; y=yy; z=zz; w=ww; }
  sVector4(const sVector30 &a)                      { x=a.x; y=a.y; z=a.z; w=0; }
  sVector4(const sVector31 &a)                      { x=a.x; y=a.y; z=a.z; w=1; }
  sVector4(const sVector4 &a)                       { x=a.x; y=a.y; z=a.z; w=a.w; }
  sVector4& operator= (const sVector30 &a)          { x=a.x; y=a.y; z=a.z; w=0; return *this; }
  sVector4& operator= (const sVector31 &a)          { x=a.x; y=a.y; z=a.z; w=1; return *this; }
  sVector4& operator= (const sVector4 &a)           { x=a.x; y=a.y; z=a.z; w=a.w; return *this; }

  void InitColor(sU32 col);
  sU32 GetColor();

  void InitMRGB8(sU32 col);
  sU32 GetMRGB8()const;
  void InitMRGB16(sU64 col);
  sU64 GetMRGB16()const;

  void InitPlane(const sVector31 &pos,const sVector30 &norm);
  void InitPlane(const sVector31 &p0,const sVector31 &p1,const sVector31 &p2);
  void Neg0()                                        { x = -x; y = -y, z = -z; }
  void Neg1()                                        { x = -x; y = -y, z = -z; w=-w;}
  void Unit4();

  sF32 &operator[](sInt i)                          { return (&x)[i]; }
  const sF32 &operator[](sInt i) const              { return (&x)[i]; }

  sBool operator==(const sVector4 &v)const;
};

inline sVector2 operator+ (const sVector2 &a,const sVector2 &b)     { sVector2 r(a.x+b.x,a.y+b.y); return r; }
inline sVector2 operator- (const sVector2 &a,const sVector2 &b)     { sVector2 r(a.x-b.x,a.y-b.y); return r; }
inline sVector2 operator* (const sVector2 &a, sF32 s)               { sVector2 r(s*a.x,s*a.y); return r; }
inline sVector2 operator* (sF32 s, const sVector2 &a)               { sVector2 r(s*a.x,s*a.y); return r; }
inline sVector2 operator/ (const sVector2 &a, sF32 s)               { sF32 inv = 1.0f/s; sVector2 r(inv*a.x,inv*a.y); return r; }
inline sF32 operator^ (const sVector2 &a,const sVector2 &b)         { return a.x*b.x+a.y*b.y; }
inline sVector2 &operator+= (sVector2 &a,const sVector2 &b)         { a = a+b; return a; }
inline sVector2 &operator-= (sVector2 &a,const sVector2 &b)         { a = a-b; return a; }
inline sVector2 &operator*= (sVector2 &a,sF32 s)                    { a = a*s; return a; }
inline sVector2 &operator/= (sVector2 &a,sF32 s)                    { a = a/s; return a; }

inline sVector30 operator+ (const sVector30 &a,const sVector30 &b)   { sVector30 r(a.x+b.x,a.y+b.y,a.z+b.z); return r; }
inline sVector31 operator+ (const sVector31 &a,const sVector30 &b)   { sVector31 r(a.x+b.x,a.y+b.y,a.z+b.z); return r; }
inline sVector31 operator+ (const sVector30 &a,const sVector31 &b)   { sVector31 r(a.x+b.x,a.y+b.y,a.z+b.z); return r; }
inline sVector4  operator+ (const sVector4  &a,const sVector4  &b)   { sVector4  r(a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w); return r; }

inline sVector30 operator- (const sVector30 &a,const sVector30 &b)   { sVector30 r(a.x-b.x,a.y-b.y,a.z-b.z); return r; }
inline sVector31 operator- (const sVector31 &a,const sVector30 &b)   { sVector31 r(a.x-b.x,a.y-b.y,a.z-b.z); return r; }
inline sVector30 operator- (const sVector31 &a,const sVector31 &b)   { sVector30 r(a.x-b.x,a.y-b.y,a.z-b.z); return r; }
inline sVector4  operator- (const sVector4  &a,const sVector4  &b)   { sVector4  r(a.x-b.x,a.y-b.y,a.z-b.z,a.w-b.w); return r; }

inline sVector30 operator- (const sVector30 &a)                      { sVector30 r(-a.x,-a.y,-a.z); return r; }
inline sVector31 operator- (const sVector31 &a)                      { sVector31 r(-a.x,-a.y,-a.z); return r; }

inline sVector30 operator* (const sVector30 &a,sF32 s)               { sVector30 r(a.x*s,a.y*s,a.z*s); return r; }

inline sVector4  operator* (const sVector4  &a,sF32 s)               { sVector4  r(a.x*s,a.y*s,a.z*s,a.w*s); return r; }

inline sVector30 operator* (sF32 s, const sVector30 &a)              { sVector30 r(a.x*s,a.y*s,a.z*s); return r; }
inline sVector4  operator* (sF32 s, const sVector4  &a)              { sVector4  r(a.x*s,a.y*s,a.z*s,a.w*s); return r; }
inline sVector30 operator* (const sVector30 &a, const sVector30 &b)  { sVector30 r(a.x*b.x,a.y*b.y,a.z*b.z); return r; }
inline sVector30 operator* (const sVector30 &a, const sVector31 &b)  { sVector30 r(a.x*b.x,a.y*b.y,a.z*b.z); return r; }
inline sVector30 operator* (const sVector31 &a, const sVector30 &b)  { sVector30 r(a.x*b.x,a.y*b.y,a.z*b.z); return r; }
inline sVector31 operator* (const sVector31 &a, const sVector31 &b)  { sVector31 r(a.x*b.x,a.y*b.y,a.z*b.z); return r; }
inline sVector4  operator* (const sVector4  &a, const sVector4  &b)  { sVector4  r(a.x*b.x,a.y*b.y,a.z*b.z,a.w*b.w); return r; }

inline sVector30 operator/ (const sVector30 &a,sF32 s)               { s = 1.0f/s; sVector30 r(a.x*s,a.y*s,a.z*s); return r; }
inline sVector4  operator/ (const sVector4  &a,sF32 s)               { s = 1.0f/s; sVector4  r(a.x*s,a.y*s,a.z*s,a.w*s); return r; }

inline sVector30 operator% (const sVector30 &a, const sVector30 &b)  { sVector30 r; r.Cross(a,b); return r;}

inline sF32 operator^ (const sVector30 &a,const sVector30 &b)   { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline sF32 operator^ (const sVector30 &a,const sVector31 &b)   { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline sF32 operator^ (const sVector31 &a,const sVector30 &b)   { return a.x*b.x + a.y*b.y + a.z*b.z; }
//inline sF32 operator^ (const sVector31 &a,const sVector31 &b)   { return a.x*b.x + a.y*b.y + a.z*b.z + 1; }
inline sF32 operator^ (const sVector30 &a,const sVector4 &b)   { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline sF32 operator^ (const sVector31 &a,const sVector4 &b)   { return a.x*b.x + a.y*b.y + a.z*b.z + b.w; }
inline sF32 operator^ (const sVector4 &a,const sVector30 &b)   { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline sF32 operator^ (const sVector4 &a,const sVector31 &b)   { return a.x*b.x + a.y*b.y + a.z*b.z + a.w; }
inline sF32 operator^ (const sVector4 &a,const sVector4 &b)    { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }

inline sVector30& operator+= (sVector30 &a,const sVector30 &b)  { a = a + b; return a; }
inline sVector31& operator+= (sVector31 &a,const sVector30 &b)  { a = a + b; return a; }
inline sVector4&  operator+= (sVector4  &a,const sVector4  &b)  { a = a + b; return a; }
inline sVector30& operator-= (sVector30 &a,const sVector30 &b)  { a = a - b; return a; }
inline sVector31& operator-= (sVector31 &a,const sVector30 &b)  { a = a - b; return a; }
inline sVector4&  operator-= (sVector4  &a,const sVector4  &b)  { a = a - b; return a; }
inline sVector30& operator*= (sVector30 &a,sF32 s)              { a = a * s; return a; }
inline sVector4&  operator*= (sVector4  &a,sF32 s)              { a = a * s; return a; }
inline sVector4&  operator*= (sVector4  &a,const sVector4  &b)  { a = a * b; return a; }
inline sVector30& operator/= (sVector30 &a,sF32 s)              { a = a / s; return a; }
inline sVector4&  operator/= (sVector4  &a,sF32 s)              { a = a / s; return a; }

inline sF32 sOBSOLETE dot(const sVector30 &a,const sVector30 &b)          { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline sF32 sOBSOLETE dot(const sVector30 &a,const sVector31 &b)          { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline sF32 sOBSOLETE dot(const sVector31 &a,const sVector30 &b)          { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline sF32 sOBSOLETE dot(const sVector4 &a,const sVector31 &b)           { return a.x*b.x + a.y*b.y + a.z*b.z + a.w; }
inline sF32 sOBSOLETE dot(const sVector31 &a,const sVector4 &b)           { return a.x*b.x + a.y*b.y + a.z*b.z + b.w; }
inline sF32 sOBSOLETE dot(const sVector4 &a,const sVector30 &b)           { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline sF32 sOBSOLETE dot(const sVector30 &a,const sVector4 &b)           { return a.x*b.x + a.y*b.y + a.z*b.z; }

inline sVector30::sVector30(const class sVector4 &a) { x=a.x; y=a.y; z=a.z; }
inline sVector31::sVector31(const class sVector30 &a) { x=a.x; y=a.y; z=a.z; }
inline sVector31::sVector31(const class sVector4 &a) { x=a.x; y=a.y; z=a.z; }

sFormatStringBuffer& operator% (sFormatStringBuffer &,const sVector30 &);
sFormatStringBuffer& operator% (sFormatStringBuffer &,const sVector31 &);
sFormatStringBuffer& operator% (sFormatStringBuffer &,const sVector4 &);

inline sVector30 sAverage(const sVector30 &a,const sVector30 &b) { sVector30 r; r.x=(a.x+b.x)*0.5f; r.y=(a.y+b.y)*0.5f; r.z=(a.z+b.z)*0.5f; return r; }
inline sVector31 sAverage(const sVector31 &a,const sVector31 &b) { sVector31 r; r.x=(a.x+b.x)*0.5f; r.y=(a.y+b.y)*0.5f; r.z=(a.z+b.z)*0.5f; return r; }

inline sWriter& operator| (sWriter &s,const sVector2 &a)  { s | a.x | a.y;              return s; }
inline sReader& operator| (sReader &s,      sVector2 &a)  { s | a.x | a.y;              return s; }
inline sWriter& operator| (sWriter &s,const sVector30 &a) { s | a.x | a.y | a.z;        return s; }
inline sReader& operator| (sReader &s,      sVector30 &a) { s | a.x | a.y | a.z;        return s; }
inline sWriter& operator| (sWriter &s,const sVector31 &a) { s | a.x | a.y | a.z;        return s; }
inline sReader& operator| (sReader &s,      sVector31 &a) { s | a.x | a.y | a.z;        return s; }
inline sWriter& operator| (sWriter &s,const sVector4  &a) { s | a.x | a.y | a.z | a.w;  return s; }
inline sReader& operator| (sReader &s,      sVector4  &a) { s | a.x | a.y | a.z | a.w;  return s; }

/****************************************************************************/
/***                                                                      ***/
/***   Matrix                                                             ***/
/***                                                                      ***/
/****************************************************************************/

class sMatrix34
{
public:
  sVector30 i;
  sVector30 j;
  sVector30 k;
  sVector31 l;

  sMatrix34() : i(1,0,0),j(0,1,0),k(0,0,1),l(0,0,0) {}
  sMatrix34(const sVector30 &_i, const sVector30 &_j, const sVector30 &_k, const sVector31 &_l) : i(_i), j(_j), k(_k), l(_l) {}
  explicit sMatrix34(const class sMatrix44 &m);
  explicit sMatrix34(const class sMatrix34CM &m);
  sMatrix34 &operator=(const class sMatrix34CM &m);

  // initialize matrix

  void Init();
  void sOBSOLETE Euler(sF32 a,sF32 b,sF32 c);
  void EulerXYZ(sF32 x,sF32 y,sF32 z);
  void EulerXYZ(const sVector30 &xyz) { EulerXYZ(xyz.x, xyz.y, xyz.z); }
  void FindEulerXYZ(sF32 &x,sF32 &y,sF32 &z) const;
  void CubeFace(sInt face);         // set up camera matrix for rendering to cube face
  void Look(const sVector30 &v);
  void Look(const sVector30 &dir, const sVector30 &up);
  void LookAt(const sVector31 &dest, const sVector31 &pos);
  void LookAt(const sVector31 &dest, const sVector31 &pos, const sVector30 &up);
  void Cross(const sVector30 &v);

  // manipulate matrix

  void RotateAxis(const sVector30 &v,sF32 angle); // this should initialize L. check summergames.
  void Trans3();
  void TransR();
  void InvertOrthogonal();  // invert a matrix that is Orthogonal but not Orthonormal
  sF32 Determinant3x3() const;    // calculate determinant of 3x3 part of matrix

  sBool Invert3();
  sBool Invert34();
  void Orthonormalize();    // Gram-Schmidt orthonormalization, doesn't touch l
  void Init(const class sQuaternion &);
  void Scale(sF32 x, sF32 y, sF32 z);
};

// column major stored sMatrix34 matrix for shaders
// use only for storage! don't define operations, use sMatrix34 instead
class sMatrix34CM
{
public:
  sVector4 x,y,z;  // was a,b,c. each sVector4 represents the x or y or z of i and j and k and l

  sMatrix34CM();
  explicit sMatrix34CM(const sMatrix34 &m);

  sMatrix34CM &operator=(const sMatrix34 &m);
  void Ident();

  void ConvertTo(sMatrix34 &m)const;
};

class sMatrix44
{
public:
  sVector4 i;
  sVector4 j;
  sVector4 k;
  sVector4 l;

  void Init();
  sMatrix44() : i(1,0,0,0),j(0,1,0,0),k(0,0,1,0),l(0,0,0,1) {}
  sMatrix44(const sMatrix34 &m) : i(m.i),j(m.j),k(m.k),l(m.l) {}
  sMatrix44(const sVector4 &_i, const sVector4 &_j, const sVector4 &_k, const sVector4 &_l) : i(_i), j(_j), k(_k), l(_l) {}
  void Trans4(const sMatrix44 &mat);
  void Trans4();
  void Invert(const sMatrix44 &mat);
  void Invert()                                         { sMatrix44 mat(*this); Invert(mat); }
  sF32 Determinant() const;

  sBool operator==(const sMatrix44 &mat) const;
  sBool operator!=(const sMatrix44 &mat) const          { return !(*this == mat); }

  void Scale(sF32 x, sF32 y, sF32 z);
  void Orthonormalize();    // Gram-Schmidt orthonormalization

  void Perspective(sF32 left, sF32 right, sF32 top, sF32 bottom, sF32 front, sF32 back);

  sVector4 &operator[](sInt a)                          { return (&i)[a]; }
  const sVector4 &operator[](sInt a) const              { return (&i)[a]; }
};

inline sMatrix34::sMatrix34(const sMatrix44 &m) : i(sVector30(m.i)),j(sVector30(m.j)),k(sVector30(m.k)),l(sVector31(m.l)) {}

sFormatStringBuffer& operator% (sFormatStringBuffer &,const sMatrix34 &);
sFormatStringBuffer& operator% (sFormatStringBuffer &,const sMatrix44 &);
sFormatStringBuffer& operator% (sFormatStringBuffer &,const sMatrix34CM &);

/****************************************************************************/

sMatrix44 operator* (const sMatrix44 &a,const sMatrix44 &b);
sMatrix34 operator* (const sMatrix34 &a,const sMatrix34 &b);

sVector30 operator* (const sVector30 &v,const sMatrix34 &m);
sVector31 operator* (const sVector31 &v,const sMatrix34 &m);
sVector4  operator* (const sVector4  &v,const sMatrix34 &m);

sVector4  operator* (const sVector30 &v,const sMatrix44 &m);
sVector4  operator* (const sVector31 &v,const sMatrix44 &m);
sVector4  operator* (const sVector4  &v,const sMatrix44 &m);

sMatrix34 operator* (const sMatrix34 &a,sF32 b);
sMatrix44 operator* (const sMatrix44 &a,sF32 b);

sMatrix34 operator+ (const sMatrix34 &a, const sMatrix34 &b);
sMatrix44 operator+ (const sMatrix44 &a, const sMatrix44 &b);
sMatrix34 operator- (const sMatrix34 &a, const sMatrix34 &b);
sMatrix44 operator- (const sMatrix44 &a, const sMatrix44 &b);

inline sMatrix44& operator*= (sMatrix44 &a,const sMatrix44 &b) { a = a*b; return a; }
inline sMatrix34& operator*= (sMatrix34 &a,const sMatrix34 &b) { a = a*b; return a; }

inline sVector30& operator*= (sVector30 &v,const sMatrix34 &m) { v = v*m; return v; }
inline sVector31& operator*= (sVector31 &v,const sMatrix34 &m) { v = v*m; return v; }
inline sVector4&  operator*= (sVector4  &v,const sMatrix34 &m) { v = v*m; return v; }

inline sVector4&  operator*= (sVector4  &v,const sMatrix44 &m) { v = v*m; return v; }

sMatrix34& operator*= (sMatrix34 &a, sF32 b);
sMatrix44& operator*= (sMatrix44 &a, sF32 b);

sMatrix34& operator+= (sMatrix34 &a, const sMatrix34 &b);
sMatrix44& operator+= (sMatrix44 &a, const sMatrix44 &b);
sMatrix34& operator-= (sMatrix34 &a, const sMatrix34 &b);
sMatrix44& operator-= (sMatrix44 &a, const sMatrix44 &b);

inline sWriter& operator| (sWriter &s,const sMatrix34 &a) { s | a.i | a.j | a.k | a.l;  return s; }
inline sReader& operator| (sReader &s,      sMatrix34 &a) { s | a.i | a.j | a.k | a.l;  return s; }
inline sWriter& operator| (sWriter &s,const sMatrix44 &a) { s | a.i | a.j | a.k | a.l;  return s; }
inline sReader& operator| (sReader &s,      sMatrix44 &a) { s | a.i | a.j | a.k | a.l;  return s; }

/****************************************************************************/
/***                                                                      ***/
/***   Quaternion                                                         ***/
/***                                                                      ***/
/****************************************************************************/

class sQuaternion
{
public:
  sF32 r,i,j,k;

  sQuaternion()                              { i=j=k=0; r=1; }
  sQuaternion(sF32 R,sF32 I,sF32 J,sF32 K)   { r=R;i=I;j=J;k=K; }
  void Init(sF32 R,sF32 I,sF32 J,sF32 K)     { r=R;i=I;j=J;k=K; }

  void Unit();

  void Lerp(sF32 fade,const sQuaternion &a,const sQuaternion &b);
  sOBSOLETE void Slerp(sF32 fade,const sQuaternion &a,const sQuaternion &b);
  void FastSlerp(sF32 fade,const sQuaternion &a,const sQuaternion &b); // spline approximated slerp
  inline void Fade(sF32 fade,const sQuaternion &a,const sQuaternion &b) { Lerp(fade,a,b); Unit(); } // also called "nlerp"

  void Init(const sMatrix34 &);
  void Init(const sVector30 &axis, sF32 angle); // axis must be unit length

  void Euler(sF32 h, sF32 p, sF32 b);
  inline void Euler(const sVector30 &v) { Euler(v.x,v.y,v.z); }
  sVector30 GetEuler() const;
  void GetAxisAngle(sVector30 &axis, sF32 &angle);

  void MakeRotation(const sVector30 &v1, const sVector30 &v2); // creates a minimal rotation that
                                                               // maps v1 to v2 
  void Invert();
  void Invert(const sQuaternion &from);
  void InvertNorm();      // fast invert for unit quaternions
};

sQuaternion operator* (const sQuaternion &a,const sQuaternion &b);
sVector30 operator* (const sVector30 &a,const sQuaternion &b);
sVector31 operator* (const sVector31 &a,const sQuaternion &b);

inline sQuaternion& operator*= (sQuaternion &a,const sQuaternion &b) { a=a*b; return a; }
inline sVector30& operator*= (sVector30 &a,const sQuaternion &b) { a=a*b; return a; }
inline sVector31& operator*= (sVector31 &a,const sQuaternion &b) { a=a*b; return a; }

inline sQuaternion operator~ (const sQuaternion &q) { sQuaternion res(q); res.Invert(); return res; }

inline sF32 dot(const sQuaternion &a,const sQuaternion &b)          { return a.r*b.r + a.i*b.i + a.j*b.j + a.k*b.k; }

sFormatStringBuffer& operator% (sFormatStringBuffer &,const sQuaternion &);

inline sWriter& operator| (sWriter &s,const sQuaternion &a) { s | a.r | a.i | a.j | a.k;  return s; }
inline sReader& operator| (sReader &s,      sQuaternion &a) { s | a.r | a.i | a.j | a.k;  return s; }

/****************************************************************************/
/***                                                                      ***/
/***   Axis Aligned Bounding Box                                          ***/
/***                                                                      ***/
/****************************************************************************/

class sAABBox                     // axis aligned bounding box
{
public:

  sVector31 Min;
  sVector31 Max;

  sAABBox();
  void Clear();

  sBool HitPoint(const sVector31 &p) const;
  sBool HitRay(sF32 &dist,const sRay &ray) const;
  void Add(const sAABBox &box);
  void Add(const sVector31 &p);
  sVector31 Center() const { sVector31 p((Min.x+Max.x)/2,(Min.y+Max.y)/2,(Min.z+Max.z)/2); return p; }
  void MakePoints(sVector31 *points) const;
  sF32 CalcArea()const;
  sF32 CalcVolume()const;

  sAABBox &operator*=(const sMatrix34 &m);
};

inline sAABBox operator* (const sAABBox &bbox,const sMatrix34 &m)  { sAABBox r=bbox; r*=m; return r; }

inline sWriter& operator| (sWriter &s,const sAABBox &a) { s | a.Min | a.Max;  return s; }
inline sReader& operator| (sReader &s,      sAABBox &a) { s | a.Min | a.Max;  return s; }

/****************************************************************************/
/***                                                                      ***/
/***   Ray with Start and direction                                       ***/
/***                                                                      ***/
/****************************************************************************/

class sRay
{
public:
  sVector31 Start;
  sVector30 Dir;
  sRay() {}
  sRay(const sVector31 &s,const sVector30 &d) : Start(s),Dir(d) {}

  sBool HitTriangle(sF32 &dist,const sVector31 &p0,const sVector31 &p1,const sVector31 &p2) const;
  sBool HitPlane(sVector31 &intersect,const sVector4 &plane) const;
  sBool HitPlaneDoubleSided(sVector31 &intersect,const sVector4 &plane) const;
  sBool HitSphere(sF32 *dist, const sVector31 &center, sF32 radius) const;
  sBool HitAABB(sF32 &min, sF32 &max, const sVector31& bbmin,const sVector31& bbmax)const;
  sInt IntersectPlane(sF32 &t, const sVector4 &plane)const;  // 0 not intersection, 1 normal intersection, 2 ray in plane

  // warning: (out)dist is NOT ACCURATE, but about in the area of where it should be.
  //sBool HitCappedCylinder(const sVector31 &cylstart, const sVector31 &cylend, sF32 radius, sF32 *dist=0) const; 
};

inline sRay operator* (const sRay &r,const sMatrix34 &m) { sRay x(r.Start*m,r.Dir*m); return x; }

// ird : 1 / ray dir
sBool sIntersectRayAABB(sF32 &min, sF32 &max, const sVector31 &ro, const sVector30 &ird, const sVector31 &bbmin, const sVector31 &bbmax);

inline sWriter& operator| (sWriter &s,const sRay &a) { s | a.Start | a.Dir;  return s; }
inline sReader& operator| (sReader &s,      sRay &a) { s | a.Start | a.Dir;  return s; }

/****************************************************************************/
/***                                                                      ***/
/***   Scale-Rotate-Translate                                             ***/
/***                                                                      ***/
/****************************************************************************/

class sSRT
{
public:
  sVector31 Scale;                // scale before rotate
  sVector30 Rotate;               // 0..1 is full circle
  sVector31 Translate;            // translate after rotate

  sSRT();
  void Init(sF32 *);
  void MakeMatrix(sMatrix34 &mat) const;
};

inline sWriter& operator| (sWriter &s,const sSRT &a) { s | a.Scale | a.Rotate | a.Translate; return s; }
inline sReader& operator| (sReader &s,      sSRT &a) { s | a.Scale | a.Rotate | a.Translate; return s; }

/****************************************************************************/
/****************************************************************************/
/***                                                                      ***/
/***   Other Applications                                                 ***/
/***                                                                      ***/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/***                                                                      ***/
/***   Damping timeslice syncronisation                                   ***/
/***                                                                      ***/
/****************************************************************************/
/***                                                                      ***/
/***   if you use X' = X*DAMP+FORCE for a simple fake physic, you need    ***/
/***   to adjust DAMP and FORCE when you change the timeslice             ***/
/***                                                                      ***/
/***   if D and F are the factors for a timeslice of 1, then the factors  ***/
/***   for larger timeslices are:                                         ***/
/***                                                                      ***/
/***   X'  = X*D   +F             = XD+F                                  ***/
/***   X'2 = X*DD  +FD  +F        = XDD+(D+1)F                            ***/
/***   X'2 = X*DDD +FDD +FD +F    = XDDD+(DD+D+1)F                        ***/
/***   X'2 = X*DDDD+FDDD+FDD+FD+F = XDDDD+(DDD+DD+D+1)F                   ***/
/***                                                                      ***/
/***   When changing from Timeslice 1 to Timeslice N, this function       ***/
/***   calclulates D' and F' for use:                                     ***/
/***                                                                      ***/
/***   X' = X*DAMP+FORCE          // timeslice 1                          ***/
/***   X''= X*D'*DAMP + F'*FORCE  // timeslice N                          ***/
/***                                                                      ***/
/****************************************************************************/

void sMakeDamp(sInt timeslice,sF32 damp,sF32 &d,sF32 &f);

/****************************************************************************/
/***                                                                      ***/
/***   Filter (2Pole IIR)                                                 ***/
/***                                                                      ***/
/****************************************************************************/

struct sFilter2PoleTemp
{
  sF64 x1,x2;
  sF64 y1,y2;
  void Init() { sClear(*this); }
};

struct sFilter2Pole
{
  sF64 a0,a1,a2;
  sF64 b1,b2;

  void Init();
  void Scale(sF32 s);
  void Band(sF32 f,sF32 bw);      // bw = 0 .. 0.5
  void Low(sF32 f,sF32 reso);     // reso = 0 .. 1 (1 is dangerous!)
  void High(sF32 f,sF32 reso);    // reso = 0 .. 1 (1 is dangerous!)

  sF32 Filter(sFilter2PoleTemp &temp,sF32);
  void Filter(sFilter2PoleTemp &temp,sF32 *in,sF32 *out,sInt count);
  void FilterStereo(sFilter2PoleTemp *temp,sF32 *in,sF32 *out,sInt count);
};


/****************************************************************************/
/***                                                                      ***/
/***   Perlin Noise                                                       ***/
/***                                                                      ***/
/****************************************************************************/

// x,y = 8:16 (größere zahlen loopen)

sF32 sPerlin2D(sInt x,sInt y,sInt mask=255,sInt seed=0);  
sF32 sPerlin3D(sInt x,sInt y,sInt z,sInt mask=255,sInt seed=0);  
void sPerlinDerive3D(sInt x,sInt y,sInt z,sInt mask,sInt seed,sF32 &value,sVector30 &dir);

// x,y = 0..256 (größere zahlen loopen)
// mode = 0: x
// mode = 1: abs(x)
// mode = 2: sin(x*sPI)
// mode = 3: sin(abs(x)*sPI)

sF32 sPerlin2D(sF32 x,sF32 y,sInt octaves=1,sF32 falloff=1.0f,sInt mode=0,sInt seed=0);

/****************************************************************************/
/***                                                                      ***/
/***  Intersection                                                        ***/
/***                                                                      ***/
/****************************************************************************/

sBool sGetIntersection(const sVector2 &u1, const sVector2 &u2, const sVector2 &v1, const sVector2 &v2, sF32 *s=sNULL, sF32 *t=sNULL, sVector2 *p=sNULL);
sBool sGetIntersection(const sVector2 &u1, const sVector2 &u2, const sFRect &rect, sF32 *s=sNULL, sVector2 *p=sNULL);

/****************************************************************************/
/***                                                                      ***/
/***   Bezier Splines                                                     ***/
/***                                                                      ***/
/****************************************************************************/

// n = 0..1, returns 4 factors to be multiplied with four points each.
sVector30 sGetBezierSplineFactors3(sF32 n);
sVector4 sGetBezierSplineFactors4(sF32 n);
void sGetBezierSplinePoint(sF32 *out, sF32 n, const sF32 *v1, const sF32 *v2, const sF32 *v3, const sU32 d);
void sGetBezierSplinePoint(sF32 *out, sF32 n, const sF32 *v1, const sF32 *v2, const sF32 *v3, const sF32 *v4, const sU32 d);
template<typename T>
inline T sGetBezierSplinePoint(sF32 n, const T& v1, const T& v2, const T& v3) { T out; sGetBezierSplinePoint(&out[0], n, &v1[0], &v2[0], &v3[0], T::Dimensions); return out; }
template<typename T>
inline T sGetBezierSplinePoint(sF32 n, const T& v1, const T& v2, const T& v3, const T& v4) { T out; sGetBezierSplinePoint(&out[0], n, &v1[0], &v2[0], &v3[0], &v4[0], T::Dimensions); return out; }

/****************************************************************************/

// HEADER_ALTONA_BASE_MATH
#endif
