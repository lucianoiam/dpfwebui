/*
 * dpfwebui / Web User Interfaces support for DISTRHO Plugin Framework
 * Copyright (C) 2021-2024 Luciano Iam <oss@lucianoiam.com>
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

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <X11/Xlib.h>

#include "ipc.h"
#include "ipc_message.h"
#include "scaling.h"

#include "DistrhoPluginInfo.h"

// The WebKitGTK web view is created with a fixed size, see comprehensive
// explanation in realize(). Set DPF_WEBUI_LINUX_GTK_WEBVIEW_FAKE_VIEWPORT=1
// in DistrhoPluginInfo.h to allow the UI to grow, whether it is by using the
// host's own resize control (DISTRHO_UI_USER_RESIZABLE=1) or by binding a
// custom resize control to UI::setSize(). Maximum width and height can be
// adjusted with DPF_WEBUI_LINUX_GTK_WEBVIEW_WIDTH/HEIGHT. When the workaround
// is disabled the web view size will be fixed to the init size. In all cases
// CSS viewport dimensions (vw/vh/vmin/vmax) will not follow changes in size.
#if DPF_WEBUI_LINUX_GTK_WEBVIEW_FAKE_VIEWPORT
# if !defined(DPF_WEBUI_LINUX_GTK_WEBVIEW_WIDTH)
#  define DPF_WEBUI_LINUX_GTK_WEBVIEW_WIDTH  2560
# endif
# if !defined(DPF_WEBUI_LINUX_GTK_WEBVIEW_HEIGHT)
#  define DPF_WEBUI_LINUX_GTK_WEBVIEW_HEIGHT 1600
# endif
#endif

// CSS touch-action based approach seems to be failing for WebKitGTK. Looks like a bug.
#define JS_DISABLE_PINCH_ZOOM_WORKAROUND "if (document.body.children.length > 0) " \
                                         "  document.body.children[0].addEventListener('touchstart', (ev) => {" \
                                         "    ev.preventDefault();" \
                                         "  });"

#define JS_POST_MESSAGE_SHIM "window.host.postMessage = (payload) => window.webkit.messageHandlers.host.postMessage(payload);"

typedef struct {
    ipc_t*          ipc;
    Display*        display;
    float           pixelRatio;
    float           zoom;
    msg_view_size_t size;
    Window          container;
    GtkWindow*      window;
    WebKitWebView * webView;
    gboolean        focus;
    Window          focusXWin;
    pthread_t       watchdog;
    char            scripts[262144];
} context_t;

static void realize(context_t *ctx, const msg_view_cfg_t *config);
static void navigate(context_t *ctx, const char *url);
static void run_script(const context_t *ctx, const char *js);
static void inject_script(context_t *ctx, const char *js);
static void set_size(context_t *ctx, const msg_view_size_t *size);
static void apply_size(const context_t *ctx);
static void set_keyboard_focus(context_t *ctx, gboolean focus);
static void* focus_watchdog_worker(void *arg);
static gboolean release_focus(gpointer data);
static void web_view_load_changed_cb(WebKitWebView *view, WebKitLoadEvent event, gpointer data);
static void web_view_script_message_cb(WebKitUserContentManager *manager, WebKitJavascriptResult *res, gpointer data);
static gboolean web_view_keypress_cb(GtkWidget *widget, GdkEventKey *event, gpointer data);
static gboolean ipc_read_cb(GIOChannel *source, GIOCondition condition, gpointer data);
static int ipc_write_simple(const context_t *ctx, msg_opcode_t opcode, const void *payload, int payload_sz);

int main(int argc, char* argv[])
{
    context_t ctx;
    ipc_conf_t conf;
    GIOChannel* channel;

    memset(&ctx, 0, sizeof(ctx));

    if (argc < 3) {
        fprintf(stderr, "gtk_helper : invalid argument count\n");
        return -1;
    }

    if ((sscanf(argv[1], "%d", &conf.fd_r) == 0) || (sscanf(argv[2], "%d", &conf.fd_w) == 0)) {
        fprintf(stderr, "gtk_helper : invalid file descriptor\n");
        return -1;
    }

    ctx.ipc = ipc_init(&conf);

    ctx.display = XOpenDisplay(NULL);
    if (ctx.display == NULL) {
        fprintf(stderr, "gtk_helper : cannot open display\n");
        return -1;
    }

    // Device pixel ratio depends on Xft.dpi, GDK_SCALE and GDK_DPI_SCALE
    ctx.pixelRatio = device_pixel_ratio();
    
    // GDK_DPI_SCALE is useful for fractional scaling
    float dpiScale = gdk_dpi_scale();

    if (dpiScale > 1.f) {
        // WebKitGTK follows GDK_DPI_SCALE environment variable only for scaling
        // text, leaving out images and pixel values. Work around this by
        // unsetting DPI scaling before gtk_init() [following line for clarity]
        unsetenv("GDK_DPI_SCALE");
        // And later setting zoom for compensating [see realize()]
        ctx.zoom = dpiScale * /*explain this?*/xdpi_scale();
        // Text also follows Xft.dpi, re-set GDK_DPI_SCALE to counteract effect.
        char temp[8];
        sprintf(temp, "%.2f", 96.f / xft_dpi());
        setenv("GDK_DPI_SCALE", temp, 1);
    }

    gdk_set_allowed_backends("x11");
    gtk_init(0, NULL);

    channel = g_io_channel_unix_new(conf.fd_r);    
    g_io_add_watch(channel, G_IO_IN|G_IO_ERR|G_IO_HUP, ipc_read_cb, &ctx);

    ipc_write_simple(&ctx, OP_HANDLE_INIT, &ctx.pixelRatio, sizeof(ctx.pixelRatio));

    gtk_main();

    set_keyboard_focus(&ctx, FALSE);
    g_io_channel_shutdown(channel, TRUE, NULL);
    ipc_destroy(ctx.ipc);

    if (ctx.container != 0) {
        XDestroyWindow(ctx.display, ctx.container);
    }

    XCloseDisplay(ctx.display);

    return 0;
}

