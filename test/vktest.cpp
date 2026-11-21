// Known-quad test for the Vulkan backend (matrix layout, Y flip, colour order, cull, blend, depth)
#include "base/types.hpp"
#include "base/system.hpp"
#include "base/math.hpp"
#include "base/graphics.hpp"
#include "msglib/shaders.hpp"
#include "msglib/ippmtrl.hpp"

struct V { sF32 x,y,z; sU32 c; sF32 u,v; };
static sU32 Desc[] = { sVF_POSITION,sVF_COLOR0,sVF_UV0,0 };

class TestApp : public sApp
{
  sVertexFormatHandle *Fmt;
  sGeometry *Geo, *Geo2;
  sMsgMaterial *Opaque, *CullOn, *Blend, *ZTest;
  sTexture2D *Rt; sIppToolMaterial *Ipp; sGeometry *IppGeo;
public:
  TestApp()
  {
    Fmt = sCreateVertexFormat(Desc);
    Rt = new sTexture2D(256,256,sTEX_ARGB8888|sTEX_2D|sTEX_RENDERTARGET|sTEX_NOMIPMAPS);
    Ipp = new sIppToolMaterial; Ipp->Texture[0] = Rt; Ipp->TFlags[0] = sMTF_LEVEL0|sMTF_CLAMP; Ipp->Flags = sMTRL_ZOFF|sMTRL_CULLOFF; Ipp->Prepare(sVertexFormatDouble);
    IppGeo = new sGeometry; IppGeo->Init(sGF_QUADLIST,sVertexFormatDouble);
    // 1 quad = 2 triangles, indexed (CW on screen when y is up in clip space: 0,1,2 / 0,2,3 -> see below)
    Geo = new sGeometry; Geo2 = new sGeometry;
    Opaque = Mat(0); CullOn = Mat(sMTRL_CULLON); Blend = Mat(sMTRL_ZOFF|sMTRL_CULLOFF,sMB_ALPHA); ZTest = Mat(sMTRL_ZON|sMTRL_CULLOFF);
  }
  sMsgMaterial *Mat(sU32 flags,sU32 blend=0)
  {
    sMsgMaterial *m = new sMsgMaterial; m->BlendColor = blend;
    m->Flags = (flags&sMTRL_CULLMASK) | ((flags&sMTRL_ZMASK)?(flags&sMTRL_ZMASK):sMTRL_ZOFF);
    m->FuncFlags[sMFT_DEPTH] = sMFF_LESSEQUAL;
    m->Prepare(Fmt);
    return m;
  }
  void Quad(sMsgMaterial *m,sF32 x0,sF32 y0,sF32 x1,sF32 y1,sF32 z,sU32 c0,sU32 c1,sBool flipwinding,const sMatrix44 &mvp)
  {
    // clip space, y up. vertices: 0=(x0,y0) 1=(x1,y0) 2=(x1,y1) 3=(x0,y1)
    sGeometry *g = Geo;
    g->Init(sGF_TRILIST|sGF_INDEX16,Fmt);
    V *vp; sU16 *ip;
    g->BeginLoad(Fmt,sGF_TRILIST|sGF_INDEX16,sGD_STREAM,4,6,(void**)&vp,(void**)&ip);
    V v[4] = { {x0,y0,z,c0,0,0},{x1,y0,z,c1,1,0},{x1,y1,z,c1,1,1},{x0,y1,z,c0,0,1} };
    for(int i=0;i<4;i++) vp[i] = v[i];
    // y-up clip space: 0->1->2 is counter-clockwise in a y-up picture
    static const sU16 a[6] = {0,1,2, 0,2,3}, b[6] = {0,2,1, 0,3,2};
    for(int i=0;i<6;i++) ip[i] = flipwinding ? b[i] : a[i];
    g->EndLoad();
    sCBuffer<sMsgMaterialEnvPara> cb; cb.Data->mvp = mvp; cb.Data->color.Init(1,1,1,1); cb.Data->grow.Init(1,1,1,1);
    m->Set(&cb); g->Draw();
  }
  void OnPaint3D()
  {
    sSetRendertarget(0,sCLEAR_ALL,0xff202040);
    sMatrix44 id; id.Init();
    // 1: red, top-left, identity matrix          (expect: pixel (64,64) red)
    Quad(Opaque,-0.8f,0.1f,-0.1f,0.8f,0.5f,0xffff0000,0xffff0000,sFALSE,id);
    // 2: green, bottom-left, shifted +0.5 in x by the matrix translation (row l) (expect: x 96..160 (clip -0.4..0.1 -> px 76..140), y bottom)
    sMatrix44 tr; tr.Init(); tr.l.x = 0.5f;
    Quad(Opaque,-0.9f,-0.8f,-0.4f,-0.1f,0.5f,0xff00ff00,0xff00ff00,sFALSE,tr);
    // 3: blue/white gradient, top-right. u dir -> colour c0 on left, c1 on right
    Quad(Opaque,0.2f,0.1f,0.8f,0.8f,0.5f,0xff0000ff,0xffffffff,sFALSE,id);
    // 4: backface culling: two quads, only the CCW(y-up) one may be visible. bottom-right.
    Quad(CullOn,0.1f,-0.8f,0.4f,-0.1f,0.5f,0xffffff00,0xffffff00,sFALSE,id);   // yellow
    Quad(CullOn,0.5f,-0.8f,0.8f,-0.1f,0.5f,0xff00ffff,0xff00ffff,sTRUE,id);    // cyan (opposite winding)
    // 5: alpha blend: white 50% over everything, thin strip in the middle
    Quad(Blend,-1.0f,-0.05f,1.0f,0.05f,0.5f,0x80ffffff,0x80ffffff,sFALSE,id);
    // 6: depth: far (z=0.8) magenta drawn AFTER near (z=0.2) orange, same place, ZON -> orange must stay. left-bottom corner
    Quad(ZTest,-0.95f,-0.95f,-0.7f,-0.7f,0.2f,0xffff8000,0xffff8000,sFALSE,id);
    Quad(ZTest,-0.95f,-0.95f,-0.7f,-0.7f,0.8f,0xffff00ff,0xffff00ff,sFALSE,id);
    // 7: grab the screen into a texture, clear, blit it back at quarter size into the lower right (QUADLIST, uv (0,0) = top-left of the picture)
    sGrabScreen(Rt);
    sSetRendertarget(0,sCLEAR_ALL,0xff000000);
    sVertexDouble *vp;
    IppGeo->BeginLoadVB(4,sGD_STREAM,(void**)&vp);
    vp[0].Init(0.0f,-1.0f,0.5f,0xffffffff,0,1,0,1);
    vp[1].Init(1.0f,-1.0f,0.5f,0xffffffff,1,1,1,1);
    vp[2].Init(1.0f, 0.0f,0.5f,0xffffffff,1,0,1,0);
    vp[3].Init(0.0f, 0.0f,0.5f,0xffffffff,0,0,0,0);
    IppGeo->EndLoadVB();
    sCBuffer<sIppToolMaterialVSPara> vs; vs.Data->mvp = id;
    sCBuffer<sIppToolMaterialPSPara> ps; ps.Data->ColorAdd.Init(0,0,0,0); ps.Data->Color0.Init(1,1,1,1); ps.Data->Color1.Init(0,0,0,0); ps.Data->ColorMul.Init(0,0,0,0);
    Ipp->Set(&vs,&ps);
    IppGeo->Draw();
  }
};

void sMain()
{
  sInit(sISF_3D|sISF_CONTINUOUS,256,256);
  sSetApp(new TestApp);
}
