/*
 * TODO: supports other keyboard layouts than US.
 * FIXME: wrong event coordinates are reported after switching full screen mode.
 */
#include "version.h"
#include <assert.h>
#include <stdbool.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "sdldefs.h"
#include "byteswapdefs.h"
#include "lispemul.h"
#include "lsptypes.h"
#include "keyboard.h"
#include "lspglob.h"  /* for MiscStats */
#include "display.h"  /* for CURSORWIDTH, CURSORHEIGHT, DisplayRegion68k */

#if SDL == 3
#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_version.h>
#else
#error SDL version number must be 3.
#endif
#if SDL_MINOR_VERSION < 4
#error requires SDL >= 3.4.0
#endif


/*
 * Interface to other subsystems.
 */
void sdl_notify_damage(int x, int y, int w, int h);
void sdl_setCursor(int hot_x, int hot_y);
void sdl_set_invert(int flag);
void sdl_setMousePosition(int x, int y);
void process_Display_events(void *dsp);
int init_SDL(char *windowtitle, int w, int h, int s);

int sdl_displaywidth;
int sdl_displayheight;


/*
 * Imported functions and variables.
 */
extern void kb_trans(u_short keycode, u_short upflg);     /* keyevent.c */
extern int error(const char *s);                          /* common.c */

extern int KBDEventFlg;                                   /* keyevent.c */
extern DLword *EmKbdAd068K;                               /* initkbd.c */
extern DLword *EmKbdAd168K;                               /* initkbd.c */
extern DLword *EmKbdAd268K;                               /* initkbd.c */
extern DLword *EmKbdAd368K;                               /* initkbd.c */
extern DLword *EmKbdAd468K;                               /* initkbd.c */
extern DLword *EmKbdAd568K;                               /* initkbd.c */
extern DLword *EmRealUtilin68K;                           /* initkbd.c */
extern DLword *EmCursorBitMap68K;                         /* initkbd.c */
extern DLword *CTopKeyevent;                              /* keyevent.c */
extern int URaid_req;                                     /* keyevent.c */
extern LispPTR *KEYBUFFERING68k;                          /* keyevent.c */
extern char foregroundColorName[64];                      /* main.c */
extern char backgroundColorName[64];                      /* main.c */
extern DLword *EmCursorX68K;                              /* initkbd.c */
extern DLword *EmCursorY68K;                              /* initkbd.c */
extern DLword *EmMouseX68K;                               /* initkbd.c */
extern DLword *EmMouseY68K;                               /* initkbd.c */
extern LispPTR *CLastUserActionCell68k;                   /* keyevent.c */


/*
 * Macros for development
 */
#if 0
#define JIS 1
#define SELENIZED 1
#endif

#ifdef SELENIZED
#  define BG_COLORNAME "#fcf4dc"
#  define FG_COLORNAME "#52676f"
#else
#  define BG_COLORNAME "white"
#  define FG_COLORNAME "black"
#endif


/*
 * Compiler directive
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ == 202311L
#  define ARGUSED [[maybe_unused]]
#elif defined(__GNUC__)
#  define ARGUSED  __attribute__((unused))
#else
#  define ARGUSED
#endif


/*
 * Diagnostic Macros
 */
#define DIAG_STREAM stderr

#define DIAG_OUT(fmt, ...)                                                    \
   do {                                                                       \
     (void)fprintf(DIAG_STREAM, fmt __VA_OPT__(,) __VA_ARGS__);               \
     (void)fflush(DIAG_STREAM);                                               \
   } while(false)

#define DIAG_SDL(fmt, ...)                                                    \
   DIAG_OUT(fmt ": %s\n", __VA_OPT__(,) __VA_ARGS__ SDL_GetError())

#define GUARD_SDL(call, retval, fmt, ...)                                     \
   do {                                                                       \
     if (!(call)) {                                                           \
       DIAG_SDL(fmt, __VA_OPT__(,) __VA_ARGS__);                              \
       return retval;                                                         \
     }                                                                        \
   } while (false)


/*
 * Constants, structs, and file scope variables
 */
#define KEYCODE_OFFSET 0

#define KEYSET_LEFT 8
#define KEYSET_LEFTMIDDLE 9
#define KEYSET_MIDDLE 10
#define KEYSET_RIGHTMIDDLE 11
#define KEYSET_RIGHT 12

/* Mouse buttons */
#define MOUSE_LEFT 13
#define MOUSE_RIGHT 14
#define MOUSE_MIDDLE 15

/* Cursor */
struct CachedCursor {
  struct CachedCursor *next;
  DLword EmCursorBitMap[CURSORHEIGHT];
  SDL_Cursor *cursor;
};

/* The pixel format of the framebuffer is 1-bit index color, but modern GPUs do
 * not support it.  Uses 8-bit index color format instead. */
#define PIXELFORMAT SDL_PIXELFORMAT_INDEX8

