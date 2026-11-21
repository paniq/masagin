// Renderer-core smoke test: a single triangle through vkcore, without Altona and
// without content. Build it with ./build_test.sh corehello and run it from the
// repository root, where ./shaders is found.
#include "base/vkcore.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
struct V { float x,y,z; uint32_t c; float u,v; };
int main()
{
  VkcConfig c; memset(&c,0,sizeof(c)); c.Width=128; c.Height=128; c.ShaderDir="shaders"; c.DumpDir="/tmp/core"; c.DumpEvery=1; mkdir(c.DumpDir,0755);
  if(!vkc_init(c)) return 1;
  vkc_begin_frame();
  vkc_set_target(0,getenv("NOCLEAR")?0:3,0xff202040,0);
  VkcBuf vb=vkc_alloc_frame(sizeof(V)*3); V *v=(V*)vb.Map;
  v[0]={-0.5f,-0.5f,0.5f,0xffff0000,0,0}; v[1]={0.5f,-0.5f,0.5f,0xff00ff00,0,0}; v[2]={0,0.5f,0.5f,0xff0000ff,0,0};
  VkcDraw d; memset(&d,0,sizeof(d));
  d.Key.Kind=getenv("KIND")?atoi(getenv("KIND")):1; d.Key.Perm=1; d.Key.AlphaRef=-1; d.Key.Stride=sizeof(V); d.Key.AttrCount=2;
  d.Key.Attr[0]={0,1,0}; d.Key.Attr[1]={1,3,12};
  d.Key.ColorMask=15; d.Key.DepthFunc=3;
  float m[24]={1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1, 1,1,1,1, 1,1,1,1};
  d.Push=m; d.PushSize=96; d.Vertex=vb; d.Count=3;
  vkc_draw(d);
  vkc_end_frame();
  return 0;
}
