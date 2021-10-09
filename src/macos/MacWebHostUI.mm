/*
 * Hip-Hop / High Performance Hybrid Audio Plugins
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
        setWebView(new CocoaWebView()); // base class owns web view
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
    NSWindow* nsw = [(id)window isKindOfClass:[NSView class]] ?
        [(NSView *)window window] : (NSWindow *)window;
    return (nsw.screen ? nsw.screen : [NSScreen mainScreen]).backingScaleFactor;
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
