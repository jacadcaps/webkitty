/*
 * Copyright (C) 2006-2020 Apple Inc. All rights reserved.
 * Copyright (C) 2007-2009 Torch Mobile, Inc.
 * Copyright (C) 2010, 2011 Research In Motion Limited. All rights reserved.
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

/* Include compiler specific macros */
#include <wtf/Compiler.h>

/* This ensures that users #include <wtf/Platform.h> rather than one of the helper files files directly. */
#define WTF_PLATFORM_GUARD_AGAINST_INDIRECT_INCLUSION


/* ==== Platform adaptation macros: these describe properties of the target environment. ==== */

/* CPU() - the target CPU architecture */
#include <wtf/PlatformCPU.h>

/* OS() - underlying operating system; only to be used for mandated low-level services like
   virtual memory, not to choose a GUI toolkit */
#include <wtf/PlatformOS.h>

/* PLATFORM() - handles OS, operating environment, graphics API, and
   CPU. This macro will be phased out in favor of platform adaptation
   macros, policy decision macros, and top-level port definitions. */
#include <wtf/PlatformLegacy.h>

/* HAVE() - specific system features (headers, functions or similar) that are present or not */

#ifdef __MORPHOS__
#define WTF_OS_MORPHOS 1
#endif

/* ==== Policy decision macros: these define policy choices for a particular port. ==== */

/* USE() - use a particular third-party library or optional OS service */
#include <wtf/PlatformUse.h>

/* ENABLE() - turn on a specific feature of WebKit */
#include <wtf/PlatformEnable.h>


/* ==== Helper macros ==== */

/* Macros for specifing specific calling conventions. */
#include <wtf/PlatformCallingConventions.h>


/* ==== Platform additions: additions to Platform.h from outside the main repository ==== */

#if USE(APPLE_INTERNAL_SDK) && __has_include(<WebKitAdditions/AdditionalPlatform.h>)
#include <WebKitAdditions/AdditionalPlatform.h>
#endif


#undef WTF_PLATFORM_GUARD_AGAINST_INDIRECT_INCLUSION


/* FIXME: The following are currenly positioned at the bottom of this file as they either
   are currently dependent on macros they should not be and need to be refined or do not
   belong as part of Platform.h at all. */


#if PLATFORM(GTK)
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_36
#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_6
#endif

#if PLATFORM(WPE)
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_40
#endif

#if USE(SOUP)
#define SOUP_VERSION_MIN_REQUIRED SOUP_VERSION_2_42
#endif

#if PLATFORM(COCOA)
/* Cocoa defines a series of platform macros for debugging. */
/* Some of them are really annoying because they use common names (e.g. check()). */
/* Disable those macros so that we are not limited in how we name methods and functions. */
#undef __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES
#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
<<<<<<< HEAD
=======

#endif

#if PLATFORM(MAC)

#define HAVE_RUNLOOP_TIMER 1
#define HAVE_SEC_KEYCHAIN 1
#define USE_APPKIT 1
#define USE_PASSKIT 1

#if CPU(X86_64)
#define HAVE_NETWORK_EXTENSION 1
#define USE_PLUGIN_HOST_PROCESS 1
#endif
#endif /* PLATFORM(MAC) */

#if PLATFORM(IOS_FAMILY)

#define HAVE_NETWORK_EXTENSION 1
#define HAVE_READLINE 1
#define USE_UIKIT_EDITING 1
#define USE_WEB_THREAD 1

#if CPU(ARM64)
#define ENABLE_JIT_CONSTANT_BLINDING 0
#endif

#if CPU(ARM_NEON)
#undef HAVE_ARM_NEON_INTRINSICS
#define HAVE_ARM_NEON_INTRINSICS 0
#endif

#endif /* PLATFORM(IOS_FAMILY) */

#if !defined(HAVE_ACCESSIBILITY)
#if PLATFORM(COCOA) || PLATFORM(WIN) || PLATFORM(GTK) || PLATFORM(WPE)
#define HAVE_ACCESSIBILITY 1
#endif
#endif /* !defined(HAVE_ACCESSIBILITY) */

/* FIXME: Remove after CMake build enabled on Darwin */
#if OS(DARWIN)
#define HAVE_ERRNO_H 1
#define HAVE_LANGINFO_H 1
#define HAVE_LOCALTIME_R 1
#define HAVE_MMAP 1
#define HAVE_REGEX_H 1
#define HAVE_SIGNAL_H 1
#define HAVE_STAT_BIRTHTIME 1
#define HAVE_STRINGS_H 1
#define HAVE_STRNSTR 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_TIME_H 1 
#define HAVE_TM_GMTOFF 1
#define HAVE_TM_ZONE 1
#define HAVE_TIMEGM 1
#define HAVE_PTHREAD_MAIN_NP 1

#if CPU(X86_64) || CPU(ARM64)
#define HAVE_INT128_T 1
#endif
#endif /* OS(DARWIN) */

#if OS(UNIX)
#define USE_PTHREADS 1
#endif /* OS(UNIX) */

#if OS(MORPHOS)
#define USE_PTHREADS 1
#endif

