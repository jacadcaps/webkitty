/*
 * Copyright (C) 2014-2017 Apple Inc. All rights reserved.
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

#pragma once

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

#if HAVE(IOSURFACE)
#include <pal/spi/cocoa/IOSurfaceSPI.h>
#endif

#if USE(APPLE_INTERNAL_SDK)

#include <CoreGraphics/CGContextDelegatePrivate.h>
#include <CoreGraphics/CGFontCache.h>
#include <CoreGraphics/CGPathPrivate.h>
#include <CoreGraphics/CoreGraphicsPrivate.h>
#include <CoreGraphics/CGStylePrivate.h>

#if PLATFORM(MAC)
#include <CoreGraphics/CGAccessibility.h>
#endif

#else

struct CGFontHMetrics {
    int ascent;
    int descent;
    int lineGap;
    int maxAdvanceWidth;
    int minLeftSideBearing;
    int minRightSideBearing;
};

typedef CF_ENUM (int32_t, CGContextDelegateCallbackName)
{
    deDrawImage = 7,
    deDrawGlyphs = 8,
    deBeginLayer = 17,
    deEndLayer = 18,
};

typedef const struct CGColorTransform* CGColorTransformRef;

typedef enum {
    kCGContextTypeUnknown,
    kCGContextTypePDF,
    kCGContextTypePostScript,
    kCGContextTypeWindow,
    kCGContextTypeBitmap,
    kCGContextTypeGL,
    kCGContextTypeDisplayList,
    kCGContextTypeKSeparation,
    kCGContextTypeIOSurface,
    kCGContextTypeCount
} CGContextType;

typedef enum {
    kCGCompositeCopy = 1,
    kCGCompositeSover = 2,
} CGCompositeOperation;

enum {
    kCGFontRenderingStyleAntialiasing = 1 << 0,
    kCGFontRenderingStyleSmoothing = 1 << 1,
    kCGFontRenderingStyleSubpixelPositioning = 1 << 2,
    kCGFontRenderingStyleSubpixelQuantization = 1 << 3,
    kCGFontRenderingStylePlatformNative = 1 << 9,
    kCGFontRenderingStyleMask = 0x20F,
};
typedef uint32_t CGFontRenderingStyle;

enum {
    kCGFontAntialiasingStyleUnfiltered = 0 << 7,
    kCGFontAntialiasingStyleFilterLight = 1 << 7,
#if PLATFORM(MAC)
    kCGFontAntialiasingStyleUnfilteredCustomDilation = (8 << 7),
#endif
};
typedef uint32_t CGFontAntialiasingStyle;

enum {
    kCGImageCachingTransient = 1,
    kCGImageCachingTemporary = 3,
};
typedef uint32_t CGImageCachingFlags;

#if PLATFORM(COCOA)
typedef struct CGSRegionEnumeratorObject* CGSRegionEnumeratorObj;
typedef struct CGSRegionObject* CGSRegionObj;
typedef struct CGSRegionObject* CGRegionRef;
#endif

#ifdef CGFLOAT_IS_DOUBLE
#define CGRound(value) round((value))
#define CGFloor(value) floor((value))
#define CGCeiling(value) ceil((value))
#define CGFAbs(value) fabs((value))
#else
#define CGRound(value) roundf((value))
#define CGFloor(value) floorf((value))
#define CGCeiling(value) ceilf((value))
#define CGFAbs(value) fabsf((value))
#endif

static inline CGFloat CGFloatMin(CGFloat a, CGFloat b) { return isnan(a) ? b : ((isnan(b) || a < b) ? a : b); }

typedef struct CGFontCache CGFontCache;

#if PLATFORM(COCOA)

enum {
    kCGSWindowCaptureNominalResolution = 0x0200,
    kCGSCaptureIgnoreGlobalClipShape = 0x0800,
};
typedef uint32_t CGSWindowCaptureOptions;

typedef CF_ENUM (int32_t, CGStyleDrawOrdering) {
    kCGStyleDrawOrderingStyleOnly = 0,
    kCGStyleDrawOrderingBelow = 1,
    kCGStyleDrawOrderingAbove = 2,
};

typedef CF_ENUM (int32_t, CGFocusRingOrdering) {
    kCGFocusRingOrderingNone = kCGStyleDrawOrderingStyleOnly,
    kCGFocusRingOrderingBelow = kCGStyleDrawOrderingBelow,
    kCGFocusRingOrderingAbove = kCGStyleDrawOrderingAbove,
};

typedef CF_ENUM (int32_t, CGFocusRingTint) {
    kCGFocusRingTintBlue = 0,
    kCGFocusRingTintGraphite = 1,
};

struct CGFocusRingStyle {
    unsigned int version;
    CGFocusRingTint tint;
    CGFocusRingOrdering ordering;
    CGFloat alpha;
    CGFloat radius;
    CGFloat threshold;
    CGRect bounds;
    int accumulate;
};
typedef struct CGFocusRingStyle CGFocusRingStyle;

#endif // PLATFORM(COCOA)

struct CGShadowStyle {
    unsigned a;
    CGFloat b;
    CGFloat azimuth;
    CGFloat c;
    CGFloat height;
    CGFloat radius;
    CGFloat d;
};
typedef struct CGShadowStyle CGShadowStyle;

typedef CF_ENUM (int32_t, CGStyleType)
{
    kCGStyleShadow = 1,
};

#if PLATFORM(MAC)

typedef CF_ENUM(uint32_t, CGSNotificationType) {
    kCGSFirstConnectionNotification = 900,
    kCGSFirstSessionNotification = 1500,
};

static const CGSNotificationType kCGSConnectionWindowModificationsStarted = (CGSNotificationType)(kCGSFirstConnectionNotification + 6);
static const CGSNotificationType kCGSConnectionWindowModificationsStopped = (CGSNotificationType)(kCGSFirstConnectionNotification + 7);
static const CGSNotificationType kCGSessionConsoleConnect = kCGSFirstSessionNotification;
static const CGSNotificationType kCGSessionConsoleDisconnect = (CGSNotificationType)(kCGSessionConsoleConnect + 1);

#endif // PLATFORM(MAC)

typedef struct CGContextDelegate *CGContextDelegateRef;
typedef void (*CGContextDelegateCallback)(void);
typedef struct CGRenderingState *CGRenderingStateRef;
typedef struct CGGState *CGGStateRef;
typedef struct CGStyle *CGStyleRef;

#endif // USE(APPLE_INTERNAL_SDK)

#if PLATFORM(COCOA)
typedef uint32_t CGSByteCount;
typedef uint32_t CGSConnectionID;
typedef uint32_t CGSWindowCount;
typedef uint32_t CGSWindowID;

typedef CGSWindowID* CGSWindowIDList;
typedef struct CF_BRIDGED_TYPE(id) CGSRegionObject* CGSRegionObj;

typedef void* CGSNotificationArg;
typedef void* CGSNotificationData;
#endif

#if PLATFORM(MAC)
typedef void (*CGSNotifyConnectionProcPtr)(CGSNotificationType, void* data, uint32_t data_length, void* arg, CGSConnectionID);
typedef void (*CGSNotifyProcPtr)(CGSNotificationType, void* data, uint32_t data_length, void* arg);
#endif

WTF_EXTERN_C_BEGIN

bool CGColorTransformConvertColorComponents(CGColorTransformRef, CGColorSpaceRef, CGColorRenderingIntent, const CGFloat srcComponents[], CGFloat dstComponents[]);
CGColorRef CGColorTransformConvertColor(CGColorTransformRef, CGColorRef, CGColorRenderingIntent);
CGColorTransformRef CGColorTransformCreate(CGColorSpaceRef, CFDictionaryRef attributes);

CGAffineTransform CGContextGetBaseCTM(CGContextRef);
CGCompositeOperation CGContextGetCompositeOperation(CGContextRef);
CGColorRef CGContextGetFillColorAsColor(CGContextRef);
CGFloat CGContextGetLineWidth(CGContextRef);
bool CGContextGetShouldSmoothFonts(CGContextRef);
bool CGContextGetShouldAntialias(CGContextRef);
void CGContextSetBaseCTM(CGContextRef, CGAffineTransform);
void CGContextSetCTM(CGContextRef, CGAffineTransform);
void CGContextSetCompositeOperation(CGContextRef, CGCompositeOperation);
void CGContextSetShouldAntialiasFonts(CGContextRef, bool shouldAntialiasFonts);
void CGContextResetClip(CGContextRef);
CGContextType CGContextGetType(CGContextRef);

CFStringRef CGFontCopyFamilyName(CGFontRef);
bool CGFontGetGlyphAdvancesForStyle(CGFontRef, const CGAffineTransform* , CGFontRenderingStyle, const CGGlyph[], size_t count, CGSize advances[]);
void CGFontGetGlyphsForUnichars(CGFontRef, const UniChar[], CGGlyph[], size_t count);
const CGFontHMetrics* CGFontGetHMetrics(CGFontRef);
const char* CGFontGetPostScriptName(CGFontRef);
bool CGFontIsFixedPitch(CGFontRef);
void CGFontSetShouldUseMulticache(bool);

void CGImageSetCachingFlags(CGImageRef, CGImageCachingFlags);
CGImageCachingFlags CGImageGetCachingFlags(CGImageRef);

CGDataProviderRef CGPDFDocumentGetDataProvider(CGPDFDocumentRef);

CGFontAntialiasingStyle CGContextGetFontAntialiasingStyle(CGContextRef);
void CGContextSetFontAntialiasingStyle(CGContextRef, CGFontAntialiasingStyle);
bool CGContextGetAllowsFontSubpixelPositioning(CGContextRef);
bool CGContextDrawsWithCorrectShadowOffsets(CGContextRef);
CGPatternRef CGPatternCreateWithImage2(CGImageRef, CGAffineTransform, CGPatternTiling);

CGContextDelegateRef CGContextDelegateCreate(void* info);
void CGContextDelegateSetCallback(CGContextDelegateRef, CGContextDelegateCallbackName, CGContextDelegateCallback);
CGContextRef CGContextCreateWithDelegate(CGContextDelegateRef, CGContextType, CGRenderingStateRef, CGGStateRef);
void* CGContextDelegateGetInfo(CGContextDelegateRef);
void CGContextDelegateRelease(CGContextDelegateRef);
CGFloat CGGStateGetAlpha(CGGStateRef);
CGFontRef CGGStateGetFont(CGGStateRef);
const CGAffineTransform *CGGStateGetCTM(CGGStateRef);
CGColorRef CGGStateGetFillColor(CGGStateRef);
CGColorRef CGGStateGetStrokeColor(CGGStateRef);
CGStyleRef CGGStateGetStyle(CGGStateRef);
CGStyleType CGStyleGetType(CGStyleRef);
const void *CGStyleGetData(CGStyleRef);
CGColorRef CGStyleGetColor(CGStyleRef);
bool CGColorSpaceEqualToColorSpace(CGColorSpaceRef, CGColorSpaceRef);
CFStringRef CGColorSpaceCopyICCProfileDescription(CGColorSpaceRef);

#if HAVE(CGPATH_GET_NUMBER_OF_ELEMENTS)
size_t CGPathGetNumberOfElements(CGPathRef);
#endif

#if HAVE(IOSURFACE)
CGContextRef CGIOSurfaceContextCreate(IOSurfaceRef, size_t, size_t, size_t, size_t, CGColorSpaceRef, CGBitmapInfo);
CGImageRef CGIOSurfaceContextCreateImage(CGContextRef);
CGImageRef CGIOSurfaceContextCreateImageReference(CGContextRef);
CGColorSpaceRef CGIOSurfaceContextGetColorSpace(CGContextRef);
void CGIOSurfaceContextSetDisplayMask(CGContextRef, uint32_t mask);
#endif // HAVE(IOSURFACE)

#if PLATFORM(COCOA)
bool CGColorSpaceUsesExtendedRange(CGColorSpaceRef);

typedef struct CGPDFAnnotation *CGPDFAnnotationRef;
typedef bool (^CGPDFAnnotationDrawCallbackType)(CGContextRef context, CGPDFPageRef page, CGPDFAnnotationRef annotation);
void CGContextDrawPDFPageWithAnnotations(CGContextRef, CGPDFPageRef, CGPDFAnnotationDrawCallbackType);
void CGContextDrawPathDirect(CGContextRef, CGPathDrawingMode, CGPathRef, const CGRect* boundingBox);

CGColorSpaceRef CGContextCopyDeviceColorSpace(CGContextRef);
CFPropertyListRef CGColorSpaceCopyPropertyList(CGColorSpaceRef);
CGError CGSNewRegionWithRect(const CGRect*, CGRegionRef*);
CGError CGSPackagesEnableConnectionOcclusionNotifications(CGSConnectionID, bool flag, bool* outCurrentVisibilityState);
CGError CGSPackagesEnableConnectionWindowModificationNotifications(CGSConnectionID, bool flag, bool* outConnectionIsCurrentlyIdle);
CGError CGSReleaseRegion(const CGRegionRef CF_RELEASES_ARGUMENT);
CGError CGSReleaseRegionEnumerator(const CGSRegionEnumeratorObj);
CGError CGSSetWindowAlpha(CGSConnectionID, CGSWindowID, float alpha);
CGError CGSSetWindowClipShape(CGSConnectionID, CGSWindowID, CGRegionRef shape);
CGError CGSSetWindowWarp(CGSConnectionID, CGSWindowID, int w, int h, const float* mesh);
CGRect* CGSNextRect(const CGSRegionEnumeratorObj);
CGSRegionEnumeratorObj CGSRegionEnumerator(CGRegionRef);
CGStyleRef CGStyleCreateFocusRingWithColor(const CGFocusRingStyle*, CGColorRef);
void CGContextSetStyle(CGContextRef, CGStyleRef);
void CGContextDrawConicGradient(CGContextRef, CGGradientRef, CGPoint center, CGFloat angle);
void CGPathAddUnevenCornersRoundedRect(CGMutablePathRef, const CGAffineTransform *, CGRect, const CGSize corners[4]);
bool CGFontRenderingGetFontSmoothingDisabled(void);

#endif // PLATFORM(COCOA)

#if PLATFORM(WIN)
CGFontCache* CGFontCacheGetLocalCache();
void CGFontCacheSetShouldAutoExpire(CGFontCache*, bool);
void CGFontCacheSetMaxSize(CGFontCache*, size_t);
void CGContextSetFontSmoothingContrast(CGContextRef, CGFloat);
void CGContextSetFontSmoothingStyle(CGContextRef, uint32_t);
uint32_t CGContextGetFontSmoothingStyle(CGContextRef);
void CGContextSetShouldUsePlatformNativeGlyphs(CGContextRef, bool);
void CGContextSetFocusRingWithColor(CGContextRef, CGFloat blur, CGColorRef, const CGRect *clipRect, CFDictionaryRef options);
#endif // PLATFORM(WIN)

#if PLATFORM(MAC)

bool CGDisplayUsesForceToGray(void);

void CGSShutdownServerConnections(void);

CGSConnectionID CGSMainConnectionID(void);
CFArrayRef CGSHWCaptureWindowList(CGSConnectionID, CGSWindowIDList windowList, CGSWindowCount, CGSWindowCaptureOptions);
CGError CGSSetConnectionProperty(CGSConnectionID, CGSConnectionID ownerCid, CFStringRef key, CFTypeRef value);
CGError CGSCopyConnectionProperty(CGSConnectionID, CGSConnectionID ownerCid, CFStringRef key, CFTypeRef *value);
CGError CGSGetScreenRectForWindow(CGSConnectionID, CGSWindowID, CGRect *);
CGError CGSRegisterConnectionNotifyProc(CGSConnectionID, CGSNotifyConnectionProcPtr, CGSNotificationType, void* arg);
CGError CGSRegisterNotifyProc(CGSNotifyProcPtr, CGSNotificationType, void* arg);

size_t CGDisplayModeGetPixelsWide(CGDisplayModeRef);
size_t CGDisplayModeGetPixelsHigh(CGDisplayModeRef);

CGError CGSSetDenyWindowServerConnections(bool);
typedef int32_t CGSDisplayID;
CGSDisplayID CGSMainDisplayID(void);

#endif // PLATFORM(MAC)

#if ENABLE(PDFKIT_PLUGIN) && !USE(APPLE_INTERNAL_SDK)

extern const off_t kCGDataProviderIndeterminateSize;
extern const CFStringRef kCGDataProviderHasHighLatency;

typedef void (*CGDataProviderGetByteRangesCallback)(void *info,
    CFMutableArrayRef buffers, const CFRange *ranges, size_t count);
    
struct CGDataProviderDirectAccessRangesCallbacks {
    unsigned version;
    CGDataProviderGetBytesAtPositionCallback getBytesAtPosition;
    CGDataProviderGetByteRangesCallback getBytesInRanges;
    CGDataProviderReleaseInfoCallback releaseInfo;
};
typedef struct CGDataProviderDirectAccessRangesCallbacks CGDataProviderDirectAccessRangesCallbacks;

extern void CGDataProviderSetProperty(CGDataProviderRef, CFStringRef key, CFTypeRef value);
extern CGDataProviderRef CGDataProviderCreateMultiRangeDirectAccess(
    void *info, off_t size,
    const CGDataProviderDirectAccessRangesCallbacks *);

#endif // ENABLE(PDFKIT_PLUGIN) && !USE(APPLE_INTERNAL_SDK)

WTF_EXTERN_C_END
