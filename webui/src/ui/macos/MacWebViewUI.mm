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

#import <AppKit/Appkit.h>

#include "MacWebViewUI.hpp"

#define fNsWindow ((NSWindow*)fWindow)

USE_NAMESPACE_DISTRHO

MacWebViewUI::MacWebViewUI(uint widthCssPx, uint heightCssPx,
        const char* backgroundCssColor, bool startLoading)
    : WebViewUI(widthCssPx, heightCssPx, backgroundCssColor, [NSScreen mainScreen].backingScaleFactor)
    , fWindow(0)
{
    if (isDryRun()) {
        return;
    }

    setWebView(new CocoaWebView(String(kWebViewUserAgent)));

    if (startLoading) {
        load();
    }
}

MacWebViewUI::~MacWebViewUI()
{
    [fNsWindow orderOut:nil];
    [fNsWindow release];
}

void MacWebViewUI::openSystemWebBrowser(String& url)
{
    NSString *s = [[NSString alloc] initWithCString:url.buffer() encoding:NSUTF8StringEncoding];
    NSURL *nsUrl = [[NSURL alloc] initWithString:s];
    [[NSWorkspace sharedWorkspace] openURL:nsUrl];
    [nsUrl release];
    [s release];
}

uintptr_t MacWebViewUI::createStandaloneWindow()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];

    CGFloat k = [NSScreen mainScreen].backingScaleFactor;

    CGRect contentRect = NSMakeRect(0, 0, (CGFloat)getWidth() / k, (CGFloat)getHeight() / k);
    NSWindowStyleMask styleMask = NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskTitled;
    NSWindow* window = [[NSWindow alloc] initWithContentRect:contentRect
                                                   styleMask:styleMask
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];
    [window makeKeyAndOrderFront:window];
    fWindow = (uintptr_t)window;

    [pool release];

    return (uintptr_t)window.contentView;
}

void MacWebViewUI::processStandaloneEvents()
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
