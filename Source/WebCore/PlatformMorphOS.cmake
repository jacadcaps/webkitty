include(platform/Cairo.cmake)
include(platform/Curl.cmake)
include(platform/FreeType.cmake)
include(platform/ImageDecoders.cmake)
include(platform/GCrypt.cmake)

list(APPEND WebCore_PRIVATE_INCLUDE_DIRECTORIES
    "${WEBKIT_LIBRARIES_DIR}/include"
)

list(APPEND WebCore_PRIVATE_INCLUDE_DIRECTORIES
    ${WEBCORE_DIR}/platform
    ${WEBCORE_DIR}/platform/morphos
    ${WEBCORE_DIR}/platform/generic
    ${WEBCORE_DIR}/platform/graphics/morphos
    ${WEBCORE_DIR}/platform/mediacapabilities
)

list(APPEND WebCore_SYSTEM_INCLUDE_DIRECTORIES
    ${ICU_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIR}
    ${SQLITE_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${WPE_INCLUDE_DIRS}
    ${HYPHEN_INCLUDE_DIRS}
    ${AVCODEC_INCLUDE_DIR}
)

list(APPEND WebCore_LIBRARIES
    ${ICU_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${SQLITE_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${WPE_LIBRARIES}
    ${HYPHEN_LIBRARIES}
)

list(APPEND WebCore_SOURCES
    editing/morphos/EditorMorphOS.cpp
    editing/morphos/AutofillElements.cpp
    platform/morphos/PasteboardMorphOS.cpp
    platform/morphos/CursorMorphOS.cpp
    platform/morphos/PlatformKeyboardEvent.cpp
    platform/morphos/PlatformScreenMorphOS.cpp
    platform/morphos/ScrollbarThemeMorphOS.cpp
#    platform/morphos/EventLoopMorphOS.cpp
    platform/morphos/MIMETypeRegistryMorphOS.cpp
    platform/morphos/DragDataMorphOS.cpp
    platform/morphos/SelectionData.cpp
    platform/generic/KeyedDecoderGeneric.cpp
    platform/generic/KeyedEncoderGeneric.cpp
    platform/graphics/morphos/GraphicsLayerMorphOS.cpp
    platform/graphics/morphos/ImageMorphOS.cpp
    platform/network/morphos/CurlSSLHandleMorphOS.cpp
    platform/network/morphos/NetworkStateNotifierMorphOS.cpp
    platform/posix/SharedBufferPOSIX.cpp
    platform/text/LocaleICU.cpp
    platform/text/hyphen/HyphenationLibHyphen.cpp
    rendering/RenderThemeMorphOS.cpp
    page/morphos/DragControllerMorphOS.cpp
    platform/audio/morphos/AudioDestinationMorphOS.cpp
    platform/audio/morphos/AudioBusMorphOS.cpp
    platform/audio/morphos/AudioFileReaderMorphOS.cpp
    platform/audio/morphos/FFTFrameMorphOS.cpp
    platform/graphics/morphos/MediaPlayerPrivateMorphOS.cpp
    platform/graphics/morphos/DisplayRefreshMonitorMorphOS.cpp
)

if (ENABLE_ACINERELLA)
    list(APPEND WebCore_SOURCES
    platform/graphics/morphos/acinerella.c
    )
endif ()

list(APPEND WebCore_PRIVATE_FRAMEWORK_HEADERS
    platform/morphos/SelectionData.h
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
