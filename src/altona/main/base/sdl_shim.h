/* Plain C interface to SDL3 (window, events, audio). Keeps SDL headers out of the Altona C++98 code. */
#ifndef SDL_SHIM_H
#define SDL_SHIM_H
#ifdef __cplusplus
extern "C" {
#endif
enum { SHIM_QUIT=1, SHIM_KEYDOWN, SHIM_KEYUP, SHIM_MOUSEMOVE, SHIM_MOUSEDOWN, SHIM_MOUSEUP, SHIM_WHEEL, SHIM_RESIZE };
typedef struct { int type; int key; int x, y; int mods; int repeat; } ShimEvent;   /* key: Altona key code */
int   shim_init(int video);
void *shim_create_window(int w,int h,int fullscreen,const char *title);   /* Vulkan-capable SDL_Window* */
void  shim_set_title(const char *title);
void  shim_desktop_size(int *w,int *h);
int   shim_poll(ShimEvent *e);
void  shim_mouse_state(int *x,int *y,int *buttons);
unsigned shim_key_mods(void);                                              /* Altona sKEYQ_* qualifiers */
void  shim_quit(void);
void *shim_audio_open(int rate,int channels,int latency_frames,void (*render)(float *buf,int frames,void *user),void *user);
void  shim_audio_close(void *a);
int   shim_audio_queued_frames(void *a);
#ifdef __cplusplus
}
#endif
#endif
