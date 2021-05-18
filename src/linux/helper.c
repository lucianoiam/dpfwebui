/*
 * Hip-Hop / High Performance Hybrid Audio Plugins
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

static void create_browser();
static gboolean ipc_read_cb(GIOChannel *source, GIOCondition condition, gpointer data);
static gboolean ipc_write_cb(GIOChannel *source, GIOCondition condition, gpointer data);
static gboolean close_webview_cb(WebKitWebView* webView, GtkWidget* window);
static void destroy_window_cb(GtkWidget* widget, GtkWidget* window);


int main(int argc, char* argv[])
{
    unsigned int pluginWindow;
    Display* display;
    char zendp[32];

    // TODO: the only required argument should be a numeric ipc channel identifier
    if (argc < 3) {
        fprintf(stderr, "Invalid argument count\n");
        return -1;
    }

    /*
        TODO: receive commands via IPC:
        - Set content, arg = url
        - Reparent window, arg = window id
        - Shutdown
    */
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

    // Initialize GTK+
    gtk_init(0, NULL);
    
    create_browser(display, pluginWindow, argv[2]);

    // Setup the IPC channel
    int fd1 = open("/tmp/helper2", O_RDWR);
    int fd2 = open("/tmp/helper1", O_RDWR);
    GIOChannel* channel1 = g_io_channel_unix_new(fd1);    
    g_io_add_watch(channel1, G_IO_IN|G_IO_ERR|G_IO_HUP, ipc_read_cb, NULL);


    int opcode = 0;
    const char *payload = "Hello world";
    int size = 1 + strlen(payload);
    write(fd2, &opcode, sizeof(opcode));
    write(fd2, &size, sizeof(size));
    write(fd2, payload, size);


    // Run the main GTK+ event loop
    gtk_main();

    // Cleanup
    close(fd1);
    close(fd2);
}

static void create_browser(Display *display, Window pluginWindow, const char *url)
{
    // Query plugin window attributes
    XWindowAttributes attr;
    XGetWindowAttributes(display, pluginWindow, &attr);

    // Create a window that will contain the browser instance
    GtkWidget *gtkHelperWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(gtkHelperWindow), attr.width, attr.height);

    // Create a browser instance
    WebKitWebView *webView = WEBKIT_WEB_VIEW(webkit_web_view_new());

    // Put the browser area into the main window
    gtk_container_add(GTK_CONTAINER(gtkHelperWindow), GTK_WIDGET(webView));

    // Set up callbacks so that if either the main window or the browser instance is
    // closed, the program will exit
    g_signal_connect(gtkHelperWindow, "destroy", G_CALLBACK(destroy_window_cb), NULL);
    g_signal_connect(webView, "close", G_CALLBACK(close_webview_cb), gtkHelperWindow);

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
     - Keep browser size in sync with plugin window size
     - Inter process communication
     - Ability to receive a close command for graceful shutdown,
       ie. prevent Gdk-WARNING **: GdkWindow unexpectedly destroyed
    */

    XFlush(display);
}

static gboolean ipc_read_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
    //printf("ipc_read_cb()\n");

    if (condition == G_IO_IN) {
        int fd = g_io_channel_unix_get_fd(source);
        int opcode, size;
        read(fd, &opcode, sizeof(opcode));
        read(fd, &size, sizeof(size));
        void *payload = malloc(size);
        read(fd, payload, size);
        printf("Helper Rx: %s\n", (char*)payload);
        free(payload);
    }

    return TRUE;
}

static gboolean close_webview_cb(WebKitWebView* webView, GtkWidget* window)
{
    fprintf(stderr, "close_webview_cb()\n");
    gtk_widget_destroy(window);
    return TRUE;
}

static void destroy_window_cb(GtkWidget* widget, GtkWidget* window)
{
    fprintf(stderr, "destroy_window_cb()\n");
    gtk_main_quit();
}