#if OS(UNIX) && !OS(FUCHSIA)
#define HAVE_RESOURCE_H 1
#define HAVE_PTHREAD_SETSCHEDPARAM 1
#endif

#if OS(DARWIN)
#define HAVE_DISPATCH_H 1
#define HAVE_MADV_FREE 1
#define HAVE_MADV_FREE_REUSE 1
#define HAVE_MADV_DONTNEED 1
#define HAVE_MERGESORT 1
#define HAVE_PTHREAD_SETNAME_NP 1
#define HAVE_READLINE 1
#define HAVE_SYS_TIMEB_H 1

#if __has_include(<mach/mach_exc.defs>) && !PLATFORM(GTK)
#define HAVE_MACH_EXCEPTIONS 1
#endif

#if !PLATFORM(GTK)
#define USE_ACCELERATE 1
#endif
#if !PLATFORM(IOS_FAMILY)
#define HAVE_HOSTED_CORE_ANIMATION 1
#endif

#endif /* OS(DARWIN) */

#if OS(DARWIN) || OS(FUCHSIA) || ((OS(FREEBSD) || defined(__GLIBC__) || defined(__BIONIC__)) && (CPU(X86) || CPU(X86_64) || CPU(ARM) || CPU(ARM64) || CPU(MIPS)))
#define HAVE_MACHINE_CONTEXT 1
#endif

#if OS(DARWIN) || (OS(LINUX) && defined(__GLIBC__) && !defined(__UCLIBC__))
#define HAVE_BACKTRACE 1
>>>>>>> WTF partial port
#endif

#if COMPILER(MSVC)
#undef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#undef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

/* FIXME: Any remaining use of TARGET_OS_IPHONE should be removed outside of Apple only files and replaced with OS() checks. */
/* Set TARGET_OS_IPHONE to 0 by default to allow using it as a guard
 * in cross-platform the same way as it is used in OS(DARWIN) code. */ 
#if !defined(TARGET_OS_IPHONE) && !OS(DARWIN)
#define TARGET_OS_IPHONE 0
#endif

/* FIXME: This does not belong in Platform.h and should instead be included in another mechanism (compiler option, prefix header, config.h, etc) */
#if COMPILER(MSVC)
/* Enable strict runtime stack buffer checks. */
#pragma strict_gs_check(on)
#endif

/* FIXME: This does not belong in Platform.h and should instead be included in another mechanism (prefix header, config.h, etc) */
#if USE(GLIB)
#include <wtf/glib/GTypedefs.h>
#endif

/* FIXME: The availability of RSA_PSS should not depend on the policy decision to USE(GCRYPT). */
#if PLATFORM(MAC) || PLATFORM(IOS) || PLATFORM(MACCATALYST) || USE(GCRYPT)
#define HAVE_RSA_PSS 1
#endif

/* FIXME: Remove dependence on ENABLE(WEB_RTC). */
#if PLATFORM(COCOA) && ENABLE(WEB_RTC)
#define USE_LIBWEBRTC 1
#endif


#if PLATFORM(COCOA)
#if ENABLE(WEBGL)

/* USE_ANGLE=1 uses ANGLE for the WebGL backend.
   It replaces USE_OPENGL, USE_OPENGL_ES and USE_EGL. */
#if PLATFORM(MAC) || (PLATFORM(MACCATALYST) && __has_include(<OpenGL/OpenGL.h>))
#define USE_OPENGL 1
#define USE_OPENGL_ES 0
#define USE_ANGLE 0
#else
#define USE_OPENGL 0
#define USE_OPENGL_ES 1
#define USE_ANGLE 0
#endif

#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION 1
#endif

#if USE(OPENGL) && !defined(HAVE_OPENGL_4)
#define HAVE_OPENGL_4 1
#endif

#if USE(OPENGL_ES) && !defined(HAVE_OPENGL_ES_3)
#define HAVE_OPENGL_ES_3 1
#endif

#if USE_ANGLE && (USE_OPENGL || USE_OPENGL_ES)
#error USE_ANGLE is incompatible with USE_OPENGL and USE_OPENGL_ES
#endif

#endif /* ENABLE(WEBGL) */
#endif /* PLATFORM(COCOA) */

#if ENABLE(WEBGL)
#if !defined(USE_ANGLE)
#define USE_ANGLE 0
#endif

#if (USE_ANGLE && (USE_OPENGL || USE_OPENGL_ES || (defined(USE_EGL) && USE_EGL))) && !USE(TEXTURE_MAPPER)
#error USE_ANGLE is incompatible with USE_OPENGL, USE_OPENGL_ES and USE_EGL
#endif
#endif

#if USE(TEXTURE_MAPPER) && ENABLE(GRAPHICS_CONTEXT_GL) && !defined(USE_TEXTURE_MAPPER_GL)
#define USE_TEXTURE_MAPPER_GL 1
#endif

/* FIXME: This is used to "turn on a specific feature of WebKit", so should be converted to an ENABLE macro. */
#if PLATFORM(COCOA) && ENABLE(ACCESSIBILITY)
#define USE_ACCESSIBILITY_CONTEXT_MENUS 1
#endif
