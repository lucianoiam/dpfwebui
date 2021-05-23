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

#include "helper.h"

#include <stdint.h>
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include <gdk/gdkwayland.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "../log.h"
#include "ipc.h"

typedef struct {
    ipc_t*         ipc;
    Display*       display;
    GtkWindow*     window;
    WebKitWebView* webView;
} context_t;

static int create_webview(context_t *ctx);
static int ipc_write_simple(context_t *ctx, char opcode, const void *payload, int size);
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
        LOG_STDERR("Invalid argument count");
        return -1;
    }

    if ((sscanf(argv[1], "%d", &r_fd) == 0) || (sscanf(argv[2], "%d", &w_fd) == 0)) {
        LOG_STDERR("Invalid file descriptor");
        return -1;
    }

    ctx.ipc = ipc_init(r_fd, w_fd);
    channel = g_io_channel_unix_new(r_fd);    
    g_io_add_watch(channel, G_IO_IN|G_IO_ERR|G_IO_HUP, ipc_read_cb, &ctx);


    // FIXME
    ipc_write_simple(&ctx, 0, "Hello plugin!", 14);


    gtk_init(0, NULL);
    
    if (create_webview(&ctx) == 0) {
        gtk_main();
    }

    g_io_channel_shutdown(channel, TRUE, NULL);
    ipc_destroy(ctx.ipc);

    return 0;
}

static int ipc_write_simple(context_t *ctx, char opcode, const void *payload, int size)
{
    int retval;
    ipc_msg_t msg;

    msg.opcode = opcode;
    msg.payload_sz = size;
    msg.payload = payload;

    if ((retval = ipc_write(ctx->ipc, &msg)) == -1) {
        LOG_STDERR_ERRNO_INT("Failed ipc_write(), opcode", opcode);
    }

    return retval;
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
            LOG_STDERR("Cannot open display");
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
        //       separate window. Ideally include a button to focus such separate window.
        LOG_STDERR_COLOR("Running Wayland, plugin will be displayed in a separate window");
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
            LOG_STDERR_ERRNO("Failed ipc_read()");
            terminate(ctx);
            return FALSE;
        }

        dispatch(ctx, &message);
    }

    return TRUE;
}
