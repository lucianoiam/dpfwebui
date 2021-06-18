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

#include "CocoaWebWidget.hpp"

#define fWebView         ((WKWebView*)fView)
#define fWebViewDelegate ((WebViewDelegate*)fDelegate)

#define JS_POST_MESSAGE_SHIM "window.webviewHost.postMessage = (args) => window.webkit.messageHandlers.host.postMessage(args);"

// ☢️ NO ARC

USE_NAMESPACE_DISTRHO

@interface WebViewDelegate: NSObject<WKNavigationDelegate, WKScriptMessageHandler>
@property (assign, nonatomic) CocoaWebWidget *cppView;
@end

CocoaWebWidget::CocoaWebWidget(Widget *parentWidget)
    : AbstractWebWidget(parentWidget)
{
    setSkipDrawing(true);
    
    // Create the web view
    fView = [[WKWebView alloc] initWithFrame:CGRectZero];
    fWebView.hidden = YES;

    // Create a ObjC object that responds to some web view callbacks
    fDelegate = [[WebViewDelegate alloc] init];
    fWebViewDelegate.cppView = this;
    fWebView.navigationDelegate = fWebViewDelegate;
    [fWebView.configuration.userContentController addScriptMessageHandler:fWebViewDelegate name:@"host"];

    // windowId is either a PuglCairoView* or PuglOpenGLViewDGL* depending
    // on the value of UI_TYPE in the Makefile. Both are NSView subclasses.
    NSView *parentView = (NSView *)parentWidget->getWindow().getNativeWindowHandle();
    CGSize parentSize = parentView.frame.size;
    [fWebView removeFromSuperview];
    fWebView.frame = CGRectMake(0, 0, parentSize.width, parentSize.height);
    [parentView addSubview:fWebView];

    String js = String(JS_POST_MESSAGE_SHIM);
    injectDefaultScripts(js);
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
    CGRect frame = fWebView.superview.frame;
    frame.origin.x = 0;
    frame.origin.y = 0;
    fWebView.frame = frame;
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

@implementation WebViewDelegate

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

    self.cppView->didReceiveScriptMessage(args);
}

@end
