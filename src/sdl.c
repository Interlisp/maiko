#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include "lispemul.h"
#include "miscstat.h"
#include "keyboard.h"
static SDL_Window *sdl_window = NULL;
static SDL_Renderer *sdl_renderer = NULL;
static SDL_Texture *sdl_texture = NULL;
static Uint32 *buffer = NULL;

extern int KBDEventFlg;
int keymap[] = {
  0, SDLK_5,
  1, SDLK_4,
  2, SDLK_6,
  3, SDLK_e,
  4, SDLK_7,
  5, SDLK_d,
  6, SDLK_u,
  7, SDLK_v,
  8, SDLK_RIGHTPAREN,
  8, SDLK_0,
  9, SDLK_k,
  10, SDLK_MINUS,
  11, SDLK_p,
  12, SDLK_SLASH,
  13, SDLK_KP_PERIOD,
  14, SDLK_SCROLLLOCK,
  15, SDLK_BACKSPACE,
  16, SDLK_3,
  17, SDLK_2,
  18, SDLK_w,
  19, SDLK_q,
  20, SDLK_s,
  21, SDLK_a,
  22, SDLK_LEFTPAREN,
  22, SDLK_9,
  23, SDLK_i,
  24, SDLK_x,
  25, SDLK_o,
  26, SDLK_l,
  27, SDLK_COMMA,
  28, SDLK_QUOTE,
  29, SDLK_RIGHTBRACKET,
  31, SDLK_LALT, /* Meta, Sun-4 usual key */
  32, SDLK_1,
  33, SDLK_ESCAPE,
  34, SDLK_TAB,
  35, SDLK_f,
  36, SDLK_LCTRL,
  37, SDLK_c,
  38, SDLK_j,
  39, SDLK_b,
  40, SDLK_z,
  41, SDLK_LSHIFT,
  42, SDLK_PERIOD,
  43, SDLK_SEMICOLON,
  43, SDLK_COLON,
  44, SDLK_RETURN,
  45, SDLK_BACKQUOTE,
  47, SDLK_RCTRL,
  48, SDLK_r,
  49, SDLK_t,
  50, SDLK_g,
  51, SDLK_y,
  52, SDLK_h,
  53, SDLK_8,
  54, SDLK_n,
  55, SDLK_m,
  56, SDLK_CAPSLOCK,
  57, SDLK_SPACE,
  58, SDLK_LEFTBRACKET,
  59, SDLK_EQUALS,
  60, SDLK_RSHIFT,
  61, SDLK_F11,
  61, SDLK_PAUSE,
  62, SDLK_HOME,
  63, SDLK_PAGEUP,
  64, SDLK_KP_EQUALS,
  65, SDLK_KP_DIVIDE,
  66, SDLK_F7,
  67, SDLK_F4,
  68, SDLK_F5,
  69, SDLK_KP_2,
  70, SDLK_KP_3,
  // 71, XK_Linefeed,
  // 73, XK_Numlock,
  76, SDLK_KP_ENTER,
  80, SDLK_F9,
  81, SDLK_KP_7,
  82, SDLK_KP_8,
  83, SDLK_KP_9,
  84, SDLK_KP_4,
  85, SDLK_KP_5,
  86, SDLK_LALT, /* (sun left-diamond key) */
  87, SDLK_KP_6,
  89, SDLK_INSERT,
  90, SDLK_END,
  91, SDLK_F12,
  92, SDLK_PRINTSCREEN, // is this XK_Print??
  93, SDLK_MODE, // is this XK_Mode_switch
  94, SDLK_KP_1,
  95, SDLK_KP_MULTIPLY,
  96, SDLK_KP_MINUS,
  97, SDLK_HELP,
  98, SDLK_KP_0,
  99, SDLK_F2,
  100, SDLK_F3,
  101, SDLK_F6,
  102, SDLK_KP_PLUS,
  104, SDLK_F8,
  105, SDLK_BACKSLASH,
  106, SDLK_F10,
  107, SDLK_F11,
  108, SDLK_F12,
  -1, -1
};
// all of the following are overwritten, the values here are irrelevant defaults!
// actual size of the lisp display in pixels.
int sdl_displaywidth = 0;
int sdl_displayheight = 0;
// current size of the window, in pixels
int sdl_windowwidth = 0;
int sdl_windowheight = 0;
// each pixel is shown as this many pixels
int sdl_pixelscale = 0;

