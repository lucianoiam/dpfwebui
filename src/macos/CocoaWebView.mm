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

#include "macro.h"

// Avoid symbol name collisions
#define OBJC_INTERFACE_NAME_HELPER_1(INAME, SEP, SUFFIX) INAME ## SEP ## SUFFIX
#define OBJC_INTERFACE_NAME_HELPER_2(INAME, SUFFIX) OBJC_INTERFACE_NAME_HELPER_1(INAME, _, SUFFIX)
#define OBJC_INTERFACE_NAME(INAME) OBJC_INTERFACE_NAME_HELPER_2(INAME, HIPHOP_PROJECT_ID_HASH)

#define DistrhoWebView         OBJC_INTERFACE_NAME(DistrhoWebView)
#define DistrhoWebViewDelegate OBJC_INTERFACE_NAME(DistrhoWebViewDelegate)

#define fTopView         ((NSView *)fViewBg)
#define fWebView         ((DistrhoWebView *)fView)
#define fWebViewDelegate ((DistrhoWebViewDelegate *)fDelegate)

#define JS_POST_MESSAGE_SHIM "window.webviewHost.postMessage = (args) => window.webkit.messageHandlers.host.postMessage(args);"

// Do not assume an autorelease pool exists or ARC is enabled.

USE_NAMESPACE_DISTRHO

@interface DistrhoWebView: WKWebView
@property (readonly, nonatomic) CocoaWebView* cppView;
@property (readonly, nonatomic) NSView* pluginRootView;
@end

@interface DistrhoWebViewDelegate: NSObject<WKNavigationDelegate, WKScriptMessageHandler>
@property (assign, nonatomic) CocoaWebView *cppView;
@end

CocoaWebView::CocoaWebView()
{
    fViewBg = [[NSView alloc] initWithFrame:CGRectZero];
    fTopView.autoresizesSubviews = YES;

    fView = [[DistrhoWebView alloc] initWithFrame:CGRectZero];
    fWebView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    [fTopView addSubview:fWebView];

    fDelegate = [[DistrhoWebViewDelegate alloc] init];
    fWebViewDelegate.cppView = this;
    fWebView.navigationDelegate = fWebViewDelegate;
    [fWebView.configuration.userContentController addScriptMessageHandler:fWebViewDelegate name:@"host"];

    String js = String(JS_POST_MESSAGE_SHIM);
    injectDefaultScripts(js);
}

CocoaWebView::~CocoaWebView()
{
    [fWebViewDelegate release];

    [fWebView removeFromSuperview];
    [fWebView release];

    [fTopView removeFromSuperview];
    [fTopView release];
}

void CocoaWebView::realize()
{
    [(NSView *)getParent() addSubview:fTopView];
    
    @try {
        if ([fTopView respondsToSelector:@selector(setBackgroundColor:)]) {
            CGFloat c[] = { DISTRHO_UNPACK_RGBA_NORM(getBackgroundColor(), CGFloat) };
            NSColor* color = [NSColor colorWithRed:c[0] green:c[1] blue:c[2] alpha:c[3]];
            [fTopView setValue:color forKey:@"backgroundColor"];
            [color release];
        }

        if ([fWebView respondsToSelector:@selector(_setDrawsBackground:)]) {
            NSNumber *no = [[NSNumber alloc] initWithBool:NO];
            [fWebView setValue:no forKey:@"drawsBackground"];
            [no release];
        }
    } @catch (NSException *e) {
        NSLog(@"Could not set background color");
    }
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

void CocoaWebView::onSize(uint width, uint height)
{
    CGRect frame;
    frame.origin.x = 0;
    frame.origin.y = 0;
    frame.size.width = (CGFloat)width;
    frame.size.height = (CGFloat)height;
    frame = [fTopView.window convertRectFromBacking:frame];
    frame.origin.y = fTopView.superview.frame.origin.y;
    fTopView.frame = frame;
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

@end

@implementation DistrhoWebViewDelegate

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation
{
    self.cppView->didFinishNavigation();
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

    self.cppView->didReceiveScriptMessage(args);
}

@end
