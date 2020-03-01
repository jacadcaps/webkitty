ROOTPATH:=$(abspath ../../../)
LIB:=$(ROOTPATH)/lib
GEN:=$(ROOTPATH)/gen/host/libnix

PKG_ICU:=$(LIB)/libicu/instdir/lib/pkgconfig/
PKG_SQLITE:=$(LIB)/sqlite/instdir/lib/pkgconfig/
PKG:=$(PKG_ICU)

DEBIAN_PKG:=libicu-dev ruby-dev clang-7
NATIVE_GCC:=/home/jaca/gcc7/inst/bin/x86_64-pc-linux-gnu-

all:

configure-native:
	rm -rf build
	mkdir build
	(cd build && PATH=~/cmake-3.10.3-Linux-x86_64/bin/:${PATH} \
		cmake -DCMAKE_MODULE_PATH=$(realpath Source/cmake) \
		-DCMAKE_BUILD_TYPE=Release -DPORT=JSCOnly -DUSE_SYSTEM_MALLOC=YES -DCMAKE_CXX_FLAGS="-O2 -fPIC" -DCMAKE_C_FLAGS="-O2 -fPIC" \
		-DCMAKE_C_COMPILER=$(NATIVE_GCC)gcc -DCMAKE_CXX_COMPILER=$(NATIVE_GCC)g++ \
		$(realpath ./))

jscore-native:
	rm -rf WebKitBuild build
	mkdir build
	(cd build && PATH=~/cmake-3.16.2-Linux-x86_64/bin/:${PATH} \
		$(realpath Tools/Scripts/run-javascriptcore-tests) --jsc-only --no-flt-jit \
		--cmakeargs='-DCMAKE_MODULE_PATH=$(realpath Source/cmake) -DJAVASCRIPTCORE_DIR=$(realpath Source/JavaScriptCore) \
                -DCMAKE_BUILD_TYPE=Release -DPORT=JSCOnly -DUSE_SYSTEM_MALLOC=YES -DCMAKE_CXX_FLAGS="-O2 -fPIC" -DCMAKE_C_FLAGS="-O2 -fPIC" \
                -DCMAKE_C_COMPILER=$(NATIVE_GCC)gcc -DCMAKE_CXX_COMPILER=$(NATIVE_GCC)g++')
	cp -a Source/JavaScriptCore/API/tests/testapiScripts ~/morphos/morphoswb/apps/webkitty/WebKitBuild/Release/Source/JavaScriptCore/shell/
	Tools/Scripts/run-javascriptcore-tests --root WebKitBuild/Release/Source/JavaScriptCore/shell/ --no-jsc-stress --no-jit-stress-test

jscore-morphos: morphos.cmake
	rm -rf WebKitBuild cross-build
	mkdir -p cross-build WebKitBuild/Release/bin
	(cd cross-build && PKG_CONFIG_PATH=$(PKG) PATH=~/cmake-3.16.2-Linux-x86_64/bin/:${PATH} \
		$(realpath Tools/Scripts/run-javascriptcore-tests) --jsc-only --no-flt-jit \
		--cmakeargs='-DCMAKE_CROSSCOMPILING=ON -DCMAKE_TOOLCHAIN_FILE=$(realpath morphos.cmake) -DCMAKE_MODULE_PATH=$(realpath Source/cmake) \
		-DJAVASCRIPTCORE_DIR=$(realpath Source/JavaScriptCore) -DBUILD_SHARED_LIBS=NO \
		-DJPEG_LIBRARY=$(LIB)/libjpeg -DJPEG_INCLUDE_DIR=$(LIB)/libjpeg \
		-DLIBXML2_LIBRARY=$(LIB)/libxml2/instdir/lib -DLIBXML2_INCLUDE_DIR=$(LIB)/libxml2/instdir/include/libxml2 \
		-DPNG_LIBRARY=$(GEN)/libpng16/lib/ -DPNG_INCLUDE_DIR=$(GEN)/libpng16/include \
		-DLIBXSLT_LIBRARIES=$(LIB)/libxslt/instdir/lib -DLIBXSLT_INCLUDE_DIR=$(LIB)/libxslt/instdir/include \
		-DSQLITE_LIBRARIES=$(LIB)/sqlite/instdir/lib -DSQLITE_INCLUDE_DIR=$(LIB)/sqlite/instdir/include \
                -DCMAKE_BUILD_TYPE=Release -DPORT=JSCOnly -DUSE_SYSTEM_MALLOC=YES \
		-DCMAKE_FIND_LIBRARY_SUFFIXES=".a" ')
	cp -a Source/JavaScriptCore/API/tests/testapiScripts ./WebKitBuild/Release/Source/JavaScriptCore/shell/
