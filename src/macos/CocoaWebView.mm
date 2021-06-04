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

#define fWebView ((WKWebView*)fView)

// NOTE: ARC is not available

@interface WebViewDelegate: NSObject<WKNavigationDelegate>
{}
@end

USE_NAMESPACE_DISTRHO

CocoaWebView::CocoaWebView(WebViewScriptMessageHandler& handler)
    : BaseWebView(handler)
{
    fView = [[WKWebView alloc] initWithFrame:CGRectZero];
    [fWebView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    fWebView.navigationDelegate = [[WebViewDelegate alloc] init];
    fWebView.hidden = YES;
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
}

CocoaWebView::~CocoaWebView()
{
    [fWebView.navigationDelegate release];
    [fWebView removeFromSuperview];
    [fWebView release];
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
}

void CocoaWebView::navigate(String url)
{
    NSString *urlStr = [[NSString alloc] initWithCString:url encoding:NSUTF8StringEncoding];
    NSURL *urlObj = [[NSURL alloc] initWithString:urlStr];
    NSURLRequest *request = [[NSURLRequest alloc] initWithURL:urlObj];
    [fWebView loadRequest:request];
    [request release];
    [urlObj release];
    [urlStr release];
}

void CocoaWebView::runScript(String source)
{
    // TODO
}

@implementation WebViewDelegate

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation
{
    webView.hidden = NO;
    // TODO: CocoaWebView::contentReady()
}

@end
