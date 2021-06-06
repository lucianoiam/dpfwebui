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

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <WebKit/WebKit.h>

#include "CocoaWebView.hpp"

#define fWebView         ((WKWebView*)fView)
#define fWebViewDelegate ((WebViewDelegate*)fDelegate)

// NOTE: ARC is not available here

USE_NAMESPACE_DISTRHO

@interface WebViewDelegate: NSObject<WKNavigationDelegate, WKScriptMessageHandler>
@property (assign, nonatomic) CocoaWebView *cppView;
- (ScriptValue)scriptValueFromObjCInstance:(id)obj;
@end

CocoaWebView::CocoaWebView(WebViewScriptMessageHandler& handler)
    : BaseWebView(handler)
{
    // Create web view
    fView = [[WKWebView alloc] initWithFrame:CGRectZero];
    [fWebView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    fWebView.hidden = YES;
    // Create a ObjC object that responds to some web view callbacks
    fDelegate = [[WebViewDelegate alloc] init];
    fWebViewDelegate.cppView = this;
    fWebView.navigationDelegate = fWebViewDelegate;
    // Play safe when calling undocumented APIs 
    if ([fWebView respondsToSelector:@selector(_setDrawsBackground:)]) {
        @try {
            NSNumber *no = [[NSNumber alloc] initWithBool:NO];
            [fWebView setValue:no forKey: @"drawsBackground"];
            [no release];
        }
        @catch (NSException * e) {
            NSLog(@"Could not set transparent color for web view");
        }
    }

    createConsole();
}

CocoaWebView::~CocoaWebView()
{
    [fWebView removeFromSuperview];
    [fWebView release];
    [fWebViewDelegate release];
}

void CocoaWebView::reparent(uintptr_t windowId)
{
    // windowId is either a PuglCairoView* or PuglOpenGLViewDGL* depending
    // on the value of UI_TYPE in the Makefile. Both are NSView subclasses.
    NSView *parentView = (NSView *)windowId;
    CGSize parentSize = parentView.frame.size;
    [fWebView removeFromSuperview];
    fWebView.frame = CGRectMake(0, 0, parentSize.width, parentSize.height);
    [parentView addSubview:fWebView];
}

void CocoaWebView::resize(const DGL::Size<uint>& size)
{
    // The WKWebView automatically resizes to match its parent dimensions
    (void)size;
}

void CocoaWebView::navigate(String url)
{
    NSString *urlStr = [[NSString alloc] initWithCString:url encoding:NSUTF8StringEncoding];
    NSURL *urlObj = [[NSURL alloc] initWithString:urlStr];
    [fWebView loadFileURL:urlObj allowingReadAccessToURL:urlObj];
    [urlObj release];
    [urlStr release];
}

void CocoaWebView::runScript(String source)
{
    NSString *js = [[NSString alloc] initWithCString:source encoding:NSUTF8StringEncoding];
    [fWebView evaluateJavaScript:js completionHandler: nil];
    [js release];
}

void CocoaWebView::injectScript(String source)
{
    NSString *js = [[NSString alloc] initWithCString:source encoding:NSUTF8StringEncoding];
    WKUserScript *script = [[WKUserScript alloc] initWithSource:js
        injectionTime:WKUserScriptInjectionTimeAtDocumentStart forMainFrameOnly:YES];
    [fWebView.configuration.userContentController addUserScript:script];
    [script release];
    [js release];
}

void CocoaWebView::addScriptMessageHandler(String name)
{
    NSString *nameStr = [[NSString alloc] initWithCString:name encoding:NSUTF8StringEncoding];
    [fWebView.configuration.userContentController addScriptMessageHandler:fWebViewDelegate name:nameStr];
    [nameStr release];
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
    String name = String([message.name cStringUsingEncoding:NSUTF8StringEncoding]);
    ScriptMessageArguments args;
    for (id objcArg : (NSArray *)message.body) {
        args.push_back([self scriptValueFromObjCInstance:objcArg]);
    }
    self.cppView->didReceiveScriptMessage(name, args);
}

- (ScriptValue)scriptValueFromObjCInstance:(id)obj
{
    if ([obj isKindOfClass:[NSNull class]]) {
        return ScriptValue();
    } else if (CFGetTypeID(obj) == CFBooleanGetTypeID()) {
        return ScriptValue(static_cast<bool>([obj boolValue]));
    } else if ([obj isKindOfClass:[NSNumber class]]) {
        return ScriptValue([obj doubleValue]);
    } else if ([obj isKindOfClass:[NSString class]]) {
        return ScriptValue(String([obj cStringUsingEncoding:NSUTF8StringEncoding]));
    }
    return ScriptValue(); // null
}

@end
