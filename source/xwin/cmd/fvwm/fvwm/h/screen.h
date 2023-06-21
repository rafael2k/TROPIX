/*
 ****************************************************************
 *								*
 *			h/screen.h				*
 *								*
 *	Diversas defini��es relativas � imagem na tela		*
 *								*
 *	Vers�o	3.0.0, de 11.09.97				*
 *		4.4.0, de 20.03.03				*
 *								*
 *	M�dulo: cmd/fvwm					*
 *		X Windows - Clientes				*
 *								*
 *	TROPIX: Sistema Operacional Tempo-Real Multiprocessado	*
 *		Copyright (c) 2003 TROPIX Technologies Ltd.	*
 * 								*
 ****************************************************************
 */

/****************************************************************************
 * This module is based on Twm, but has been siginificantly modified 
 * by Rob Nation
 ****************************************************************************/
/*
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/***********************************************************************
 *
 * fvwm per-screen data include file
 *
 ***********************************************************************/

#ifndef _SCREEN_
#define _SCREEN_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include "misc.h"
#include "menus.h"

#define SIZE_HINDENT		5
#define SIZE_VINDENT		3
#define MAX_WINDOW_WIDTH	32767
#define MAX_WINDOW_HEIGHT	32767


/* Cursor types */
#define POSITION	0	/* upper Left corner cursor */
#define TITLE_CURSOR	1       /* title-bar cursor */
#define DEFAULT		2	/* cursor for apps to inherit */
#define SYS		3       /* sys-menu and iconify boxes cursor */
#define MOVE		4       /* resize cursor */
#define WAIT		5   	/* wait a while cursor */
#define MENU		6  	/* menu cursor */
#define SELECT		7       /* dot cursor for f.move, etc. from menus */
#define DESTROY		8	/* skull and cross bones, f.destroy */
#define TOP		9
#define RIGHT		10
#define BOTTOM		11
#define LEFT		12
#define TOP_LEFT	13
#define TOP_RIGHT	14
#define BOTTOM_LEFT	15
#define BOTTOM_RIGHT	16
#define MAX_CURSORS	18

/* colormap focus styes */
#define COLORMAP_FOLLOWS_MOUSE	1	/* default */
#define COLORMAP_FOLLOWS_FOCUS	2

typedef struct 
{
	Window	win;
	int	isMapped;

}	PanFrame;

typedef enum
{ 
	NormalButton, 
	PixmapButton,
	FullPixmapButton,
	SimpleButton

}	ButtonFaceStyle;

typedef struct
{
	Picture	*up,
		*down;

}	PixmapButtonData;