/* Colors and their names */
#define NCOLORS 2
#define INDEX_BGCOLOR 0
#define INDEX_FGCOLOR 1
static const char *palette_names[NCOLORS];
struct ColorNameToRGB {
  char * name;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

/* Core data of SDL display */
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;

/* Regional update of display */
static bool damage_f;
static int min_x, min_y, max_x, max_y;

/* Misc flags */
static bool window_focus_f;
static bool capslock_f;

/* clang-format off */
static int keymap[] = {
  0, SDLK_5,                                     /* (5 %% FIVE) */
  1, SDLK_4,                                     /* (4 $ FOUR) */
  2, SDLK_6,                                     /* (6 ~ SIX) */
  3, SDLK_E,                                     /* (e E) */
  4, SDLK_7,                                     /* (7 & SEVEN) */
  5, SDLK_D,                                     /* (d D) */
  6, SDLK_U,                                     /* (u U) */
  7, SDLK_V,                                     /* (v V) */
  8, SDLK_RIGHTPAREN, 8, SDLK_0,                 /* (0 %) ZERO) */
  9, SDLK_K,                                     /* (k K) */
  10, SDLK_MINUS,                                /* (- %^X) */
  11, SDLK_P,                                    /* (p P) */
  12, SDLK_SLASH,                                /* (/ ?) */
  13, SDLK_KP_PERIOD,                            /* (\ %| FONT LOOKS) */
  14, SDLK_SCROLLLOCK,                           /* (LF SAME) */
  15, SDLK_BACKSPACE,                            /* (BS <-) */
  16, SDLK_3,                                    /* (3 %# THREE) */
  17, SDLK_2,                                    /* (2 @ TWO) */
  18, SDLK_W,                                    /* (w W) */
  19, SDLK_Q,                                    /* (q Q) */
  20, SDLK_S,                                    /* (s S) */
  21, SDLK_A,                                    /* (a A) */
  22, SDLK_LEFTPAREN,  22, SDLK_9,               /* (9 %( NINE) */
  23, SDLK_I,                                    /* (i I) */
  24, SDLK_X,                                    /* (x X) */
  25, SDLK_O,                                    /* (o O) */
  26, SDLK_L,                                    /* (l L) */
  27, SDLK_COMMA,                                /* (%, <) */
#ifndef JIS
  28, SDLK_APOSTROPHE,                           /* (%' %") */
  29, SDLK_RIGHTBRACKET,                         /* (%] }) */
#else
  28, SDLK_COLON,                                /* (: *) */
  29, SDLK_LEFTBRACKET,                          /* (%[ {) */
#endif
  // 30,                                         /* (BLANK-MIDDLE OPEN DBK-HELP) */
  31, SDLK_LALT, /* Meta, Sun-4 usual key */     /* (BLANK-TOP KEYBOARD DBK-META) */
  32, SDLK_1,                                    /* (1 ! ONE) */
  33, SDLK_ESCAPE,                               /* (ESC ESCAPE ->) */
  34, SDLK_TAB,                                  /* (TAB =>) */
  35, SDLK_F,                                    /* (f F) */
  36, SDLK_LCTRL,                                /* (CTRL PROP'S EDIT) */
  37, SDLK_C,                                    /* (c C) */
  38, SDLK_J,                                    /* (j J) */
  39, SDLK_B,                                    /* (b B) */
  40, SDLK_Z,                                    /* (z Z) */
  41, SDLK_LSHIFT,                               /* (LSHIFT) */
  42, SDLK_PERIOD,                               /* (%. >) */
  43, SDLK_SEMICOLON,  43, SDLK_COLON,           /* (; %:) */
  44, SDLK_RETURN,                               /* (CR <-%|) */
#ifndef JIS
  45, SDLK_GRAVE,                                /* (_ ^) */
#else
  45,SDLK_RIGHTBRACKET,                          /* (] }) */
#endif
  // 46,                                         /* (DEL DELETE) */
  47, SDLK_RCTRL,                                /* (SKIP NEXT) */
  48, SDLK_R,                                    /* (r R) */
  49, SDLK_T,                                    /* (t T) */
  50, SDLK_G,                                    /* (g G) */
  51, SDLK_Y,                                    /* (y Y) */
  52, SDLK_H,                                    /* (h H) */
  53, SDLK_8,                                    /* (8 * EIGHT) */
  54, SDLK_N,                                    /* (n N) */
  55, SDLK_M,                                    /* (m M) */
  56, SDLK_CAPSLOCK,                             /* (LOCK) */
  57, SDLK_SPACE,                                /* (SPACE) */
#ifndef JIS
  58, SDLK_LEFTBRACKET,                          /* (%[ {) */
  59, SDLK_EQUALS,                               /* (= +) */
#else
  58, SDLK_AT,                                   /* (@ `) JIS */
  59, SDLK_CARET,                                /* (^ ~ JIS */
#endif
  60, SDLK_RSHIFT,                               /* (RSHIFT) */
  61, SDLK_F11,  61, SDLK_PAUSE,                 /* (BLANK-BOTTOM STOP) */
  62, SDLK_HOME,                                 /* (MOVE) */
  63, SDLK_PAGEUP,                               /* (UNDO) */
  64, SDLK_KP_EQUALS,                            /* (UTIL0 SUN-KEYPAD=) */
  65, SDLK_KP_DIVIDE,                            /* (UTIL1 SUN-KEYPAD/) */
  66, SDLK_F7,                                   /* (UTIL2 SUPER/SUB) */
  67, SDLK_F4,                                   /* (UTIL3 CASE) */
  68, SDLK_F5,                                   /* (UTIL4 STRIKEOUT) */
  69, SDLK_KP_2,                                 /* (UTIL5 KEYPAD2) */
  70, SDLK_KP_3,                                 /* (UTIL6 KEYPAD3 PGDN) */
  // 71, XK_Linefeed,                            /* (UTIL7 SUN-LF) */
  // 72,                                         /* (PAD1 LEFTKEY CAPSLOCK KEYPAD+) */
  // 73, XK_Numlock,                             /* (PAD2 LEFTMIDDLEKEY NUMLOCK KEYPAD-) */
  // 74,                                         /* (PAD3 MIDDLEKEY SCROLLLOCK KEYPAD*) */
  // 75,                                         /* (PAD4 RIGHTMIDDLEKEY BREAK KEYPAD/ SUN-PAUSE) */
  76, SDLK_KP_ENTER,                             /* (PAD5 RIGHTKEY DOIT PRTSC) */
  // 77,                                         /* (LEFT RED MOUSERED) */
  // 78,                                         /* (RIGHT BLUE MOUSEBLUE) */
  // 79,                                         /* (MIDDLE YELLOW MOUSEYELLOW) */
  80, SDLK_F9,                                   /* (MARGINS) */
  81, SDLK_KP_7,                                 /* (K41 KEYPAD7 HOME) */
  82, SDLK_KP_8,                                 /* (K42 KEYPAD8) */
  83, SDLK_KP_9,                                 /* (K43 KEYPAD9 PGUP) */
  84, SDLK_KP_4,                                 /* (K44 KEYPAD4) */
  85, SDLK_KP_5,                                 /* (K45 KEYPAD5) */
  86, SDLK_LALT, /* (sun left-diamond key) */    /* (K46 SUN-LEFT-SPACE) */
  87, SDLK_KP_6,                                 /* (K47 KEYPAD6) */
  // 88,                                         /* (K48 RIGHT-COMMAND SUN-RIGHT-SPACE) */
  89, SDLK_INSERT,                               /* (COPY) */
  90, SDLK_END,                                  /* (FIND) */
  91, SDLK_F12,                                  /* (AGAIN) */
  92, SDLK_PRINTSCREEN, // is this XK_Print??    /* (HELP) */
  93, SDLK_MODE, // is this XK_Mode_switch       /* (DEF'N EXPAND) */
  94, SDLK_KP_1,                                 /* (K4E KEYPAD1 END) */
  95, SDLK_KP_MULTIPLY,                          /* (ALWAYS-ON-1) */
  96, SDLK_KP_MINUS,                             /* (ALWAYS-ON-2) */
  97, SDLK_HELP,                                 /* (CENTER) */
  98, SDLK_KP_0,                                 /* (K52 KEYPAD0 INS) */
  99, SDLK_F2,                                   /* (BOLD) */
  100, SDLK_F3,                                  /* (ITALICS) */
  101, SDLK_F6,                                  /* (UNDERLINE) */
  102, SDLK_KP_PLUS,                             /* (SUPERSCRIPT) */
  //  103,                                       /* (SUBSCRIPT) */
  104, SDLK_F8,                                  /* (LARGER SMALLER) */
#ifndef JIS
  105, SDLK_BACKSLASH,                           /* (K59 KEYPAD%| KEYPAD.) */
#else
  105, 165,                                      /* (Yen |) */
#endif
  106, SDLK_F10,                                 /* (K5A KEYPAD\ KEYPAD, SUN-F10) */
  107, SDLK_F11,                                 /* (K5B SUN-F11) */
  108, SDLK_F12,                                 /* (K5C SUN-F12) */
  // 109,                                        /* (DEFAULTS SUN-PROP) */
  // 110,                                        /* (K5E SUN-PRTSC) */
  // 111,                                        /* (K5F SUN-OPEN) */
  -1, -1
};

static const struct ColorNameToRGB colornames[] = {
{"alice blue",  240, 248, 255},
{"AliceBlue",  240, 248, 255},
{"antique white",  250, 235, 215},
{"AntiqueWhite",  250, 235, 215},
{"AntiqueWhite1",  255, 239, 219},
{"AntiqueWhite2",  238, 223, 204},
{"AntiqueWhite3",  205, 192, 176},
{"AntiqueWhite4",  139, 131, 120},
{"aquamarine",  127, 255, 212},
{"aquamarine1",  127, 255, 212},
{"aquamarine2",  118, 238, 198},
{"aquamarine3",  102, 205, 170},
{"aquamarine4",  69, 139, 116},
{"azure",  240, 255, 255},
{"azure1",  240, 255, 255},
{"azure2",  224, 238, 238},
{"azure3",  193, 205, 205},
{"azure4",  131, 139, 139},
{"beige",  245, 245, 220},
{"bisque",  255, 228, 196},
{"bisque1",  255, 228, 196},
{"bisque2",  238, 213, 183},
{"bisque3",  205, 183, 158},
{"bisque4",  139, 125, 107},
{"black",  0, 0, 0},
{"blanched almond",  255, 235, 205},
{"BlanchedAlmond",  255, 235, 205},
{"blue",  0, 0, 255},
{"blue violet",  138, 43, 226},
{"blue1",  0, 0, 255},
{"blue2",  0, 0, 238},
{"blue3",  0, 0, 205},
{"blue4",  0, 0, 139},
{"BlueViolet",  138, 43, 226},
{"brown",  165, 42, 42},
{"brown1",  255, 64, 64},
{"brown2",  238, 59, 59},
{"brown3",  205, 51, 51},
{"brown4",  139, 35, 35},
{"burlywood",  222, 184, 135},
{"burlywood1",  255, 211, 155},
{"burlywood2",  238, 197, 145},
{"burlywood3",  205, 170, 125},
{"burlywood4",  139, 115, 85},
{"cadet blue",  95, 158, 160},
{"CadetBlue",  95, 158, 160},
{"CadetBlue1",  152, 245, 255},
{"CadetBlue2",  142, 229, 238},
{"CadetBlue3",  122, 197, 205},
{"CadetBlue4",  83, 134, 139},
{"chartreuse",  127, 255, 0},
{"chartreuse1",  127, 255, 0},
{"chartreuse2",  118, 238, 0},
{"chartreuse3",  102, 205, 0},
{"chartreuse4",  69, 139, 0},
{"chocolate",  210, 105, 30},
{"chocolate1",  255, 127, 36},
{"chocolate2",  238, 118, 33},
{"chocolate3",  205, 102, 29},
{"chocolate4",  139, 69, 19},
{"coral",  255, 127, 80},
{"coral1",  255, 114, 86},
{"coral2",  238, 106, 80},
{"coral3",  205, 91, 69},
{"coral4",  139, 62, 47},
{"cornflower blue",  100, 149, 237},
{"CornflowerBlue",  100, 149, 237},
{"cornsilk",  255, 248, 220},
{"cornsilk1",  255, 248, 220},
{"cornsilk2",  238, 232, 205},
{"cornsilk3",  205, 200, 177},
{"cornsilk4",  139, 136, 120},
{"cyan",  0, 255, 255},
{"cyan1",  0, 255, 255},
{"cyan2",  0, 238, 238},
{"cyan3",  0, 205, 205},
{"cyan4",  0, 139, 139},
{"dark blue",  0, 0, 139},
{"dark cyan",  0, 139, 139},
{"dark goldenrod",  184, 134, 11},
{"dark gray",  169, 169, 169},
{"dark green",  0, 100, 0},
{"dark grey",  169, 169, 169},
{"dark khaki",  189, 183, 107},
{"dark magenta",  139, 0, 139},
{"dark olive green",  85, 107, 47},
{"dark orange",  255, 140, 0},
{"dark orchid",  153, 50, 204},
{"dark red",  139, 0, 0},
{"dark salmon",  233, 150, 122},
{"dark sea green",  143, 188, 143},
{"dark slate blue",  72, 61, 139},
{"dark slate gray",  47, 79, 79},
{"dark slate grey",  47, 79, 79},
{"dark turquoise",  0, 206, 209},
{"dark violet",  148, 0, 211},
{"DarkBlue",  0, 0, 139},
{"DarkCyan",  0, 139, 139},
{"DarkGoldenrod",  184, 134, 11},
{"DarkGoldenrod1",  255, 185, 15},
{"DarkGoldenrod2",  238, 173, 14},
{"DarkGoldenrod3",  205, 149, 12},
{"DarkGoldenrod4",  139, 101, 8},
{"DarkGray",  169, 169, 169},
{"DarkGreen",  0, 100, 0},
{"DarkGrey",  169, 169, 169},
{"DarkKhaki",  189, 183, 107},
{"DarkMagenta",  139, 0, 139},
{"DarkOliveGreen",  85, 107, 47},
{"DarkOliveGreen1",  202, 255, 112},
{"DarkOliveGreen2",  188, 238, 104},
{"DarkOliveGreen3",  162, 205, 90},
{"DarkOliveGreen4",  110, 139, 61},
{"DarkOrange",  255, 140, 0},
{"DarkOrange1",  255, 127, 0},
{"DarkOrange2",  238, 118, 0},
{"DarkOrange3",  205, 102, 0},
{"DarkOrange4",  139, 69, 0},
{"DarkOrchid",  153, 50, 204},
{"DarkOrchid1",  191, 62, 255},
{"DarkOrchid2",  178, 58, 238},
{"DarkOrchid3",  154, 50, 205},
{"DarkOrchid4",  104, 34, 139},
{"DarkRed",  139, 0, 0},
{"DarkSalmon",  233, 150, 122},
{"DarkSeaGreen",  143, 188, 143},
{"DarkSeaGreen1",  193, 255, 193},
{"DarkSeaGreen2",  180, 238, 180},
{"DarkSeaGreen3",  155, 205, 155},
{"DarkSeaGreen4",  105, 139, 105},
{"DarkSlateBlue",  72, 61, 139},
{"DarkSlateGray",  47, 79, 79},
{"DarkSlateGray1",  151, 255, 255},
{"DarkSlateGray2",  141, 238, 238},
{"DarkSlateGray3",  121, 205, 205},
{"DarkSlateGray4",  82, 139, 139},
{"DarkSlateGrey",  47, 79, 79},
{"DarkTurquoise",  0, 206, 209},
{"DarkViolet",  148, 0, 211},
{"deep pink",  255, 20, 147},
{"deep sky blue",  0, 191, 255},
{"DeepPink",  255, 20, 147},
{"DeepPink1",  255, 20, 147},
{"DeepPink2",  238, 18, 137},
{"DeepPink3",  205, 16, 118},
{"DeepPink4",  139, 10, 80},
{"DeepSkyBlue",  0, 191, 255},
{"DeepSkyBlue1",  0, 191, 255},
{"DeepSkyBlue2",  0, 178, 238},
{"DeepSkyBlue3",  0, 154, 205},
{"DeepSkyBlue4",  0, 104, 139},
{"dim gray",  105, 105, 105},
{"dim grey",  105, 105, 105},
{"DimGray",  105, 105, 105},
{"DimGrey",  105, 105, 105},
{"dodger blue",  30, 144, 255},
{"DodgerBlue",  30, 144, 255},
{"DodgerBlue1",  30, 144, 255},
{"DodgerBlue2",  28, 134, 238},
{"DodgerBlue3",  24, 116, 205},
{"DodgerBlue4",  16, 78, 139},
{"firebrick",  178, 34, 34},
{"firebrick1",  255, 48, 48},
{"firebrick2",  238, 44, 44},
{"firebrick3",  205, 38, 38},
{"firebrick4",  139, 26, 26},
{"floral white",  255, 250, 240},
{"FloralWhite",  255, 250, 240},
{"forest green",  34, 139, 34},
{"ForestGreen",  34, 139, 34},
{"gainsboro",  220, 220, 220},
{"ghost white",  248, 248, 255},
{"GhostWhite",  248, 248, 255},
{"gold",  255, 215, 0},
{"gold1",  255, 215, 0},
{"gold2",  238, 201, 0},
{"gold3",  205, 173, 0},
{"gold4",  139, 117, 0},
{"goldenrod",  218, 165, 32},
{"goldenrod1",  255, 193, 37},
{"goldenrod2",  238, 180, 34},
{"goldenrod3",  205, 155, 29},
{"goldenrod4",  139, 105, 20},
{"gray",  190, 190, 190},
{"gray0",  0, 0, 0},
{"gray1",  3, 3, 3},
{"gray10",  26, 26, 26},
{"gray100",  255, 255, 255},
{"gray11",  28, 28, 28},
{"gray12",  31, 31, 31},
{"gray13",  33, 33, 33},
{"gray14",  36, 36, 36},
{"gray15",  38, 38, 38},
{"gray16",  41, 41, 41},
{"gray17",  43, 43, 43},
{"gray18",  46, 46, 46},
{"gray19",  48, 48, 48},
{"gray2",  5, 5, 5},
{"gray20",  51, 51, 51},
{"gray21",  54, 54, 54},
{"gray22",  56, 56, 56},
{"gray23",  59, 59, 59},
{"gray24",  61, 61, 61},
{"gray25",  64, 64, 64},
{"gray26",  66, 66, 66},
{"gray27",  69, 69, 69},
{"gray28",  71, 71, 71},
{"gray29",  74, 74, 74},
{"gray3",  8, 8, 8},
{"gray30",  77, 77, 77},
{"gray31",  79, 79, 79},
{"gray32",  82, 82, 82},
{"gray33",  84, 84, 84},
{"gray34",  87, 87, 87},
{"gray35",  89, 89, 89},
{"gray36",  92, 92, 92},
{"gray37",  94, 94, 94},
{"gray38",  97, 97, 97},
{"gray39",  99, 99, 99},
{"gray4",  10, 10, 10},
{"gray40",  102, 102, 102},
{"gray41",  105, 105, 105},
{"gray42",  107, 107, 107},
{"gray43",  110, 110, 110},
{"gray44",  112, 112, 112},
{"gray45",  115, 115, 115},
{"gray46",  117, 117, 117},
{"gray47",  120, 120, 120},
{"gray48",  122, 122, 122},
{"gray49",  125, 125, 125},
{"gray5",  13, 13, 13},
{"gray50",  127, 127, 127},
{"gray51",  130, 130, 130},
{"gray52",  133, 133, 133},
{"gray53",  135, 135, 135},
{"gray54",  138, 138, 138},
{"gray55",  140, 140, 140},
{"gray56",  143, 143, 143},
{"gray57",  145, 145, 145},
{"gray58",  148, 148, 148},
{"gray59",  150, 150, 150},
{"gray6",  15, 15, 15},
{"gray60",  153, 153, 153},
{"gray61",  156, 156, 156},
{"gray62",  158, 158, 158},
{"gray63",  161, 161, 161},
{"gray64",  163, 163, 163},
{"gray65",  166, 166, 166},
{"gray66",  168, 168, 168},
{"gray67",  171, 171, 171},
{"gray68",  173, 173, 173},
{"gray69",  176, 176, 176},
{"gray7",  18, 18, 18},
{"gray70",  179, 179, 179},
{"gray71",  181, 181, 181},
{"gray72",  184, 184, 184},
{"gray73",  186, 186, 186},
{"gray74",  189, 189, 189},
{"gray75",  191, 191, 191},
{"gray76",  194, 194, 194},
{"gray77",  196, 196, 196},
{"gray78",  199, 199, 199},
{"gray79",  201, 201, 201},
{"gray8",  20, 20, 20},
{"gray80",  204, 204, 204},
{"gray81",  207, 207, 207},
{"gray82",  209, 209, 209},
{"gray83",  212, 212, 212},
{"gray84",  214, 214, 214},
{"gray85",  217, 217, 217},
{"gray86",  219, 219, 219},
{"gray87",  222, 222, 222},
{"gray88",  224, 224, 224},
{"gray89",  227, 227, 227},
{"gray9",  23, 23, 23},
{"gray90",  229, 229, 229},
{"gray91",  232, 232, 232},
{"gray92",  235, 235, 235},
{"gray93",  237, 237, 237},
{"gray94",  240, 240, 240},
{"gray95",  242, 242, 242},
{"gray96",  245, 245, 245},
{"gray97",  247, 247, 247},
{"gray98",  250, 250, 250},
{"gray99",  252, 252, 252},
{"green",  0, 255, 0},
{"green yellow",  173, 255, 47},
{"green1",  0, 255, 0},
{"green2",  0, 238, 0},
{"green3",  0, 205, 0},
{"green4",  0, 139, 0},
{"GreenYellow",  173, 255, 47},
{"grey",  190, 190, 190},
{"grey0",  0, 0, 0},
{"grey1",  3, 3, 3},
{"grey10",  26, 26, 26},
{"grey100",  255, 255, 255},
{"grey11",  28, 28, 28},
{"grey12",  31, 31, 31},
{"grey13",  33, 33, 33},
{"grey14",  36, 36, 36},
{"grey15",  38, 38, 38},
{"grey16",  41, 41, 41},
{"grey17",  43, 43, 43},
{"grey18",  46, 46, 46},
{"grey19",  48, 48, 48},
{"grey2",  5, 5, 5},
{"grey20",  51, 51, 51},
{"grey21",  54, 54, 54},
{"grey22",  56, 56, 56},
{"grey23",  59, 59, 59},
{"grey24",  61, 61, 61},
{"grey25",  64, 64, 64},
{"grey26",  66, 66, 66},
{"grey27",  69, 69, 69},
{"grey28",  71, 71, 71},
{"grey29",  74, 74, 74},
{"grey3",  8, 8, 8},
{"grey30",  77, 77, 77},
{"grey31",  79, 79, 79},
{"grey32",  82, 82, 82},
{"grey33",  84, 84, 84},
{"grey34",  87, 87, 87},
{"grey35",  89, 89, 89},
{"grey36",  92, 92, 92},
{"grey37",  94, 94, 94},
{"grey38",  97, 97, 97},
{"grey39",  99, 99, 99},
{"grey4",  10, 10, 10},
{"grey40",  102, 102, 102},
{"grey41",  105, 105, 105},
{"grey42",  107, 107, 107},
{"grey43",  110, 110, 110},
{"grey44",  112, 112, 112},
{"grey45",  115, 115, 115},
{"grey46",  117, 117, 117},
{"grey47",  120, 120, 120},
{"grey48",  122, 122, 122},
{"grey49",  125, 125, 125},
{"grey5",  13, 13, 13},
{"grey50",  127, 127, 127},
{"grey51",  130, 130, 130},
{"grey52",  133, 133, 133},
{"grey53",  135, 135, 135},
{"grey54",  138, 138, 138},
{"grey55",  140, 140, 140},
{"grey56",  143, 143, 143},
{"grey57",  145, 145, 145},
{"grey58",  148, 148, 148},
{"grey59",  150, 150, 150},
{"grey6",  15, 15, 15},
{"grey60",  153, 153, 153},
{"grey61",  156, 156, 156},
{"grey62",  158, 158, 158},
{"grey63",  161, 161, 161},
{"grey64",  163, 163, 163},
{"grey65",  166, 166, 166},
{"grey66",  168, 168, 168},
{"grey67",  171, 171, 171},
{"grey68",  173, 173, 173},
{"grey69",  176, 176, 176},
{"grey7",  18, 18, 18},
{"grey70",  179, 179, 179},
{"grey71",  181, 181, 181},
{"grey72",  184, 184, 184},
{"grey73",  186, 186, 186},
{"grey74",  189, 189, 189},
{"grey75",  191, 191, 191},
{"grey76",  194, 194, 194},
{"grey77",  196, 196, 196},
{"grey78",  199, 199, 199},
{"grey79",  201, 201, 201},
{"grey8",  20, 20, 20},
{"grey80",  204, 204, 204},
{"grey81",  207, 207, 207},
{"grey82",  209, 209, 209},
{"grey83",  212, 212, 212},
{"grey84",  214, 214, 214},
{"grey85",  217, 217, 217},
{"grey86",  219, 219, 219},
{"grey87",  222, 222, 222},
{"grey88",  224, 224, 224},
{"grey89",  227, 227, 227},
{"grey9",  23, 23, 23},
{"grey90",  229, 229, 229},
{"grey91",  232, 232, 232},
{"grey92",  235, 235, 235},
{"grey93",  237, 237, 237},
{"grey94",  240, 240, 240},
{"grey95",  242, 242, 242},
{"grey96",  245, 245, 245},
{"grey97",  247, 247, 247},
{"grey98",  250, 250, 250},
{"grey99",  252, 252, 252},
{"honeydew",  240, 255, 240},
{"honeydew1",  240, 255, 240},
{"honeydew2",  224, 238, 224},
{"honeydew3",  193, 205, 193},
{"honeydew4",  131, 139, 131},
{"hot pink",  255, 105, 180},
{"HotPink",  255, 105, 180},
{"HotPink1",  255, 110, 180},
{"HotPink2",  238, 106, 167},
{"HotPink3",  205, 96, 144},
{"HotPink4",  139, 58, 98},
{"indian red",  205, 92, 92},
{"IndianRed",  205, 92, 92},
{"IndianRed1",  255, 106, 106},
{"IndianRed2",  238, 99, 99},
{"IndianRed3",  205, 85, 85},
{"IndianRed4",  139, 58, 58},
{"ivory",  255, 255, 240},
{"ivory1",  255, 255, 240},
{"ivory2",  238, 238, 224},
{"ivory3",  205, 205, 193},
{"ivory4",  139, 139, 131},
{"khaki",  240, 230, 140},
{"khaki1",  255, 246, 143},
{"khaki2",  238, 230, 133},
{"khaki3",  205, 198, 115},
{"khaki4",  139, 134, 78},
{"lavender",  230, 230, 250},
{"lavender blush",  255, 240, 245},
{"LavenderBlush",  255, 240, 245},
{"LavenderBlush1",  255, 240, 245},
{"LavenderBlush2",  238, 224, 229},
{"LavenderBlush3",  205, 193, 197},
{"LavenderBlush4",  139, 131, 134},
{"lawn green",  124, 252, 0},
{"LawnGreen",  124, 252, 0},
{"lemon chiffon",  255, 250, 205},
{"LemonChiffon",  255, 250, 205},
{"LemonChiffon1",  255, 250, 205},
{"LemonChiffon2",  238, 233, 191},
{"LemonChiffon3",  205, 201, 165},
{"LemonChiffon4",  139, 137, 112},
{"light blue",  173, 216, 230},
{"light coral",  240, 128, 128},
{"light cyan",  224, 255, 255},
{"light goldenrod",  238, 221, 130},
{"light goldenrod yellow",  250, 250, 210},
{"light gray",  211, 211, 211},
{"light green",  144, 238, 144},
{"light grey",  211, 211, 211},
{"light pink",  255, 182, 193},
{"light salmon",  255, 160, 122},
{"light sea green",  32, 178, 170},
{"light sky blue",  135, 206, 250},
{"light slate blue",  132, 112, 255},
{"light slate gray",  119, 136, 153},
{"light slate grey",  119, 136, 153},
{"light steel blue",  176, 196, 222},
{"light yellow",  255, 255, 224},
{"LightBlue",  173, 216, 230},
{"LightBlue1",  191, 239, 255},
{"LightBlue2",  178, 223, 238},
{"LightBlue3",  154, 192, 205},
{"LightBlue4",  104, 131, 139},
{"LightCoral",  240, 128, 128},
{"LightCyan",  224, 255, 255},
{"LightCyan1",  224, 255, 255},
{"LightCyan2",  209, 238, 238},
{"LightCyan3",  180, 205, 205},
{"LightCyan4",  122, 139, 139},
{"LightGoldenrod",  238, 221, 130},
{"LightGoldenrod1",  255, 236, 139},
{"LightGoldenrod2",  238, 220, 130},
{"LightGoldenrod3",  205, 190, 112},
{"LightGoldenrod4",  139, 129, 76},
{"LightGoldenrodYellow",  250, 250, 210},
{"LightGray",  211, 211, 211},
{"LightGreen",  144, 238, 144},
{"LightGrey",  211, 211, 211},
{"LightPink",  255, 182, 193},
{"LightPink1",  255, 174, 185},
{"LightPink2",  238, 162, 173},
{"LightPink3",  205, 140, 149},
{"LightPink4",  139, 95, 101},
{"LightSalmon",  255, 160, 122},
{"LightSalmon1",  255, 160, 122},
{"LightSalmon2",  238, 149, 114},
{"LightSalmon3",  205, 129, 98},
{"LightSalmon4",  139, 87, 66},
{"LightSeaGreen",  32, 178, 170},
{"LightSkyBlue",  135, 206, 250},
{"LightSkyBlue1",  176, 226, 255},
{"LightSkyBlue2",  164, 211, 238},
{"LightSkyBlue3",  141, 182, 205},
{"LightSkyBlue4",  96, 123, 139},
{"LightSlateBlue",  132, 112, 255},
{"LightSlateGray",  119, 136, 153},
{"LightSlateGrey",  119, 136, 153},
{"LightSteelBlue",  176, 196, 222},
{"LightSteelBlue1",  202, 225, 255},
{"LightSteelBlue2",  188, 210, 238},
{"LightSteelBlue3",  162, 181, 205},
{"LightSteelBlue4",  110, 123, 139},
{"LightYellow",  255, 255, 224},
{"LightYellow1",  255, 255, 224},
{"LightYellow2",  238, 238, 209},
{"LightYellow3",  205, 205, 180},
{"LightYellow4",  139, 139, 122},
{"lime green",  50, 205, 50},
{"LimeGreen",  50, 205, 50},
{"linen",  250, 240, 230},
{"magenta",  255, 0, 255},
{"magenta1",  255, 0, 255},
{"magenta2",  238, 0, 238},
{"magenta3",  205, 0, 205},
{"magenta4",  139, 0, 139},
{"maroon",  176, 48, 96},
{"maroon1",  255, 52, 179},
{"maroon2",  238, 48, 167},
{"maroon3",  205, 41, 144},
{"maroon4",  139, 28, 98},
{"medium aquamarine",  102, 205, 170},
{"medium blue",  0, 0, 205},
{"medium orchid",  186, 85, 211},
{"medium purple",  147, 112, 219},
{"medium sea green",  60, 179, 113},
{"medium slate blue",  123, 104, 238},
{"medium spring green",  0, 250, 154},
{"medium turquoise",  72, 209, 204},
{"medium violet red",  199, 21, 133},
{"MediumAquamarine",  102, 205, 170},
{"MediumBlue",  0, 0, 205},
{"MediumOrchid",  186, 85, 211},
{"MediumOrchid1",  224, 102, 255},
{"MediumOrchid2",  209, 95, 238},
{"MediumOrchid3",  180, 82, 205},
{"MediumOrchid4",  122, 55, 139},
{"MediumPurple",  147, 112, 219},
{"MediumPurple1",  171, 130, 255},
{"MediumPurple2",  159, 121, 238},
{"MediumPurple3",  137, 104, 205},
{"MediumPurple4",  93, 71, 139},
{"MediumSeaGreen",  60, 179, 113},
{"MediumSlateBlue",  123, 104, 238},
{"MediumSpringGreen",  0, 250, 154},
{"MediumTurquoise",  72, 209, 204},
{"MediumVioletRed",  199, 21, 133},
{"midnight blue",  25, 25, 112},
{"MidnightBlue",  25, 25, 112},
{"mint cream",  245, 255, 250},
{"MintCream",  245, 255, 250},
{"misty rose",  255, 228, 225},
{"MistyRose",  255, 228, 225},
{"MistyRose1",  255, 228, 225},
{"MistyRose2",  238, 213, 210},
{"MistyRose3",  205, 183, 181},
{"MistyRose4",  139, 125, 123},
{"moccasin",  255, 228, 181},
{"navajo white",  255, 222, 173},
{"NavajoWhite",  255, 222, 173},
{"NavajoWhite1",  255, 222, 173},
{"NavajoWhite2",  238, 207, 161},
{"NavajoWhite3",  205, 179, 139},
{"NavajoWhite4",  139, 121, 94},
{"navy",  0, 0, 128},
{"navy blue",  0, 0, 128},
{"NavyBlue",  0, 0, 128},
{"old lace",  253, 245, 230},
{"OldLace",  253, 245, 230},
{"olive drab",  107, 142, 35},
{"OliveDrab",  107, 142, 35},
{"OliveDrab1",  192, 255, 62},
{"OliveDrab2",  179, 238, 58},
{"OliveDrab3",  154, 205, 50},
{"OliveDrab4",  105, 139, 34},
{"orange",  255, 165, 0},
{"orange red",  255, 69, 0},
{"orange1",  255, 165, 0},
{"orange2",  238, 154, 0},
{"orange3",  205, 133, 0},
{"orange4",  139, 90, 0},
{"OrangeRed",  255, 69, 0},
{"OrangeRed1",  255, 69, 0},
{"OrangeRed2",  238, 64, 0},
{"OrangeRed3",  205, 55, 0},
{"OrangeRed4",  139, 37, 0},
{"orchid",  218, 112, 214},
{"orchid1",  255, 131, 250},
{"orchid2",  238, 122, 233},
{"orchid3",  205, 105, 201},
{"orchid4",  139, 71, 137},
{"pale goldenrod",  238, 232, 170},
{"pale green",  152, 251, 152},
{"pale turquoise",  175, 238, 238},
{"pale violet red",  219, 112, 147},
{"PaleGoldenrod",  238, 232, 170},
{"PaleGreen",  152, 251, 152},
{"PaleGreen1",  154, 255, 154},
{"PaleGreen2",  144, 238, 144},
{"PaleGreen3",  124, 205, 124},
{"PaleGreen4",  84, 139, 84},
{"PaleTurquoise",  175, 238, 238},
{"PaleTurquoise1",  187, 255, 255},
{"PaleTurquoise2",  174, 238, 238},
{"PaleTurquoise3",  150, 205, 205},
{"PaleTurquoise4",  102, 139, 139},
{"PaleVioletRed",  219, 112, 147},
{"PaleVioletRed1",  255, 130, 171},
{"PaleVioletRed2",  238, 121, 159},
{"PaleVioletRed3",  205, 104, 137},
{"PaleVioletRed4",  139, 71, 93},
{"papaya whip",  255, 239, 213},
{"PapayaWhip",  255, 239, 213},
{"peach puff",  255, 218, 185},
{"PeachPuff",  255, 218, 185},
{"PeachPuff1",  255, 218, 185},
{"PeachPuff2",  238, 203, 173},
{"PeachPuff3",  205, 175, 149},
{"PeachPuff4",  139, 119, 101},
{"peru",  205, 133, 63},
{"pink",  255, 192, 203},
{"pink1",  255, 181, 197},
{"pink2",  238, 169, 184},
{"pink3",  205, 145, 158},
{"pink4",  139, 99, 108},
{"plum",  221, 160, 221},
{"plum1",  255, 187, 255},
{"plum2",  238, 174, 238},
{"plum3",  205, 150, 205},
{"plum4",  139, 102, 139},
{"powder blue",  176, 224, 230},
{"PowderBlue",  176, 224, 230},
{"purple",  160, 32, 240},
{"purple1",  155, 48, 255},
{"purple2",  145, 44, 238},
{"purple3",  125, 38, 205},
{"purple4",  85, 26, 139},
{"red",  255, 0, 0},
{"red1",  255, 0, 0},
{"red2",  238, 0, 0},
{"red3",  205, 0, 0},
{"red4",  139, 0, 0},
{"rosy brown",  188, 143, 143},
{"RosyBrown",  188, 143, 143},
{"RosyBrown1",  255, 193, 193},
{"RosyBrown2",  238, 180, 180},
{"RosyBrown3",  205, 155, 155},
{"RosyBrown4",  139, 105, 105},
{"royal blue",  65, 105, 225},
{"RoyalBlue",  65, 105, 225},
{"RoyalBlue1",  72, 118, 255},
{"RoyalBlue2",  67, 110, 238},
{"RoyalBlue3",  58, 95, 205},
{"RoyalBlue4",  39, 64, 139},
{"saddle brown",  139, 69, 19},
{"SaddleBrown",  139, 69, 19},
{"salmon",  250, 128, 114},
{"salmon1",  255, 140, 105},
{"salmon2",  238, 130, 98},
{"salmon3",  205, 112, 84},
{"salmon4",  139, 76, 57},
{"sandy brown",  244, 164, 96},
{"SandyBrown",  244, 164, 96},
{"sea green",  46, 139, 87},
{"SeaGreen",  46, 139, 87},
{"SeaGreen1",  84, 255, 159},
{"SeaGreen2",  78, 238, 148},
{"SeaGreen3",  67, 205, 128},
{"SeaGreen4",  46, 139, 87},
{"seashell",  255, 245, 238},
{"seashell1",  255, 245, 238},
{"seashell2",  238, 229, 222},
{"seashell3",  205, 197, 191},
{"seashell4",  139, 134, 130},
{"sienna",  160, 82, 45},
{"sienna1",  255, 130, 71},
{"sienna2",  238, 121, 66},
{"sienna3",  205, 104, 57},
{"sienna4",  139, 71, 38},
{"sky blue",  135, 206, 235},
{"SkyBlue",  135, 206, 235},
{"SkyBlue1",  135, 206, 255},
{"SkyBlue2",  126, 192, 238},
{"SkyBlue3",  108, 166, 205},
{"SkyBlue4",  74, 112, 139},
{"slate blue",  106, 90, 205},
{"slate gray",  112, 128, 144},
{"slate grey",  112, 128, 144},
{"SlateBlue",  106, 90, 205},
{"SlateBlue1",  131, 111, 255},
{"SlateBlue2",  122, 103, 238},
{"SlateBlue3",  105, 89, 205},
{"SlateBlue4",  71, 60, 139},
{"SlateGray",  112, 128, 144},
{"SlateGray1",  198, 226, 255},
{"SlateGray2",  185, 211, 238},
{"SlateGray3",  159, 182, 205},
{"SlateGray4",  108, 123, 139},
{"SlateGrey",  112, 128, 144},
{"snow",  255, 250, 250},
{"snow1",  255, 250, 250},
{"snow2",  238, 233, 233},
{"snow3",  205, 201, 201},
{"snow4",  139, 137, 137},
{"spring green",  0, 255, 127},
{"SpringGreen",  0, 255, 127},
{"SpringGreen1",  0, 255, 127},
{"SpringGreen2",  0, 238, 118},
{"SpringGreen3",  0, 205, 102},
{"SpringGreen4",  0, 139, 69},
{"steel blue",  70, 130, 180},
{"SteelBlue",  70, 130, 180},
{"SteelBlue1",  99, 184, 255},
{"SteelBlue2",  92, 172, 238},
{"SteelBlue3",  79, 148, 205},
{"SteelBlue4",  54, 100, 139},
{"tan",  210, 180, 140},
{"tan1",  255, 165, 79},
{"tan2",  238, 154, 73},
{"tan3",  205, 133, 63},
{"tan4",  139, 90, 43},
{"thistle",  216, 191, 216},
{"thistle1",  255, 225, 255},
{"thistle2",  238, 210, 238},
{"thistle3",  205, 181, 205},
{"thistle4",  139, 123, 139},
{"tomato",  255, 99, 71},
{"tomato1",  255, 99, 71},
{"tomato2",  238, 92, 66},
{"tomato3",  205, 79, 57},
{"tomato4",  139, 54, 38},
{"turquoise",  64, 224, 208},
{"turquoise1",  0, 245, 255},
{"turquoise2",  0, 229, 238},
{"turquoise3",  0, 197, 205},
{"turquoise4",  0, 134, 139},
{"violet",  238, 130, 238},
{"violet red",  208, 32, 144},
{"VioletRed",  208, 32, 144},
{"VioletRed1",  255, 62, 150},
{"VioletRed2",  238, 58, 140},
{"VioletRed3",  205, 50, 120},
{"VioletRed4",  139, 34, 82},
{"wheat",  245, 222, 179},
{"wheat1",  255, 231, 186},
{"wheat2",  238, 216, 174},
{"wheat3",  205, 186, 150},
{"wheat4",  139, 126, 102},
{"white",  255, 255, 255},
{"white smoke",  245, 245, 245},
{"WhiteSmoke",  245, 245, 245},
{"yellow",  255, 255, 0},
{"yellow green",  154, 205, 50},
{"yellow1",  255, 255, 0},
{"yellow2",  238, 238, 0},
{"yellow3",  205, 205, 0},
{"yellow4",  139, 139, 0},
{"YellowGreen",  154, 205, 50},
};
/* clang-format on */
static const int ncolornames = sizeof colornames / sizeof colornames[0];


/*
 * Forward Declaration
 */
static bool revert_window(void);


/*
 * Utility Functions
 */
static inline int
min(int a, int b)
{
  return (a < b) ? a : b;
}


static inline int
align32(int n)
{
  return (n + 31) / 32 * 32;
}


/*
 * Mouse and Cursor Functions.
 */
/* a simple linked list to remember generated cursors because cursors don't have
 * any identifying information except for the actual bitmap in Lisp, just cache
 * that.  16 DLwords, to give a 16x16 bitmap cursor. */
static bool
cursor_equal_p(const DLword *a, const DLword *b)
{
  for (int i = 0; i < CURSORHEIGHT; i++)
    if (a[i] != b[i]) return false;
  return true;
}


/*
 * Try to find cursor CURSOR on the sdl_cursorlist, if it isn't there, add it.
 * Return an SDL_Cursor that can be used directly.
 */
static
SDL_Cursor *getOrAllocateCursor(DLword cursor[], int hot_x, int hot_y)
{
  static const size_t cursize = CURSORWIDTH * CURSORHEIGHT / 8;
  static struct CachedCursor *sdl_cursorlist = NULL;

  hot_x = 0;
  hot_y = 0;
  /* try to find the cursor by checking the full bitmap */
  struct CachedCursor *pclp = NULL;
  struct CachedCursor *clp = sdl_cursorlist;
  SDL_Cursor *c;
  while (clp != NULL) {
    if (cursor_equal_p(clp->EmCursorBitMap, cursor) == true) {
      /* if it's in the first two elements of the list, leave the order alone.
       * There is a high probability of flipping back and forth between two */
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
  clp = (struct CachedCursor *)malloc(sizeof (struct CachedCursor));
  memcpy(clp->EmCursorBitMap, cursor, sizeof clp->EmCursorBitMap);
  Uint8 sdl_cursor_data[cursize];
  /* memcpy/memmove is no use here.  Endianness is significant for cursor[]
   * (array of DLword) while it's insignificant for SDL_Cursor data (byte
   * array). */
  for (int i = 0; i < cursize; ++i)
    sdl_cursor_data[i] = GETBYTE(((Uint8 *)cursor) + i);
  c = SDL_CreateCursor(sdl_cursor_data,
                       sdl_cursor_data, CURSORWIDTH, CURSORHEIGHT,
                       hot_x, hot_y);
  if (!c) DIAG_SDL("ERROR creating cursor");
  clp->cursor = c;
  clp->next = sdl_cursorlist;
  sdl_cursorlist = clp;
  return clp->cursor;
}


/*
 * Read a cursor bitmap from lisp. Try to find a cached cursor, then use that.
 * Use HOT_X and HOT_Y as the cursor hotspot.
 */
void
sdl_setCursor(int hot_x, int hot_y)
{
  SDL_Cursor *c = getOrAllocateCursor(EmCursorBitMap68K, hot_x, hot_y);
  if (!SDL_SetCursor(c))
   DIAG_SDL("SDL_SetCursor error");
}


void
sdl_setMousePosition(int x, int y)
{
  float win_x, win_y;

   if (!SDL_RenderCoordinatesToWindow(renderer,
                                      (float)x, (float)y, &win_x, &win_y))
     DIAG_SDL("SDL_RenderCoordinatesToWindow error");
  SDL_WarpMouseInWindow(window, win_x, win_y);
}


/*
 * Keyboard Functions
 */
static void
do_ring()
{
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


static int
map_key(SDL_Keycode k)
{
  for (int i = 0; keymap[i] != -1; i += 2) {
    if (keymap[i + 1] == k) return keymap[i];
  }
  return -1;
}


static bool
proc_shortcuts(SDL_Keycode k, unsigned short mod)
{
  if (k == SDLK_F12) {
    revert_window();
    return true;
  }
  return false;
}


static void
handle_keydown(SDL_Keycode k, unsigned short mod)
{
  int lk = map_key(k);

  if (proc_shortcuts(k, mod))
    return;
  if (lk == -1) {
    DIAG_OUT("No mapping for key %s\n", SDL_GetKeyName(k));
  } else {
#if 0
    DIAG_OUT("dn %s -> lisp keycode %d (0x%x)\n", SDL_GetKeyName(k), lk, mod);
#endif
    kb_trans(lk - KEYCODE_OFFSET, FALSE);
    do_ring();
    if ((KBDEventFlg += 1) > 0) Irq_Stk_End = Irq_Stk_Check = 0;
  }
}


static void
handle_keyup(SDL_Keycode k, unsigned short mod)
{
  int lk = map_key(k);
  if (lk == -1) {
    DIAG_OUT("No mapping for key %s\n", SDL_GetKeyName(k));
  } else {
#if 0
    DIAG_OUT("up %s -> lisp keycode %d (0x%x)\n", SDL_GetKeyName(k), lk, mod);
#endif
    kb_trans(lk - KEYCODE_OFFSET, TRUE);
    do_ring();
    if ((KBDEventFlg += 1) > 0) Irq_Stk_End = Irq_Stk_Check = 0;
  }
}


static inline bool
get_capslock_state(void)
{
  return SDL_GetModState() & SDL_KMOD_CAPS ? true : false;
}


/*
 * Display Functions
 */
static void
bitblt_to_texture(int _x, int _y, int _w, int _h)
{
  static const int bitsperword = 8 * sizeof (DLword);

  const DLword *src = DisplayRegion68k;
  /* Avoid dealing with partial words in the update by stretching the source
   * rectangle left and right to cover complete units and lock the corresponding
   * region in the texture */
  const int srcpitchwords = sdl_displaywidth / bitsperword;
  const int xstart = _x / bitsperword;
  const int xlimit = (_x + _w + bitsperword - 1) / bitsperword;
  const int ystart = _y * srcpitchwords;
  const int ylimit = (_y + _h) * srcpitchwords;
  const SDL_Rect dstrect = {
    .x = xstart * bitsperword,
    .w = (xlimit - xstart) * bitsperword,
    .y = _y,
    .h = _h
  };
  uint8_t *dst;
  int dstpitchbytes;

  SDL_LockTexture(texture, &dstrect, (void **)&dst, &dstpitchbytes);
  /* For each line in the source image */
  for (int sy = ystart, dy = 0; sy < ylimit; sy += srcpitchwords, dy += dstpitchbytes) {
    /* For each word in the line */
    for (int sx = xstart, dx = 0; sx < xlimit; ++sx, dx += bitsperword) {
      DLword srcw = GETBASEWORD(src, sy + sx);
      /* For each bit in the word */
      for (int b = 0; b < bitsperword; ++b) {
        /* Stretches 1-bit index color to 8-bit one. */
        dst[dy + dx + b] = (uint8_t)(srcw >> (bitsperword - b - 1)) & 0x01;
      }
    }
  }
  SDL_UnlockTexture(texture);
}


static inline void
redraw(void)
{
  (void)SDL_RenderTexture(renderer, texture, NULL, NULL);
  (void)SDL_RenderPresent(renderer);
}


static bool
live_redraw(ARGUSED void *userdata, SDL_Event *event)
{
  int w, h;

  if (event->type == SDL_EVENT_WINDOW_EXPOSED) {
    redraw();
  }
  return true;
}


void
sdl_notify_damage(int x, int y, int w, int h)
{
  if (x < min_x) min_x = x;
  if (y < min_y) min_y = y;
  if (x + w > max_x) max_x = min(x + w, sdl_displaywidth - 1);
  if (y + h > max_y) max_y = min(y + h, sdl_displayheight - 1);
  damage_f = true;
}


static bool
set_color(SDL_Color *color, const char *colorname)
{
  int i;
  uint8_t r, g, b;

  color->a = 255;
  if (colorname[0]=='#' && strlen(colorname) == 7
      && strspn(&colorname[1], "0123456789abcdefABCDEF") == 6) {
    /* Hex color code */
    uint32_t pixval = strtoul(&colorname[1], NULL, 16);
    *color = (SDL_Color){
      .r = pixval >> 16 & 0xFF,
      .g = pixval >> 8 & 0xFF,
      .b = pixval & 0xFF
    };
    return true;
  } else {
    /* Color name */
    for (i = 0; i < ncolornames; i++) {
      if (0 == strcasecmp(colorname, colornames[i].name)) {
        *color = (SDL_Color){
          .r = colornames[i].red,
          .g = colornames[i].green,
          .b = colornames[i].blue
        };
        return true;
      }
    }
    return false;
  }
}


static bool
set_texture_colors(const char *bg_colorname, const char *fg_colorname)
{
  bool new_f = false;
  SDL_Color cs[NCOLORS];
  SDL_Palette *palette;

  if (!set_color(&cs[INDEX_BGCOLOR], bg_colorname)) {
    DIAG_OUT("Unknown color: %s\n", bg_colorname);
    return false;
  }
  if (!set_color(&cs[INDEX_FGCOLOR], fg_colorname)) {
    DIAG_OUT("Unknown color: %s\n", fg_colorname);
    return false;
  }

  /* It is necessary to identify the palette is newly created or existing one
   * because SDL_GetTexturePalette() doesn't increate palette's refcount. */
  palette = SDL_GetTexturePalette(texture);
  if (!palette) {
    GUARD_SDL(
     palette = SDL_CreatePalette(NCOLORS),
     false, "Creating a palette failed"
     );
    new_f = true;
  }
  GUARD_SDL(
   SDL_SetPaletteColors(palette, cs, 0, NCOLORS),
   false, "Setting palette colors failed"
   );
  GUARD_SDL(
   SDL_SetTexturePalette(texture, palette),
   false, "Setting palette to texture failed"
   );
  if (new_f)
    SDL_DestroyPalette(palette);
  return true;
}

void
sdl_set_invert(int flag)
{
  if (flag) {
    (void)set_texture_colors(palette_names[INDEX_FGCOLOR],
                             palette_names[INDEX_BGCOLOR]);
  } else {
    (void)set_texture_colors(palette_names[INDEX_BGCOLOR],
                             palette_names[INDEX_FGCOLOR]);
  }
  sdl_notify_damage(0, 0, sdl_displaywidth, sdl_displayheight);
}


static inline void
heal_damage(void)
{
  bitblt_to_texture(min_x, min_y, max_x - min_x, max_y - min_y);
  redraw();

  damage_f = false;
  min_x = min_y = INT_MAX;
  max_x = max_y = 0;
}


/*
 * HiDPI Adaptation
 *
 * According to `SDL3/README-highdpi - SDL Wiki`_, HiDPI window sizes differ
 * between on Windows/X11 and on macOS/Wayland.  The former always measures the
 * size in pixels while the latter takes the display scale into account.  To
 * cope with this difference, the document introduces the display content scale.
 *
 * Although SDL3 provides SDL_GetDisplayContentScale() to get the display
 * content scale, it is difficult to get the correct one *before* creating a
 * window.  Hosts may have multiple displays and it is hard to tell in which
 * display a newly created window will reside.
 *
 * Instead of SDL_GetDisplayContentScale(), the content scale is calculated
 * *after* window creation in this code.  First it creates a window temporarily
 * with the virtual desktop size (init_SDL()).  Then it calculates the content
 * scale from the window (calc_content_scale()) and rescales the window based on
 * that content scale (rescale_window()).  This procedure also takes place when
 * SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED occurs.
 *
 * The content of the virtual desktop is independently managed by the renderer.
 * It automatically fits the content to the window by setting its logical
 * presentation to SDL_LOGICAL_PRESENTATION_LETTERBOX.
 *
 * .. _SDL3/README-highdpi - SDL Wiki:
 *    https://wiki.libsdl.org/SDL3/README-highdpi>
 */
static inline float
calc_content_scale(void)
{
  float ds = SDL_GetWindowDisplayScale(window);
  float pd = SDL_GetWindowPixelDensity(window);
  return pd == 0.0f ? 0.0f : ds / pd;
}


static bool
rescale_window(void)
{
  int w, h;
  float content_scale;

  GUARD_SDL(
   SDL_GetWindowSize(window, &w, &h),
   false, "Getting window size failed"
   );
  GUARD_SDL(
   (content_scale = calc_content_scale()) > 0.0f,
   false, "Calculating content scale failed"
   );
  GUARD_SDL(
   SDL_SetWindowSize(window, w * content_scale, h * content_scale),
   false, "Resizing window failed"
   );
  return true;
}


static bool
revert_window()
{
  const float cs = calc_content_scale();

  GUARD_SDL(
   SDL_SetWindowFullscreen(window, false),
   false, "Setting windowed mode failed"
   );
  GUARD_SDL(
   SDL_SetWindowSize(window,
                     (int)(sdl_displaywidth * cs),
                     (int)(sdl_displayheight * cs)),
   false,
   "Setting window size failed"
   );
  return true;
}


void
process_Display_events(void *dsp)
{
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (!SDL_ConvertEventToRenderCoordinates(renderer, &event))
      DIAG_SDL("Mouse Coordinates conversion failed");
    switch (event.type) {
      case SDL_EVENT_QUIT:
        DIAG_OUT("quitting\n");
        exit(0);
        break;
      case SDL_EVENT_WINDOW_FOCUS_GAINED:
        window_focus_f = true;
        if (get_capslock_state() != capslock_f) {
          handle_keydown(SDLK_CAPSLOCK, event.key.mod);
          handle_keyup(SDLK_CAPSLOCK, event.key.mod);
        }
        break;
      case SDL_EVENT_WINDOW_FOCUS_LOST:
        window_focus_f = false;
        capslock_f = get_capslock_state();
        break;
      case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
      case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
        redraw();
        break;
      case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
        (void)rescale_window();
        redraw();
        break;
      case SDL_EVENT_KEY_DOWN:
        if (event.key.repeat) {
          /* Lisp needs to see the UP transition before the DOWN transition */
          handle_keyup(event.key.key, event.key.mod);
        }
        handle_keydown(event.key.key, event.key.mod);
        break;
      case SDL_EVENT_KEY_UP:
        handle_keyup(event.key.key, event.key.mod);
        break;
      case SDL_EVENT_MOUSE_MOTION: {
        int ix, iy;
        float x, y;
        if (!window_focus_f) break;
        *CLastUserActionCell68k = MiscStats->secondstmp;
        ix = event.motion.x;
        iy = event.motion.y;
        *EmCursorX68K = (*((DLword *)EmMouseX68K)) = (short)(ix & 0xFFFF);
        *EmCursorY68K = (*((DLword *)EmMouseY68K)) = (short)(iy & 0xFFFF);
        do_ring();
        if ((KBDEventFlg += 1) > 0) Irq_Stk_End = Irq_Stk_Check = 0;
        break;
      }
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
        switch (event.button.button) {
          case SDL_BUTTON_LEFT: PUTBASEBIT68K(EmRealUtilin68K, MOUSE_LEFT, FALSE); break;
          case SDL_BUTTON_MIDDLE: PUTBASEBIT68K(EmRealUtilin68K, MOUSE_MIDDLE, FALSE); break;
          case SDL_BUTTON_RIGHT: PUTBASEBIT68K(EmRealUtilin68K, MOUSE_RIGHT, FALSE); break;
        }
        do_ring();
        if ((KBDEventFlg += 1) > 0) Irq_Stk_End = Irq_Stk_Check = 0;
        break;
      case SDL_EVENT_MOUSE_BUTTON_UP:
        switch (event.button.button) {
          case SDL_BUTTON_LEFT: PUTBASEBIT68K(EmRealUtilin68K, MOUSE_LEFT, TRUE); break;
          case SDL_BUTTON_MIDDLE: PUTBASEBIT68K(EmRealUtilin68K, MOUSE_MIDDLE, TRUE); break;
          case SDL_BUTTON_RIGHT: PUTBASEBIT68K(EmRealUtilin68K, MOUSE_RIGHT, TRUE); break;
        }
        do_ring();
        if ((KBDEventFlg += 1) > 0) Irq_Stk_End = Irq_Stk_Check = 0;
        break;
      case SDL_EVENT_MOUSE_WHEEL:
#if 0
         DIAG_OUT("mousewheel mouse %d x %d y %d direction %s\n", event.wheel.which, event.wheel.x,
               event.wheel.y,
               event.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? "normal" : "flipped");

          these are the 4 key bits for mouse wheel/trackpad scrolling - which unlike X11 are
          *not* presented as mouse button down/up events for each scroll action

          case 4: PUTBASEBIT68K(EmRealUtilin68K, KEYSET_LEFT, FALSE); break;
          case 5: PUTBASEBIT68K(EmRealUtilin68K, KEYSET_LEFTMIDDLE, FALSE); break;
          case 6: PUTBASEBIT68K(EmRealUtilin68K, KEYSET_RIGHT, FALSE); break;
          case 7: PUTBASEBIT68K(EmRealUtilin68K, KEYSET_RIGHTMIDDLE, FALSE); break;

#endif
        break;
#if 0
        case SDL_KEYMAPCHANGED:
          DIAG_OUT("SDL_KEYMAPCHANGED\n"); break;
        case SDL_TEXTINPUT:
          DIAG_OUT("SDL_TEXTINPUT\n"); break;
#endif
    default: /* DIAG_OUT("other event type: %d\n", event.type); */ break;
    }
  }
  if (damage_f)
    heal_damage();
}


/*
 * Initializer
 */
static inline const char*
select_colorname(char *colorname, const char *default_color)
{
  return *colorname != '\0' ? colorname : default_color;
}


int
init_SDL(char *windowtitle, int w, int h, ARGUSED int s)
{
  sdl_displaywidth = w = align32(w);
  sdl_displayheight = h;

  GUARD_SDL(
   SDL_Init(SDL_INIT_VIDEO),
   1, "Initializing SDL failed"
   );

  /* HiDPI setting for mouse and cursor. (requires SDL3 >= 3.4.0). */
  GUARD_SDL(
   SDL_SetHint(SDL_HINT_MOUSE_DPI_SCALE_CURSORS, "1"),
   2, "HiDPI mouse setting failed"
   );

  /* Creates a window and a renderer temporarily with the virtual desktop
   * size. */
  GUARD_SDL(
   SDL_CreateWindowAndRenderer(windowtitle, w, h,
                               SDL_WINDOW_RESIZABLE
                               | SDL_WINDOW_HIGH_PIXEL_DENSITY
                               | SDL_WINDOW_KEYBOARD_GRABBED,
                               &window, &renderer),
   2, "Creating window and renderer failed"
   );

  /* Sets the window up. */
  GUARD_SDL(
   SDL_SetWindowAspectRatio(window, (float)w / h, (float)w / h),
   1, "Setting window aspect ratio failed"
   );
  if (!rescale_window())
    return 2;

  /* Sets the renderer up. */
  GUARD_SDL(
   SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_PIXELART),
   2, "Setting default texture scale mode failed"
   );
  GUARD_SDL(
   SDL_SetRenderLogicalPresentation(renderer,
                                    w, h,
                                    SDL_LOGICAL_PRESENTATION_LETTERBOX),
   2, "Setting renderer's logical presentation failed"
   );

  /* Creates and sets up a texture */
  GUARD_SDL(
   texture = SDL_CreateTexture(renderer,
                               PIXELFORMAT, SDL_TEXTUREACCESS_STREAMING,
                               w, h),
   2, "Creating texture failed"
   );
  palette_names[INDEX_BGCOLOR] = select_colorname(backgroundColorName,
                                                  BG_COLORNAME);
  palette_names[INDEX_FGCOLOR] = select_colorname(foregroundColorName,
                                                  FG_COLORNAME);
  if (!set_texture_colors(palette_names[INDEX_BGCOLOR],
                          palette_names[INDEX_FGCOLOR]))
    return 2;

  /* Adds a watcher to handle live redrawing during resizing. */
  GUARD_SDL(
   SDL_AddEventWatch(live_redraw, NULL),
   2, "Adding a watcher failed"
   );

  window_focus_f = true;
  heal_damage();  /* Sets {min,max}_{x,y} to defaults. */
  return 0;
}


/* vim: set expandtab shiftwidth=2 tabstop=2 softtabstop=2: */
