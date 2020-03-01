list(APPEND WTF_SOURCES
    generic/WorkQueueGeneric.cpp
)

    list(APPEND WTF_SOURCES
        generic/MainThreadGeneric.cpp

        posix/FileSystemPOSIX.cpp
        posix/OSAllocatorPOSIX.cpp
        posix/ThreadingPOSIX.cpp

        text/unix/TextBreakIteratorInternalICUUnix.cpp

        unix/LanguageUnix.cpp
        unix/UniStdExtrasUnix.cpp
    )
    if (WTF_OS_FUCHSIA)
        list(APPEND WTF_SOURCES
            fuchsia/CPUTimeFuchsia.cpp
        )
    else ()
        list(APPEND WTF_SOURCES
            unix/CPUTimeUnix.cpp
        )
    endif ()

    list(APPEND WTF_SOURCES
        generic/MemoryFootprintGeneric.cpp
        generic/MemoryPressureHandlerGeneric.cpp
    )

    list(APPEND WTF_SOURCES
        generic/RunLoopGeneric.cpp
    )

list(APPEND WTF_LIBRARIES
    ${CMAKE_THREAD_LIBS_INIT}
)
