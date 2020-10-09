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
    ${OBJC_INCLUDE}
)

list(APPEND WebKitLegacy_SOURCES_Classes
    morphos/WebFrame.cpp
    morphos/WebPage.cpp
    morphos/WebProcess.cpp
    morphos/BackForwardClient.cpp
    morphos/WebApplicationCache.cpp
    morphos/storage/WebDatabaseProvider.cpp
    morphos/WebDocumentLoader.cpp
    morphos/CacheModel.cpp
    morphos/WebDragClient.cpp
    morphos/PopupMenu.cpp
)

list(APPEND WebKitLegacy_SOURCES_Classes
    morphos/WkWebView.mm
    morphos/WkNetworkRequestMutable.mm
    morphos/WkHistory.mm
    morphos/WkSettings.mm
    morphos/WkCertificate.mm
    morphos/WkCertificateViewer.mm
    morphos/WkError.mm
    morphos/WkDownload.mm
    morphos/WkFileDialog.mm
    morphos/WkHitTest.mm
    morphos/WkFavIcon.mm
)

list(APPEND WebKitLegacy_SOURCES_WebCoreSupport
    morphos/WebCoreSupport/WebVisitedLinkStore.cpp
    morphos/WebCoreSupport/WebEditorClient.cpp
    morphos/WebCoreSupport/WebChromeClient.cpp
    morphos/WebCoreSupport/WebPluginInfoProvider.cpp
    morphos/WebCoreSupport/WebPlatformStrategies.cpp
    morphos/WebCoreSupport/WebInspectorClient.cpp
    morphos/WebCoreSupport/WebFrameLoaderClient.cpp
    morphos/WebCoreSupport/WebFrameNetworkingContext.cpp
    morphos/WebCoreSupport/WebContextMenuClient.cpp
    morphos/WebCoreSupport/WebPageGroup.cpp
    morphos/WebCoreSupport/WebProgressTrackerClient.cpp
)

list(APPEND WebKitLegacy_ABP
	morphos/ABPFilterParser/ABPFilterParser.cpp
	morphos/ABPFilterParser/BloomFilter.cpp
	morphos/ABPFilterParser/cosmeticFilter.cpp
	morphos/ABPFilterParser/filter.cpp
	morphos/ABPFilterParser/hashFn.cpp
)

list(APPEND WebKitLegacy_SOURCES ${WebKitLegacy_INCLUDES} ${WebKitLegacy_SOURCES_Classes} ${WebKitLegacy_SOURCES_WebCoreSupport} ${WebKitLegacy_ABP})

set_source_files_properties(morphos/WkWebView.mm PROPERTIES COMPILE_FLAGS "-Wno-protocol -Wundeclared-selector -fobjc-call-cxx-cdtors -fobjc-exceptions -fconstant-string-class=OBConstantString -DDEBUG=0")
set_source_files_properties(morphos/WkNetworkRequestMutable.mm PROPERTIES COMPILE_FLAGS "-Wno-protocol -Wundeclared-selector -fobjc-call-cxx-cdtors -fobjc-exceptions -fconstant-string-class=OBConstantString -DDEBUG=0")
set_source_files_properties(morphos/WkHistory.mm PROPERTIES COMPILE_FLAGS "-Wno-protocol -Wundeclared-selector -fobjc-call-cxx-cdtors -fobjc-exceptions -fconstant-string-class=OBConstantString -DDEBUG=0")
set_source_files_properties(morphos/WkSettings.mm PROPERTIES COMPILE_FLAGS "-Wno-protocol -Wundeclared-selector -fobjc-call-cxx-cdtors -fobjc-exceptions -fconstant-string-class=OBConstantString -DDEBUG=0")
set_source_files_properties(morphos/WkCertificate.mm PROPERTIES COMPILE_FLAGS "-Wno-protocol -Wundeclared-selector -fobjc-call-cxx-cdtors -fobjc-exceptions -fconstant-string-class=OBConstantString -DDEBUG=0")
set_source_files_properties(morphos/WkCertificateViewer.mm PROPERTIES COMPILE_FLAGS "-Wno-protocol -Wundeclared-selector -fobjc-call-cxx-cdtors -fobjc-exceptions -fconstant-string-class=OBConstantString -DDEBUG=0")
set_source_files_properties(morphos/WkError.mm PROPERTIES COMPILE_FLAGS "-Wno-protocol -Wundeclared-selector -fobjc-call-cxx-cdtors -fobjc-exceptions -fconstant-string-class=OBConstantString -DDEBUG=0")
set_source_files_properties(morphos/WkDownload.mm PROPERTIES COMPILE_FLAGS "-Wno-protocol -Wundeclared-selector -fobjc-call-cxx-cdtors -fobjc-exceptions -fconstant-string-class=OBConstantString -DDEBUG=0")
set_source_files_properties(morphos/WkFileDialog.mm PROPERTIES COMPILE_FLAGS "-Wno-protocol -Wundeclared-selector -fobjc-call-cxx-cdtors -fobjc-exceptions -fconstant-string-class=OBConstantString -DDEBUG=0")
set_source_files_properties(morphos/WkHitTest.mm PROPERTIES COMPILE_FLAGS "-Wno-protocol -Wundeclared-selector -fobjc-call-cxx-cdtors -fobjc-exceptions -fconstant-string-class=OBConstantString -DDEBUG=0")
set_source_files_properties(morphos/WkFavIcon.mm PROPERTIES COMPILE_FLAGS "-Wno-protocol -Wundeclared-selector -fobjc-call-cxx-cdtors -fobjc-exceptions -fconstant-string-class=OBConstantString -DDEBUG=0")

set(WebKitLegacy_OUTPUT_NAME
    WebKit${DEBUG_SUFFIX}
)
