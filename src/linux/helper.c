/*
 * dpf-webui
 * Copyright (C) 2021 Luciano Iam <lucianoiam@protonmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>


static void destroyWindowCb(GtkWidget* widget, GtkWidget* window);
static gboolean closeWebViewCb(WebKitWebView* webView, GtkWidget* window);

void createBrowser();


int main(int argc, char* argv[])
{
    unsigned int pluginWindow;
    Display* display;

    if (argc < 3) {
        fprintf(stderr, "Invalid argument count\n");
        return -1;
    } 

    if (sscanf(argv[1], "%x", &pluginWindow) == 0) {
        fprintf(stderr, "Invalid parent window ID\n");
        return -1;
    }

    if (strlen(argv[2]) == 0) {
        fprintf(stderr, "Invalid URL\n");
        return -1;
    }

    if ((display = XOpenDisplay(NULL)) == NULL)  {
        fprintf(stderr, "Cannot open display\n");
        return -1;
    }

    fprintf(stderr, "Plugin window at %x\n", pluginWindow);

    /*
      For debug purposes, how to get a window id given its title
      wmctrl -l | grep -i < title > | awk '{print $1}'
    */
    createBrowser(display, pluginWindow, argv[2]);

    // Run the main GTK+ event loop
    gtk_main();
}

void createBrowser(Display *display, Window pluginWindow, const char *url)
{
    // Initialize GTK+
    gtk_init(0, NULL);

    // Query plugin window attributes
    XWindowAttributes *attr;
    XGetWindowAttributes(display, pluginWindow, attr);

    // Create a window that will contain the browser instance
    GtkWidget *gtkHelperWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(gtkHelperWindow), attr->width, attr->height);

    // Create a browser instance
    WebKitWebView *webView = WEBKIT_WEB_VIEW(webkit_web_view_new());

    // Put the browser area into the main window
    gtk_container_add(GTK_CONTAINER(gtkHelperWindow), GTK_WIDGET(webView));

    // Set up callbacks so that if either the main window or the browser instance is
    // closed, the program will exit
    g_signal_connect(gtkHelperWindow, "destroy", G_CALLBACK(destroyWindowCb), NULL);
    g_signal_connect(webView, "close", G_CALLBACK(closeWebViewCb), gtkHelperWindow);

    // Load a web page into the browser instance
    webkit_web_view_load_uri(webView, url);

    // Make sure that when the browser area becomes visible, it will get mouse
    // and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(webView));

    // Make sure the main window and all its contents are visible
    gtk_widget_show_all(gtkHelperWindow);

    // Move helper window into plugin window
    Window xHelperWindow = gdk_x11_window_get_xid(gtk_widget_get_window(gtkHelperWindow));
    XReparentWindow(display, xHelperWindow, pluginWindow, 0, 0);

    /*
       TODO
     - Create a Makefile for the helper, meanwhile
       gcc helper.c -o ../../bin/helper -lX11 `pkg-config --cflags --libs gtk+-3.0` `pkg-config --cflags --libs webkit2gtk-4.0` 
     - Keep browser size in sync with plugin window size
     - Inter process communication
     - Ability to receive a close command for graceful shutdown,
       ie. prevent Gdk-WARNING **: GdkWindow unexpectedly destroyed
    */

    XFlush(display);
}

static void destroyWindowCb(GtkWidget* widget, GtkWidget* window)
{
    fprintf(stderr, "destroyWindowCb()\n");
    gtk_main_quit();
}

static gboolean closeWebViewCb(WebKitWebView* webView, GtkWidget* window)
{
    fprintf(stderr, "closeWebViewCb()\n");
    gtk_widget_destroy(window);
    return TRUE;
}