static void realize(context_t *ctx, const msg_view_cfg_t *config)
{
    // Create a native container window
#if DPF_WEBUI_LINUX_GTK_WEBVIEW_FAKE_VIEWPORT
    int width = DPF_WEBUI_LINUX_GTK_WEBVIEW_WIDTH;
    int height = DPF_WEBUI_LINUX_GTK_WEBVIEW_HEIGHT;
#else
    int width = config->size.width;
    int height = config->size.height;
#endif
    ctx->container = XCreateSimpleWindow(ctx->display, (Window)config->parent, 0, 0,
                                         width, height, 0, 0, 0);
    XSync(ctx->display, False);

    // Wrap container in a GDK window. Web view text input colored focus boxes
    // do not show in wrapped windows but show correctly in regular windows.
    GdkWindow* gdkWindow = gdk_x11_window_foreign_new_for_display(gdk_display_get_default(),
        ctx->container);
    ctx->window = GTK_WINDOW(gtk_widget_new(GTK_TYPE_WINDOW, NULL));
    g_signal_connect(ctx->window, "realize", G_CALLBACK(gtk_widget_set_window), gdkWindow);

    // After the web view becomes visible, gtk_window_resize() will not cause
    // its contents to resize anymore. The issue is probably related to the
    // GdkWindow wrapping a X11 window and not emitting Glib events like
    // configure-event. The workaround consists in creating the window with a
    // predetermined max size and using JavaScript to resize HTML body instead
    // of resizing the window natively. It is an ugly solution that works.
    // Note this renders viewport based units useless (vw/vh/vmin/vmax). 
    gtk_window_resize(ctx->window, width, height);

    char *name = (char*)config->userAgent;
    char *sep = strstr(name, "/");
    char *version = NULL;

    if (sep != NULL) {
        version = strlen(sep) > 1 ? sep + 1 : sep;
        name[sep - name] = '\0';
    }

    WebKitSettings *settings = webkit_settings_new();
    webkit_settings_set_user_agent_with_application_details(settings, name, version);

    ctx->webView = WEBKIT_WEB_VIEW(webkit_web_view_new_with_settings(settings));

    if (ctx->zoom > 0) {
        webkit_web_view_set_zoom_level(ctx->webView, ctx->zoom);
    }

    g_signal_connect(ctx->webView, "load-changed", G_CALLBACK(web_view_load_changed_cb), ctx);
    g_signal_connect(ctx->webView, "key-press-event", G_CALLBACK(web_view_keypress_cb), ctx);
    WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager(ctx->webView);
    g_signal_connect(manager, "script-message-received::host", G_CALLBACK(web_view_script_message_cb), ctx);
    webkit_user_content_manager_register_script_message_handler(manager, "host");

    gtk_container_add(GTK_CONTAINER(ctx->window), GTK_WIDGET(ctx->webView));  
}

