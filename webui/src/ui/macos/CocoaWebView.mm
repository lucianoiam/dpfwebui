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

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <WebKit/WebKit.h>

#include "CocoaWebView.hpp"

#define UNPACK_RGBA_NORMALIZED(c,t)  ( c >> 24)               / (t)(255), \
                                     ((c & 0x00ff0000) >> 16) / (t)(255), \
                                     ((c & 0x0000ff00) >> 8 ) / (t)(255), \
                                     ( c & 0x000000ff)        / (t)(255)

// Avoid symbol name collisions
#define OBJC_INTERFACE_NAME_HELPER_1(INAME, SEP, SUFFIX) INAME ## SEP ## SUFFIX
#define OBJC_INTERFACE_NAME_HELPER_2(INAME, SUFFIX) OBJC_INTERFACE_NAME_HELPER_1(INAME, _, SUFFIX)
#define OBJC_INTERFACE_NAME(INAME) OBJC_INTERFACE_NAME_HELPER_2(INAME, DPF_WEBUI_PROJECT_ID_HASH)

#define DistrhoWebView         OBJC_INTERFACE_NAME(DistrhoWebView)
#define DistrhoWebViewDelegate OBJC_INTERFACE_NAME(DistrhoWebViewDelegate)

#define fNsBackground ((NSView *)fBackground)
#define fNsWebView    ((DistrhoWebView *)fWebView)
#define fNsDelegate   ((DistrhoWebViewDelegate *)fDelegate)

#define JS_POST_MESSAGE_SHIM "window.host.postMessage = (payload) => window.webkit.messageHandlers.host.postMessage(payload);"

@interface DistrhoWebView: WKWebView
@property (readonly, nonatomic) CocoaWebView* cppView;
@property (readonly, nonatomic) NSView* pluginRootView;
@end

@interface DistrhoWebViewDelegate: NSObject<WKNavigationDelegate, WKUIDelegate, WKScriptMessageHandler>
@property (assign, nonatomic) CocoaWebView *cppView;
@end

USE_NAMESPACE_DISTRHO

CocoaWebView::CocoaWebView(String userAgentComponent)
{
    fBackground = [[NSView alloc] initWithFrame:CGRectZero];
    fNsBackground.autoresizesSubviews = YES;

    WKWebViewConfiguration* configuration = [[WKWebViewConfiguration alloc] init];
    NSString* userAgent = [[NSString alloc] initWithCString:userAgentComponent.buffer() encoding:NSUTF8StringEncoding];
    configuration.applicationNameForUserAgent = userAgent;
    [userAgent release];

    fWebView = [[DistrhoWebView alloc] initWithFrame:CGRectZero configuration:configuration];
    [configuration release];
    fNsWebView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    [fNsBackground addSubview:fNsWebView];

    fDelegate = [[DistrhoWebViewDelegate alloc] init];
    fNsDelegate.cppView = this;
    fNsWebView.navigationDelegate = fNsDelegate;
    fNsWebView.UIDelegate = fNsDelegate;
    [fNsWebView.configuration.userContentController addScriptMessageHandler:fNsDelegate name:@"host"];

    // EventTarget() constructor is unavailable on Safari < 14 (2020-09-16)
    // https://developer.mozilla.org/en-US/docs/Web/API/EventTarget/EventTarget
    // https://github.com/ungap/event-target
    String js = String(
#include "ui/macos/polyfill.js.inc"
    );
    injectScript(js);

    injectHostObjectScripts();

    js = String(JS_POST_MESSAGE_SHIM);
    injectScript(js);
}

CocoaWebView::~CocoaWebView()
{
    [fNsDelegate release];
    [fNsWebView.configuration.userContentController removeScriptMessageHandlerForName:@"host"];
    [fNsWebView removeFromSuperview];
    [fNsWebView release];
    [fNsBackground removeFromSuperview];
    [fNsBackground release];
}

float CocoaWebView::getDevicePixelRatio()
{
    if (fNsWebView.window == nil) {
        return [NSScreen mainScreen].backingScaleFactor;
    }
    
    return fNsWebView.window.backingScaleFactor;
}

void CocoaWebView::realize()
{
    onSize(getWidth(), getHeight());
    
    @try {
        if ([fNsBackground respondsToSelector:@selector(setBackgroundColor:)]) {
            CGFloat c[] = { UNPACK_RGBA_NORMALIZED(getBackgroundColor(), CGFloat) };
            NSColor* color = [NSColor colorWithRed:c[0] green:c[1] blue:c[2] alpha:c[3]];
            [fNsBackground setValue:color forKey:@"backgroundColor"];
        }

        if ([fNsWebView respondsToSelector:@selector(_setDrawsBackground:)]) {
            NSNumber *no = [[NSNumber alloc] initWithBool:NO];
            [fNsWebView setValue:no forKey:@"drawsBackground"];
            [no release];
        }
    } @catch (NSException *e) {
        NSLog(@"Could not set background color");
    }
}

