NAME
fvwm Virtual Window Manager for X11

SYNOPSIS
fvwm [ options ]

DESCRIPTION
Fvwm is a window manager for X11.  It is a derivative of
twm, redesigned to minimize memory consumption, provide a 3-D
look to window frames, and provide a simple virtual desktop.  Version
2.xx uses only slightly more memory than 1.xx, mostly due to some
global options being able to be window specific now.

Fvwm provides both a large virtual desktop and multiple disjoint
desktops which can be used separately or together.  The virtual desktop
allows you to pretend that your video screen is really quite large,
and you can scroll around within the desktop.  The multiple disjoint
desktops allow you to pretend that you really have several screens to
work at, but each screen is completely unrelated to the others.

Fvwm provides keyboard accelerators which allow you to perform most
window-manager functions, including moving and resizing windows, and
operating the window-manager's menus, using keyboard shortcuts.

Fvwm has also blurred the distinction between configuration commands
and built-in commands that most window-managers make.  Configuration
commands typically set fonts, colors, menu contents, key and mouse
function bindings, while built-in commands typically do things like
raise and lower windows.  Fvwm makes no such distinction, and allows,
to the extent that is practical, anything to be changed at any time.

Other noteworthy differences between Fvwm and other X11 window managers
are the introduction of the SloppyFocus and per-window focus methods.
SloppyFocus is focus-follows-mouse, but focus is not removed from
windows when the mouse leaves a window and enters the root window.
When sloppy focus is used as the default focus style, it is nice to
make windows in which you do not typically type into (xmag, xman,
xgraph, xclock, xbiff, etc) click-to-focus, so that your terminal
window doesn't loose focus unnecessarily.

COPYRIGHTS
Since fvwm is derived from twm code it shares twm's 
copyrights.  Since nearly every line of twm code has been changed, the
twm copyright has been removed from most of the individual code files.
I do still recognize the influence of twm code in the overall package,
so fvwm's copyright is still considered to be the same as twm's.

fvwm is copyright 1988 by Evans and Sutherland Computer
Corporation, Salt Lake City, Utah, and 1989 by the Massachusetts
Institute of Technology, Cambridge, Massachusetts, All rights
reserved.  It is also copyright 1993 and 1994 by Robert Nation.


Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Evans & Sutherland and
M.I.T. not be used in advertising in publicity pertaining to
distribution of the software without specific, written prior
permission.

ROBERT NATION, CHARLES HINES, EVANS & SUTHERLAND, AND M.I.T. DISCLAIM
ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL EVANS &
SUTHERLAND OR M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTUOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

ANATOMY OF A WINDOW
Fvwm puts a decorative border around most windows.  This border
consists of a bar on each side and a small "L" shaped section on each
corner.  There is an additional top bar called the title bar which is
used to display the name of the window.  In addition, there are up to
10 title-bar buttons.  The top, side, and bottom bars are collectively
known as the side-bars.  The corner pieces are called the frame.

Unless the standard defaults files are modified, pressing mouse button
1 in the title or side-bars will begin a move operation on the
window.  Pressing button 1 in the corner frame pieces will begin a
resize operation.  Pressing button 2 anywhere in the border brings up
an extensive list of window operations.

Up to ten title-bar buttons may exist.  Their use is completely user
definable.  The default configuration has a title-bar button on each
side of the title-bar.  The one on the left is used to bring up a list
of window options, regardless of which mouse button is used.  The one
on the right is used to iconify the window.  The number of title-bar
buttons used depends on which ones have mouse actions bound to
them.  See the section on the "Mouse" configuration parameter below.


THE VIRTUAL DESKTOP
Fvwm provides multiple virtual desktops for users who wish to
use them.  The screen is a viewport onto a desktop which may be larger
than the screen.  Several distinct desktops can be accessed (concept:
one desktop for each project, or one desktop for each application,
when view applications are distinct).  Since each desktop can be
larger than the physical screen, windows which are larger than the
screen or large groups of related windows can easily be viewed.

The size of the virtual desktops can be changed any time, by using the
DeskTopSize built-in command.  All virtual desktops must be the same
size.  The total number of distinct desktops need not be specified, but
is limited to approximately 4 billion total.  All windows on a range of
desktops can be viewed in the Pager, a miniature view of the
desktops.  The pager is an accessory program, called a module, which is
not essential for the window manager to operate.  Windows may also be
listed, along with their geometries, in a window list, accessible as a
pop-up menu, or as a separate window, called the FvwmWinList (another
module).

"Sticky" windows are windows which transcend the virtual desktop by
"Sticking to the screen's glass."  They always stay put on the screen.
This is convenient for things like clocks and xbiff's, so you only need
to run one such gadget and it always stays with you.  Icons can also be
made to stick to the glass, if desired.

Window geometries are specified relative to the current viewport.  That
is:

	xterm -geometry +0+0

will always show up in the upper-left hand
corner of the visible portion of the screen.  It is permissible to
specify geometries which place windows on the virtual desktop, but off
the screen.  For example, if the visible screen is 1000 by 1000 pixels,
and the desktop size is 3x3, and the current viewport is at the upper
left hand corner of the desktop, then invoking:

	xterm -geometry +1000+1000

will place the window just off of the lower right hand corner of the
screen.  It can be found by moving the mouse to the lower right hand
corner of the screen and waiting for it to scroll into view.

There is currently no way to cause a window to map onto a desktop
other than the currently active desk, or is there...

A geometry specified as something like:

	xterm -geometry -5-5

will generally place the window's lower right hand corner 5 pixels from
the lower right corner of the visible portion of the screen. Not all
applications support window geometries with negative offsets.

Some applications that understand standard Xt command line arguments
and X resources, like xterm and xfontsel, allow the user to specify
the start-up desk on the command line:

	xterm -xrm "*Desk:1"

will start an xterm on desk number 1. Not all applications understand
this option, however.

You could achieve the same result with the following line in your
.Xdefaults file:

	XTerm*Desk: 1


INITIALIZATION
During initialization, fvwm will search for a configuration file
which describes key and button bindings, and a few other things.  The
format of these files will be described later.  First, fvwm will
search for a file named .fvwmrc (or .fvwmrc based on how it was
compiled - .fvwmrc is the default) in the users home directory.
Failing that, it will look for /usr/lib/X11/fvwm/.fvwmrc for
system-wide defaults.  If that file is not found, fvwm will be
basically useless.

