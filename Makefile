ROOTPATH:=$(abspath ../../../)
LIB:=$(ROOTPATH)/lib
GEN:=$(ROOTPATH)/gen/host/libnix

PKG_ICU:=$(LIB)/libicu/instdir/lib/pkgconfig/
PKG_SQLITE:=$(LIB)/sqlite/instdir/lib/pkgconfig/
PKG:=$(PKG_ICU)

all:

configure: morphos.cmake CMakeLists.txt
	rm -rf cross-build
	mkdir cross-build
	(cd cross-build && PKG_CONFIG_PATH=$(PKG) PATH=~/cmake-3.16.2-Linux-x86_64/bin/:${PATH} \
		cmake -DCMAKE_CROSSCOMPILING=ON -DCMAKE_TOOLCHAIN_FILE=../morphos.cmake -DCMAKE_BUILD_TYPE=Release \
		-DBUILD_SHARED_LIBS=NO -DPORT=MorphOS -DUSE_SYSTEM_MALLOC=YES \
		-DJPEG_LIBRARY=$(LIB)/libjpeg -DJPEG_INCLUDE_DIR=$(LIB)/libjpeg \
		-DLIBXML2_LIBRARY=$(LIB)/libxml2/instdir/lib -DLIBXML2_INCLUDE_DIR=$(LIB)/libxml2/instdir/include \
		-DPNG_LIBRARY=$(GEN)/libpng16/lib/ -DPNG_INCLUDE_DIR=$(GEN)/libpng16/include \
		-DLIBXSLT_LIBRARIES=$(LIB)/libxslt/instdir/lib -DLIBXSLT_INCLUDE_DIR=$(LIB)/libxslt/instdir/include \
		-DSQLITE_LIBRARIES=$(LIB)/sqlite/instdir/lib -DSQLITE_INCLUDE_DIR=$(LIB)/sqlite/instdir/include \
		-DCMAKE_MODULE_PATH=$(realpath Source/cmake) $(realpath ./))

build:
	(cd cross-build && make)

#		-Wdev --debug-output --trace --trace-expand \

morphos.cmake: morphos.cmake.in
	gcc -xc -E -P -C -o$@ -nostdinc morphos.cmake.in -D_IN_ROOTPATH=/home/jaca/morphos

clean:
	rm -rf morphos.cmake cross-build

install:

install-iso:

source:

sdk:

