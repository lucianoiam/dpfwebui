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
    initWebView(fWebView);
}

MacWebHostUI::~MacWebHostUI()
{
    // TODO
    NSLog(@"~MacWebHostUI()");

    [fNsWindow orderOut: nil];
    [fNsWindow release];
}

uintptr_t MacWebHostUI::createStandaloneWindow()
{
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
    fWindow = (uintptr_t)window;

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
