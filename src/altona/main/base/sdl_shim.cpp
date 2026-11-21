#include "sdl_shim.h"
#include <SDL3/SDL.h>
#include <vector>
#include <atomic>
#include <stdio.h>

static SDL_Window *Win;

int shim_init(int video)
{
  if(!SDL_Init((video?SDL_INIT_VIDEO:0)|SDL_INIT_AUDIO|SDL_INIT_EVENTS)) { fprintf(stderr,"[sdl] init: %s\n",SDL_GetError()); return 0; }
  return 1;
}
void *shim_create_window(int w,int h,int fullscreen,const char *title)
{
  SDL_WindowFlags f = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
  if(fullscreen) f |= SDL_WINDOW_FULLSCREEN;
  Win = SDL_CreateWindow(title,w,h,f);
  if(!Win) fprintf(stderr,"[sdl] window: %s\n",SDL_GetError());
  return Win;
}
void shim_set_title(const char *t) { if(Win) SDL_SetWindowTitle(Win,t); }
void shim_desktop_size(int *w,int *h)
{
  *w = *h = 0;
  if(!SDL_InitSubSystem(SDL_INIT_VIDEO)) return;      // may be called before sInit()
  SDL_DisplayID d = SDL_GetPrimaryDisplay();
  const SDL_DisplayMode *m = d ? SDL_GetDesktopDisplayMode(d) : 0;
  if(m) { *w = m->w; *h = m->h; }
}

static int MapKey(SDL_Keycode k)
{
  if(k>='a' && k<='z') return (int)k;
  if(k>='0' && k<='9') return (int)k;
  switch(k)
  {
  case SDLK_ESCAPE: return 27;  case SDLK_BACKSPACE: return 8; case SDLK_TAB: return 9; case SDLK_SPACE: return 32;
  case SDLK_RETURN: case SDLK_KP_ENTER: return 10;
  case SDLK_UP: return 0xe000;  case SDLK_DOWN: return 0xe001; case SDLK_LEFT: return 0xe002; case SDLK_RIGHT: return 0xe003;
  case SDLK_PAGEUP: return 0xe004; case SDLK_PAGEDOWN: return 0xe005; case SDLK_HOME: return 0xe006; case SDLK_END: return 0xe007;
  case SDLK_INSERT: return 0xe008; case SDLK_DELETE: return 0xe009;
  }
  if(k>=SDLK_F1 && k<=SDLK_F12) return 0xe020 + (int)(k-SDLK_F1);
  if(k>=32 && k<127) return (int)k;
  return 0;
}

unsigned shim_key_mods(void)
{
  SDL_Keymod m = SDL_GetModState(); unsigned q = 0;
  if(m & SDL_KMOD_LSHIFT) q |= 0x00010000; if(m & SDL_KMOD_RSHIFT) q |= 0x00020000;
  if(m & SDL_KMOD_LCTRL)  q |= 0x00080000; if(m & SDL_KMOD_RCTRL)  q |= 0x00100000;
  if(m & SDL_KMOD_LALT)   q |= 0x00200000; if(m & SDL_KMOD_RALT)   q |= 0x00400000;
  return q;
}

int shim_poll(ShimEvent *e)
{
  SDL_Event ev;
  while(SDL_PollEvent(&ev))
  {
    e->x = e->y = e->key = e->repeat = 0; e->mods = (int)shim_key_mods();
    switch(ev.type)
    {
    case SDL_EVENT_QUIT: case SDL_EVENT_WINDOW_CLOSE_REQUESTED: e->type = SHIM_QUIT; return 1;
    case SDL_EVENT_KEY_DOWN: case SDL_EVENT_KEY_UP:
      e->key = MapKey(ev.key.key); if(!e->key) continue;
      e->type = (ev.type==SDL_EVENT_KEY_DOWN) ? SHIM_KEYDOWN : SHIM_KEYUP; e->repeat = ev.key.repeat; return 1;
    case SDL_EVENT_MOUSE_MOTION: e->type = SHIM_MOUSEMOVE; e->x = (int)ev.motion.x; e->y = (int)ev.motion.y; return 1;
    case SDL_EVENT_MOUSE_BUTTON_DOWN: case SDL_EVENT_MOUSE_BUTTON_UP:
      e->type = (ev.type==SDL_EVENT_MOUSE_BUTTON_DOWN) ? SHIM_MOUSEDOWN : SHIM_MOUSEUP; e->key = ev.button.button; e->x = (int)ev.button.x; e->y = (int)ev.button.y; return 1;
    case SDL_EVENT_MOUSE_WHEEL: e->type = SHIM_WHEEL; e->key = ev.wheel.y>0 ? 1 : -1; return 1;
    case SDL_EVENT_WINDOW_RESIZED: e->type = SHIM_RESIZE; e->x = ev.window.data1; e->y = ev.window.data2; return 1;
    default: break;
    }
  }
  return 0;
}

