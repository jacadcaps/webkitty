# rules for finding the FFmpeg libraries

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_FFMPEG libavformat libavcodec libavutil libswscale)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h HINTS ${PC_FFMPEG_LIBAVFORMAT_INCLUDEDIR} ${PC_FFMPEG_INCLUDE_DIRS})
find_library(AVFORMAT_LIBRARY NAMES avformat HINTS ${PC_FFMPEG_LIBAVFORMAT_LIBDIR} ${PC_FFMPEG_LIBRARY_DIRS})

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h HINTS ${PC_FFMPEG_LIBAVCODEC_INCLUDEDIR} ${PC_FFMPEG_INCLUDE_DIRS})
find_library(AVCODEC_LIBRARY NAMES avcodec HINTS ${PC_FFMPEG_LIBAVCODEC_LIBDIR} ${PC_FFMPEG_LIBRARY_DIRS})

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h HINTS ${PC_FFMPEG_LIBAVUTIL_INCLUDEDIR} ${PC_FFMPEG_INCLUDE_DIRS})
find_library(AVUTIL_LIBRARY NAMES avutil HINTS ${PC_FFMPEG_LIBAVUTIL_LIBDIR} ${PC_FFMPEG_LIBRARY_DIRS})

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h HINTS ${PC_FFMPEG_LIBSWSCALE_INCLUDEDIR} ${PC_FFMPEG_INCLUDE_DIRS})
find_library(SWSCALE_LIBRARY NAMES swscale HINTS ${PC_FFMPEG_LIBSWSCALE_LIBDIR} ${PC_FFMPEG_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AVFormat DEFAULT_MSG AVFORMAT_LIBRARY AVFORMAT_INCLUDE_DIR)
find_package_handle_standard_args(AVCodec DEFAULT_MSG AVCODEC_LIBRARY AVCODEC_INCLUDE_DIR)
find_package_handle_standard_args(AVUtil DEFAULT_MSG AVUTIL_LIBRARY AVUTIL_INCLUDE_DIR)
find_package_handle_standard_args(SWScale DEFAULT_MSG SWSCALE_LIBRARY SWSCALE_INCLUDE_DIR)

mark_as_advanced(AVFORMAT_INCLUDE_DIR AVFORMAT_LIBRARY)
mark_as_advanced(AVCODEC_INCLUDE_DIR AVCODEC_LIBRARY)
mark_as_advanced(AVUTIL_INCLUDE_DIR AVUTIL_LIBRARY)
mark_as_advanced(SWSCALE_INCLUDE_DIR SWSCALE_LIBRARY)

set(FFMPEG_INCLUDE_DIRS ${AVFORMAT_INCLUDE_DIR} ${AVCODEC_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${SWSCALE_INCLUDE_DIR})
set(FFMPEG_LIBRARIES ${AVFORMAT_LIBRARY} ${AVCODEC_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY})

if(${AVFORMAT_FOUND} AND ${AVCODEC_FOUND} AND ${AVUTIL_FOUND} AND ${SWSCALE_FOUND})
	set(FFMPEG_FOUND TRUE)
else()
	set(FFMPEG_FOUND FALSE)
endif()

