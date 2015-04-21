/* $Id: XKeymap.h,v 1.2 1999/01/03 02:05:48 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */




/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/*	The contents of this file are proprietary information 		*/
/*	belonging to Venue, and are provided to you under license.	*/
/*	They may not be further distributed or disclosed to third	*/
/*	parties without the specific permission of Venue.		*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*	Generic X-keyboard map for Medley.  This table is used at	*/
/*	start-up time to create the table that maps X keycodes to	*/
/*	Medley key numbers.						*/
/*									*/
/*	This is done by asking X for the keycodes that correspond	*/
/*	to given KEYSYMs (X's machine-independent coding scheme),	*/
/*	and building the table.  This has one problem:  The mapping	*/
/*	for non-ASCII characters isn't standard among keyboards.  To	*/
/*	get as reasonable a map as possible, the table below contains	*/
/*	possibly several mappings for each Medley key number.   Since	*/
/*	not every keyboard has every key (e.g., Alt vs Meta), there	*/
/*	may also be several mappings for a single KEYSYM.		*/
/*									*/
/*	Here's how it works:  Each entry is tried in turn.  If the	*/
/*	Medley key number we'd be assigning is already assigned a	*/
/*	mapping, skip this entry.  If not, assign this mapping, and	*/
/*	set the "this-KEYSYM-used" flag.  If this is a new KEYSYM,	*/
/*	reset the flag before trying anything.  If the "used" flag	*/
/*	is set, skip until we find a new KEYSYM.			*/
/*									*/
/*	Constraints:							*/
/*		Put the better key-number assignment earlier.		*/
/*		Put the better KEYSYM assignment earlier.		*/
/*		All entries for a single KEYSYM -must- be adjacent	*/
/*		Final entry in the map has key number -1.		*/
/*									*/
/************************************************************************/
/*									*/
/*		      C H A N G E   H I S T O R Y			*/
/*									*/
/*	23 SEP 91 JDS:	Rearrange CUT, BS, and DEL keys to assure that	*/
/*			we get a BS key first, then a CUT key, then,	*/
/*			if there's a key left, a DEL (BW) key.		*/
/*									*/
/*	26 MAY 92 JDS:	Rearrange keys for PC kbd layout.		*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#ifdef XWINDOW

int generic_X_keymap[] =
 {
    0, 107,  0x1000ff10,	/* Un-named KEYSYM used on Sun kbd for F11 */
    0, 108,  0x1000ff11,	/* Un-named KEYSYM used on Sun kbd for F12 */
    0, 107,  XK_F11,
    0, 61,   XK_F11,		/* STOP, usual sun type-4 kbd key */
    0, 61,   XK_Pause,		/* STOP, Pause/break key on PC */
    0, 108,  XK_F12,
    0, 91,   XK_F12,
    0, 61,   XK_L1,
    0, 91,   XK_L2,
    0, 97,   XK_Help,
    0, 99,   XK_F2,
    0, 106,  XK_F10,
    0, 100,  XK_F3,
    0, 67,   XK_F4,
    0, 68,   XK_F5,
    0, 101,  XK_F6,
    0, 66,   XK_F7,
    0, 104,  XK_F8,
    0, 80,   XK_F9,
    0, 31,   XK_Meta_L,		/* Meta, standard meta key */
    0, 86,   XK_Meta_L,		/* (sun left-diamond key)  */
    0, 31,   XK_Alt_L,		/* Meta, Sun-4 usual key   */
    0, 86,   XK_Alt_L,		/* (sun left-diamond key)  */
    0, 75,   XK_F21,
    0, 110,  XK_F22,
    0, 74,   XK_F23,
    0, 109,  XK_F13,
    0, 14,   XK_F15,		/* SAME key on Sun type-4 kbd */
#ifdef XK_Scroll_Lock
    0, 14,   XK_Scroll_Lock,	/* SAME key on PC kbd */
#endif /* .. scroll .. */
    0, 33,   XK_Escape,
    0, 32,   XK_1,
    0, 32,   XK_exclam,
    0, 17,   XK_2,
    0, 17,   XK_at,
    0, 16,   XK_3,
    0, 16,   XK_numbersign,
    0, 1,    XK_4,
    0, 1,    XK_dollar,
    0, 0,    XK_5,
    0, 0,    XK_percent,
    0, 2,    XK_6,
    0, 2,    XK_asciicircum,
    0, 4,    XK_7,
    0, 4,    XK_ampersand,
    0, 53,   XK_8,
    0, 53,   XK_asterisk,
    0, 22,   XK_9,
    0, 22,   XK_parenleft,
    0, 8,    XK_0,
    0, 8,    XK_parenright,
    0, 10,   XK_minus,
    0, 10,   XK_underscore,
    0, 59,   XK_equal,
    0, 59,   XK_plus,
    0, 45,   XK_quoteleft,
    0, 45,   XK_asciitilde,
    0, 89,   XK_F16,		/* Copy, Sun-4 kbd L6 */
    0, 34,   XK_Tab,
    0, 19,   XK_Q,
    0, 18,   XK_W,
    0, 3,    XK_E,
    0, 48,   XK_R,
    0, 49,   XK_T,
    0, 51,   XK_Y,
    0, 6,    XK_U,
    0, 23,   XK_I,
    0, 25,   XK_O,
    0, 11,   XK_P,
    0, 58,   XK_bracketleft,
    0, 58,   XK_braceleft,
    0, 29,   XK_bracketright,
    0, 29,   XK_braceright,
    0, 13,   XK_Delete,
    0, 111,  XK_F17,
    0, 36,   XK_Control_L,
    0, 21,   XK_A,
    0, 20,   XK_S,
    0, 5,    XK_D,
    0, 35,   XK_F,
    0, 50,   XK_G,
    0, 52,   XK_H,
    0, 38,   XK_J,
    0, 9,    XK_K,
    0, 26,   XK_L,
    0, 43,   XK_semicolon,
    0, 28,   XK_quoteright,
    0, 105,  XK_backslash,
    0, 105,  XK_bar,
    0, 105,  XK_brokenbar,
    0, 44,   XK_Return,