Fvwm will set two environment variables which will be inherited
by its children.  These are $DISPLAY which describes the display on
which fvwm is running.  $DISPLAY may be unix:0.0 or :0.0, which
doesn't work too well when passed through rsh to another machine, so
$HOSTDISPLAY will also be set and will use a network-ready description
of the display.  $HOSTDISPLAY will always use the TCP/IP transport
protocol (even for a local connection) so $DISPLAY should be used for
local connections, as it may use Unix-domain sockets, which are
faster.

Fvwm has a two special functions for inititalization:
InitFunction and RestartFunction, which are executed during
Initialization and Restarts (respectively).  These may be customized
in the user's rc file via the AddToFunc facilitly (described later) to
start up modules, xterms, or whatever you'd like have started by
fvwm.

Fvwm also has a special exit function: ExitFunction, executed
when exitting or restarting before actually quitting or anything else.
It could be used to explicitly kill modules, etc.

ICONS
The basic Fvwm configuration uses monochrome bitmap icons,
similar to twm.  If XPM extensions are compiled in, then color
icons similar to ctwm, MS-Windows, or the Macintosh icons can be used.
In order to use these options you will need the XPM package, as
described in the Fvwm.tmpl Imake configuration file.

If both the SHAPE and XPM options are compiled in you will get shaped
color icons, which are very spiffy.

MODULES
A module is a separate program which runs as a separate Unix process
but transmits commands to fvwm to execute.  Users can write
their own modules to do any weird or bizarre manipulations without
bloating or affecting the integrity of fvwm itself.

Modules MUST be spawned by fvwm so that it can set up two pipes for
fvwm and the module to communicate with.  The pipes will already be
open for the module when it starts and the file descriptors for the
pipes are provided as command line arguments.

Modules can be spawned during fvwm at any time during the X
session by use of the Module built-in command.  Modules can exist for
the duration of the X session, or can perform a single task and exit.
If the module is still active when fvwm is told to quit, then
fvwm will close the communication pipes and wait to receive a
SIGCHLD from the module, indicating that it has detected the pipe
closure and has exited.  If modules fail to detect the pipe closure
fvwm will exit after approximately 30 seconds anyway.  The
number of simultaneously executing modules is limited by the operating
system's maximum number of simultaneously open files, usually between
60 and 256.

Modules simply transmit text commands to the fvwm built-in
command engine.  Text commands are formatted just as in the case of a
mouse binding in the .fvwmrc setup file.  Certain auxiliary
information is also transmitted, as in the sample module FvwmButtons.
The FvwmButtons module is documented in its own man page.

ICCCM COMPLIANCE
Fvwm attempts to be ICCCM 1.1 compliant.  In addition, ICCCM
states that it should be possible for applications to receive ANY
keystroke, which is not consistent with the keyboard shortcut approach
used in fvwm and most other window managers.

The ICCCM states that windows possessing the property

	WM_HINTS(WM_HINTS):
                Client accepts input or input focus: False         

should not be given the keyboard input focus by the window manager.
These windows can take the input focus by themselves, however.  A
number of applications set this property, and yet expect the
window-manager to give them the keyboard focus anyway, so fvwm
provides a window-style, "Lenience", which will allow fvwm to overlook
this ICCCM rule.


M4 PREPROCESSING
M4 pre-processing is handled by a module in fvwm-2.0.  To get more
details, try man FvwmM4.  In short, if you want fvwm to parse your
files with m4, then replace the word "Read" with "FvwmM4" in
your .fvwmrc file (if it appears at all), and start fvwm with the
command 

	fvwm -f "FvwmM4 .fvwmrc"


CPP PREPROCESSING
Cpp is the C-language pre-processor.  fvwm-2.0 offers cpp processing
which mirrors the m4 pre-processing.  To find out about it, re-read
the M4 section above, but replace "m4" with "cpp".

AUTO-RAISE
Windows can be automatically raised when it receives focus, or some
number of milliseconds after it receives focus, by using the
auto-raise module, FvwmAuto.

OPTIONS
These are the command line options that are recognized by fvwm:

-f config_command
Causes fvwm to use config_command instead of "Read .fvwmrc" 
as its initialization command.

-debug
Puts X transactions in synchronous mode, which dramatically slows things
down, but guarantees that fvwm's internal error messages are correct.

-d displayname
Manage the display called "displayname" instead of the name obtained from 
the environment variable $DISPLAY.

-s
On a multi-screen display, run fvwm only on the screen named in
the $DISPLAY environment variable or provided through the -d
option. Normally, fvwm will attempt to start up on all screens
of a multi-screen display.

-version
Print the version of fvwm to stderr.

CONFIGURATION FILES
The configuration file is used to describe mouse and button bindings,
colors, the virtual display size, and related items.  The
initialization configuration file is typically called ".fvwmrc".  By
using the "Read" built-in, it is easy to read in new configuration
files as you go.

Lines beginning with '#' will be ignored by fvwm.  Lines
starting with '*' are expected to contain module configuration
commands (rather than configuration commands for fvwm itself).

Fvwm makes no distinction between configuration commands and built-in
commands, so anything mentioned in the built-in commands section  can
be placed on a line by itself for fvwm to execute as it reads the
configuration file, or it can be placed as an executable command in a
menu or bound to a mouse button or a keyboard key.  It is left as an
exercise for the user to decide which function make sense for
initialization and which ones make sense for run-time.

BUILT IN FUNCTIONS
Fvwm supports a set of built-in functions which can be bound to
keyboard or mouse buttons.  If fvwm expects to find a built-in function
in a command, but fails, it will check to see if the specified command
should have been "Function (rest of command)" or "Module (rest of
command)".  This allows complex functions or modules to be invoked in a
manner which is fairly transparent to the configuration file.

Example: the .fvwmrc file contains the line "HelpMe".  Fvwm will look
for a built-in command called "HelpMe", and will fail. Next it will
look for a user-defined complex function called "HelpMe".  If no such
user defined function exists, Fvwm will try to execute a module called
"HelpMe".

In previous versions of fvwm, quoting was critical and irrational in
the .fvwmrc file.  As of fvwm-2, most of this has been cleared up.
Quotes are required only when needed to make fvwm consider two or more
words to be a single argument.  Unnecessary quoting is allowed.  If you
want a quote character in your text, you must escape it by using the
backslash character.  For example, if you have a pop-up menu called
Window-Ops, then you don't need quotes: Popup Window-Ops, but if you
replace the dash with a space, then you need quotes: Popup "Window
Ops".


