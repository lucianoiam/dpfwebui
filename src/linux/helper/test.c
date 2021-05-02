/*
 * Copyright (C) 2006, 2007 Apple Inc.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2011 Lukasz Slachciak
 * Copyright (C) 2011 Bob Murphy
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// gcc test.c -lX11 `pkg-config --cflags --libs gtk+-3.0` `pkg-config --cflags --libs webkit2gtk-4.0`

/*
  GTK WebKit
  https://wiki.gnome.org/Projects/WebKitGtk/ProgrammingGuide/Tutorial
  libwebkit2gtk-4.0-dev
  libgtk-3-dev 
*/
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

/*
  https://stackoverflow.com/questions/14788439/getting-x11-window-handle-from-gtkwidget
*/
#include <gdk/gdkx.h>

/*
  https://www.geeks3d.com/20120102/programming-tutorial-simple-x11-x-window-code-sample-for-linux-and-mac-os-x/
*/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include <sys/utsname.h>

/*
  https://www.systutorials.com/a-posix_spawn-example-in-c-to-create-child-process-on-linux/
*/
#include <spawn.h>


static void destroyWindowCb(GtkWidget* widget, GtkWidget* window);
static gboolean closeWebViewCb(WebKitWebView* webView, GtkWidget* window);

void host();
void web_browser();

extern char **environ;


int main(int argc, char* argv[])
{
    if ((argc > 2) && (strcmp(argv[1], "web") == 0)) {
        unsigned long windowId;
        sscanf(argv[2], "%lu", &windowId);
        web_browser(windowId);
    } else {
        host();
    }

    return 0;
}


void host()
{
  Display* dpy = XOpenDisplay(NULL);
  if (dpy == NULL) 
  {
    fprintf(stderr, "Cannot open display\n");
    exit(1);
  }
 
  int s = DefaultScreen(dpy);
  Window win = XCreateSimpleWindow(dpy, RootWindow(dpy, s), 10, 10, 800, 600, 1,
                                   BlackPixel(dpy, s), WhitePixel(dpy, s));
  XSelectInput(dpy, win, ExposureMask | KeyPressMask);
  XMapWindow(dpy, win);
  XStoreName(dpy, win, "Web view test");
 
  Atom WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False); 
  XSetWMProtocols(dpy, win, &WM_DELETE_WINDOW, 1);  
 
  bool uname_ok = false;
  struct utsname sname;  
  int ret = uname(&sname);
  if (ret != -1)
  {
    uname_ok = true;
  }

    char winId[32];
    sprintf(winId, "%lu", (unsigned long)win);
    pid_t pid;
    char *argv[] = {"a.out", "web", winId, NULL};
    int status;
    printf("Host window %s\n", winId);
    status = posix_spawn(&pid, "a.out", NULL, NULL, argv, environ);

  XEvent e;
  while (1) 
  {
    XNextEvent(dpy, &e);
    if (e.type == Expose) 
    {
      int y_offset = 20;
      XWindowAttributes  wa;
      XGetWindowAttributes(dpy, win, &wa);
      int width = wa.width;
      int height = wa.height;
      char buf[128]={0};
      sprintf(buf, "Current window size: %dx%d", width, height);
      XDrawString(dpy, win, DefaultGC(dpy, s), 10, y_offset, buf, strlen(buf));
    }
 
    if (e.type == KeyPress)
    {
      char buf[128] = {0};
      KeySym keysym;
      int len = XLookupString(&e.xkey, buf, sizeof buf, &keysym, NULL);
      if (keysym == XK_Escape)
        break;
    }
 
    if ((e.type == ClientMessage) && 
        (unsigned int)(e.xclient.data.l[0]) == WM_DELETE_WINDOW)
    {
      break;
    }
  }
 
  XDestroyWindow(dpy, win);
  XCloseDisplay(dpy);
}


void web_browser(unsigned long hostWindowId)
{
    printf("Received host window %lu\n", hostWindowId);

    // Initialize GTK+
    //gtk_init(&argc, &argv);
    gtk_init(0, NULL);

    // Create an 800x600 window that will contain the browser instance
    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);

    // Create a browser instance
    WebKitWebView *webView = WEBKIT_WEB_VIEW(webkit_web_view_new());

    // Put the browser area into the main window
    gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(webView));

    // Set up callbacks so that if either the main window or the browser instance is
    // closed, the program will exit
    g_signal_connect(main_window, "destroy", G_CALLBACK(destroyWindowCb), NULL);
    g_signal_connect(webView, "close", G_CALLBACK(closeWebViewCb), main_window);

    // Load a web page into the browser instance
    webkit_web_view_load_uri(webView, "http://www.webkitgtk.org/");

    // Make sure that when the browser area becomes visible, it will get mouse
    // and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(webView));

    // Make sure the main window and all its contents are visible
    gtk_widget_show_all(main_window);


    Display* dpy = XOpenDisplay(NULL);
    Window browserWindowId = gdk_x11_window_get_xid(gtk_widget_get_window(main_window));
    XReparentWindow(dpy, browserWindowId, hostWindowId, 0, 0);
    XFlush(dpy);

    // Run the main GTK+ event loop
    gtk_main();
}


static void destroyWindowCb(GtkWidget* widget, GtkWidget* window)
{
    gtk_main_quit();
}

static gboolean closeWebViewCb(WebKitWebView* webView, GtkWidget* window)
{
    gtk_widget_destroy(window);
    return TRUE;
}
