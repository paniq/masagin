/*+**************************************************************************/
/***                                                                      ***/
/***   Copyright (C) 2005-2006 by Dierk Ohlerich                          ***/
/***   all rights reserverd                                               ***/
/***                                                                      ***/
/***   To license this software, please contact the copyright holder.     ***/
/***                                                                      ***/
/**************************************************************************+*/

#include "shape.hpp"
#include "base/system.hpp"
#include "tools.hpp"

/****************************************************************************/

sShape2D::sShape2D()
{
  MsgClass = -1;
}

sShape2D::~sShape2D()
{
}


void sShape2D::GetPoint(sF32 pos, sVector2 &pt)
{
  pos = sClamp(pos, 0.0f, 1.0f);
  if (pos <= 0.0f)
  {
    pt = Points[0];
    return;
  }
  else if (pos >= 1.0f)
  {
    pt = Points.GetTail();
    return;
  }
  else
  {
    pos *= Lengths.GetTail();

    sInt lower = 0;
    sInt upper = Lengths.GetCount()-1;
    sInt i = 0;
    while ((upper - lower) > 1)
    {
      i = (upper + lower)/2;
      if (pos > Lengths[i])
        lower = i;
      else if (pos < Lengths[i])
        upper = i;
      else if (pos == Lengths[i])
      {
        pt = Points[i];
        return;
      }
    }    
    pt = Points[lower];
    pos = (pos - Lengths[lower]) / (Lengths[upper] - Lengths[lower]);
    pt = Points[lower] + (Points[upper] - Points[lower]) * pos;
  }
}

void sShape2D::Serialize(sReader &s)
{
  sString<64> id;
  sReadMSGString(id, s);
  s | MsgClass;
  sU32 pointcount;
  s | pointcount;
  sVERIFY(pointcount < 32767);
  Points.HintSize(pointcount);
  for (sU32 j = 0; j < pointcount; ++j)
  {
    sVector2 v;
    s | v;
    v.x = (1.0f + v.x) * 0.5f;
    v.y = (1.0f + v.y) * 0.5f;
    Points.AddTail(v);
  }

  switch (MsgClass)
  {
    case MSGCLASS_ROOT:
      {
        Matrix.Init();
        sVector2 d = (Points.GetTail() - Points[0]);
        sF32 scale = 1.0f/d.Length();
        sF32 angle = sATan2(d.x, d.y);
        
        sMatrix34 transmtx;
        transmtx.l.x -= Points[0].x;
        transmtx.l.y -= Points[0].y;

        sMatrix34 scalemtx;
        scalemtx.Scale(scale, scale, scale);
        
        sMatrix34 rotmtx;
        rotmtx.EulerXYZ(0,0,angle);

        Matrix *= transmtx;
        Matrix *= rotmtx;
        Matrix *= scalemtx;
      } break;
    case MSGCLASS_ANCHOR:
      {

        Matrix.Init();
        sVector2 d = (Points.GetTail() - Points[0]);
        sF32 scale = d.Length();
        sF32 angle = sATan2(d.x, d.y);
        
        sMatrix34 transmtx;
        transmtx.l.x += Points[0].x;
        transmtx.l.y += Points[0].y;

        sMatrix34 scalemtx;
        scalemtx.Scale(scale, scale, scale);
        
        sMatrix34 rotmtx;
        rotmtx.EulerXYZ(0,0,-angle);

        Matrix *= scalemtx;
        Matrix *= rotmtx;
        Matrix *= transmtx;
      } break;
    case MSGCLASS_ROUTE:
      {
        Lengths.HintSize(pointcount);
        sF32 length = 0.0f;
        sVector2 bp = Points[0];
        sVector2 delta = sVector2(0,0);
        sVector2 *pt;
        sFORALL(Points, pt)
        {
          *pt = *pt - bp;
          length += (*pt - delta).Length();
          delta = *pt;
          Lengths.AddTail(length);
        }
      } break;
    default:
      break;
  }
}

void sShape2D::Draw()
{
}

/****************************************************************************/
