#include <SDL.h>
#include <SDL_keycode.h>
#include <assert.h>
#include <limits.h>
#include "sdldefs.h"
#include "byteswapdefs.h"
#include "lispemul.h"
#include "lsptypes.h"
#include "miscstat.h"
#include "keyboard.h"
#include "lspglob.h"  // for IOPage
#include "display.h"  // for CURSORHEIGHT, DisplayRegion68k

#define  SDLRENDERING 1

static SDL_Window *sdl_window = NULL;
#if defined(SDLRENDERING)
static SDL_Renderer *sdl_renderer = NULL;
static SDL_RendererInfo sdl_rendererinfo = {0};
static SDL_Texture *sdl_texture = NULL;
#else
static SDL_Surface *sdl_windowsurface = NULL;
static SDL_Surface *sdl_buffersurface = NULL;
static int buffer_size = 0;
static void *buffer = NULL;
#endif
static Uint32 sdl_white;
static Uint32 sdl_black;
static Uint32 sdl_foreground;
static Uint32 sdl_background;
static int sdl_bytesperpixel;
static SDL_PixelFormat *sdl_pixelformat;

extern void kb_trans(u_short keycode, u_short upflg);
extern int error(const char *s);

extern int KBDEventFlg;
/* clang-format off */
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
/* clang-format on */
static const DLword bitmask[16] = {1 << 15, 1 << 14, 1 << 13, 1 << 12, 1 << 11, 1 << 10,
                                   1 << 9,  1 << 8,  1 << 7,  1 << 6,  1 << 5,  1 << 4,
                                   1 << 3,  1 << 2,  1 << 1,  1 << 0};
// all of the following are overwritten, the values here are irrelevant defaults!
// actual size of the lisp display in pixels.
int sdl_displaywidth = 0;
int sdl_displayheight = 0;
// current size of the window, in pixels
int sdl_windowwidth = 0;
int sdl_windowheight = 0;
// each pixel is shown as this many pixels
int sdl_pixelscale = 0;
extern DLword *EmKbdAd068K, *EmKbdAd168K, *EmKbdAd268K, *EmKbdAd368K, *EmKbdAd468K, *EmKbdAd568K,
    *EmRealUtilin68K;
extern DLword *EmCursorBitMap68K;
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

static int min(int a, int b) {
  if (a < b) return a;
  return b;
}

static int display_update_needed = 0;

static int min_x = INT_MAX;
static int min_y = INT_MAX;
static int max_x = 0;
static int max_y = 0;
void sdl_notify_damage(int x, int y, int w, int h) {
  if (x < min_x) min_x = x;
  if (y < min_y) min_y = y;
  if (x + w > max_x) max_x = min(x + w, sdl_displaywidth - 1);
  if (y + h > max_y) max_y = min(y + h, sdl_displayheight - 1);
  display_update_needed = 1;
}

/* a simple linked list to remember generated cursors
 * because cursors don't have any identifying information
 * except for the actual bitmap in Lisp, just cache that.
 * 16 DLwords, to give a 16x16 bitmap cursor.
 */
struct CachedCursor {
  struct CachedCursor *next;
  DLword EmCursorBitMap[CURSORHEIGHT];
  SDL_Cursor *cursor;
} *sdl_cursorlist = NULL;

/*
 * given a 16-bit value and a repeat count modify an array
 * of bytes to contain the same bit pattern with each bit
 * repeated "reps" times consecutively in the output
 */
static void replicate_bits(int bits, int reps, Uint8 *out) {
  int dbyte = 0;
  int dbit = 7;
  for (int ibit = 15; ibit >= 0; --ibit) {
    for (int r = 0; r < reps; r++) {
      if (bits & (1 << ibit))
        out[dbyte] |= 1 << dbit;
      if (--dbit < 0) {
        dbyte++;
        dbit = 7;
      }
    }
  }
}

