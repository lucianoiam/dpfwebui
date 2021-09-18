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

#import <AppKit/Appkit.h>

#include "MacWebHostUI.hpp"

#define fNsWindow ((NSWindow*)fWindow)

USE_NAMESPACE_DISTRHO

MacWebHostUI::MacWebHostUI(uint baseWidth, uint baseHeight, uint32_t backgroundColor)
    : AbstractWebHostUI(baseWidth, baseHeight, backgroundColor)
    , fWindow(0)
{
    if (shouldCreateWebView()) {
        setWebView(new CocoaWebView());
    }
}

MacWebHostUI::~MacWebHostUI()
{
    if (fNsWindow != 0) {
        [fNsWindow orderOut:nil];
        [fNsWindow release];
    }
}

float MacWebHostUI::getDisplayScaleFactor(uintptr_t window)
{
    NSWindow *w;

    // DGL::Window::getNativeWindowHandle() returns NSView* instead of NSWindow*

    if ([(id)window isKindOfClass:[NSView class]]) {
        w = [(NSView *)window window];
    } else {
        w = (NSWindow *)window;
    }

    return (w.screen ? w.screen : [NSScreen mainScreen]).backingScaleFactor;
}

void MacWebHostUI::openSystemWebBrowser(String& url)
{
    NSString *s = [[NSString alloc] initWithCString:url.buffer() encoding:NSUTF8StringEncoding];
    NSURL *nsUrl = [[NSURL alloc] initWithString:s];
    [[NSWorkspace sharedWorkspace] openURL:nsUrl];
    [nsUrl release];
    [s release];
}

uintptr_t MacWebHostUI::createStandaloneWindow()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];

    CGRect contentRect = NSMakeRect(0, 0, (CGFloat)getWidth(), (CGFloat)getHeight());
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

void MacWebHostUI::processStandaloneEvents()
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
