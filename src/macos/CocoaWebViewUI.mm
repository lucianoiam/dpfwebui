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

#include <dlfcn.h>
#include <libgen.h>

#include "CocoaWebViewUI.hpp"
#include "Platform.hpp"

@interface WebViewDelegate: NSObject<WKNavigationDelegate>
{}
@end

USE_NAMESPACE_DISTRHO

UI* DISTRHO::createUI()
{
    return new CocoaWebViewUI;
}

CocoaWebViewUI::CocoaWebViewUI()
{
    // No ARC here
    fView = [[WKWebView alloc] initWithFrame:CGRectZero];
    [fView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    fView.hidden = YES;
    fView.navigationDelegate = [[WebViewDelegate alloc] init];
    NSString *urlStr = [[NSString alloc] initWithCString:getContentUrl() encoding:NSUTF8StringEncoding];
    NSURL *url = [[NSURL alloc] initWithString:urlStr];
    NSURLRequest *request = [[NSURLRequest alloc] initWithURL:url];
    [fView loadRequest:request];
    [request release];
    [url release];
    [urlStr release];
}

CocoaWebViewUI::~CocoaWebViewUI()
{
    [fView.navigationDelegate release];
    [fView removeFromSuperview];
    [fView release];
}

void CocoaWebViewUI::parameterChanged(uint32_t index, float value)
{
    // unused
    (void)index;
    (void)value;
}

void CocoaWebViewUI::reparent(uintptr_t windowId)
{
    // windowId is either a PuglCairoView* or PuglOpenGLViewDGL* depending
    // on the value of UI_TYPE in the Makefile. Both are NSView subclasses.
    NSView *parentView = (NSView *)windowId;
    CGSize parentSize = parentView.frame.size;
    [fView removeFromSuperview];
    fView.frame = CGRectMake(0, 0, parentSize.width, parentSize.height);
    [parentView addSubview:fView];
}

String CocoaWebViewUI::getResourcePath()
{
    // There is no DPF method for querying plugin format during runtime
    // Anyways the ideal solution is to modify the Makefile and rely on macros
    // Mac VST is the only special case
    char path[PATH_MAX];
    ::strcpy(path, platform::getSharedLibraryPath());
    void *handle = ::dlopen(path, RTLD_NOLOAD);
    if (handle != 0) {
        void *addr = ::dlsym(handle, "VSTPluginMain");
        ::dlclose(handle);
        if (addr != 0) {
            return String(::dirname(path)) + "/../Resources";
        }
    }
    return WebUI::getResourcePath();
}

@implementation WebViewDelegate

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation
{
    webView.hidden = NO;
}

@end
