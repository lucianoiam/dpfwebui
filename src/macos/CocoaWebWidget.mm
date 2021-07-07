/*
 * dpf-webui
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

#include "dgl/Application.hpp" // MACJACKBUG

#include "CocoaWebWidget.hpp"

// Avoid symbol name collisions
#define OBJC_INTERFACE_NAME_HELPER_1(INAME, SEP, SUFFIX) INAME ## SEP ## SUFFIX
#define OBJC_INTERFACE_NAME_HELPER_2(INAME, SUFFIX) OBJC_INTERFACE_NAME_HELPER_1(INAME, _, SUFFIX)
#define OBJC_INTERFACE_NAME(INAME) OBJC_INTERFACE_NAME_HELPER_2(INAME, PROJECT_ID_HASH)

#define DistrhoWebView         OBJC_INTERFACE_NAME(DistrhoWebView)
#define DistrhoWebViewDelegate OBJC_INTERFACE_NAME(DistrhoWebViewDelegate)

#define fWebView         ((DistrhoWebView*)fView)
#define fWebViewDelegate ((DistrhoWebViewDelegate*)fDelegate)

#define JS_POST_MESSAGE_SHIM "window.webviewHost.postMessage = (args) => window.webkit.messageHandlers.host.postMessage(args);"

// Do not assume an autorelease pool exists or ARC is enabled.

USE_NAMESPACE_DISTRHO

@interface DistrhoWebView: WKWebView
@property (readonly, nonatomic) CocoaWebWidget* cppWidget;
@property (readonly, nonatomic) NSView* pluginRootView;
@end

@interface DistrhoWebViewDelegate: NSObject<WKNavigationDelegate, WKScriptMessageHandler>
@property (assign, nonatomic) CocoaWebWidget *cppWidget;
@end

CocoaWebWidget::CocoaWebWidget(Widget *parentWidget)
    : AbstractWebWidget(parentWidget)
    , fLastKeyboardEventTime(0)
{
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
    NSView *parentView = (NSView *)parentWidget->getWindow().getNativeWindowHandle();
    [parentView addSubview:fWebView];

    String js = String(JS_POST_MESSAGE_SHIM);
    injectDefaultScripts(js);

    // MACJACKBUG: Allow standalone application window to gain keyboard focus
    if (getWindow().getApp().isStandalone()) {
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp activateIgnoringOtherApps:YES]; // focus window
    }
}

CocoaWebWidget::~CocoaWebWidget()
{
    [fWebView removeFromSuperview];
    [fWebView release];
    [fWebViewDelegate release];
}

void CocoaWebWidget::onResize(const ResizeEvent& ev)
{
    (void)ev;
    updateWebViewFrame();
}

void CocoaWebWidget::onPositionChanged(const PositionChangedEvent& ev)
{
    (void)ev;
    updateWebViewFrame();
}

bool CocoaWebWidget::onKeyboard(const KeyboardEvent& ev)
{
    // Some hosts like REAPER prevent the web view from gaining keyboard focus.
    // In such cases the web view can still get touch/mouse input, so assuming
    // the user wants to type into a <input> element, such element can be
    // focused by clicking on it, and all subsequent key events received by the
    // root plugin window here by this method will be conveniently injected into
    // the web view, effectively reaching the <input> element.

    if (!isKeyboardFocus()) {
        return false;
    }

    if ((ev.time != 0) && (fLastKeyboardEventTime == ev.time)) {
        //NSLog(@"Break onKeyboard() loop");
        return true;
    }

    // Make a distinction between DPF delivered key events (ev.time always 0)
    // and synthesized key events below by setting their timestamps to now. This
    // is needed to break potential onKeyboard() loops while allowing key repeat

    uint now = (uint)CFAbsoluteTimeGetCurrent();

    fLastKeyboardEventTime = ev.time == 0 ? now : ev.time;

    // TODO: conversion method does not work for anything non-ASCII

    uint32_t key = OSSwapHostToLittleInt32(ev.key);
    NSString *ch = [[NSString alloc] initWithBytes:&key length:4 encoding:NSUTF32LittleEndianStringEncoding];

    NSEventModifierFlags flags =  ((ev.mod & kModifierShift  ) ? NSEventModifierFlagShift   : 0)
                                | ((ev.mod & kModifierControl) ? NSEventModifierFlagControl : 0)
                                | ((ev.mod & kModifierAlt    ) ? NSEventModifierFlagOption  : 0)
                                | ((ev.mod & kModifierSuper  ) ? NSEventModifierFlagCommand : 0);
    if (ev.press) {
        NSEvent *event = [NSEvent keyEventWithType: NSEventTypeKeyDown
            location: NSZeroPoint
            modifierFlags: flags
            timestamp: (NSTimeInterval)now
            windowNumber: 0
            context: nil
            characters: ch
            charactersIgnoringModifiers: ch
            isARepeat: NO
            keyCode: ev.keycode
        ];

        [fWebView keyDown:event];
    } else {
        NSEvent *event = [NSEvent keyEventWithType: NSEventTypeKeyUp
            location: NSZeroPoint
            modifierFlags: flags
            timestamp: (NSTimeInterval)now
            windowNumber: 0
            context: nil
            characters: ch
            charactersIgnoringModifiers: ch
            isARepeat: NO
            keyCode: ev.keycode
        ];

        [fWebView keyUp:event];
    }

    [ch release];

    return true; // stop propagation
}

void CocoaWebWidget::setBackgroundColor(uint32_t rgba)
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

void CocoaWebWidget::navigate(String& url)
{
    NSString *urlStr = [[NSString alloc] initWithCString:url encoding:NSUTF8StringEncoding];
    NSURL *urlObj = [[NSURL alloc] initWithString:urlStr];
    [fWebView loadFileURL:urlObj allowingReadAccessToURL:urlObj];
    [urlObj release];
    [urlStr release];
}

void CocoaWebWidget::runScript(String& source)
{
    NSString *js = [[NSString alloc] initWithCString:source encoding:NSUTF8StringEncoding];
    [fWebView evaluateJavaScript:js completionHandler: nil];
    [js release];
}

void CocoaWebWidget::injectScript(String& source)
{
    NSString *js = [[NSString alloc] initWithCString:source encoding:NSUTF8StringEncoding];
    WKUserScript *script = [[WKUserScript alloc] initWithSource:js
        injectionTime:WKUserScriptInjectionTimeAtDocumentStart forMainFrameOnly:YES];
    [fWebView.configuration.userContentController addUserScript:script];
    [script release];
    [js release];
}

void CocoaWebWidget::updateWebViewFrame()
{
    // There is a mismatch between DGL and AppKit coordinates
    // https://github.com/DISTRHO/DPF/issues/291
    CGFloat k = [NSScreen mainScreen].backingScaleFactor;
    CGRect frame;
    frame.origin.x = (CGFloat)getAbsoluteX() / k;
    frame.origin.y = (CGFloat)getAbsoluteY() / k;
    frame.size.width = (CGFloat)getWidth() / k;
    frame.size.height = (CGFloat)getHeight() / k;
    fWebView.frame = frame;
}

@implementation DistrhoWebView

- (CocoaWebWidget *)cppWidget
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

    ScriptValueVector args;

    for (id objcArg : (NSArray *)message.body) {
        if (CFGetTypeID(objcArg) == CFBooleanGetTypeID()) {
            args.push_back(ScriptValue(static_cast<bool>([objcArg boolValue])));
        } else if ([objcArg isKindOfClass:[NSNumber class]]) {
            args.push_back(ScriptValue([objcArg doubleValue]));
        } else if ([objcArg isKindOfClass:[NSString class]]) {
            args.push_back(ScriptValue(String([objcArg cStringUsingEncoding:NSUTF8StringEncoding])));
        } else {
            args.push_back(ScriptValue()); // null
        }
    }

    self.cppWidget->didReceiveScriptMessage(args);
}

@end
