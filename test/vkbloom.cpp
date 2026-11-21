// Bloom centring test: white square at a known place, run the exact bloom sequence from sMasagin::Paint.
#include "base/types.hpp"
#include "base/system.hpp"
#include "base/math.hpp"
#include "base/graphics.hpp"
#include "msglib/shaders.hpp"
#include "msglib/ipptool.hpp"

struct V { sF32 x,y,z; sU32 c; sF32 u,v; };
static sU32 Desc[] = { sVF_POSITION,sVF_COLOR0,sVF_UV0,0 };

class TestApp : public sApp
{
  sVertexFormatHandle *Fmt; sGeometry *Geo; sMsgMaterial *Mtrl; sIppTool *Ipp;
public:
  TestApp()
  {
    Fmt = sCreateVertexFormat(Desc);
    Geo = new sGeometry;
    Mtrl = new sMsgMaterial; Mtrl->Flags = sMTRL_ZOFF|sMTRL_CULLOFF; Mtrl->Prepare(Fmt);
    Ipp = new sIppTool();
    for(int i=1;i<=8;i++) Ipp->AddTargetRel(i,0,0);
    Ipp->AddTargetRel(9,1,1); Ipp->AddTargetRel(10,1,1);
  }
  void OnPaint3D()
  {
    sSetRendertarget(0,sCLEAR_ALL,sGetShellSwitch(L"white")?0xffffffff:0xff000000);
    // 32x32 px square centred at pixel (400,150) on a 640x400 screen
    sF32 cx = (400.0f/640.0f)*2-1, cy = 1-(150.0f/400.0f)*2, hx = 16.0f/640.0f*2, hy = 16.0f/400.0f*2;
    Geo->Init(sGF_TRILIST|sGF_INDEX16,Fmt);
    V *vp; sU16 *ip;
    Geo->BeginLoad(Fmt,sGF_TRILIST|sGF_INDEX16,sGD_STREAM,4,6,(void**)&vp,(void**)&ip);
    V v[4] = { {cx-hx,cy-hy,0.5f,0xffffffff,0,0},{cx+hx,cy-hy,0.5f,0xffffffff,0,0},{cx+hx,cy+hy,0.5f,0xffffffff,0,0},{cx-hx,cy+hy,0.5f,0xffffffff,0,0} };
    for(int i=0;i<4;i++) vp[i]=v[i];
    static const sU16 a[6]={0,2,1,0,3,2}; for(int i=0;i<6;i++) ip[i]=a[i];
    Geo->EndLoad();
    sMatrix44 id; id.Init();
    sCBuffer<sMsgMaterialEnvPara> cb; cb.Data->mvp=id; cb.Data->color.Init(1,1,1,1); cb.Data->grow.Init(1,1,1,1);
    Mtrl->Set(&cb); Geo->Draw();
    if(!sGetShellSwitch(L"nobloom"))
    {
      sIppToolGlarePara para; para.Init();
      para.Source1 = 9; para.Source2 = 10; para.SubColor = 0xff000000; para.ImageColor = 0xffc0c0c0; sF32 bs = sGetShellInt(L"bloomstrength",0,100)/100.0f;
      sU32 gl = sClamp(sInt(0x40*bs+0.5f),0,255); para.GlareColor = 0xff000000 | (gl*0x010101);
      if (bs!=1.0f) { sU32 ic = sClamp(0xff-sInt(gl)+1,0,255); para.ImageColor = 0xff000000 | (ic*0x010101); }
      para.Radius0 = 3.0f; para.Radius1 = 0.0f; para.Amplify = 1.0f;
      Ipp->Grab(1); Ipp->Glare(para);
      sIppToolBlit blit; Ipp->InitBlit(blit,0,1); Ipp->Blit(blit,sIPB_OFFSETODD);
    }
  }
};

void sMain() { sInit(sISF_3D|sISF_CONTINUOUS,640,400); sSetApp(new TestApp); }