static void navigate(context_t *ctx, const char *url)
{
    // Inject queued scripts
    WebKitUserScript *script = webkit_user_script_new(ctx->scripts, WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
        WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START, NULL, NULL);
    WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager(ctx->webView);
    webkit_user_content_manager_add_script(manager, script);
    webkit_user_script_unref(script);
    ctx->scripts[0] = '\0'; // already injected on next navigate() call

    webkit_web_view_load_uri(ctx->webView, url);
}

static void run_script(const context_t *ctx, const char *js)
{
    if (ctx->webView != NULL) {
        webkit_web_view_run_javascript(ctx->webView, js, NULL, NULL, NULL);
    }
}

static void inject_script(context_t *ctx, const char *js)
{
    strcat(ctx->scripts, (const char *)js);
}

static void set_size(context_t *ctx, const msg_view_size_t *size)
{
    ctx->size = *size;

    if (ctx->webView != NULL) {
        apply_size(ctx);
    }
}

static void apply_size(const context_t *ctx)
{
    unsigned width = ctx->size.width;
    unsigned height = ctx->size.height;

    if ((width == 0) || (height == 0)) {
        return;
    }

#if DPF_WEBUI_LINUX_GTK_WEBVIEW_FAKE_VIEWPORT
    width = (unsigned)((float)width / ctx->pixelRatio);
    height = (unsigned)((float)height / ctx->pixelRatio);

    char js[1024];

    sprintf(js, "document.documentElement.style.width = '%dpx';"
                "document.documentElement.style.height = '%dpx';",
                width, height);
    run_script(ctx, js);
#else
    // Currently does not result in webview contents size update
    gtk_window_resize(ctx->window, width, height);
#endif
}

static void set_keyboard_focus(context_t *ctx, gboolean focus)
{
    if (ctx->focus == focus) {
        return;
    }

    if (ctx->webView == NULL) {
        return;
    }

    ctx->focusXWin = 0;
    ctx->focus = focus;

    // Some hosts grab focus back from the plugin, avoid that

    GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(ctx->webView));
    GdkSeat *seat = gdk_display_get_default_seat(gdk_display_get_default());

    if (ctx->focus) {
        gdk_seat_grab(seat, window, GDK_SEAT_CAPABILITY_KEYBOARD, FALSE, NULL, NULL, NULL, NULL);
        pthread_create(&ctx->watchdog, NULL, focus_watchdog_worker, ctx);
    } else {
        gdk_seat_ungrab(seat);

        if (ctx->watchdog != 0) {
            pthread_join(ctx->watchdog, NULL);
            ctx->watchdog = 0;
        }
    }
}

static void* focus_watchdog_worker(void *arg)
{
    // GdkWindow instances created with gdk_x11_window_foreign_new_for_display()
    // do not seem to emit focus events like a regular GdkWindow instance. That
    // makes it difficult to detect when the plugin window goes out of focus in
    // order to release the keyboard lock. This solution uses a thread to poll
    // X11 focus, detect changes and release the keyboard lock when needed.

    context_t *ctx = (context_t *)arg;

    while (ctx->focus) {
        if (ctx->focus && (ctx->focusXWin != 0)) {
            Window focus;
            int revert;

            XLockDisplay(ctx->display);
            XGetInputFocus(ctx->display, &focus, &revert);
            XUnlockDisplay(ctx->display);

            if (ctx->focusXWin != focus) {
                g_idle_add(release_focus, ctx);
            }
        }

        usleep(100000L); // 100ms
    }
}

static gboolean release_focus(gpointer data)
{
    set_keyboard_focus((context_t *)data, FALSE);
    return FALSE;
}

static void web_view_load_changed_cb(WebKitWebView *view, WebKitLoadEvent event, gpointer data)
{
    context_t *ctx = (context_t *)data;

    switch (event) {
        case WEBKIT_LOAD_FINISHED:
            // Load completed. All resources are done loading or there was an error during the load operation. 
            run_script(ctx, JS_DISABLE_PINCH_ZOOM_WORKAROUND);
            apply_size(ctx);
            gtk_widget_show_all(GTK_WIDGET(ctx->window));
            ipc_write_simple(ctx, OP_HANDLE_LOAD_FINISHED, NULL, 0);
            // TODO : look for a better solution than a delay to prevent black flicker
            usleep(50000L);
            break;
        default:
            break;
    }
}