"AddToMenu"
Begins or adds to a menu definition.  Typically a menu definition looks
like this:

AddToMenu Utilities "Utilities"     Title
+                   "Xterm"         Exec  xterm -e tcsh
+                   "Rxvt"          Exec  rxvt
+                   "Remote Logins" Popup Remote-Logins
+                   "Top"           Exec  rxvt -T Top -n Top -e top
+                   "Calculator"    Exec  xcalc
+                   "Xman"          Exec  xman
+                   "Xmag"          Exec  xmag
+                   "emacs"         Exec  xemacs
+                   "Mail"          MailFunction xmh "-font fixed"
+                   ""              Nop
+                   "Modules"       Popup Module-Popup
+                   ""              Nop
+                   "Exit Fvwm"     Popup Quit-Verify

The menu could be invoked via

Mouse 1 R       A       Menu Utilities Nop

or

Mouse 1 R       A       Popup Utilities

There is no end-of-menu symbol.  Menus do not have to be defined in a
contiguous region of the .fvwmrc file.  The quoted portion in the
above examples is the menu-label, which will appear in the menu when
the user pops it up.  The remaining portion is a built-in command
which should be executed if the user selects that menu item.  An empty
menu-label ("") and the Nop function can be used to insert a separator
into the menu.

If the menu-label contains a sub-string which is set off by stars,
then the text between the stars is expected to be the name of an
xpm-icon or bitmap-file to insert in the menu.  For example

+		"Calculator*xcalc.xpm*"	Exec xcalc

inserts a menu item labeled "calculator" with a picture of a
calculator above it.  The following:

+		"*xcalc.xpm*" Exec xcalc

Omits the "Calculator" label, but leaves the picture.

If the menu-label contains a sub-string which is set off by percent signs,
then the text between the percent signs is expected to be the name of an
xpm-icon or bitmap-file to insert to the left of the menu label.  For example

+		"Calculator%xcalc.xpm%"	Exec xcalc

inserts a menu item labeled "calculator" with a picture of a
calculator to the left.  The following:

+		"%xcalc.xpm%" Exec xcalc

Omits the "Calculator" label, but leaves the picture.  The pictures
used with this feature should be small (perhaps 16x16).


"AddToFunc"
Begins or add to a function definition.  Here's an example:

AddToFunc Move-or-Raise         "I" Raise
+                               "M" Move
+                               "D" Lower         

The function name is Move-or-Raise, and could be invoked from a menu
or a mouse binding or key binding:

Mouse 1 TS      A       Move-or-Raise

The quoted portion of the function tells what kind of action will
trigger the command which follows it.  "I" stands for Immediate, and is
executed as soon as the function is invoked.  "M" stands for Motion, ie
if the user starts moving the mouse.  "C" stands for Click, ie, if the
user presses and releases the mouse in a short period of time
(ClickTime milliseconds).  "D" stands for double-click.  The action "I"
will cause an action to be performed on the button-press, if the
function is invoked with prior knowledge of which window to act on.  

The special symbols $w and $0 through $9 are available in the
ComplexFunctions or Macros, or whatever you want to call them.  Within
a macro, $w is expanded to the window-id (expressed in 
hex, ie 0x10023c) of the window for which the macro was called.  $0
though $9 are the arguments to the macro, so if you call

Key F10	R	A	Function MailFunction xmh "-font fixed"

and MailFunction is


AddToFunc MailFunction     "I" Next [$0] Iconify -1
+                          "I" Next [$0] focus
+                          "I" None [$0] Exec $0 $1

Then the last line of the function becomes

+                          "I" None [xmh] Exec xmh -font fixed

The expansion is performed as the function is executed, so you can use the
same function with all sorts of different arguments.  I could use

Key F11	R	A	Function MailFunction zmail "-bg pink"

in the same .fvwmrc, if I wanted.  An example of using $w is:

AddToFunc PrintFunction         "I" Raise
+                               "I" Exec xdpr -id $w

Note that $$ is expanded to $.

"Beep"
As might be expected, the makes the terminal beep.


"ButtonStyle button# lots-of-numbers"
Defines a decoration shape to be used in a title-bar button.
button# is the title-bar button number, and is between 0 and 9.
A description of title-bar button numbers is given in the Mouse
section below.  The specification is a little cumbersome:

ButtonStyle 2 4 50x30@1 70x70@0 30x70@0 50x30@1

then the button 2 decoration will use a 4-point pattern consisting of
a line from (x=50,y=30) to (70,70) in the shadow color (@0), and then
to (30,70) in the shadow color, and finally to (50,30) in the
highlight color (@1).  Is that too confusing? See the sample .fvwmrc
for a few examples.

"ButtonStyle button# [Full]Pixmap pixmap-up [pixmap-down]"
Defines a pixmap to be displayed on a title-bar button.  button# is
the title-bar button, and is between 0 and 9.  One or two pixmaps can
be specified.  The first pixmap is shown as the "up" button position
and the second as the "down" position.  If the second pixmap is not
specified, the first one is used for both positions.  One might define
the following definitions:

ButtonStyle 2 Pixmap pixmap_both.xpm
ButtonStyle 4 Pixmap pixmap_up.xpm pixmap_down.xpm

The pixmap specification can be given as an absolute or relative
pathname (see PixmapPath).  If any of the pixmaps cannot be found, the
entire button reverts to a simple rectangle with no pixmaps.  If the
word Full is prefixed to Pixmap, then the pixmap can use the entire
height of the title-bar (i.e. the relief border around the button is
not drawn).

"ClickTime delay"
Specifies the maximum delay (in milliseconds) between a button press
and a button release for the Function built-in to consider the action
a mouse click.  The default delay is 150 milliseconds.

"Close"
If the window accepts the delete window protocol a message is sent to
the window asking it to gracefully remove itself.  If the window does
not understand the delete window protocol then the window is
destroyed.

"ColormapFocus [FollowsMouse | FollowsFocus]"
By default, fvwm installs the colormap of the window that the cursor
is in.  If you use ColormapFocus FollowsFocus, then the installed
colormap will be the one for the window that currently has the
keyboard focus.

