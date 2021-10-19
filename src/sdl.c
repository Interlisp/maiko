#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include "lispemul.h"
#include "miscstat.h"
#include "keyboard.h"
static SDL_Window *sdl_window = NULL;
static SDL_Surface *sdl_screenSurface = NULL;
static SDL_Renderer *sdl_renderer = NULL;
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
int sdl_displaywidth = 1600;
int sdl_displayheight = 1024;
extern char *DisplayRegion68k;

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

void sdl_bitblt_to_screen(uint32_t *source) {
  int width = sdl_displaywidth;
  int height = sdl_displayheight;
  int bpw = 32;
  for(int y = 0; y < height; y++) {
    for(int x = 0; x < width / bpw; x++) {
      for(int b = 0; b < bpw; b++) {
        //printf("%d/%d %d\n", x, y, b);
        int px = 0;
        if(source[y*(sdl_displaywidth/bpw) + x] & (1 << (bpw - 1 - b))) {
          px = 0xff000000;
        } else {
          px = 0xffffffff;
        }
        //printf("px is %x\n", px);
        set_pixel(sdl_screenSurface, x * bpw + b, y, px);
      }
    }
  }
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

void process_SDLevents() {
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    switch(event.type) {
    case SDL_QUIT:
      printf("quitting\n"); exit(0);
      break;
    case SDL_WINDOWEVENT:
      switch(event.window.event) {
      case SDL_WINDOWEVENT_RESIZED: sdl_screenSurface = SDL_GetWindowSurface(sdl_window); break;
      }
      break;
    case SDL_KEYDOWN:
      //      printf("type: %x, state: %x, repeat: %x, scancode: %x, sym: %x, mod: %x\n", event.key.type, event.key.state, event.key.repeat, event.key.keysym.scancode, event.key.keysym.sym, event.key.keysym.mod);
      handle_keydown(event.key.keysym.sym, event.key.keysym.mod);
      break;
    case SDL_KEYUP:
      handle_keyup(event.key.keysym.sym, event.key.keysym.mod);
      break;
    case SDL_MOUSEMOTION: {
      int x, y;
      SDL_GetMouseState(&x, &y);
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
  SDL_SetRenderDrawColor(sdl_renderer, 50, 50, 50, 255);
  sdl_bitblt_to_screen(DisplayRegion68k);
  SDL_UpdateWindowSurface(sdl_window);
}
int init_SDL() {
  int width = sdl_displaywidth;
  int height = sdl_displayheight;
  printf("requested width: %d, height: %d\n", width, height);
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not be initialized. SDL_Error: %s\n", SDL_GetError());
    return 1;
  }
  printf("initialised\n");
  sdl_window = SDL_CreateWindow("Maiko", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
  printf("Window created\n");
  if(sdl_window == NULL) {
    printf("Window could not be created. SDL_Error: %s\n", SDL_GetError());
    return 2;
  }
  sdl_screenSurface = SDL_GetWindowSurface(sdl_window);
  printf("surface: %dx%d, pitch %d\n", sdl_screenSurface->w, sdl_screenSurface->h, sdl_screenSurface->pitch);
  printf("  format %d, bitspp %d, bytespp %d\n", sdl_screenSurface->format->format, sdl_screenSurface->format->BitsPerPixel, sdl_screenSurface->format->BytesPerPixel);
  printf("SDL initialised\n");
  return 0;
}