static void web_view_script_message_cb(WebKitUserContentManager *manager, WebKitJavascriptResult *res, gpointer data)
{
    // Serialize JS values into type;value chunks. Available types are limited to
    // those defined by msg_js_arg_type_t so there is no need to encode value sizes.
    gint32 numArgs, i;
    JSCValue *jsArg;
    JSCValue *jsArgs = webkit_javascript_result_get_js_value(res);
    char *payload = NULL;
    int offset = 0;

    if (jsc_value_is_array(jsArgs)) {
        numArgs = jsc_value_to_int32(jsc_value_object_get_property(jsArgs, "length"));

        for (i = 0; i < numArgs; i++) {
            jsArg = jsc_value_object_get_property_at_index(jsArgs, i);

            if (jsc_value_is_boolean(jsArg)) {
                payload = (char *)realloc(payload, offset + 1);
                if (jsc_value_to_boolean(jsArg)) {
                    *(payload+offset) = (char)ARG_TYPE_TRUE;
                } else {
                    *(payload+offset) = (char)ARG_TYPE_FALSE;
                }
                offset += 1;

            } else if (jsc_value_is_number(jsArg)) {
                payload = (char *)realloc(payload, offset + 1 + sizeof(double));
                *(payload+offset) = (char)ARG_TYPE_DOUBLE;
                offset += 1;
                *(double *)(payload+offset) = jsc_value_to_double(jsArg);
                offset += sizeof(double);

            } else if (jsc_value_is_string(jsArg)) {
                const char *s = jsc_value_to_string(jsArg);
                int slen = strlen(s) + 1;
                payload = (char *)realloc(payload, offset + 1 + slen);
                *(payload+offset) = (char)ARG_TYPE_STRING;
                offset += 1;
                strcpy(payload+offset, s);
                offset += slen;

            } else {
                payload = (char *)realloc(payload, offset + 1);
                *(payload+offset) = (char)ARG_TYPE_NULL;
                offset += 1;
            }
        }
    }

    webkit_javascript_result_unref(res);

    ipc_write_simple((context_t *)data, OP_HANDLE_SCRIPT_MESSAGE, payload, offset);

    if (payload) {
        free(payload);
    }
}

static gboolean web_view_keypress_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    context_t *ctx = (context_t *)data;

    int revert;
    XGetInputFocus(ctx->display, &ctx->focusXWin, &revert);

    return !ctx->focus;
}

static gboolean ipc_read_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
    context_t *ctx = (context_t *)data;
    tlv_t packet;

    if ((condition & G_IO_IN) == 0) {
        return TRUE;
    }

    if (ipc_read(ctx->ipc, &packet) == -1) {
        fprintf(stderr, "gtk_helper : could not read from IPC channel - %s\n", strerror(errno));
        return TRUE;
    }

    switch (packet.t) {
        case OP_REALIZE:
            realize(ctx, (const msg_view_cfg_t *)packet.v);
            break;
        case OP_NAVIGATE:
            navigate(ctx, (const char *)packet.v);
            break;
        case OP_RUN_SCRIPT:
            run_script(ctx, (const char *)packet.v);
            break;
        case OP_INJECT_SHIMS:
            inject_script(ctx, JS_POST_MESSAGE_SHIM);
            break;
        case OP_INJECT_SCRIPT:
            inject_script(ctx, (const char *)packet.v);
            break;
        case OP_SET_SIZE:
            set_size(ctx, (const msg_view_size_t *)packet.v);
            break;
        case OP_SET_KEYBOARD_FOCUS:
            set_keyboard_focus(ctx, *((char *)packet.v) == 1 ? TRUE : FALSE);
            break;
        case OP_TERMINATE:
            gtk_main_quit();
            break;
        default:
            break;
    }

    return TRUE;
}

static int ipc_write_simple(const context_t *ctx, msg_opcode_t opcode, const void *payload, int payload_sz)
{
    int retval;
    tlv_t packet;

    packet.t = (short)opcode;
    packet.l = payload_sz;
    packet.v = payload;

    if ((retval = ipc_write(ctx->ipc, &packet)) == -1) {
        fprintf(stderr, "gtk_helper : could not write to IPC channel - %s\n", strerror(errno));
    }

    return retval;
}