"CursorMove horizonal vertical"
Moves the mouse pointer by horizontal pages in the X direction
and vertical pages in the Y direction.  Either or both entries
may be negative.  Both horizontal and vertical values are expressed in
percent of pages, so "CursorMove 100 100" means to move down and left
by one full page.  "CursorMove 50 25" means to move left half a page
and down a quarter of a page.  The CursorMove function should not be
called from pop-up menus.

"Delete"
Sends a message to a window asking that it remove itself, frequently
causing the application to exit.

"Desk arg1 arg2"
Changes to another desktop (workspace, room). 

If arg1 is non zero then the next desktop number will be the
current desktop number plus arg1.  Desktop numbers can be
negative.

If arg1 is zero then the new desktop number will be arg2.

The number of active desktops is determined dynamically.  Only
desktops which contain windows or are currently being displayed are
active.  Desktop numbers must be between 2147483647 and -2147483648
(is that enough?).

"DeskTopSize HorizontalxVertical"
Defines the virtual desktop size in units of the physical screen size.

"Destroy"
Destroys an application window, which usually causes the application
to crash and burn.

"DestroyFunc"
Deletes a function, so that subsequent references to it are no longer
valid.  You can use this to change the contents of a function during an
fvwm session.  The function can be rebuilt using AddToFunc.

	DestroyFunc "PrintFunction"

"DestroyMenu"
Deletes a menu, so that subsequent references to it are no longer
valid.  You can use this to change the contents of a menu during an
fvwm session.  The menu can be rebuilt using AddToMenu.

	DestroyMenu "Utilities"

"DestroyModuleConfig"
Deletes module configuration entries, so that new configuration lines
may be entered instead.  You can use this to change the the way a
module runs during an fvwm session without restarting.  Wildcards can
be used for portions of the name as well.

	DestroyModuleConfig FvwmFormFore
	DestroyModuleConfig FvwmButtons*

"EdgeResistance scrolling moving"
Tells how hard it should be to change the desktop viewport by moving
the mouse over the edge of the screen and how hard it should be to
move a window over the edge of the screen.

The first parameter tells how milliseconds the pointer must spend on
the screen edge before fvwm will move the viewport.  This is
intended for people who use "EdgeScroll 100 100" but find themselves
accidentally flipping pages when they don't want to.

The second parameter tells how many pixels over the edge of the screen
a window's edge must move before it actually moves partially off the
screen.

Note that, with "EdgeScroll 0 0", it is still possible to move or
resize windows across the edge of the current screen.  By making the
first parameter to EdgeResistance 10000 this type of motion is
impossible.  With EdgeResistance less than 10000 but greater than 0
moving over pages becomes difficult but not impossible.

"EdgeScroll horizontal vertical"
Specifies the percentage of a page to scroll when the cursor hits the
edge of a page.  If you don't want any paging or scrolling when you
hit the edge of a page include "EdgeScroll 0 0" in your .fvwmrc file.
If you want whole pages, use "EdgeScroll 100 100".  Both horizontal
and vertical should be positive numbers.

If the horizontal and vertical percentages are multiplied by 1000 then
scrolling will wrap around at the edge of the desktop.  If "EdgeScroll
100000 100000" is used fvwm will scroll by whole pages, wrapping
around at the edge of the desktop.

"Exec command"
Executes command.  Exec does not require an additional 'exec' at
the beginning or '&' at the end of the command.

The following example binds function key F1 in the root window, with
no modifiers, to the exec function.  The program rxvt will be started
with an assortment of options.

Key F1 R N Exec rxvt -fg yellow -bg blue -e /bin/tcsh


"ExecUseShell [shell]"
Makes the Exec command use the specified shell, or the value of the
$SHELL environment variable if no shell is specified, instead of the
default Bourne shell (/bin/sh).

ExecUseShell
ExecUseShell /usr/local/bin/tcsh

"Focus"
Moves the viewport or window as needed to make the selected window
visible.  Sets the keyboard focus to the selected window.  Raises the
window if needed to make it visible.  Does not warp the pointer into
the selected window (see WarpToWindow function).  Does not de-iconify.

"Function \FunctionName\"
Used to bind a previously defined function to a key or mouse button.

The following example binds mouse button 1 to a function called
"Move-or-Raise", whose definition was provided as an example earlier
in this man page.  After performing this binding fvwm will
execute to move-or-raise function whenever button 1 is pressed in a
window title-bar.

Mouse 1 T A Function Move-or-Raise

The keyword "Function" may be omitted if "FunctionName" does not
coincide with an fvwm built-in function name

"GotoPage  x y"
Moves the desktop viewport to page (x,y).  The upper left page is
(0,0), the upper right is (N,0), where N is one less than the current
number of horizontal pages specified in the DeskTopSize command.  The
lower left page is (0,M), and the lower right page is (N,M), where M
is the desktop's vertical size as specified in the DeskTopSize
command.  The GotoPage function should not be used in a pop-up menu.

"HilightColor textcolor backgroundcolor"
Specified the text and background colors for the decorations on the
window which currently has the keyboard focus.

"IconFont fontname"
Makes fvwm use font fontname for icon labels.  If omitted,
the menu font (specified by the Font configuration parameter) will be
used instead.

"Iconify [ value ]"
Iconifies a window if it is not already iconified or de-iconifies it
if it is already iconified.  If the optional argument value is
positive the only iconification will be allowed.  It the optional
argument is negative only de-iconification will be allowed.

"IconPath path"
Specifies a colon separated list of full path names of directories
where bitmap (monochrome) icons can be found.  Each path should start
with a slash.  Environment variables can be used here as well (i.e.
$HOME or ${HOME}).

