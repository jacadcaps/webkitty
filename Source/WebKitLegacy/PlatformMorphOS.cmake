add_definitions(-DUSE_CAIRO=1 -DUSE_CURL=1 -DWEBKIT_EXPORTS=1)
list(APPEND WebKitLegacy_PRIVATE_INCLUDE_DIRECTORIES
    ${CAIRO_INCLUDE_DIRS}
    "${WEBKIT_LIBRARIES_DIR}/include"
)

list(APPEND WebKitLegacy_LIBRARIES PRIVATE WTF${DEBUG_SUFFIX})

list(APPEND WebKitLegacy_PRIVATE_INCLUDE_DIRECTORIES
    "${CMAKE_BINARY_DIR}/../include/private"
    "${CMAKE_BINARY_DIR}/../include/private/JavaScriptCore"
    "${CMAKE_BINARY_DIR}/../include/private/WebCore"
#    "${WEBKITLEGACY_DIR}/win"
#    "${WEBKITLEGACY_DIR}/win/plugins"
#    "${WEBKITLEGACY_DIR}/win/WebCoreSupport"
    "${WebKitLegacy_DERIVED_SOURCES_DIR}/include"
    "${WebKitLegacy_DERIVED_SOURCES_DIR}/Interfaces"
)

list(APPEND WebKitLegacy_SOURCES_Classes
    morphos/WebFrame.mm
)

list(APPEND WebKitLegacy_SOURCES ${WebKitLegacy_INCLUDES} ${WebKitLegacy_SOURCES_Classes} ${WebKitLegacy_SOURCES_WebCoreSupport})

set(WebKitLegacy_OUTPUT_NAME
    WebKit${DEBUG_SUFFIX}
)