/*  46,   XK_F20,		   See DELETE, CUT, BS below */
    0, 41,   XK_Shift_L,
    0, 40,   XK_Z,
    0, 24,   XK_X,
    0, 37,   XK_C,
    0, 7,    XK_V,
    0, 39,   XK_B,
    0, 54,   XK_N,
    0, 55,   XK_M,
    0, 27,   XK_comma,
    0, 42,   XK_period,
    0, 12,   XK_slash,
    0, 60,   XK_Shift_R,
    0, 71,   XK_Linefeed,
    0, 92,   XK_Help,
    0, 92,   XK_Print,		/* PC Kbd Print key, works as HELP */
    0, 56,   XK_Caps_Lock,
    0, 86,   XK_Meta_L,
    0, 57,   XK_space,
    0, 88,   XK_Meta_R,

    /* DELETE, CUT, and BACKSPACE keys.  It's important that there */
    /* be a BS key, so we'll take over DEL for that purpose  */
    /* if nothing better offers.  We also need a CUT key, so we'll */
    /* grab it for THAT if there's a BS key. */

    0, 15,   XK_BackSpace,		/* best BS key */
    0, 46,   XK_F20,		/* CUT key on Sun kbd */
    0, 13,   XK_KP_Decimal,	/* Keypad DEL/. key if all else fails */
    0, 15,   XK_Delete,		/* Use DEL for BS if needed */
    0, 46,   XK_Delete,		/* or for CUT if needed */
    1, 13,   XK_Delete,		/* Or leave it as BackWord */


    /* The keypad (right side, numeric entry pad) */
    /* But keypad-decimal is assigned up with DEL & BS */
    0, 98,   XK_KP_0,		/* Keypad 0 */
    0, 89,   XK_Insert,		/* Copy, generic INS key */
    0, 98,   XK_Insert,		/* INS key = Keypad 0 otherwise */
    0, 94,   XK_KP_1,		/* Keypad 1 */
    0, 94,   XK_R13,
    0, 69,   XK_KP_2,
    0, 70,   XK_KP_3,
    0, 70,   XK_F35,
    0, 84,   XK_KP_4,
    0, 85,   XK_KP_5,
    0, 85,   XK_F31,
    0, 87,   XK_KP_6,
    0, 81,   XK_KP_7,
    0, 81,   XK_F27,
    0, 82,   XK_KP_8,
    0, 83,   XK_F29,
    0, 83,   XK_KP_9,
    0, 96,   XK_KP_Subtract,
    0, 76,   XK_KP_Enter,
    0, 102,  XK_KP_Add,
    0, 73,   XK_Num_Lock,
    0, 64,   XK_KP_Equal,
    0, 64,   XK_F24,
    0, 65,   XK_KP_Divide,
    0, 65,   XK_F25,
    0, 95,   XK_KP_Multiply,
    0, 95,   XK_F26,



    /* Arrow keys.  If not already assigned by the   */
    /* keypad digit assignments above, try assigning */
    /* the new arrow-key key#s assigned for RS/6000  */
    /* Failing that, try assigning the keypad #s.    */
    0, 129,  XK_Left,
    0, 84,   XK_Left,

    0, 130,  XK_Up,
    0, 82,   XK_Up,

    0, 131,  XK_Down,
    0, 69,   XK_Down,

    0, 132,  XK_Right,
    0, 87,   XK_Right,

    0, 93,   XK_Multi_key,		/* Expand, suyn type-4 */
    0, 93,   XK_Alt_R,		/* Expand, RH Alt key  */
    0, 93,   XK_Mode_switch,	/* Expand, RH Alt key on PC kbd */
    /* No key defs may come here! */
    0, 47,   XK_Mode_switch,	/* Next, Sun-4 Alt-graph key */
    1, 47,   XK_Control_R,	/* Next, PC's right-hand Ctrl key */
    0, 47,   XK_Next,		/* Next, Next or PgDn key ?  */
    0, 91,   XK_Next,		/* Again, Next or PgDn key */

    0, 63,   XK_F14,		/* Undo, Sun-4 kbd L4 */
    0, 63,   XK_Prior,		/* Undo, generic Prev or PgUp key */

    0, 90,   XK_F19,		/* Find, Sun-4 L9 key */
    0, 90,   XK_Find,		/* Find, generic Find key */
    0, 90,   XK_End,		/* End key on PC kbd */

    0, 62,   XK_F18,		/* Move key on Sun4 kbd */
    0, 62,   XK_Home,		/* Home key on PC 101-key kbd */

    -1, -1,   -1 };


#endif /* XWINDOW */

