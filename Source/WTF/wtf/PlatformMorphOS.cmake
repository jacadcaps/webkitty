list(APPEND WTF_SOURCES
    generic/WorkQueueGeneric.cpp
)

    list(APPEND WTF_SOURCES
        generic/MainThreadGeneric.cpp

        posix/FileSystemPOSIX.cpp
        posix/OSAllocatorPOSIX.cpp
        posix/ThreadingPOSIX.cpp

        text/unix/TextBreakIteratorInternalICUUnix.cpp

        unix/CPUTimeUnix.cpp
        unix/UniStdExtrasUnix.cpp

        morphos/LanguageMorphOS.cpp
        morphos/Misc.cpp

        morphos/MemoryPressureHandlerMorphOS.cpp
    )

    list(APPEND WTF_SOURCES
        generic/MemoryFootprintGeneric.cpp
    )

    list(APPEND WTF_SOURCES
        generic/RunLoopGeneric.cpp
    )

list(APPEND WTF_LIBRARIES
    ${CMAKE_THREAD_LIBS_INIT}
)