static int cursor_equal_p(DLword *a, DLword *b) {
  for (int i = 0; i < CURSORHEIGHT; i++)
    if (a[i] != b[i]) return FALSE;
  return TRUE;
}

/*
 * Try to find cursor CURSOR on the sdl_cursorlist, if it isn't there, add it.
 * Return an SDL_Cursor that can be used directly.
 */
static SDL_Cursor *sdl_getOrAllocateCursor(DLword cursor[16], int hot_x, int hot_y) {
  hot_x = 0;
  hot_y = 0;
  /* try to find the cursor by checking the full bitmap */
  struct CachedCursor *pclp = NULL;
  struct CachedCursor *clp = sdl_cursorlist;
  SDL_Cursor *c;
  while (clp != NULL) {
    if (cursor_equal_p(clp->EmCursorBitMap, cursor) == TRUE) {
      /* if it's in the first two elements of the list, leave the order alone.
       * There is a high probability of flipping back and forth between two
       */
      if (clp == sdl_cursorlist || pclp == sdl_cursorlist) {
        return clp->cursor;
      }
      /* otherwise unlink the found item and reinsert at the front */
      pclp->next = clp->next;
      clp->next = sdl_cursorlist;
      sdl_cursorlist = clp;
      return clp->cursor;
    }
    pclp = clp;
    clp = clp->next;
  }
  /* It isn't there, so build a new one */
  clp = (struct CachedCursor *)malloc(sizeof(struct CachedCursor));
  memcpy(clp->EmCursorBitMap, cursor, sizeof(clp->EmCursorBitMap));
  /* no scaling is an easy case, scale > 1 is harder */
  if (sdl_pixelscale == 1) {
    Uint8 sdl_cursor_data[32];
    for (int i = 0; i < 32; i++) sdl_cursor_data[i] = GETBYTE(((Uint8 *)cursor) + i);
    c = SDL_CreateCursor(sdl_cursor_data, sdl_cursor_data, 16, 16, hot_x, hot_y);
  } else {
    Uint8 *sdl_cursor_data = calloc(sdl_pixelscale * sdl_pixelscale, 32);
    /* fill in the cursor data expanded */
    for (int i = 0; i < 32; i += 2) {
      int v = GETBYTE(((Uint8 *)cursor) + i) << 8 | GETBYTE(((Uint8 *)cursor) + i + 1);
      int db = i * sdl_pixelscale * sdl_pixelscale;
      /* spread the bits out for the first copy of the row */
      replicate_bits(v, sdl_pixelscale, &sdl_cursor_data[db]);
      /* and then copy the replicated bits for the copies of the row */
      for (int j = 1; j < sdl_pixelscale; j++) {
        memcpy(&sdl_cursor_data[db + (j * 2 * sdl_pixelscale)], &sdl_cursor_data[db], 2 * sdl_pixelscale);
      }
    }
    c = SDL_CreateCursor(sdl_cursor_data, sdl_cursor_data, 16 * sdl_pixelscale, 16 * sdl_pixelscale, hot_x, hot_y);
  }
  if (c == NULL) printf("ERROR creating cursor: %s\n", SDL_GetError());
  clp->cursor = c;
  clp->next = sdl_cursorlist;
  sdl_cursorlist = clp;
  return clp->cursor;
}

/*
 * Read a cursor bitmap from lisp. Try to find a cached cursor, then use that.
 * Use HOT_X and HOT_Y as the cursor hotspot.
 * XXX: needs to deal with sdl_pixelscale > 1, and where is the hotspot?
 */