extern Uint32 *DisplayRegion68k;


extern DLword *EmKbdAd068K, *EmKbdAd168K, *EmKbdAd268K, *EmKbdAd368K, *EmKbdAd468K, *EmKbdAd568K,
  *EmRealUtilin68K;
extern DLword *CTopKeyevent;
extern int URaid_req;
extern LispPTR *KEYBUFFERING68k;

void DoRing() {
  DLword w, r;
  KBEVENT *kbevent;

 do_ring:
  /* DEL is not generally present on a Mac X keyboard, Ctrl-shift-ESC would be 18496 */
  if (((*EmKbdAd268K) & 2113) == 0) { /*Ctrl-shift-NEXT*/
    error("******  EMERGENCY Interrupt ******");
    *EmKbdAd268K = KB_ALLUP;          /*reset*/
    ((RING *)CTopKeyevent)->read = 0; /* reset queue */
    ((RING *)CTopKeyevent)->write = MINKEYEVENT;
    /*return(0);*/
  } else if (((*EmKbdAd268K) & 2114) == 0) { /* Ctrl-Shift-DEL */
    *EmKbdAd268K = KB_ALLUP;                 /*reset*/
    URaid_req = T;
    ((RING *)CTopKeyevent)->read = 0; /* reset queue */
    ((RING *)CTopKeyevent)->write = MINKEYEVENT;
    /*return(0);*/
  }

#ifdef OS4_TYPE4BUG
  else if (((*EmKbdAd268K) & 2120) == 0) { /* Ctrl-Shift-Return */
    *EmKbdAd268K = KB_ALLUP;               /*reset*/
    URaid_req = T;
    ((RING *)CTopKeyevent)->read = 0; /* reset queue */
    ((RING *)CTopKeyevent)->write = MINKEYEVENT;
  }
#endif

  r = RING_READ(CTopKeyevent);
  w = RING_WRITE(CTopKeyevent);

  if (r == w) /* event queue FULL */
    goto KBnext;

  kbevent = (KBEVENT *)(CTopKeyevent + w);
  /*    RCLK(kbevent->time); */
  kbevent->W0 = *EmKbdAd068K;
  kbevent->W1 = *EmKbdAd168K;
  kbevent->W2 = *EmKbdAd268K;
  kbevent->W3 = *EmKbdAd368K;
  kbevent->W4 = *EmKbdAd468K;
  kbevent->W5 = *EmKbdAd568K;
  kbevent->WU = *EmRealUtilin68K;

  if (r == 0) /* Queue was empty */
    ((RING *)CTopKeyevent)->read = w;
  if (w >= MAXKEYEVENT)
    ((RING *)CTopKeyevent)->write = MINKEYEVENT;
  else
    ((RING *)CTopKeyevent)->write = w + KEYEVENTSIZE;

 KBnext:
  if (*KEYBUFFERING68k == NIL) *KEYBUFFERING68k = ATOM_T;
}

void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
  if(x >= surface->w)
    return;
  if(y >= surface->h)
    return;
  Uint32 * const target_pixel = (Uint32 *) ((Uint8 *) surface->pixels
                                            + y * surface->pitch
                                            + x * surface->format->BytesPerPixel);
  *target_pixel = pixel;
}
int should_update_texture = 0;

