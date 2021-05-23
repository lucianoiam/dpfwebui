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

#include <stdint.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include <gdk/gdkwayland.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "ipc.h"
#include "opcode.h"

typedef struct {
    ipc_t*         ipc;
    Display*       display;
    GtkWindow*     window;
    WebKitWebView* webView;
} context_t;

static int create_webview(context_t *ctx);
static void dispatch(const context_t *ctx, const ipc_msg_t *message);
static void navigate(const context_t *ctx, const char *url);
static void reparent(const context_t *ctx, uintptr_t parentId);
static void terminate(const context_t *ctx);
static void destroy_window_cb(GtkWidget* widget, GtkWidget* window);
static gboolean ipc_read_cb(GIOChannel *source, GIOCondition condition, gpointer data);

int main(int argc, char* argv[])
{
    context_t ctx;
    int r_fd, w_fd;
    GIOChannel* channel;

    if (argc < 3) {
        fprintf(stderr, "Invalid argument count\n");
        return -1;
    }

    if ((sscanf(argv[1], "%d", &r_fd) == 0) || (sscanf(argv[2], "%d", &w_fd) == 0)) {
        fprintf(stderr, "Invalid file descriptor\n");
        return -1;
    }

    ctx.ipc = ipc_init(r_fd, w_fd);
    channel = g_io_channel_unix_new(r_fd);    
    g_io_add_watch(channel, G_IO_IN|G_IO_ERR|G_IO_HUP, ipc_read_cb, &ctx);

    // FIXME
    ipc_msg_t msg;
    msg.opcode = 0;
    msg.payload = "hello world";
    msg.payload_sz = 12;
    ipc_write(ctx.ipc, &msg);

    gtk_init(0, NULL);
    create_webview(&ctx);
    gtk_main();

    ipc_destroy(ctx.ipc);

    return 0;
}

static int create_webview(context_t *ctx)
{
    // Create a window that will contain the browser instance
    ctx->window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));

    // Set up callback so that if the main window is closed, the program will exit
    g_signal_connect(ctx->window, "destroy", G_CALLBACK(destroy_window_cb), NULL);

    if (GDK_IS_X11_DISPLAY(gdk_display_get_default())) {
        if ((ctx->display = XOpenDisplay(NULL)) == NULL) {
            // Should never happen
            gtk_widget_destroy(GTK_WIDGET(ctx->window));
            fprintf(stderr, "Cannot open display\n");
            return -1;
        }

        // Set plugin window size
        /*XWindowAttributes attr;
        XGetWindowAttributes(xDisplay, nativeParentWindow, &attr);
        gtk_window_set_default_size(ctx->window, attr.width, attr.height);
        */
    }

    // FIXME
    gtk_window_set_default_size(ctx->window, 800, 600);

    // Create a browser instance
    ctx->webView = WEBKIT_WEB_VIEW(webkit_web_view_new());

    // Put the browser area into the main window
    gtk_container_add(GTK_CONTAINER(ctx->window), GTK_WIDGET(ctx->webView));

    // Make sure the main window and all its contents are visible
    gtk_widget_show_all(GTK_WIDGET(ctx->window));

    // Make sure that when the browser area becomes visible, it will get mouse and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(ctx->webView));

    return 0;
}

static void dispatch(const context_t *ctx, const ipc_msg_t *message)
{
    switch (message->opcode) {
        case OPCODE_NAVIGATE:
            navigate(ctx, (const char *)message->payload);
            break;
        case OPCODE_REPARENT:
            reparent(ctx, *((uintptr_t *)message->payload));
            break;
        case OPCODE_TERMINATE:
            terminate(ctx);
            break;
        default:
            break;
    }
}

static void navigate(const context_t *ctx, const char *url)
{
    webkit_web_view_load_uri(ctx->webView, url);
}

static void reparent(const context_t *ctx, uintptr_t parentId)
{
    GdkDisplay *gdkDisplay = gdk_display_get_default();
    if (GDK_IS_X11_DISPLAY(gdk_display_get_default())) {
        Window childId = gdk_x11_window_get_xid(gtk_widget_get_window(GTK_WIDGET(ctx->window)));
        XReparentWindow(ctx->display, childId, parentId, 0, 0);
        XFlush(ctx->display);
    } else if (GDK_IS_WAYLAND_DISPLAY(gdk_display_get_default())) {
        // TODO: show a message in parent plugin window explaining that Wayland is not supported
        //       yet and because of that the plugin web user interface will be displayed in a
        //       floating window. Ideally include a button to focus that floating window.
        fprintf(stderr, "Running Wayland, cannot reparent browser window\n");
    }
}

static void terminate(const context_t *ctx)
{
    gtk_main_quit();
}

static void destroy_window_cb(GtkWidget* widget, GtkWidget* window)
{
    terminate(NULL);
}

static gboolean ipc_read_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
    //printf("ipc_read_cb()\n");
    context_t *ctx = (context_t *)data;
    ipc_msg_t message;

    if (condition == G_IO_IN) {
        if (ipc_read(ctx->ipc, &message) == -1) {
            fprintf(stderr, "Could not read pipe");
            terminate(ctx);
            return FALSE;
        }

        dispatch(ctx, &message);
    }

    return TRUE;
}