#	Tools/Scripts/run-javascriptcore-tests --root WebKitBuild/Release/Source/JavaScriptCore/shell/ --no-jsc-stress --no-jit-stress-test

jscore-pack:
	ppc-morphos-strip ./WebKitBuild/Release/Source/JavaScriptCore/shell/jsc
	ppc-morphos-strip ./WebKitBuild/Release/Source/JavaScriptCore/shell/testRegExp
	ppc-morphos-strip ./WebKitBuild/Release/Source/JavaScriptCore/shell/testair
	ppc-morphos-strip ./WebKitBuild/Release/Source/JavaScriptCore/shell/testapi
	ppc-morphos-strip ./WebKitBuild/Release/Source/JavaScriptCore/shell/testb3
	ppc-morphos-strip ./WebKitBuild/Release/Source/JavaScriptCore/shell/testdfg
	ppc-morphos-strip ./WebKitBuild/Release/Source/JavaScriptCore/shell/testmasm
	tar cJf testsuite.tar.xz Tools ./WebKitBuild/Release/Source/JavaScriptCore/shell/jsc \
		./WebKitBuild/Release/Source/JavaScriptCore/shell/testRegExp \
		./WebKitBuild/Release/Source/JavaScriptCore/shell/testair \
		./WebKitBuild/Release/Source/JavaScriptCore/shell/testapi \
		./WebKitBuild/Release/Source/JavaScriptCore/shell/testb3 \
		./WebKitBuild/Release/Source/JavaScriptCore/shell/testdfg \
		./WebKitBuild/Release/Source/JavaScriptCore/shell/testmasm \
		 JSTests LayoutTests PerformanceTests

configure: morphos.cmake CMakeLists.txt
	rm -rf cross-build
	mkdir cross-build
	(cd cross-build && PKG_CONFIG_PATH=$(PKG) PATH=~/cmake-3.16.2-Linux-x86_64/bin/:${PATH} \
		cmake -DCMAKE_CROSSCOMPILING=ON -DCMAKE_TOOLCHAIN_FILE=../morphos.cmake -DCMAKE_BUILD_TYPE=Release \
		-DBUILD_SHARED_LIBS=NO -DPORT=MorphOS -DUSE_SYSTEM_MALLOC=YES -DENABLE_WEBCORE=1 -DENABLE_WEBKIT_LEGACY=1 \
		-DJPEG_LIBRARY=$(LIB)/libjpeg -DJPEG_INCLUDE_DIR=$(LIB)/libjpeg \
		-DLIBXML2_LIBRARY=$(LIB)/libxml2/instdir/lib -DLIBXML2_INCLUDE_DIR=$(LIB)/libxml2/instdir/include/libxml2/ \
		-DPNG_LIBRARY=$(GEN)/libpng16/lib/ -DPNG_INCLUDE_DIR=$(GEN)/libpng16/include \
		-DLIBXSLT_LIBRARIES=$(LIB)/libxslt/instdir/lib -DLIBXSLT_INCLUDE_DIR=$(LIB)/libxslt/instdir/include \
		-DSQLITE_LIBRARIES=$(LIB)/sqlite/instdir/lib -DSQLITE_INCLUDE_DIR=$(LIB)/sqlite/instdir/include \
		-DCAIRO_INCLUDE_DIRS=$(ROOTPATH)/morphoswb/libs/cairo/MorphOS/os-include/cairo -DCAIRO_LIBRARIES=$(ROOTPATH)/morphoswb/libs/cairo/MorphOS/lib/libnix \
		-DHARFBUZZ_INCLUDE_DIRS=$(ROOTPATH)/morphoswb/libs/harfbuzz/src -DHARFBUZZ_LIBRARIES=$(ROOTPATH)/morphoswb/libs/harfbuzz/src -DHARFBUZZ_ICU_LIBRARIES=$(ROOTPATH)/morphoswb/libs/harfbuzz/src \
		-DFREETYPE_INCLUDE_DIRS=$(ROOTPATH)/morphoswb/libs/freetype/include -DFREETYPE_LIBRARY=$(ROOTPATH)/morphoswb/libs/freetype/library/lib \
		-DCMAKE_MODULE_PATH=$(realpath Source/cmake) $(realpath ./))

build:
	(cd cross-build && make)

#		-Wdev --debug-output --trace --trace-expand \

morphos.cmake: morphos.cmake.in
	gcc -xc -E -P -C -o$@ -nostdinc morphos.cmake.in -D_IN_ROOTPATH=/home/jaca/morphos

clean:
	rm -rf morphos.cmake cross-build WebKitBuild build

install:

install-iso:

source:

sdk:

