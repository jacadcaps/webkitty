add_definitions(-DUSE_CAIRO=1 -DUSE_CURL=1 -DWEBKIT_EXPORTS=1 -DWEBCORE_EXPORT=WTF_EXPORT_DECLARATION -DPAL_EXPORT=WTF_EXPORT -DUSE_SYSTEM_MALLOC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

list(APPEND WebKitLegacy_PRIVATE_INCLUDE_DIRECTORIES
    ${CAIRO_INCLUDE_DIRS}
    "${WEBKIT_LIBRARIES_DIR}/include"
)

list(APPEND WebKitLegacy_LIBRARIES PRIVATE WTF${DEBUG_SUFFIX})

list(APPEND WebKitLegacy_PRIVATE_INCLUDE_DIRECTORIES
    "${CMAKE_BINARY_DIR}/../include/private"
    "${CMAKE_BINARY_DIR}/../include/private/JavaScriptCore"
    "${CMAKE_BINARY_DIR}/../include/private/WebCore"
    "${WEBKITLEGACY_DIR}/morphos"
    "${WEBKITLEGACY_DIR}/morphos/WebCoreSupport"
    "${WebKitLegacy_DERIVED_SOURCES_DIR}/include"
    "${WebKitLegacy_DERIVED_SOURCES_DIR}/Interfaces"
    ${SQLITE_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${WPE_INCLUDE_DIRS}
)

list(APPEND WebKitLegacy_SOURCES_Classes
    morphos/WebFrame.cpp
    morphos/WebView.cpp
    morphos/BackForwardClient.cpp
)

list(APPEND WebKitLegacy_SOURCES_WebCoreSupport
    morphos/WebCoreSupport/WebVisitedLinkStore.cpp
    morphos/WebCoreSupport/WebEditorClient.cpp
#    morphos/WebCoreSupport/WebChromeClient.cpp
#    morphos/WebCoreSupport/WebFrameLoaderClient.cpp
#    morphos/WebCoreSupport/WebFrameNetworkingContext.cpp
)

list(APPEND WebKitLegacy_SOURCES ${WebKitLegacy_INCLUDES} ${WebKitLegacy_SOURCES_Classes} ${WebKitLegacy_SOURCES_WebCoreSupport})

set(WebKitLegacy_OUTPUT_NAME
    WebKit${DEBUG_SUFFIX}
)