Note: if the FvwmM4 is used to parse your rc files, then m4 may
want to mangle the word "include" which will frequently show up in the
IconPath or PixmapPath command.  To fix this add undefine(`include')
prior to the IconPath command, or better use the '-m4-prefix' option
to force all m4 directives to have a prefix of "m4_" (see the
FvwmM4 man page).

"Key keyname Context Modifiers Function"
Binds a keyboard key to a specified fvwm built-in function, or
removes the binding if Function is '-'.  Definition is the same
as for a mouse binding except that the mouse button number is replaced
with a key name.  The keyname is one of the entries from
/usr/include/X11/keysymdef.h, with the leading XK_ omitted.  The
Context and Modifiers fields are defined as in the Mouse
binding.

The following example binds the built in window list to pop up when
Alt-Ctrl-Shift-F11 is hit, no matter where the mouse pointer is:

Key F11  A  SCM  WindowList

Binding a key to a title-bar button will not cause that button to
appear unless a mouse binding also exists.

"KillModule name"
Causes the module which was invoked with name name to be killed.
name may include wild-cards.

"Lower"
Allows the user to lower a window.

"Maximize [  horizontal vertical ]"
Without its optional arguments Maximize causes the window to
alternately switch from a full-screen size to its normal size.

With the optional arguments horizontal and vertical, which are
expressed as percentage of a full screen, the user can control the new
size of the window.  If horizontal is greater than 0 then the
horizontal dimension of the window will be set to
horizontal*screen_width/100.  The vertical resizing is similar.  For
example, the following will add a title-bar button to switch a window
to the full vertical size of the screen:

Mouse 0 4 A Maximize 0 100

The following causes windows to be stretched to the full width:

Mouse 0 4 A Maximize 100 0

This makes a window that is half the screen size in each direction:

Mouse 0 4 A Maximize 50 50

Values larger than 100 can be used with caution.

If the letter "p" is appended to each coordinate (horizontal and/or
vertical), then the scroll amount will be measured in pixels.

"Menu menu-name double-click-action
Causes a previously defined menu to be popped up in a "sticky" manner.
That is, if the user invokes the menu with a click action instead of a
drag action, the menu will stay up.  The command
double-click-action will be invoked if the user double-clicks
when bringing the menu up.

"MenuStyle forecolor backcolor shadecolor font style"
Sets the menu style.  When using monochrome the colors are ignored.
The shade-color is the one used to draw a menu-selection which is
prohibited (or not recommended) by the mwm-hints which an application
has specified.  The style option is either "fvwm" or "mwm", which
changes the appearance of the menu.

"Module ModuleName"
Specifies a module which should be spawned during initialization.  At
the current time the available modules (included with fvwm) are
FvwmAudio (makes sounds to go with window manager actions), FvwmAuto
(an auto raise module), FvwmBacker (to change the background when you
change desktops), FvwmBanner (to display a spiffy XPM), FvwmButtons
(brings up a customizable tool bar), FvwmCpp (to preprocess your
.fvwmrc with cpp), FvwmForm (to bring up dialogs), FvwmIconBox (like
the mwm IconBox), FvwmIdent (to get window info), FvwmM4 (to
preprocess your .fvwmrc with m4), FvwmPager (a mini version of the
desktop), FvwmSave (saves the desktop state in .xinitrc style),
FvwmSaveDesk (saves the desktop state in fvwm commands), FvwmScroll
(puts scrollbars on any window), FvwmTalk (to interactively run fvwm
commands), and FvwmWinList (a window list).  These modules have their
own man pages.  There are other modules out on there as well.

Modules can be short lived transient programs or, like FvwmButtons,
can remain for the duration of the X session.  Modules will be
terminated by the window manager prior to restarts and quits, if
possible.  See the introductory section on modules.  The keyword
"module" may be omitted if ModuleName is distinct from all
built-in and function names.

"ModulePath"
Specifies a colon separated list of paths for fvwm to search
when looking for a module to load.  Individual directories do not need
trailing slashes.  Environment variables can be used here as well (i.e. 
$HOME or ${HOME}).

"Mouse Button Context Modifiers Function"
Defines a mouse binding, or removes the binding if Function is
'-'.  . Button is the mouse button number.  If Button is
zero then any button will perform the specified function.
Context describes where the binding applies.  Valid contexts are
R for the root window, W for an application window, T for a window
title bar, S for a window side, top, or bottom bar, F for a window
frame (the corners), I for an Icon window, or 0 through 9 for
title-bar buttons, or any combination of these letters.  A is for any
context except for title-bar buttons.  For instance, a context of FST
will apply when the mouse is anywhere in a window's border except the
title-bar buttons.

Modifiers is any combination of N for no modifiers, C for
control, S for shift, M for Meta, or A for any modifier.  For example,
a modifier of SM will apply when both the Meta and Shift keys are
down.  X11 modifiers mod1 through mod5 are represented as the digits 
1 through 5.

Function is one of fvwm's built-in functions.

The title bar buttons are numbered with odd numbered buttons on the
left side of the title bar and even numbers on the right.
Smaller-numbered buttons are displayed toward the outside of the
window while larger-numbered buttons appear toward the middle of the
window (0 is short for 10).  In summary, the buttons are numbered:

1 3 5 7 9    0 8 6 4 2

The highest odd numbered button which has an action bound to it
determines the number of buttons drawn on the left side of the title
bar.  The highest even number determines the number or right side
buttons which are drawn.  Actions can be bound to either mouse buttons
or keyboard keys.

"Move [ x y ]"
Allows the user to move a window.  If called from somewhere in a
window or its border, then that window will be moved.  If called from
the root window then the user will be allowed to select the target
window.

If the optional arguments x and y are provided, then the window will
be moved so that its upper left corner is at location (x,y).  The
units of x and y are percent-of-screen, unless a letter "p" is
appended to each coordinate, in which case the location is specified
in pixels.

Examples:

Mouse 1 T A Move
Mouse 2 T A Move 10 10
Mouse 3 T A Move 10p 10p

In the first example, an interactive move is indicated.  In the
second, the window whose title-bar is selected will be moved so that
its upper left hand corner is 10 percent of the screen width in from
the left of the screen, and 10 percent down from the top.  The final
example moves the window to coordinate (10,10) pixels.

"Nop"
Does nothing.  This is used to insert a blank line or separator in a
menu.  If the menu item specification is Nop " ", then a blank line is
inserted.  If it looks like Nop "", then a separator line is inserted.
Can also be used as the double-click action for Menu.

"Next [conditions] command"
Performs command (typically Focus) on the next window which
satisfies all conditions.  Conditions include "iconic",
"!iconic", "CurrentDesk", "Visible", "!Visible", and "CurrentScreen".
In addition, the condition may include a window name to match to.  The
window name may include the wildcards * and ?.  The window name,
class, and resource will be considered when attempting to find a
match.

"None [arguments] command"
Performs command if no window which satisfies all
conditions exists.  Conditions include "iconic", "!iconic",
"CurrentDesk", "Visible", "!Visible", and "CurrentScreen".  In
addition, the condition may include a window name to match to.  The
window name may include the wildcards * and ?.  The window name,
class, and resource will be considered when attempting to find a
match.

"OpaqueMoveSize percentage"
Tells fvwm the maximum size window with which opaque window
movement should be used.  The percentage is percent of the total
screen area.  With "OpaqueMove 0" all windows will be moved using the
traditional rubber-band outline.  With "OpaqueMove 100" all windows
will be move as solid windows.  The default is "OpaqueMove 5", which
allows small windows to be moved in an opaque manner but large windows
are moved as rubber-bands.

"PipeRead cmd"
Causes fvwm to read commands outputted from the program named
cmd.  Useful for building up dynamic menu entries based on a
directories contents, for example.

"PixmapPath path"
Specifies a colon separated list of full path names of directories
where pixmap (color) icons can be found.  Each path should start with
a slash.  Environment variables can be used here as well (i.e.  $HOME
or ${HOME}).

"Popup \PopupName"
This built-in has two purposes: to bind a menu to a key or mouse
button, and to bind a sub-menu into a menu.  The formats for the two
purposes differ slightly.

To bind a previously defined pop-up menu to a key or mouse button:

The following example binds mouse buttons 2 and 3 to a pop-up called
"Window Ops".  The menu will pop up if the buttons 2 or 3 are pressed
in the window frame, side-bar, or title-bar, with no modifiers (none
of shift, control, or meta).

Mouse 2 FST N Popup "Window Ops"
Mouse 3 FST N Popup "Window Ops"

Pop-ups can be bound to keys through the use of the Key built in.
Pop-ups can be operated without using the mouse by binding to keys and
operating via the up arrow, down arrow, and enter keys.

To bind a previously defined pop-up menu to another menu, for use as a 
sub-menu:

The following example defines a sub menu, "Quit-Verify" and binds it into a
main menu, called "RootMenu":

AddToMenu Quit-Verify   "Really Quit Fvwm?" Title
+                       "Yes, Really Quit"  Quit
+                       "Restart Fvwm2"     Restart fvwm
+                       "Restart Fvwm 1.xx" Restart fvwm
+                       ""                  Nop
+                       "No, Don't Quit"    Nop

AddToMenu RootMenu      "Root Menu"         Title
+ "Open an XTerm Window"  Popup NewWindowMenu
+ "Login as Root"         Exec xterm -fg green -T Root -n Root -e su -
+ "Login as Anyone"       Popup AnyoneMenu
+ "Remote Hosts"          Popup HostMenu
+ ""                      Nop
+ "X utilities"           Popup Xutils
+ ""                      Nop
+ "Fvwm Modules"          Popup Module-Popup
+ "Fvwm Window Ops"       Popup Window-Ops
+ ""                      Nop
+ "Previous Focus"        Prev [*] Focus
+ "Next Focus"            Next [*] Focus
+ ""                      Nop
+ "Refresh screen"        Refresh
+ "Recapture screen"      Recapture
+ ""                      Nop
+ "Reset X defaults"      Exec xrdb -load $HOME/.Xdefaults
+ ""                      Nop
+ ""                      Nop
+ "Quit"                  Popup Quit-Verify

Popup differs from Menu in that pop-ups do not stay up if the user
simply clicks.  These are Twm style popup-menus, which are a little
hard on the wrist.  Menu provides Motif or Microsoft-Windows style
menus which will stay up on a click action.

"Prev"
Performs command (typically Focus) on the previous window which
satisfies all conditions.  Conditions include "iconic",
"!iconic", "CurrentDesk", "Visible", "!Visible", and "CurrentScreen".
In addition, the condition may include a window name to match to.  The
window name may include the wildcards * and ?.  The window name,
class, and resource will be considered when attempting to find a
match.

"Quit"
Exits fvwm, generally causing X to exit too.

"Raise"
Allows the user to raise a window.

"RaiseLower"
Alternately raises and lowers a window.

"Read filename"
Causes fvwm to read commands from the file named filename.

"Recapture"
Causes fvwm to recapture all of its windows.  This ensures that the
latest style parameters will be used.  The recapture operation is
visually disturbing.

"Refresh"
Causes all windows on the screen to redraw themselves.

"Resize [ x y ]"
Allows the user to resize a window.

If the optional arguments x and y are provided, then the window will
be resized so that its dimensions are x by y).  The units
of x and y are percent-of-screen, unless a letter "p" is appended to
each coordinate, in which case the location is specified in pixels.

"Restart  WindowManagerName "
Causes fvwm to restart itself if WindowManagerName is "fvwm",
or to switch to an alternate window manager if WindowManagerName is
other than "fvwm".  If the window manager is not in your default
search path, then you should use the full path name for
WindowManagerName.

This command should not have a trailing ampersand or any command line
arguments and should not make use of any environmental variables.  Of
the following examples, the first two are sure losers, but the third
is OK:

Key F1 R N Restart fvwm &
Key F1 R N Restart $(HOME)/bin/fvwm
Key F1 R N Restart /home/nation/bin/fvwm

"SendToModule modulename string"
Sends an arbitrary string (no quotes required) to all modules matching
modulename, which may contain wildcards.  This only makes sense
if the module is set up to understand and deal with these strings
though...  Can be used for module to module communication, or
implementation of more complex commands in modules.

"Scroll horizonal vertical"
Scrolls the virtual desktop's viewport by horizontal pages in
the x-direction and vertical pages in the y-direction.  Either
or both entries may be negative.  Both horizontal and vertical values
are expressed in percent of pages, so "Scroll 100 100" means to scroll
down and left by one full page.  "Scroll 50 25" means to scroll left
half a page and down a quarter of a page.  The scroll function should
not be called from pop-up menus. Normally, scrolling stops at the edge
of the desktop.

If the horizontal and vertical percentages are multiplied by 1000 then
scrolling will wrap around at the edge of the desktop.  If "Scroll
100000 0" is executed over and over fvwm will move to the next
desktop page on each execution and will wrap around at the edge of the
desktop, so that every page is hit in turn.

If the letter "p" is appended to each coordinate (horizontal and/or
vertical), then the scroll amount will be measured in pixels.

"Stick"
Makes a window sticky if it is not already sticky, or non-sticky if it
is already sticky.

"Style windowname options"
This command is intended to replace the old fvwm 1.xx global commands
NoBorder, NoTitle, StartsOnDesk, Sticky, StaysOnTop, Icon,
WindowListSkip, CirculateSkip, SuppressIcons, BoundaryWidth,
NoBoundaryWidth, StdForeColor, and StdBackColor with a single flexible
and comprehensive window(s) specific command.  This command is used to
set attributes of a window to values other than the default or to set
the window manager default styles.

windowname can be a window's name, class, or resource string.
It can contain the wildcards * and/or ?, which are matched in the
usual Unix filename manner.  They are searched in the reverse order
stated, so that Style commands based on the name override or augment
those based on the class, which override or augment those based on the
resource string.

Note - windows that have no name (WM_NAME) are given a name of
"Untitled", and windows that don't have a class (WM_CLASS, res_class)
are given Class = "NoClass" and those that don't have a resource
(WM_CLASS, res_name) are given Resource = "NoResource".

options is a comma separated list containing some or all of the
keywords BorderWidth, HandleWidth, NoIcon/Icon, IconBox,
NoTitle/Title, NoHandles/Handles, WindowListSkip/WindowListHit,
CirculateSkip/CirculateHit, StaysOnTop/StaysPut, Sticky/Slippery,
StartIconic/StartNormal, Color, ForeColor, BackColor,
StartsOnDesk/StartsAnyWhere, IconTitle/NoIconTitle,
MWMButtons/FvwmButtons, MWMBorder/FvwmBorder, MWMDecor/NoDecorHint,
MWMFunctions/NoFuncHint, HintOverride/NoOverride, NoButton/Button,
OLDecor/NoOLDecor, StickyIcon/SlipperyIcon,
SmartPlacement/DumbPlacement, RandomPlacement/ActivePlacement,
DecorateTransient/NakedTransient, SkipMapping/ShowMapping, UseStyle,
NoPPosition/UsePPosition, Lenience/NoLenience,
ClickToFocus/SloppyFocus/MouseFocus|FocusFollowsMouse.

In the above list some options are listed as
style-option/opposite-style-option.  The opposite-style-option for
entries that have them describes the fvwm default behavior and
can be used if you want to change the fvwm default behavior.

Icon takes an (optional) unquoted string argument which is the icon
bitmap or pixmap to use.

IconBox takes four numeric arguments:

IconBox	l t r b

Where l is the left coordinate, t is the top, r is right and b is
bottom. Negative coordinates indicate distance from the right or
bottom of the screen.  The iconbox is a region of the screen will fvwm
will attempt to put icons for this window, as long as they do not
overlap other icons.

StartsOnDesk takes a numeric argument which is the desktop number on
which the window should be initially placed.  Note that standard Xt
programs can also specify this via a resource (eg "-xrm '*Desk: 1'").

BorderWidth takes a numeric argument which is the width of the border
to place the window if it does not have resize-handles.

HandleWidth takes a numeric argument which is the width of the border
to place the window if it does have resize-handles.

Button and NoButton take a numeric argument which is the number of the
title-bar button which is to be included/omitted.

StickyIcon makes the window sticky when its iconified.  It will
deiconify on top the active desktop.

MWMButtons makes the Maximize button look "pressed in" when the window
is maximized.

MWMBorder makes the 3-D bevel more closely match mwm's.

MWMDecor makes fvwm attempt to recognize and respect the mwm
decoration hints that applications occasionally use.

MWMFunctions makes fvwm attempt to recognize and respect the mwm
prohibited operations hints that applications occasionally use.
HintOverride makes fvwm shade out operations that mwm would prohibit,
but it lets you perform the operation anyway.

OLDecor makes fvwm attempt to recognize and respect the olwm and olvwm
hints that many older XView and OLIT applications use.

Color takes two arguments.  The first is the window-label text color
and the second is the window decoration's normal background color.
The two colors are separated with a slash.  If the use of a slash
causes problems then the separate ForeColor and BackColor options can
be used.

UseStyle takes one arg, which is the name of another style.  That way
you can have unrelated window names easily inherit similiar traits
without retyping.  For example: 'Style "rxvt" UseStyle "XTerm"'.

SkipMapping tells fvwm not to switch to the desk the window is on when
it gets mapped initially (useful with StartsOnDesk).

Lenience instructs fvwm to ignore the convention in the ICCCM which
states that if an application sets the input field of the wm_hints
structure to False, then it never wants the window manager to give it
the input focus.  The only application that I know of which needs this
is sxpm, and that is a silly bug with a trivial fix and has no overall
effect on the program anyway.  Rumor is that some older applications
have problems too.

ClickToFocus instructs fvwm to give the focus to the window when it is
clicked in.  The default MouseFocus (or its alias FocusFollowsMouse)
tells fvwm to give the window the focus as soon as the pointer enters
the window, and take it away when the pointer leaves the window.
SloppyFocus is similiar, but doesn't give up the focus if the pointer
leaves the window to pass over the root window or a ClickToFocus
window (unless you click on it, that is), which makes it possible to
move the mouse out of the way without losing focus.

NoPPosition instructs fvwm to ignore the PPosition field when adding
new windows.  Adherence to the PPosition field is required for some
applications, but if you don't have one of those its a real headache.

RandomPlacement causes windows which would normally require user
placement to be automatically placed in ever-so-slightly random
locations.  For the best of all possible worlds use both
RandomPlacement and SmartPlacement.

SmartPlacement causes windows which would normally require user
placement to be automatically placed in a smart location - a location
in which they do not overlap any other windows on the screen.  If no
such position can be found user placement or random placement (if
specified) will be used as a fall-back method.  For the best of all
possible worlds use both RandomPlacement and SmartPlacement.

An example:

# Change default fvwm behavior to no title-bars on windows!
# Also define a default icon.
Style "*" NoTitle,Icon unknown1.xpm, BorderWidth 4,HandleWidth 5

# now, window specific changes:
Style "Fvwm*"     NoHandles,Sticky,WindowListSkip,BorderWidth 0
Style "Fvwm Pager"                 StaysOnTop, BorderWidth 0
Style "*lock"     NoHandles,Sticky,StaysOnTop,WindowListSkip
Style "xbiff"               Sticky,           WindowListSkip
Style "FvwmButtons" NoHandles,Sticky,WindowListSkip
Style "sxpm"      NoHandles
Style "makerkit"  

# Put title-bars back on xterms only!
Style "xterm"     Title, Color black/grey

Style "rxvt"      Icon term.xpm
Style "xterm"     Icon rterm.xpm
Style "xcalc"     Icon xcalc.xpm
Style "xbiff"     Icon mail1.xpm
Style "xmh"       Icon mail1.xpm, StartsOnDesk 2
Style "xman"      Icon xman.xpm
Style "matlab"    Icon math4.xpm, StartsOnDesk 3
Style "xmag"      Icon magnifying_glass2.xpm
Style "xgraph"    Icon graphs.xpm
Style "FvwmButtons" Icon toolbox.xpm

Style "Maker"     StartsOnDesk 1
Style "signal"    StartsOnDesk 3           

Note that all properties for a window will be OR'ed together.  In the
above example "FvwmPager" gets the property StaysOnTop via an exact
window name match but also gets NoHandles, Sticky, and WindowListSkip
by a match to "Fvwm*".  It will get NoTitle by virtue of a match to
"*".  If conflicting styles are specified for a window, then the last
style specified will be used.

If the NoIcon attribute is set then the specified window will simply
disappear when it is iconified.  The window can be recovered through
the window-list.  If Icon is set without an argument then the NoIcon
attribute is cleared but no icon is specified.  An example which
allows only the FvwmPager module icon to exist:

Style "*" NoIcon
Style "Fvwm Pager" Icon

"Title"
Does nothing.  This is used to insert a title line in a popup or menu.

"TitleStyle [justification] [appearance]"
Sets the title bar style.  Justifications can be "Centered",
"RightJustified", or "LeftJustified" and Appearance can be "Raised",
"Sunk", or "Flat".  Defaults to Centered and Raised.  One or both
values can be specified when used.  Examples:

TitleStyle Centered Raised
TitleStyle LeftJustified Flat

"WarpToWindow x y"
Warps the cursor to the associated window.  The parameters x and y
default to percentage of window down and in from the upper left hand
corner (or number of pixels down and in if 'p' is appended to the
numbers).

"Wait name"
This built-in is intended to be used in fvwm functions only.  It
causes execution of a function to pause until a new window name
name appears. Fvwm remains fully functional during a wait.
This is particularly useful in the InitFunction if you are trying to
start windows on specific desktops:

AddToFunc InitFunction "I" exec xterm -geometry 80x64+0+0
+                      "I" Wait xterm
+                      "I" Desk	0 2
+                      "I" Exec	xmh -font fixed -geometry 507x750+0+0
+                      "I" Wait xmh
+                      "I" Desk 0 0

The above function starts an xterm on the current desk, waits for it
to map itself, then switches to desk 2 and starts an xmh.  After the
xmh window appears control moves to desk 0.

"WindowList arg1 arg2"
Generates a pop-up menu (and pops it up) in which the title and
geometry of each of the windows currently on the desk top are shown.
The geometry of iconified windows is shown in brackets.  Selecting an
item from the window list pop-up menu will cause that window to be
moved onto the desktop if it is currently not on it, will move the
desktop viewport to the page containing the upper left hand corner of
the window, will de-iconify the window if it is iconified, and will
raise the window.

If arg1 is an even number then the windows will be listed using
the window name (the name that shows up in the title-bar).  If it is
odd then the window's icon name is used.

If arg1 is less than 2 then all windows on all desktops (except
those listed in WindowListSkip directives) will be shown.

If arg1 is 2 or 3 then only windows on the current desktop will
be shown.

If arg1 is 4 or 5 then only windows on desktop number arg2
will be shown.

"WindowFont fontname"
Makes fvwm use font fontname instead of "fixed" for window
title-bars.

"WindowsDesk new_desk"
Moves the selected window the the desktop specified as new_desk.

"XORvalue number"
Changes the value with which bits are XOR'ed when doing rubber-band
window moving or resizing.  Setting this value is a trial-and-error
process.

"+"
Used to continue adding to the last specified function or menu.  See
the discussion for AddToFunc and AddToMenu.


KEYBOARD SHORTCUTS
All (I think) window manager operations can be performed from the
keyboard so mouseless operation should be possible.  In addition to
scrolling around the virtual desktop by binding the Scroll built-in to
appropriate keys, pop-ups, move, resize, and most other built-ins can
be bound to keys.  Once a built-in function is started the pointer is
moved by using the up, down, left, and right arrows, and the action is
terminated by pressing return.  Holding down the shift key will cause
the pointer movement to go in larger steps and holding down the
control key will cause the cursor movement to go in smaller steps.
Standard emacs and vi cursor movement controls (^n, ^p, ^f, ^b, and
^j, ^k, ^h, ^l) can be used instead of the arrow keys.

SUPPLIED CONFIGURATION
A sample configuration file, .fvwmrc, is supplied with the fvwm
distribution.  It is well commented and can be used as a source of
examples for fvwm configuration.

USE ON MULTI-SCREEN DISPLAYS
If the -s command line argument is not given, fvwm will
automatically start up on every screen on the specified display.
After fvwm starts each screen is treated independently.
Restarts of fvwm need to be performed separately on each screen.
The use of EdgeScroll 0 0 is strongly recommended for multi-screen
displays.

You may need to quit on each screen to quit from the X session
completely.

BUGS
As of fvwm 0.99 there were exactly 39.342 unidentified bugs.
Identified bugs have mostly been fixed, though.  Since then 9.34 bugs
have been fixed.  Assuming that there are at least 10 unidentified
bugs for every identified one, that leaves us with 39.342 - 9.32 + 10
* 9.34 = 123.402 unidentified bugs.  If we follow this to its logical
conclusion we will have an infinite number of unidentified bugs before
the number of bugs can start to diminish, at which point the program
will be bug-free.  Since this is a computer program infinity =
3.4028e+38 if you don't insist on double-precision.  At the current
rate of bug discovery we should expect to achieve this point in
3.37e+27 years.  I guess I better plan on passing this thing on to my
children....

Known bugs can be found in the BUGS file in the distribution, and in
the TO-DO list.

Bug reports can be sent to the FVWM mailing list (see the FAQ).

AUTHOR
Robert Nation with help from many people, based on twm code,
which was written by Tom LaStrange.  Rob has since 'retired' from
working on fvwm though, so Charles Hines maintains it's care and
feeding currently.

