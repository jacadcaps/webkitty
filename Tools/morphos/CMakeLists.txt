cmake_minimum_required(VERSION 3.10)
include(WebKitCommon)

set(MINIBROWSER_DIR "./morphos")

add_definitions("-DWEBCORE_EXPORT=")

set(MiniBrowser_SOURCES
    test.cpp
)

set(MiniBrowser_INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/Source
    ${FORWARDING_HEADERS_DIR}
    ${MINIBROWSER_DIR}
    ${WTF_FRAMEWORK_HEADERS_DIR}
    ${JavaScriptCore_FRAMEWORK_HEADERS_DIR}
    ${JavaScriptCore_PRIVATE_FRAMEWORK_HEADERS_DIR}
    ${PAL_FRAMEWORK_HEADERS_DIR}
    ${WebCore_PRIVATE_FRAMEWORK_HEADERS_DIR}
    ${WebKitLegacy_FRAMEWORK_HEADERS_DIR}
    ${WebKit_FRAMEWORK_HEADERS_DIR}
    ${WebKit_PRIVATE_FRAMEWORK_HEADERS_DIR}
)

set(MiniBrowser_LIBRARIES
    WebKitLegacy
)

set(EXECUTABLE_NAME MiniBrowser)
set(PRODUCT_NAME MiniBrowser)

include_directories(${MiniBrowser_INCLUDE_DIRECTORIES})
add_executable(MiniBrowser.db ${MiniBrowser_SOURCES})
target_link_libraries(MiniBrowser.db ${MiniBrowser_LIBRARIES})
add_custom_command(OUTPUT MiniBrowser
	COMMAND "ppc-morphos-strip -o $<TARGET_FILE:MiniBrowser> $<TARGET_FILE:MiniBrowser>.db"
	DEPENDS MiniBrowser.db
        )

