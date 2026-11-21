/****************************************************************************/
/***                                                                      ***/
/***   (C) 2005 Dierk Ohlerich, all rights reserved                       ***/
/***                                                                      ***/
/****************************************************************************/

#include "main.hpp"
#include "base/windows.hpp"
#include "util/image.hpp"
#include "util/shaders.hpp"


/****************************************************************************/

// initialize resources


MyApp::MyApp() : Player(sTRUE)
{
  // debug output

  Painter = new sPainter;

  // geometry 

  Geo = new sGeometry();
//  Geo->Init(sGF_TRILIST|sGF_INDEX16,sVertexFormatStandard);
  Geo->Init(sGF_TRILIST|sGF_INDEX16,sVertexFormatSingle);

  // texture

  sBeginAlt();
  sImage *img = new sImage(64,64);
  sEndAlt();

  if(Geo->GetFormat()==sVertexFormatSingle)
    img->Checker(0xffff8080,0xff80ff80,8,8);
  else
    img->Checker(0xff808080,0xff404040,8,8);

  img->Glow(0.5f,0.5f,0.25f,0.25f,0xffffffff,1.0f,4.0f);

  Tex = sLoadTexture2D(img,sTEX_ARGB8888|sTEX_2D);
  delete img;
  sCheckAlt();

  // material

  Mtrl = new sSimpleMaterial;
  Mtrl->Flags = sMTRL_ZON | sMTRL_CULLON;
  Mtrl->Flags |= sMTRL_LIGHTING;
  Mtrl->Texture[0] = Tex;
  Mtrl->TFlags[0] = sMTF_LEVEL2|sMTF_CLAMP;
  Mtrl->Prepare(Geo->GetFormat());

  // light

  sClear(Env);
  Env.AmbientColor  = 0x00202020;
  Env.LightColor[0] = 0x00ff4000;
  Env.LightColor[1] = 0x0040ff00;
  Env.LightColor[2] = 0x000040ff;
  Env.LightColor[3] = 0x00000000;
  Env.LightDir[0].Init(1,0,0);
  Env.LightDir[1].Init(0,1,0);
  Env.LightDir[2].Init(0,0,1);
  Env.LightDir[3].Init(0,0,0);
  Env.Fix();
  sCheckMemWaste();

  Player.Init(L"c:\\dubman.ogg");
  Player.Play();
}

// free resources (memory leaks are evil)

MyApp::~MyApp()
{
  Player.Exit();
  sCheckMemWaste();
  delete Painter;
  delete Geo;
  delete Mtrl;
  delete Tex;
}

/****************************************************************************/

// paint a frame

void MyApp::OnPaint3D()
{ 
  // set rendertarget

  sSetRendertarget(0,sCLEAR_ALL,0xff405060);

  // get timing

  Timer.OnFrame(sGetTime());
  sInt time = Timer.GetTime();

  // set camera

  View.SetTargetCurrent();
  View.SetZoom(1.0f);
  View.Model.EulerXYZ(time*0.0011f,time*0.0013f,time*0.0015f);
  View.Model.l.Init(0,0,0);
  View.Camera.l.Init(0,0,-4);
  View.Prepare();
  View.Set();
 
  // set material

  sCBuffer<sSimpleMaterialEnvPara> cb;
  cb.Data->Set(Env);
  Mtrl->Set(&cb);

  // load vertices and indices

  if(Geo->GetFormat()==sVertexFormatSingle)
  {
    sVertexSingle *vp;
    Geo->BeginLoadVB(8,sGD_STREAM,(void **)&vp);
    vp[0].Init(-1,-1,-1,  0xff404040, 0,0);
    vp[1].Init( 1,-1,-1,  0xffff0000, 0,1);
    vp[2].Init( 1, 1,-1,  0xff00ff00, 1,1);
    vp[3].Init(-1, 1,-1,  0xffffff00, 1,0);
    vp[4].Init(-1,-1, 1,  0xff0000ff, 0,1);
    vp[5].Init( 1,-1, 1,  0xffff00ff, 1,1);
    vp[6].Init( 1, 1, 1,  0xff00ffff, 1,0);
    vp[7].Init(-1, 1, 1,  0xffffffff, 0,0);
    Geo->EndLoadVB();
  }
  else
  {
    sVertexStandard *vp;
    Geo->BeginLoadVB(8,sGD_STREAM,(void **)&vp);
    static const sF32 N=1.0f/sFSqrt(3);
    vp[0].Init(-1,-1,-1, -N,-N,-N, 0,0);
    vp[1].Init( 1,-1,-1,  N,-N,-N, 0,1);
    vp[2].Init( 1, 1,-1,  N, N,-N, 1,1);
    vp[3].Init(-1, 1,-1, -N, N,-N, 1,0);
    vp[4].Init(-1,-1, 1, -N,-N, N, 0,1);
    vp[5].Init( 1,-1, 1,  N,-N, N, 1,1);
    vp[6].Init( 1, 1, 1,  N, N, N, 1,0);
    vp[7].Init(-1, 1, 1, -N, N, N, 0,0);
    Geo->EndLoadVB();
  }

  sU16 *ip;
  Geo->BeginLoadIB(6*6,sGD_STREAM,(void **)&ip);
  sQuad(ip,0,3,2,1,0);
  sQuad(ip,0,4,5,6,7);
  sQuad(ip,0,0,1,5,4);
  sQuad(ip,0,1,2,6,5);
  sQuad(ip,0,2,3,7,6);
  sQuad(ip,0,3,0,4,7);
  Geo->EndLoadIB();

  // draw

  Geo->Draw();

  // debug output
#if sRENDERER!=sRENDER_PS3GCM
  sF32 avg = Timer.GetAverageDelta();
  sF32 pos = Player.GetPosition();
  Painter->SetTarget();
  Painter->Begin();
  Painter->SetPrint(0,0xff000000,2);
  Painter->PrintF(12,12,L"%5.2ffps %5.3fms pos:%f",1000/avg,avg,pos);
  Painter->SetPrint(0,~0,2);
  Painter->PrintF(10,10,L"%5.2ffps %5.3fms pos:%d",1000/avg,avg,pos);
  Painter->End();
#endif
}

/****************************************************************************/

// abort program when escape is pressed

void MyApp::OnInput(const sInputEvent &ie)
{
  if (ie.DeviceType==sIED_KEYBOARD) switch (ie.Key)
  {
    case sKEY_ESCAPE: sExit(); break;
    case 'a': Player.Play(); break;
    case 's': Player.Pause(); break;
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': 
      {
        sF32 pos=(ie.Key-'0')*10;
        Player.Seek(pos);
        Player.Play();
      } break;
  }
}

void MyApp::OnPrepareFrame()
{
  sJoypad *joy;
  sJoypadData data;

  joy = sGetJoypad(0);
  if(joy)
  {
    joy->GetData(data);
    if(data.Buttons)
      sExit();
  }
}


/****************************************************************************/

// register application class

void sMain()
{
  sInit(sISF_3D|sISF_CONTINUOUS/*|sISF_FULLSCREEN*/,640,480);
//  for(sInt i=10;i<14;i++) new sU8[1<<i];
  sSetApp(new MyApp());
  sSetWindowName(L"Cube");
}

/****************************************************************************/

