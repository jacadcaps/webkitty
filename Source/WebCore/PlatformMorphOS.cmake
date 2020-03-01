include(platform/Cairo.cmake)
include(platform/Curl.cmake)
include(platform/FreeType.cmake)
include(platform/ImageDecoders.cmake)

list(APPEND WebCore_PRIVATE_INCLUDE_DIRECTORIES
    "${WEBKIT_LIBRARIES_DIR}/include"
)

list(APPEND WebCore_PRIVATE_INCLUDE_DIRECTORIES
    ${WEBCORE_DIR}/platform
    ${WEBCORE_DIR}/platform/generic
    ${WEBCORE_DIR}/platform/graphics/egl
    ${WEBCORE_DIR}/platform/graphics/opengl
    ${WEBCORE_DIR}/platform/graphics/libwpe
    ${WEBCORE_DIR}/platform/mediacapabilities
)

list(APPEND WebCore_SYSTEM_INCLUDE_DIRECTORIES
    ${ICU_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIR}
    ${SQLITE_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${WPE_INCLUDE_DIRS}
)

list(APPEND WebCore_LIBRARIES
    ${ICU_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${SQLITE_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${WPE_LIBRARIES}
)

