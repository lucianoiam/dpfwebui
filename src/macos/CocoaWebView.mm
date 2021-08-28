/*
 * Hip-Hop / High Performance Hybrid Audio Plugins
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
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

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <WebKit/WebKit.h>

#include "CocoaWebView.hpp"

// Avoid symbol name collisions
#define OBJC_INTERFACE_NAME_HELPER_1(INAME, SEP, SUFFIX) INAME ## SEP ## SUFFIX
#define OBJC_INTERFACE_NAME_HELPER_2(INAME, SUFFIX) OBJC_INTERFACE_NAME_HELPER_1(INAME, _, SUFFIX)
#define OBJC_INTERFACE_NAME(INAME) OBJC_INTERFACE_NAME_HELPER_2(INAME, HIPHOP_PROJECT_ID_HASH)

#define DistrhoWebView         OBJC_INTERFACE_NAME(DistrhoWebView)
#define DistrhoWebViewDelegate OBJC_INTERFACE_NAME(DistrhoWebViewDelegate)

#define fWebView         ((DistrhoWebView*)fView)
#define fWebViewDelegate ((DistrhoWebViewDelegate*)fDelegate)

#define JS_POST_MESSAGE_SHIM "window.webviewHost.postMessage = (args) => window.webkit.messageHandlers.host.postMessage(args);"

// Do not assume an autorelease pool exists or ARC is enabled.

USE_NAMESPACE_DISTRHO

@interface DistrhoWebView: WKWebView
@property (readonly, nonatomic) CocoaWebView* cppWidget;
@property (readonly, nonatomic) NSView* pluginRootView;
@end

@interface DistrhoWebViewDelegate: NSObject<WKNavigationDelegate, WKScriptMessageHandler>
@property (assign, nonatomic) CocoaWebView *cppWidget;
@end

CocoaWebView::CocoaWebView(uintptr_t parentWindowHandle)
    : AbstractWebView(parentWindowHandle)
{
    // Create a standalone window if needed
    if (parentWindowHandle == 0) {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp activateIgnoringOtherApps:YES];

        CGRect contentRect = NSMakeRect(0,0,200,200);
        NSWindowStyleMask styleMask = NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskTitled;
        NSWindow* window = [[NSWindow alloc] initWithContentRect:contentRect
                                                       styleMask:styleMask
                                                         backing:NSBackingStoreBuffered
                                                           defer:NO];
        [window makeKeyAndOrderFront:window];
        [window release];
        parentWindowHandle = (uintptr_t)window.contentView;
    }

    // Create the web view
    fView = [[DistrhoWebView alloc] initWithFrame:CGRectZero];
    fWebView.hidden = YES;

    // Create a ObjC object that responds to some web view callbacks
    fDelegate = [[DistrhoWebViewDelegate alloc] init];
    fWebViewDelegate.cppWidget = this;
    fWebView.navigationDelegate = fWebViewDelegate;
    [fWebView.configuration.userContentController addScriptMessageHandler:fWebViewDelegate name:@"host"];

    // windowId is either a PuglCairoView* or PuglOpenGLViewDGL* depending
    // on the value of UI_TYPE in the Makefile. Both are NSView subclasses.
    NSView *parentView = (NSView *)parentWindowHandle;
    [parentView addSubview:fWebView];

    String js = String(JS_POST_MESSAGE_SHIM);
    injectDefaultScripts(js);
}

CocoaWebView::~CocoaWebView()
{
    [fWebView removeFromSuperview];
    [fWebView release];
    [fWebViewDelegate release];
}

void CocoaWebView::processEvents()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    NSDate* date = [NSDate distantPast];

    for (NSEvent* event ;;) {
        event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                   untilDate:date
                                      inMode:NSDefaultRunLoopMode
                                     dequeue:YES];
        if (event == nil) {
            break;
        }

        [NSApp sendEvent:event];
    }

    [pool release];
}

void CocoaWebView::setBackgroundColor(uint32_t rgba)
{
    // macOS WKWebView apparently does not offer a method for setting a background color, so the
    // background is removed altogether to reveal the underneath window paint. Do it safely.
    (void)rgba;

    if ([fWebView respondsToSelector:@selector(_setDrawsBackground:)]) {
        @try {
            NSNumber *no = [[NSNumber alloc] initWithBool:NO];
            [fWebView setValue:no forKey:@"drawsBackground"];
            [no release];
        }
        @catch (NSException *e) {
            NSLog(@"Could not set transparent color for WKWebView");
        }
    }
}

void CocoaWebView::setSize(uint width, uint height)
{
    CGRect frame;
    frame.origin.x = 0;
    frame.origin.y = 0;
    frame.size.width = (CGFloat)width;
    frame.size.height = (CGFloat)height;
    fWebView.frame = [fWebView.window convertRectFromBacking:frame];
}

void CocoaWebView::navigate(String& url)
{
    NSString *urlStr = [[NSString alloc] initWithCString:url encoding:NSUTF8StringEncoding];
    NSURL *urlObj = [[NSURL alloc] initWithString:urlStr];
    [fWebView loadFileURL:urlObj allowingReadAccessToURL:urlObj];
    [urlObj release];
    [urlStr release];
}

void CocoaWebView::runScript(String& source)
{
    NSString *js = [[NSString alloc] initWithCString:source encoding:NSUTF8StringEncoding];
    [fWebView evaluateJavaScript:js completionHandler: nil];
    [js release];
}

void CocoaWebView::injectScript(String& source)
{
    NSString *js = [[NSString alloc] initWithCString:source encoding:NSUTF8StringEncoding];
    WKUserScript *script = [[WKUserScript alloc] initWithSource:js
        injectionTime:WKUserScriptInjectionTimeAtDocumentStart forMainFrameOnly:YES];
    [fWebView.configuration.userContentController addUserScript:script];
    [script release];
    [js release];
}

@implementation DistrhoWebView

- (CocoaWebView *)cppWidget
{
    return ((DistrhoWebViewDelegate *)self.navigationDelegate).cppWidget;
}

- (NSView *)pluginRootView
{
    return self.superview.superview;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
    // Allow the web view to immediately process clicks when the plugin window
    // is unfocused, otherwise the first click is swallowed to focus web view.
    return YES;
}

- (BOOL)performKeyEquivalent:(NSEvent *)event
{
    // Make the web view ignore key shortcuts like Cmd+Q and Cmd+H
    return NO;
}

// Route keyboard generated by the web view to the plugin root view whenever no
// text input element is focused in the web view. This allows those events to
// be picked up by the host, for example to allow playing the virtual keyboard
// on Live while the web view UI is open. Need to make sure key events are not
// coming from DPF (timestamp 0) otherwise that creates a feedback loop.

- (void)keyDown:(NSEvent *)event
{
    if (self.cppWidget->isKeyboardFocus()) {
        [super keyDown:event];
    } else {
        [self.pluginRootView keyDown:event];
    }
}

- (void)keyUp:(NSEvent *)event
{
    if (self.cppWidget->isKeyboardFocus()) {
        [super keyUp:event];
    } else {
        [self.pluginRootView keyUp:event];
    }
}

- (void)flagsChanged:(NSEvent *)event
{
    if (self.cppWidget->isKeyboardFocus()) {
        [super flagsChanged:event];
    } else {
        [self.pluginRootView flagsChanged:event];
    }
}

@end

@implementation DistrhoWebViewDelegate

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation
{
    self.cppWidget->didFinishNavigation();
    webView.hidden = NO;
}

- (void)userContentController:(WKUserContentController *)controller didReceiveScriptMessage:(WKScriptMessage *)message
{
    if (![message.body isKindOfClass:[NSArray class]]) {
        return;
    }

    JsValueVector args;

    for (id objcArg : (NSArray *)message.body) {
        if (CFGetTypeID(objcArg) == CFBooleanGetTypeID()) {
            args.push_back(JsValue(static_cast<bool>([objcArg boolValue])));
        } else if ([objcArg isKindOfClass:[NSNumber class]]) {
            args.push_back(JsValue([objcArg doubleValue]));
        } else if ([objcArg isKindOfClass:[NSString class]]) {
            args.push_back(JsValue(String([objcArg cStringUsingEncoding:NSUTF8StringEncoding])));
        } else {
            args.push_back(JsValue()); // null
        }
    }

    self.cppWidget->didReceiveScriptMessage(args);
}

@end