void shim_mouse_state(int *x,int *y,int *buttons)
{
  float fx,fy; SDL_MouseButtonFlags b = SDL_GetMouseState(&fx,&fy);
  *x = (int)fx; *y = (int)fy; *buttons = 0;
  if(b & SDL_BUTTON_LMASK) *buttons |= 1; if(b & SDL_BUTTON_RMASK) *buttons |= 2; if(b & SDL_BUTTON_MMASK) *buttons |= 4;
}
void shim_quit(void) { if(Win) SDL_DestroyWindow(Win); Win = 0; SDL_Quit(); }

/* ------------------------------------------------------------------ audio */
struct Audio
{
  SDL_AudioStream *Stream; int Rate, Channels, Latency;
  void (*Render)(float*,int,void*); void *User;
  std::vector<float> Buf;
  // Published by the audio callback (which runs under SDL's stream lock). shim_audio_queued_frames() must NOT
  // call into SDL: the caller holds the player's output lock, and the callback takes that lock while SDL holds
  // the stream lock -> lock order inversion (deadlock). So it extrapolates from the last published value.
  std::atomic<long long> PubFrames;   // frames queued right after the callback's last PutAudioStreamData
  std::atomic<long long> PubNs;       // SDL_GetTicksNS() at that moment
};

static void SDLCALL AudioCb(void *ud,SDL_AudioStream *st,int additional,int total)
{
  Audio *a = (Audio*)ud;
  int fs = a->Channels*(int)sizeof(float);
  int queued = SDL_GetAudioStreamQueued(st);
  int need = a->Latency*fs - queued;
  if(additional>need) need = additional;
  while(need>0)
  {
    int frames = need/fs; if(frames>(int)(a->Buf.size()/a->Channels)) frames = (int)(a->Buf.size()/a->Channels);
    if(frames<=0) break;
    for(int i=0;i<frames*a->Channels;i++) a->Buf[i] = 0;
    a->Render(&a->Buf[0],frames,a->User);
    SDL_PutAudioStreamData(st,&a->Buf[0],frames*fs);
    need -= frames*fs;
  }
  a->PubFrames = SDL_GetAudioStreamQueued(st)/fs;
  a->PubNs = (long long)SDL_GetTicksNS();
}

void *shim_audio_open(int rate,int channels,int latency_frames,void (*render)(float*,int,void*),void *user)
{
  Audio *a = new Audio; a->PubFrames = 0; a->PubNs = (long long)SDL_GetTicksNS(); a->Rate = rate; a->Channels = channels; a->Latency = latency_frames; a->Render = render; a->User = user;
  a->Buf.resize(4096*channels);
  SDL_AudioSpec spec; spec.format = SDL_AUDIO_F32; spec.channels = channels; spec.freq = rate;
  a->Stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,&spec,AudioCb,a);
  if(!a->Stream) { fprintf(stderr,"[sdl] audio: %s\n",SDL_GetError()); delete a; return 0; }
  SDL_ResumeAudioStreamDevice(a->Stream);
  return a;
}
void shim_audio_close(void *p)
{
  Audio *a = (Audio*)p; if(!a) return;
  SDL_DestroyAudioStream(a->Stream); delete a;
}
int shim_audio_queued_frames(void *p)
{
  Audio *a = (Audio*)p;
  long long elapsed = (long long)SDL_GetTicksNS() - a->PubNs.load();
  long long q = a->PubFrames.load() - elapsed*a->Rate/1000000000LL;
  return q>0 ? (int)q : 0;
}
