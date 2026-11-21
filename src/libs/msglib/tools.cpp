/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#include "tools.hpp"
#include "base/system.hpp"

/****************************************************************************/

sF32 sAbsMod(sF32 v, sF32 w)
{
  if (v >= 0.0f)
    return sMod(v,w);
  return w-sMod(-v,w);
}

// HSL: Hue, Saturation, Luminance
// H: position in the spectrum
// S: color saturation
// L: color lightness

void sRgbToHsl(sF32 r, sF32 g, sF32 b, sF32 &h, sF32 &s, sF32 &l)
{
  sF32 maxc = sMax(sMax(r,g),b);
  sF32 minc = sMin(sMin(r,g),b);
  l = (minc+maxc)/2.0f;
  if (minc == maxc)
  {
    h = 0.0f; s = 0.0f;
    return;
  }
  if (l <= 0.5f)
    s = (maxc - minc) / (maxc + minc);
  else
    s = (maxc - minc) / (2.0f-maxc-minc);
  sF32 rc = (maxc-r) / (maxc-minc);
  sF32 gc = (maxc-g) / (maxc-minc);
  sF32 bc = (maxc-b) / (maxc-minc);
  if (r == maxc)
    h = bc-gc;
  else if (g == maxc)
    h = 2.0f+rc-bc;
  else
    h = 4.0f+gc-rc;
  h = sAbsMod(h/6.0f, 1.0f);

}

const sF32 ONE_THIRD = 1.0f/3.0f;
const sF32 ONE_SIXTH = 1.0f/6.0f;
const sF32 TWO_THIRD = 2.0f/3.0f;

static sF32 _v(sF32 m1, sF32 m2, sF32 hue)
{
  hue = sAbsMod(hue, 1.0f);
  if (hue < ONE_SIXTH)
    return m1 + (m2-m1)*hue*6.0f;
  if (hue < 0.5f)
    return m2;
  if (hue < TWO_THIRD)
    return m1 + (m2-m1)*(TWO_THIRD-hue)*6.0f;
  return m1;
}

void sHslToRgb(sF32 h, sF32 s, sF32 l, sF32 &r, sF32 &g, sF32 &b)
{
  if (s == 0.0f)
  {
    r = l;
    g = l;
    b = l;
    return;
  }
  sF32 m1,m2;
  if (l <= 0.5f)
    m2 = l * (1.0f+s);
  else
    m2 = l+s-(l*s);
  m1 = 2.0f*l - m2;
  r = _v(m1, m2, h+ONE_THIRD);
  g = _v(m1, m2, h);
  b = _v(m1, m2, h-ONE_THIRD);
}

void sFixRectAspect(sFRect &rc, sF32 w, sF32 h, sBool enlarge)
{
  sF32 aspect = w / h;
  if (!enlarge) // one of the scales is 1.0, the other is smaller
  {
    if (aspect > 1.0f)
      rc.ScaleCenter(1.0f, 1.0f/aspect);
    else
      rc.ScaleCenter(aspect, 1.0f);
  }
  else // one of the scales is 1.0, the other is larger
  {
    if (aspect > 1.0f)
      rc.ScaleCenter(aspect, 1.0f);
    else
      rc.ScaleCenter(1.0f, 1.0f/aspect);
  }
}

/****************************************************************************/

