/*
 * Copyright (C) 2015 Apple Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <wtf/Platform.h>

#if USE(APPKIT)

#import <pal/spi/cocoa/QuartzCoreSPI.h>

#if USE(APPLE_INTERNAL_SDK)
#import <AppKit/NSView_Private.h>
#else

#if USE(NSVIEW_SEMANTICCONTEXT)

typedef NS_ENUM(NSInteger, NSViewSemanticContext) {
    NSViewSemanticContextForm = 8,
};

#endif

@interface NSView ()

- (NSView *)_findLastViewInKeyViewLoop;

#if USE(NSVIEW_SEMANTICCONTEXT)
@property (nonatomic, setter=_setSemanticContext:) NSViewSemanticContext _semanticContext;
#endif

@end

#endif // USE(APPLE_INTERNAL_SDK)

@interface NSView () <CALayerDelegate>
@end

@interface NSView (SubviewsIvar)
@property (assign, setter=_setSubviewsIvar:) NSMutableArray<__kindof NSView *> *_subviewsIvar;
@end

#endif // PLATFORM(MAC)