void CocoaWebView::navigate(String& url)
{
    NSString *urlStr = [[NSString alloc] initWithCString:url encoding:NSUTF8StringEncoding];
    NSURL *urlObj = [NSURL URLWithString:[urlStr stringByAddingPercentEncodingWithAllowedCharacters:
        [NSCharacterSet URLFragmentAllowedCharacterSet]]];
    if ([urlObj.scheme isEqual:@"file"]) {
        NSURL *urlBase = [urlObj URLByDeletingLastPathComponent];
        [fNsWebView loadFileURL:urlObj allowingReadAccessToURL:urlBase];
    } else {
        [fNsWebView loadRequest:[NSURLRequest requestWithURL:urlObj]];
    }
    [urlStr release];
}

void CocoaWebView::runScript(String& source)
{
    NSString *js = [[NSString alloc] initWithCString:source encoding:NSUTF8StringEncoding];
    [fNsWebView evaluateJavaScript:js completionHandler: nil];
    [js release];
}

void CocoaWebView::injectScript(String& source)
{
    NSString *js = [[NSString alloc] initWithCString:source encoding:NSUTF8StringEncoding];
    WKUserScript *script = [[WKUserScript alloc] initWithSource:js
        injectionTime:WKUserScriptInjectionTimeAtDocumentStart forMainFrameOnly:YES];
    [fNsWebView.configuration.userContentController addUserScript:script];
    [script release];
    [js release];
}

void CocoaWebView::onSize(uint width, uint height)
{
    CGRect frame;
    frame.size.width = (CGFloat)width;
    frame.size.height = (CGFloat)height;
    frame = [fNsBackground.window convertRectFromBacking:frame]; // convert DPF coords. to AppKit
    frame.origin = fNsBackground.frame.origin; // keep position, for example REAPER sets it
    fNsBackground.frame = frame;
}

void CocoaWebView::onSetParent(uintptr_t parent)
{
    [(NSView *)parent addSubview:fNsBackground];
}

@implementation DistrhoWebView

- (CocoaWebView *)cppView
{
    return ((DistrhoWebViewDelegate *)self.navigationDelegate).cppView;
}

- (NSView *)pluginRootView
{
    return self.superview.superview;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
    // Allow the web view to immediately process clicks when the plugin window
    // is unfocused, otherwise the first click is swallowed to focus web view.
    (void)event;
    return YES;
}

- (BOOL)performKeyEquivalent:(NSEvent *)event
{
    // Make the web view ignore key shortcuts like Cmd+Q and Cmd+H
    (void)event;
    return NO;
}

// Passthrough keyboard events targeting the web view to the plugin root view
// whenever the keyboard focus is not requested. Focus must be switched on/off
// on demand by the UI JS code. This allows for example to play the virtual
// keyboard on Live while the web view UI is open and no text input is required.

- (void)keyDown:(NSEvent *)event
{
    if (self.cppView->getKeyboardFocus()) {
        [super keyDown:event];
    } else {
        [self.pluginRootView keyDown:event];
    }
}

- (void)keyUp:(NSEvent *)event
{
    if (self.cppView->getKeyboardFocus()) {
        [super keyUp:event];
    } else {
        [self.pluginRootView keyUp:event];
    }
}

- (void)flagsChanged:(NSEvent *)event
{
    if (self.cppView->getKeyboardFocus()) {
        [super flagsChanged:event];
    } else {
        [self.pluginRootView flagsChanged:event];
    }
}

- (void)cancelOperation:(id)sender
{
    if (self.cppView->getKeyboardFocus()) {
        // Avoid crash when hitting Escape with keyboard focus enabled
    } else {
        [super cancelOperation:sender];
    }
}

@end

@implementation DistrhoWebViewDelegate

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation
{
    (void)webView;
    (void)navigation;
    self.cppView->didFinishNavigation();
}

- (void)webView:(WKWebView *)webView
        runOpenPanelWithParameters:(WKOpenPanelParameters *)parameters 
        initiatedByFrame:(WKFrameInfo *)frame
        completionHandler:(void (^)(NSArray<NSURL *> *URLs))completionHandler
{
    NSOpenPanel* openPanel = [[NSOpenPanel alloc] init];
    openPanel.canChooseFiles = YES;

    [openPanel beginWithCompletionHandler:^(NSModalResponse result) {
        if (result == NSModalResponseOK) {
            completionHandler(openPanel.URLs);
        } else if (result == NSModalResponseCancel) {
            completionHandler(nil);
        }
    }];

    [openPanel release];
}

- (void)userContentController:(WKUserContentController *)controller didReceiveScriptMessage:(WKScriptMessage *)message
{
    (void)controller;
    
    if (![message.body isKindOfClass:[NSArray class]]) {
        return;
    }

    Variant payload = Variant::createArray();

    for (id objcArg : (NSArray *)message.body) {
        if (CFGetTypeID(objcArg) == CFBooleanGetTypeID()) {
            payload.pushArrayItem(static_cast<bool>([objcArg boolValue]));
        } else if ([objcArg isKindOfClass:[NSNumber class]]) {
            payload.pushArrayItem([objcArg doubleValue]);
        } else if ([objcArg isKindOfClass:[NSString class]]) {
            payload.pushArrayItem([objcArg cStringUsingEncoding:NSUTF8StringEncoding]);
        } else {
            payload.pushArrayItem(Variant()); // null
        }
    }

    self.cppView->didReceiveScriptMessage(payload);
}

@end