void sdl_setCursor(int hot_x, int hot_y) {
  SDL_Cursor *c = sdl_getOrAllocateCursor(EmCursorBitMap68K, hot_x, hot_y);
  SDL_SetCursor(c);
}
#if defined(SDLRENDERING)
void sdl_bitblt_to_texture(int _x, int _y, int _w, int _h) {
  DLword *src = DisplayRegion68k;
  void *dst;
  int dstpitchbytes;
  int dstpitchpixels;
  const int bitsperword = 8 * sizeof(DLword);
  int sourcepitchwords = sdl_displaywidth / bitsperword;
  int xstart = _x / bitsperword;
  int xlimit = (_x + _w + bitsperword - 1) / bitsperword;
  int ystart = _y * sourcepitchwords;
  int ylimit = (_y + _h) * sourcepitchwords;
  SDL_Rect dstrect;
  // Avoid dealing with partial words in the update by stretching the source rectangle
  // left and right to cover complete units and lock the corresponding
  // region in the texture
  dstrect.x = xstart * bitsperword;
  dstrect.w = (xlimit * bitsperword) - dstrect.x;
  dstrect.y = _y;
  dstrect.h = _h;
  SDL_LockTexture(sdl_texture, &dstrect, &dst, &dstpitchbytes);
  dstpitchpixels = dstpitchbytes / sdl_bytesperpixel;
  int dy = 0;
  // for each line in the source image
  for (int sy = ystart; sy < ylimit; sy += sourcepitchwords, dy += dstpitchpixels) {
    // for each word in the line
    int dx = 0;
    for (int sx = xstart; sx < xlimit; sx++, dx += bitsperword) {
      int srcw = GETBASEWORD(src, sy + sx);
      // for each bit in the word
      for (int b = 0; b < bitsperword; b++) {
        ((Uint32 *)dst)[dy + dx + b] = (srcw & bitmask[b]) ? sdl_foreground : sdl_background;
      }
    }
  }
  SDL_UnlockTexture(sdl_texture);
}
void sdl_bitblt_to_texture2(int _x, int _y, int _w, int _h) {
  DLword *src = DisplayRegion68k;
  void *dst;
  int dstpitchbytes;
  int dstpitchpixels;
  const int bitsperword = 8 * sizeof(DLword);
  int sourcepitchwords = sdl_displaywidth / bitsperword;
  int xstart = _x / bitsperword;   // "word" index of first accessed word in line
  int xstartb = _x % bitsperword;  // bit within word
  int xlimit = (_x + _w + bitsperword - 1) / bitsperword;  // word index
  int ystart = _y * sourcepitchwords;
  int ylimit = (_y + _h) * sourcepitchwords;
  SDL_Rect dstrect = {.x = _x, .y = _y, .w = _w, .h = _h};
  SDL_LockTexture(sdl_texture, &dstrect, &dst, &dstpitchbytes);
  dstpitchpixels = dstpitchbytes / sdl_bytesperpixel;
  int dy = 0;
  // for each line in the source image
  for (int sy = ystart; sy < ylimit; sy += sourcepitchwords, dy += dstpitchpixels) {
    int dx = 0;
    int sx = xstart;
    int b = xstartb;
    int srcw = GETBASEWORD(src, sy + sx);
    // for each pixel within the dstination region line
    for (int dx = 0; dx < _w; dx++) {
      ((Uint32 *)dst)[dy + dx] = (srcw & bitmask[b]) ? sdl_foreground : sdl_background;
      if (++b == bitsperword) {
        b = 0;
        sx++;
        srcw = GETBASEWORD(src, sy + sx);
      }
    }
  }
  SDL_UnlockTexture(sdl_texture);
}
#else
void sdl_bitblt_to_buffer(int _x, int _y, int _w, int _h) {
  Uint32 *src = (Uint32 *)DisplayRegion68k;
  int width = sdl_displaywidth;
  int height = sdl_displayheight;
  int bpw = 8 * sizeof(Uint32);
  int pitch = sdl_displaywidth / bpw;
  int xlimit = (_x + _w + bpw - 1) / bpw;
  int ylimit = _y + _h;
  for (int y = _y; y < ylimit; y++) {
    int they = y * sdl_displaywidth;
    for (int x = _x / bpw; x < xlimit; x++) {
      int srcw = src[y * pitch + x];
      int thex = x * bpw;
      for (int b = 0; b < bpw; b++) {
        uint32_t px = 0;
        if (srcw & (1 << (bpw - 1 - b))) {
          px = sdl_foreground;
        } else {
          px = sdl_background;
        }
        int pxindex = they + thex + b;
        assert(pxindex >= 0 && pxindex < buffer_size);
        ((Uint32 *)buffer)[pxindex] = px;
      }
    }
  }
}
void sdl_bitblt_to_window_surface(int _x, int _y, int _w, int _h) {
  DLword *src = DisplayRegion68k;
  Uint32 *dst = (Uint32 *)sdl_windowsurface->pixels;
  int dstpitchbytes = sdl_windowsurface->pitch;
  int dstpitchpixels = dstpitchbytes / sdl_bytesperpixel;
  const int bitsperword = 8 * sizeof(DLword);
  int sourcepitchwords = sdl_displaywidth / bitsperword;
  int xstart = _x / bitsperword;
  int xlimit = (_x + _w + bitsperword - 1) / bitsperword;
  int ystart = _y * sourcepitchwords;
  int ylimit = (_y + _h) * sourcepitchwords;
  int dy = _y * dstpitchpixels;
  // for each line in the source image
  for (int sy = ystart; sy < ylimit; sy += sourcepitchwords, dy += dstpitchpixels) {
    // for each word in the line
    int dx = (_x / bitsperword) * bitsperword;
    for (int sx = xstart; sx < xlimit; sx++, dx += bitsperword) {
      int srcw = GETBASEWORD(src, sy + sx);
      // for each bit in the word
      for (int b = 0; b < bitsperword; b++) {
        ((Uint32 *)dst)[dy + dx + b] = (srcw & bitmask[b]) ? sdl_foreground : sdl_background;
      }
    }
  }
}
#endif
static int map_key(SDL_Keycode k) {
  for (int i = 0; keymap[i] != -1; i += 2) {
    if (keymap[i + 1] == k) return keymap[i];
  }
  return -1;
}
#define KEYCODE_OFFSET 0
static void handle_keydown(SDL_Keycode k, unsigned short mod) {
  int lk = map_key(k);
  if (lk == -1) {
    printf("No mapping for key %s\n", SDL_GetKeyName(k));
  } else {
    printf("dn %s -> lisp keycode %d (0x%x)\n", SDL_GetKeyName(k), lk, mod);
    kb_trans(lk - KEYCODE_OFFSET, FALSE);
    DoRing();
    if ((KBDEventFlg += 1) > 0) Irq_Stk_End = Irq_Stk_Check = 0;
  }
}
static void handle_keyup(SDL_Keycode k, unsigned short mod) {
  int lk = map_key(k);
  if (lk == -1) {
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
#define KEYSET_MIDDLE 10
#define KEYSET_RIGHTMIDDLE 11
#define KEYSET_RIGHT 12
/* Mouse buttons */
#define MOUSE_LEFT 13
#define MOUSE_RIGHT 14
#define MOUSE_MIDDLE 15
static void sdl_update_viewport(int width, int height) {
  /* XXX: needs work */
  int w = width / 32 * 32;
  if (w > sdl_displaywidth * sdl_pixelscale) w = sdl_displaywidth * sdl_pixelscale;
  int h = height / 32 * 32;
  if (h > sdl_displayheight * sdl_pixelscale) h = sdl_displayheight * sdl_pixelscale;
  SDL_Rect r;
  r.x = 0;
  r.y = 0;
  r.w = w;
  r.h = h;
#if defined(SDLRENDERING)
  SDL_RenderSetViewport(sdl_renderer, &r);
#endif
  printf("new viewport: %d / %d\n", w, h);
}
static int last_keystate[512] = {0};
void sdl_set_invert(int flag) {
  if (flag) {
    sdl_foreground = sdl_white;
    sdl_background = sdl_black;
  } else {
    sdl_foreground = sdl_black;
    sdl_background = sdl_white;
  }
  sdl_notify_damage(0, 0, sdl_displaywidth, sdl_displayheight);
}
void sdl_setMousePosition(int x, int y) {
  SDL_WarpMouseInWindow(sdl_window, x * sdl_pixelscale, y * sdl_pixelscale);
}
#if defined(SDLRENDERING)
void sdl_update_display() {
  sdl_bitblt_to_texture(min_x, min_y, max_x - min_x, max_y - min_y);
  SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
  SDL_RenderPresent(sdl_renderer);
}
#else
void sdl_update_display() {
  SDL_Rect r, s;

  r.x = min_x;
  r.y = min_y;
  r.w = max_x - min_x;
  r.h = max_y - min_y;
  if (sdl_pixelscale == 1) {
    sdl_bitblt_to_window_surface(r.x, r.y, r.w, r.h);
    SDL_UpdateWindowSurfaceRects(sdl_window, &r, 1);
  } else {
    s.x = r.x * sdl_pixelscale;
    s.y = r.y * sdl_pixelscale;
    s.w = r.w * sdl_pixelscale;
    s.h = r.h * sdl_pixelscale;
    sdl_bitblt_to_buffer(r.x, r.y, r.w, r.h);
    SDL_BlitScaled(sdl_buffersurface, &r, sdl_windowsurface, &s);
    SDL_UpdateWindowSurfaceRects(sdl_window, &s, 1);
  }
}
#endif
int process_events_time = 0;
void process_SDLevents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        printf("quitting\n");
        exit(0);
        break;
      case SDL_WINDOWEVENT:
        switch (event.window.event) {
          case SDL_WINDOWEVENT_RESIZED:
            /* XXX: what about integer multiple of 32 requirements here? */
            sdl_windowwidth = event.window.data1;
            sdl_windowheight = event.window.data2;
            sdl_update_viewport(sdl_windowwidth, sdl_windowheight);
            break;
        }
        break;
      case SDL_KEYDOWN:
        printf("dn ts: %x, type: %x, state: %x, repeat: %x, scancode: %x, sym: %x <%s>, mod: %x\n",
               event.key.timestamp, event.key.type, event.key.state, event.key.repeat,
               event.key.keysym.scancode, event.key.keysym.sym,
               SDL_GetKeyName(event.key.keysym.sym), event.key.keysym.mod);
        if (event.key.repeat) {
          /* Lisp needs to see the UP transition before the DOWN transition */
          handle_keyup(event.key.keysym.sym, event.key.keysym.mod);
        }
        handle_keydown(event.key.keysym.sym, event.key.keysym.mod);
        break;
      case SDL_KEYUP:
        printf("up ts: %x, type: %x, state: %x, repeat: %x, scancode: %x, sym: %x <%s>, mod: %x\n",
               event.key.timestamp, event.key.type, event.key.state, event.key.repeat,
               event.key.keysym.scancode, event.key.keysym.sym,
               SDL_GetKeyName(event.key.keysym.sym), event.key.keysym.mod);
        handle_keyup(event.key.keysym.sym, event.key.keysym.mod);
        break;
      case SDL_MOUSEMOTION: {
        int x, y;
        SDL_GetMouseState(&x, &y);
        x /= sdl_pixelscale;
        y /= sdl_pixelscale;
        *CLastUserActionCell68k = MiscStats->secondstmp;
        *EmCursorX68K = (*((DLword *)EmMouseX68K)) = (short)(x & 0xFFFF);
        *EmCursorY68K = (*((DLword *)EmMouseY68K)) = (short)(y & 0xFFFF);
        DoRing();
        if ((KBDEventFlg += 1) > 0) Irq_Stk_End = Irq_Stk_Check = 0;
        break;
      }
      case SDL_MOUSEBUTTONDOWN: {
        int button = event.button.button;
        switch (button) {
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
        break;
      }
      case SDL_MOUSEBUTTONUP: {
        int button = event.button.button;
        switch (button) {
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
        break;
      }
      case SDL_MOUSEWHEEL:
        printf("mousewheel mouse %d x %d y %d direction %s\n", event.wheel.which, event.wheel.x,
               event.wheel.y,
               event.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? "normal" : "flipped");
        break;
        /* case SDL_KEYMAPCHANGED: */
        /*   printf("SDL_KEYMAPCHANGED\n"); break; */
        /* case SDL_TEXTINPUT: */
        /*   printf("SDL_TEXTINPUT\n"); break; */
      default: printf("other event type: %d\n", event.type);
    }
  }
  if (display_update_needed) {
    sdl_update_display();
    display_update_needed = 0;
    min_x = min_y = INT_MAX;
    max_x = max_y = 0;
  }
}

int init_SDL(char *windowtitle, int w, int h, int s) {
  sdl_pixelscale = s;
  // width must be multiple of 32
  w = (w + 31) / 32 * 32;
  sdl_displaywidth = w;
  sdl_displayheight = h;
  sdl_windowwidth = w * s;
  sdl_windowheight = h * s;
  int width = sdl_displaywidth;
  int height = sdl_displayheight;
  printf("requested width: %d, height: %d\n", width, height);
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not be initialized. SDL_Error: %s\n", SDL_GetError());
    return 1;
  }
  printf("initialised\n");
  sdl_window = SDL_CreateWindow(windowtitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                sdl_windowwidth, sdl_windowheight, 0);
  printf("Window created\n");
  if (sdl_window == NULL) {
    printf("Window could not be created. SDL_Error: %s\n", SDL_GetError());
    return 2;
  }
#if defined(SDLRENDERING)
  printf("Creating renderer...\n");
  sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
  if (NULL == sdl_renderer) {
    printf("SDL Error: %s\n", SDL_GetError());
    return 3;
  }
  SDL_GetRendererInfo(sdl_renderer, &sdl_rendererinfo);
  SDL_SetRenderDrawColor(sdl_renderer, 127, 127, 127, 255);
  SDL_RenderClear(sdl_renderer);
  SDL_RenderPresent(sdl_renderer);
  SDL_RenderSetScale(sdl_renderer, 1.0, 1.0);
  printf("Creating texture...\n");
  sdl_pixelformat = SDL_AllocFormat(sdl_rendererinfo.texture_formats[0]);
  sdl_texture = SDL_CreateTexture(sdl_renderer, sdl_pixelformat->format,
                                  SDL_TEXTUREACCESS_STREAMING, width, height);
  sdl_black = SDL_MapRGB(sdl_pixelformat, 0, 0, 0);
  sdl_white = SDL_MapRGB(sdl_pixelformat, 255, 255, 255);
  sdl_foreground = sdl_black;
  sdl_background = sdl_white;
  sdl_bytesperpixel = sdl_pixelformat->BytesPerPixel;
#else
  printf("Creating window surface and buffer surface\n");
  sdl_windowsurface = SDL_GetWindowSurface(sdl_window);
  sdl_pixelformat = sdl_windowsurface->format;
  sdl_black = SDL_MapRGB(sdl_pixelformat, 0, 0, 0);
  sdl_white = SDL_MapRGB(sdl_pixelformat, 255, 255, 255);
  sdl_foreground = sdl_black;
  sdl_background = sdl_white;
  sdl_bytesperpixel = sdl_pixelformat->BytesPerPixel;
  buffer_size = width * height * sdl_bytesperpixel;
  buffer = malloc(buffer_size);
  sdl_buffersurface = SDL_CreateRGBSurfaceWithFormatFrom(
      buffer, sdl_displaywidth, sdl_displayheight, sdl_bytesperpixel * 8,
      sdl_displaywidth * sdl_bytesperpixel, sdl_pixelformat->format);
#endif
  printf("SDL initialised\n");
  return 0;
}