typedef struct
{
	ulong		screen;

	int		d_depth;		/* copy of DefaultDepth(dpy, screen) */
	int		NumberOfScreens;	/* number of screens on display */
	int		MyDisplayWidth;		/* my copy of DisplayWidth(dpy, screen) */
	int		MyDisplayHeight;	/* my copy of DisplayHeight(dpy, screen) */
  
	FVWMWIN		FvwmRoot;		/* the head of the fvwm window list */

	Window		Root;			/* the root window */
	Window		SizeWindow;		/* the resize dimensions window */
	Window		NoFocusWin;		/* Window which will own focus when no other
						 * windows have it */
	PanFrame	PanFrameTop,
			PanFrameLeft,
			PanFrameRight,
			PanFrameBottom;

	Pixmap		gray_bitmap;		/* dark gray pattern for shaded out menu items*/
	Pixmap		gray_pixmap;		/* dark gray pattern for inactive borders */
	Pixmap		light_gray_pixmap;	/* light gray pattern for inactive borders */
	Pixmap		sticky_gray_pixmap;	/* light gray pattern for sticky borders */

	Binding		*AllBindings;
	MenuRoot	*AllMenus;

	int		root_pushes;		/* current push level to install root
						 * colormap windows */
	FVWMWIN		*pushed_window;		/* saved window to install when pushes drops
						 * to zero */
	Cursor		FvwmCursors[MAX_CURSORS];

	name_list	*TheList;		/* list of window names with attributes */
	char		*DefaultIcon;		/* Icon to use when no other icons are found */

	ColorPair	MenuColors;
	ColorPair	MenuStippleColors;
	ColorPair	MenuRelief;
	ColorPair	StdColors;		/* standard fore/back colors */
	ColorPair	HiColors;		/* standard fore/back colors */
	ColorPair	StdRelief;
	ColorPair	HiRelief;

	MyFont		StdFont;		/* font structure */
	MyFont		WindowFont;		/* font structure for window titles */
	MyFont		IconFont;		/* for icon labels */
  
	GC		ButtonMaskGC,		/* GC to mask pixmap buttons */
			DrawGC,			/* GC to draw lines for move and resize */
			HiReliefGC,		/* GC for highlighted window relief */
			HiShadowGC,		/* GC for highlighted window shadow */
			MenuGC,
			MenuStippleGC,
			MenuReliefGC,
			MenuShadowGC,
			ScratchGC1,
			ScratchGC2,
			ScratchGC3;

	int		SizeStringWidth;	/* minimum width of size window */
	int		CornerWidth;		/* corner width for decoratedwindows */
	int		BoundaryWidth;		/* frame width for decorated windows */
	int		NoBoundaryWidth;	/* frame width for decorated windows */
	int		TitleHeight;		/* height of the title bar window */

	struct TitleStyle			/* how do the titles look? */
	{
		int	justify;		/* left, right, centered */
		int	appearance;		/* raised, sunk, flat, not implemented yet */

	}	TitleStyle;

	FVWMWIN		*Hilite;		/* the fvwm window that is highlighted 
						 * except for networking delays, this is the
						 * window which REALLY has the focus */
	FVWMWIN		*Focus;			/* Last window which Fvwm gave the focus to 
						 * NOT the window that really has the focus */
	Window		UnknownWinFocused;	/* None, if the focus is nowhere or on an fvwm
						 * managed window. Set to id of otherwindow 
						 * with focus otherwise */
	FVWMWIN		*Ungrabbed;
	FVWMWIN		*PreviousFocus;		/* Window which had focus before fvwm stole it
						 * to do moves/menus/etc. */
	int		EntryHeight;		/* menu entry height */
	int		EdgeScrollX;		/* #pixels to scroll on screen edge */
	int		EdgeScrollY;		/* #pixels to scroll on screen edge */
	uchar		buttons2grab;		/* buttons to grab in click to focus mode */
	ulong		flags;
	int		NumBoxes;
	int		randomx;		/* values used for randomPlacement */
	int		randomy;

	FVWMWIN		*LastWindowRaised;	/* Last window which was raised. Used for raise
						 * lower func. */
	int		VxMax;			/* Max location for top left of virt desk*/
	int		VyMax;
	int		Vx;			/* Current loc for top left of virt desk */
	int		Vy;

	int		nr_left_buttons;	/* number of left-side title-bar buttons */
	int		nr_right_buttons;	/* number of right-side title-bar buttons */

	int		left_button_styles[2][5];
	int		right_button_styles[2][5];

	int		ClickTime;		/* Max button-click delay for Function built-in*/
	int		ScrollResistance;	/* resistance to scrolling in desktop */
	int		MoveResistance;		/* res to moving windows over viewport edge */
	int		OpaqueSize;
	int		CurrentDesk;		/* The current desktop number */
	int		ColormapFocus;		/* colormap focus style */

	/* button styles */
	ButtonFaceStyle	button_style[10];

	/* Simple button needs no data */

	/* Normal button data (coordinate) */
	int		left_num_coords[5];
	int		left_x_coords[5][20];
	int		left_y_coords[5][20];
	int		left_line_style[5][20];

	int		right_num_coords[5];
	int		right_x_coords[5][20];
	int		right_y_coords[5][20];
	int		right_line_style[5][20];

	/* Pixmap button data */
	PixmapButtonData pixmap_button[10];

}	ScreenInfo;

extern	ScreenInfo	Scr;

/* for the titlestyle values: */
#define JUSTIFY_CENTER	0
#define JUSTIFY_LEFT	1
#define JUSTIFY_RIGHT	2
#define TITLE_RAISED	0
#define TITLE_SUNK	1
#define TITLE_FLAT	2
#define TITLE_MWM	3

/* for the flags value - these used to be seperate Bool's */
#define WindowsCaptured		1
#define EdgeWrapX		64	/* Should EdgeScroll wrap around? */
#define EdgeWrapY		128
#define MWMMenus		1024
#endif /* _SCREEN_ */