void sdl_bitblt_to_screen(int _x, int _y, int _w, int _h) {
  //  printf("bitblting\n");
  int before = SDL_GetTicks();
  int width = sdl_displaywidth;
  int height = sdl_displayheight;
  int bpw = 32;
  int pitch = sdl_displaywidth / bpw;
  int xlimit = (_x + _w + bpw - 1) / bpw;
  int ylimit = _y + _h;
  for(int y = _y; y < ylimit; y++) {
    for(int x = _x / bpw; x < xlimit; x++) {
      int w = DisplayRegion68k[y * pitch + x];
      int thex = x * bpw;
      for(int b = 0; b < bpw; b++) {
        //printf("%d/%d %d\n", x, y, b);
        int px = 0;
        if(w & (1 << (bpw - 1 - b))) {
          px = 0xff000000;
        } else {
          px = 0xffffffff;
        }
        //printf("px is %x\n", px);
        int xx = thex + b;
        buffer[y * sdl_displaywidth + xx] = px;
      }
    }
  }
  should_update_texture = 1;
  int after = SDL_GetTicks();
  //  printf("bitblting took %dms\n", after - before);
  /* before = SDL_GetTicks(); */
  /* SDL_UpdateTexture(sdl_texture, NULL, buffer, sdl_displaywidth * sizeof(Uint32)); */
  /* after = SDL_GetTicks(); */
  /* printf("UpdateTexture took %dms\n", after - before); */
}
int map_key(SDL_Keycode k) {
  for(int i = 0; keymap[i] != -1; i+= 2) {
    if(keymap[i+1] == k)
      return keymap[i];
  }
  return -1;
}
#define KEYCODE_OFFSET 0
void handle_keydown(SDL_Keycode k, unsigned short mod) {
  int lk = map_key(k);
  if(lk == -1) {
    printf("No mapping for key %s\n", SDL_GetKeyName(k));
  } else {
    printf("dn %s -> lisp keycode %d (0x%x)\n", SDL_GetKeyName(k), lk, mod);
    kb_trans(lk - KEYCODE_OFFSET, FALSE);
    DoRing();
    if ((KBDEventFlg += 1) > 0) Irq_Stk_End = Irq_Stk_Check = 0;
  }
}
void handle_keyup(SDL_Keycode k, unsigned short mod) {
  int lk = map_key(k);
  if(lk == -1) {
    printf("No mapping for key %s\n", SDL_GetKeyName(k));
  } else {
    printf("up %s -> lisp keycode %d (0x%x)\n", SDL_GetKeyName(k), lk, mod);
    kb_trans(lk - KEYCODE_OFFSET, TRUE);
    DoRing();
    if ((KBDEventFlg += 1) > 0) Irq_Stk_End = Irq_Stk_Check = 0;
  }
}
extern DLword *EmCursorX68K, *EmCursorY68K;
extern DLword *EmMouseX68K, *EmMouseY68K, *EmKbdAd068K, *EmRealUtilin68K;
extern LispPTR *CLastUserActionCell68k;
extern MISCSTATS *MiscStats;

