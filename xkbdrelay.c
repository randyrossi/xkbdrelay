/* Copyright (c) 2007
 *      Randy Rossi (randy.rossi@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 ****************************************************************
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/XTest.h>

#define TEST_DELAY 0

GC src_win_gc;
Window src_window;

int buttonsX[] = { 20, 120};
int buttonsY[] = { 0, 0}; 
int buttonsXW[] = { 80, 140};
int buttonsYW[] = { 34, 34};
char* buttonsText[] = { "Close", "Grab"};

int pointerIsGrabbed = 0;

static int CLOSE   = 0;
static int GRAB    = 1;

static int NUM_BUTTONS = 2;


char* instructions[] = { 
	"Keep this window in focus.",
	"All keystrokes will be sent",
	"to the destination DISPLAY",
	"you specified on the command",
	"line."
};


Window createSourceWindow(Display *display, Window root_window, int screen_num, int argc, char* argv[], int px,int py,int pw,int ph)
{
   XSizeHints size_hints;
   XWMHints wm_hints;
   XClassHint class_hints;
   char *src_window_name_str = "Type Here";
   char *src_window_icon_str = "";
   XTextProperty src_window_name;
   XTextProperty src_window_icon;

   src_window = XCreateSimpleWindow(display,root_window,px,py,pw,ph,1,BlackPixel(display,screen_num),WhitePixel(display,screen_num));

   size_hints.flags = PPosition | PSize | PMinSize; 
   size_hints.min_width = pw;
   size_hints.min_height = ph;

   wm_hints.initial_state = NormalState;
   wm_hints.input = True;
   wm_hints.flags = StateHint | InputHint;

   class_hints.res_name = "Test";
   class_hints.res_class = "BasicWin";

   XStringListToTextProperty(&src_window_name_str,1,&src_window_name);
   XStringListToTextProperty(&src_window_icon_str,1,&src_window_icon);

   XSetWMProperties(display,src_window, &src_window_name, &src_window_icon, argv,argc, &size_hints, &wm_hints, &class_hints);

   XSelectInput(display,src_window, ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ButtonMotionMask );

   src_win_gc = XCreateGC(display,src_window,0,NULL);


   return src_window;
}

void drawinstructions(Display* display, Window win, GC gc, XFontStruct *fontinfo, unsigned int wwidth,unsigned int wheight, char *str[], int numLines, int buttonInvertIndex)
{
   int width;
   int heightPerLine;
   int line;
   int Y;
   int buttonIndex;

   XClearWindow(display, win);

   heightPerLine = 15;

   Y = (wheight/2) - (numLines*heightPerLine)/2;

   for (line=0;line<numLines;line++) {
       width = XTextWidth(fontinfo, str[line], strlen(str[line]));
       XDrawString(display, win, gc, (wwidth-width)/2,Y,str[line],strlen(str[line]));
       Y=Y+heightPerLine;
   }
   
   Y=Y+10;
   
   for (buttonIndex = 0; buttonIndex < NUM_BUTTONS; buttonIndex++) {
       XDrawRectangle(display, win, gc, buttonsX[buttonIndex], buttonsY[buttonIndex], buttonsXW[buttonIndex], buttonsYW[buttonIndex]);
       width = XTextWidth(fontinfo, buttonsText[buttonIndex], strlen(buttonsText[buttonIndex]));
       XDrawString(display, win, gc, buttonsXW[buttonIndex]/2-width/2+buttonsX[buttonIndex],buttonsYW[buttonIndex]/2+buttonsY[buttonIndex],buttonsText[buttonIndex],strlen(buttonsText[buttonIndex]));
       if (buttonIndex == buttonInvertIndex) {
          XSetFunction(display, gc, GXinvert);
          XFillRectangle(display, win, gc, buttonsX[buttonIndex], buttonsY[buttonIndex], buttonsXW[buttonIndex], buttonsYW[buttonIndex]);
       } else {
          XSetFunction(display, gc, GXcopy);
          XDrawRectangle(display, win, gc, buttonsX[buttonIndex], buttonsY[buttonIndex], buttonsXW[buttonIndex], buttonsYW[buttonIndex]);
       }
   }
}

int main(int argc, char* argv[]) {
   int buttonIndex;
   int buttonHitIndex;

   Display *src_display;
   Display *dest_display;
   int src_screen_num;
   Window src_window;
   XEvent event;
   XKeyPressedEvent *kpev;
   XKeyPressedEvent *krev;
   XMotionEvent *mot;
   XButtonPressedEvent *bpev;
   XButtonReleasedEvent *brev;
   XFontStruct *fontinfo;
   int grabPointer;
   int i;
   int gx =0;
   int gy =0;
   unsigned int gw = 800;
   unsigned int gh = 600;
   int escaped = 0;

   char srcDisplayEnv[32];
   char destDisplayEnv[32];

   if (argc < 3) {
      printf("Usage: %s <SENDER_DISPLAY> <RECEIVER_DISPLAY> [-grab]\n\n",argv[0]);
      printf("Example:\n\n");
      printf("   %s from_host:0.0 to_host:0.0\n\n", argv[0]);
      
      exit(0);
   }

   strcpy(srcDisplayEnv , argv[1]);
   strcpy(destDisplayEnv , argv[2]);

   grabPointer = 0;
   if (argc > 3) { 
      for (i=3;i<argc;i++) {
         if (strcasecmp(argv[i],"-grab") == 0) {
            grabPointer = 1;
         } else if (strcasecmp(argv[i],"-geometry") == 0) {
            XParseGeometry(argv[i+1],&gx,&gy,&gw,&gh);
	    i++;
         }
      }
   }

   if (gw < 320) gw = 320;
   if (gh < 200) gh = 200;

   buttonsY[0] = gh - 50;
   buttonsY[1] = gh - 50;

   if ((src_display=XOpenDisplay(srcDisplayEnv)) == NULL) {
      fprintf(stderr,"%s: cannot connect to X server\n",srcDisplayEnv);
      exit(1);
   }

   if ((dest_display=XOpenDisplay(destDisplayEnv)) == NULL) {
      fprintf(stderr,"%s: cannot connect to X server\n",destDisplayEnv);
      exit(1);
   }


   src_screen_num = DefaultScreen(src_display);

   src_window = createSourceWindow(src_display, RootWindow(src_display,src_screen_num), src_screen_num, argc, argv, gx,gy, gw, gh);

   if ((fontinfo=XLoadQueryFont(src_display,"9x15")) == NULL) {
      fprintf (stderr,"Can't open 9x15 font\n");
      exit(1);
   }

   XSetForeground(src_display, src_win_gc, BlackPixel(src_display, src_screen_num));
   XSetFont(src_display,src_win_gc,fontinfo->fid);
   XMapWindow(src_display,src_window);

   if (grabPointer) {
       XTestFakeMotionEvent(src_display, 0, 0, 0, TEST_DELAY);
       XFlush(src_display);
       XGrabPointer(src_display, src_window, True, 0, GrabModeAsync, GrabModeAsync, src_window, None, CurrentTime);
       pointerIsGrabbed = 1;
       buttonsText[GRAB]="Press Cntrl-Alt";
   }
	       
   printf ("Version 2.0\n");
   printf ("Sending keyboard and mouse events from %s to %s\n",srcDisplayEnv, destDisplayEnv);

   while (1) {
      XNextEvent(src_display,&event);
      fflush(stdout);
      switch (event.type)
      {
      case MotionNotify:
	 mot = (XMotionEvent*) &event;
         XTestFakeMotionEvent(dest_display, 0, mot->x, mot->y, TEST_DELAY);
         XFlush(dest_display);
         break;
      case KeyPress:
         kpev = (XKeyPressedEvent*) &event;
	 if (kpev->keycode == 64 && (kpev->state & ControlMask) > 0 ) {
	    if (pointerIsGrabbed == 1 && escaped == 0) {
               XUngrabPointer(src_display, CurrentTime);
	       pointerIsGrabbed = 0;
               buttonsText[GRAB]="Grab";
               drawinstructions(src_display, src_window, src_win_gc, fontinfo, gw, gh, instructions, 5, -1);
	       break;
	    }
	 }
         XTestFakeKeyEvent(dest_display, kpev->keycode, True, TEST_DELAY);
         XFlush(dest_display);
         break;
      case KeyRelease:
         krev = (XKeyReleasedEvent*) &event;
printf ("%d\n",krev->keycode);
         XTestFakeKeyEvent(dest_display, kpev->keycode, False, TEST_DELAY);
         XFlush(dest_display);
         break;
      case Expose:
         drawinstructions(src_display, src_window, src_win_gc, fontinfo, gw, gh, instructions, 5, -1);
         
         break;
      case ButtonPress:
         bpev = (XButtonPressedEvent*) &event;
	 if (pointerIsGrabbed == 0) {
	    buttonHitIndex = -1;
	    for (buttonIndex = 0; buttonIndex < NUM_BUTTONS; buttonIndex++) {
                if (bpev->x >= buttonsX[buttonIndex] && bpev->x <= (buttonsX[buttonIndex]+buttonsXW[buttonIndex]) &&
                    bpev->y >= buttonsY[buttonIndex] && bpev->y <= (buttonsY[buttonIndex]+buttonsYW[buttonIndex])) {
                    buttonHitIndex = buttonIndex;
		    break;
                } 
	    }

            if (buttonHitIndex != -1) {
                drawinstructions(src_display, src_window, src_win_gc, fontinfo, gw, gh, instructions, 5, buttonHitIndex);
            }
	 }

         XTestFakeButtonEvent(dest_display, bpev->button, True, TEST_DELAY);
         XFlush(dest_display);

         break;
      case ButtonRelease:
         bpev = (XButtonReleasedEvent*) &event;
	 if (pointerIsGrabbed == 0) {
            drawinstructions(src_display, src_window, src_win_gc, fontinfo, gw, gh, instructions, 5, -1);

	    buttonHitIndex = -1;
	    for (buttonIndex = 0; buttonIndex < NUM_BUTTONS; buttonIndex++) {
                if (bpev->x >= buttonsX[buttonIndex] && bpev->x <= (buttonsX[buttonIndex]+buttonsXW[buttonIndex]) &&
                    bpev->y >= buttonsY[buttonIndex] && bpev->y <= (buttonsY[buttonIndex]+buttonsYW[buttonIndex])) {
                    buttonHitIndex = buttonIndex;
		    break;
                }
	    }
	 
	    if (buttonHitIndex == CLOSE) {
               XFreeGC(src_display, src_win_gc);
               XCloseDisplay(src_display);
               XCloseDisplay(dest_display);
               exit(0);
            } else if (buttonHitIndex == GRAB) {
               XGrabPointer(src_display, src_window, True, 0, GrabModeAsync, GrabModeAsync, src_window, None, CurrentTime);
	       pointerIsGrabbed = 1;
               buttonsText[GRAB]="Press Cntrl-Alt";
               drawinstructions(src_display, src_window, src_win_gc, fontinfo, gw, gh, instructions, 5, -1);
            }
	 }
	 
         XTestFakeButtonEvent(dest_display, bpev->button, False, TEST_DELAY);
         XFlush(dest_display);

      }
   }
   
}