/* bits within the EmRealUtilin word */
#define KEYSET_LEFT 8
#define KEYSET_LEFTMIDDLE 9
#define KEYSET_MIDDLE  10
#define KEYSET_RIGHTMIDDLE 11
#define KEYSET_RIGHT 12
/* Mouse buttons */
#define MOUSE_LEFT 13
#define MOUSE_RIGHT 14
#define MOUSE_MIDDLE 15
void sdl_update_viewport(int width, int height) {
  int w = width / 32 * 32;
  if(w > sdl_displaywidth * sdl_pixelscale)
    w = sdl_displaywidth * sdl_pixelscale;
  int h = height / 32 * 32;
  if(h > sdl_displayheight * sdl_pixelscale)
    h = sdl_displayheight * sdl_pixelscale;
  SDL_Rect r;
  r.x = 0;
  r.y = 0;
  r.w = w;
  r.h = h;
  SDL_RenderSetViewport(sdl_renderer, &r);
  printf("new viewport: %d / %d\n", w, h);
}
static int min(int a, int b) {
  if(a < b)
    return a;
  return b;
}
static int last_draw = 0;
static int last_keystate[512] = { 0 };
/* void handle_keyboard() { */
/*   SDL_PumpEvents(); */
/*   int numkeys; */
/*   const Uint8 *keystates = SDL_GetKeyboardState(&numkeys); */
/*   for(int i = 0; keymap[i] != -1; i+= 2) { */
/*     int keycode = keymap[i+1]; */
/*     int lk = keymap[i]; */
/*     int scancode = SDL_GetScancodeFromKey(keycode); */
/*     if(scancode < numkeys) { */
/*       int state = keystates[scancode]; */
/*       if(state != last_keystate[scancode]) { */
/*         printf("lk %d scancode %d %s <%s>\n", lk, scancode, state ? "pressed" : "released", SDL_GetKeyName(keycode)); */
/*         kb_trans(lk - KEYCODE_OFFSET, state ? FALSE : TRUE); */
/*         DoRing(); */
/*         if ((KBDEventFlg += 1) > 0) Irq_Stk_End = Irq_Stk_Check = 0; */
/*       } */
/*       last_keystate[scancode] = state; */
/*     } */
/*   } */
/* } */
int process_events_time = 0;
void process_SDLevents() {
  //  printf("processing events delta %dms\n", SDL_GetTicks() - process_events_time);
  process_events_time = SDL_GetTicks();
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    switch(event.type) {
    case SDL_QUIT:
      printf("quitting\n"); exit(0);
      break;
    case SDL_WINDOWEVENT:
      switch(event.window.event) {
      case SDL_WINDOWEVENT_RESIZED: {
        sdl_windowwidth = event.window.data1;
        sdl_windowheight = event.window.data2;
        sdl_update_viewport(sdl_windowwidth, sdl_windowheight);
      }
        break;
      }
      break;
    case SDL_KEYDOWN:
      printf("dn ts: %x, type: %x, state: %x, repeat: %x, scancode: %x, sym: %x <%s>, mod: %x\n", event.key.timestamp, event.key.type, event.key.state, event.key.repeat, event.key.keysym.scancode, event.key.keysym.sym, SDL_GetKeyName(event.key.keysym.sym), event.key.keysym.mod);
      handle_keydown(event.key.keysym.sym, event.key.keysym.mod);
      break;
    case SDL_KEYUP:
      printf("up ts: %x, type: %x, state: %x, repeat: %x, scancode: %x, sym: %x <%s>, mod: %x\n", event.key.timestamp, event.key.type, event.key.state, event.key.repeat, event.key.keysym.scancode, event.key.keysym.sym, SDL_GetKeyName(event.key.keysym.sym), event.key.keysym.mod);
      handle_keyup(event.key.keysym.sym, event.key.keysym.mod);
      break;
    case SDL_MOUSEMOTION: {
      int x, y;
      SDL_GetMouseState(&x, &y);
      x /= sdl_pixelscale;
      y /= sdl_pixelscale;
      *CLastUserActionCell68k = MiscStats->secondstmp;
      *EmCursorX68K = (*((DLword *)EmMouseX68K)) =
        (short)(x & 0xFFFF);
      *EmCursorY68K = (*((DLword *)EmMouseY68K)) =
        (short)(y & 0xFFFF);
      DoRing();
      if ((KBDEventFlg += 1) > 0) Irq_Stk_End = Irq_Stk_Check = 0;
    }
      break;
    case SDL_MOUSEBUTTONDOWN: {
      int button = event.button.button;
      switch(button) {
      case 1: PUTBASEBIT68K(EmRealUtilin68K, MOUSE_LEFT, FALSE); break;
      case 2: PUTBASEBIT68K(EmRealUtilin68K, MOUSE_MIDDLE, FALSE); break;
      case 3: PUTBASEBIT68K(EmRealUtilin68K, MOUSE_RIGHT, FALSE); break;
      case 4: PUTBASEBIT68K(EmRealUtilin68K, KEYSET_LEFT, FALSE); break;
      case 5: PUTBASEBIT68K(EmRealUtilin68K, KEYSET_LEFTMIDDLE, FALSE); break;
      case 6: PUTBASEBIT68K(EmRealUtilin68K, KEYSET_RIGHT, FALSE); break;
      case 7: PUTBASEBIT68K(EmRealUtilin68K, KEYSET_RIGHTMIDDLE, FALSE); break;
      }
      DoRing();
      if ((KBDEventFlg += 1) > 0) Irq_Stk_End = Irq_Stk_Check = 0;
    }
      break;
    case SDL_MOUSEBUTTONUP: {
      int button = event.button.button;
      switch(button) {
      case 1: PUTBASEBIT68K(EmRealUtilin68K, MOUSE_LEFT, TRUE); break;
      case 2: PUTBASEBIT68K(EmRealUtilin68K, MOUSE_MIDDLE, TRUE); break;
      case 3: PUTBASEBIT68K(EmRealUtilin68K, MOUSE_RIGHT, TRUE); break;
      case 4: PUTBASEBIT68K(EmRealUtilin68K, KEYSET_LEFT, TRUE); break;
      case 5: PUTBASEBIT68K(EmRealUtilin68K, KEYSET_LEFTMIDDLE, TRUE); break;
      case 6: PUTBASEBIT68K(EmRealUtilin68K, KEYSET_RIGHT, TRUE); break;
      case 7: PUTBASEBIT68K(EmRealUtilin68K, KEYSET_RIGHTMIDDLE, TRUE); break;
      }
      DoRing();
      if ((KBDEventFlg += 1) > 0) Irq_Stk_End = Irq_Stk_Check = 0;
    }
      break;
      /* case SDL_KEYMAPCHANGED: */
      /*   printf("SDL_KEYMAPCHANGED\n"); break; */
      /* case SDL_TEXTINPUT: */
      /*   printf("SDL_TEXTINPUT\n"); break; */
    default:
      printf("other event type: %d\n", event.type);
    }
  }
  //  handle_keyboard();
  /* sdl_bitblt_to_screen(); */
  // SDL_UpdateTexture(sdl_texture, NULL, buffer, sdl_displaywidth * sizeof(Uint32));
  int before = 0;
  int after = 0;
  /* if(should_update_texture) { */
  /*   before = SDL_GetTicks(); */
  /*   SDL_UpdateTexture(sdl_texture, NULL, buffer, sdl_displaywidth * sizeof(Uint32)); */
  /*   after = SDL_GetTicks(); */
  /*   should_update_texture = 0; */
  /* } */
  //  printf("UpdateTexture took %dms\n", after - before);
  int this_draw = SDL_GetTicks();
  before = SDL_GetTicks();
  //  printf("processing events took %dms\n", before - process_events_time);
  if(this_draw - last_draw > 16) {
    before = SDL_GetTicks();
    SDL_UpdateTexture(sdl_texture, NULL, buffer, sdl_displaywidth * sizeof(Uint32));
    after = SDL_GetTicks();
    should_update_texture = 0;

    sdl_bitblt_to_screen(0, 0, sdl_displaywidth, sdl_displayheight);
    SDL_RenderClear(sdl_renderer);
    SDL_Rect r;
    r.x = 0;
    r.y = 0;
    r.w = min(sdl_windowwidth / sdl_pixelscale, sdl_displaywidth);
    r.h = min(sdl_windowheight / sdl_pixelscale, sdl_displayheight);
    SDL_Rect s;
    s.x = 0;
    s.y = 0;
    s.w = min(sdl_windowwidth / sdl_pixelscale * sdl_pixelscale, sdl_displaywidth * sdl_pixelscale);
    s.h = min(sdl_windowheight / sdl_pixelscale * sdl_pixelscale, sdl_displayheight * sdl_pixelscale);
    SDL_RenderCopy(sdl_renderer, sdl_texture, &r, &s);
    SDL_RenderPresent(sdl_renderer);
    last_draw = this_draw;
    after = SDL_GetTicks();
    //    printf("rendering took %dms\n", after - before);
  }
}
int init_SDL(char *windowtitle, int w, int h, int s) {
  sdl_pixelscale = s;
  // must be multiples of 32
  w = w / 32 * 32;
  h = h / 32 * 32;
  sdl_displaywidth = w;
  sdl_displayheight = h;
  sdl_windowwidth = w * s;
  sdl_windowheight = h * s;
  int width = sdl_displaywidth;
  int height = sdl_displayheight;
  printf("requested width: %d, height: %d\n", width, height);
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not be initialized. SDL_Error: %s\n", SDL_GetError());
    return 1;
  }
  printf("initialised\n");
  sdl_window = SDL_CreateWindow(windowtitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, sdl_windowwidth, sdl_windowheight, 0);
  printf("Window created\n");
  if(sdl_window == NULL) {
    printf("Window could not be created. SDL_Error: %s\n", SDL_GetError());
    return 2;
  }
  printf("Creating renderer...\n");
  sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
  if(NULL == sdl_renderer) {
    printf("SDL Error: %s\n", SDL_GetError());
    return 3;
  }
  SDL_SetRenderDrawColor(sdl_renderer, 50, 50, 50, 255);
  SDL_RenderSetScale(sdl_renderer, 1.0, 1.0);
  printf("Creating texture...\n");
  sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
  printf("-> %x\n", sdl_texture);
  buffer = malloc(width * height * sizeof(Uint32));
  printf("SDL initialised\n");
  return 0;
}
