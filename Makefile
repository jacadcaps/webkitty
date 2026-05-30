ROOTPATH:=$(abspath ../../../)
LIB:=$(ROOTPATH)/lib
GEN:=$(ROOTPATH)/gen/host/libnix

PKG_ICU:=$(LIB)/libicu70/instdir/lib/pkgconfig/
PKG_SQLITE:=$(LIB)/sqlite/instdir/lib/pkgconfig/
PKG_FONTCONFIG:=$(ROOTPATH)/morphoswb/libs/fontconfig/MorphOS/
PKG:=$(PKG_ICU):$(PKG_SQLITE)

DEBIAN_PKG:=libicu-dev ruby-dev clang-7
NATIVE_GCC:=/home/jaca/gcc7/inst/bin/x86_64-pc-linux-gnu-

OBJC:=$(ROOTPATH)/morphoswb/classes/frameworks/includes/

all:

configure: morphos.cmake link.sh CMakeLists.txt Dummy/libdummy.a ffmpeg/.buildstamp
	rm -rf cross-build
	mkdir cross-build
	(cd cross-build && PKG_CONFIG_PATH=$(PKG) \
		cmake -DCMAKE_CROSSCOMPILING=ON -DCMAKE_BUILD_TYPE=RelWithDebugInfo -DCMAKE_TOOLCHAIN_FILE=$(realpath morphos.cmake) -DCMAKE_DL_LIBS="syscall" \
		-DBUILD_SHARED_LIBS=NO -DPORT=MorphOS -DENABLE_WEBCORE=1 -DENABLE_WEBKIT_LEGACY=1 -DLOG_DISABLED=0 -DMORPHOS_MINIMAL=0 -DROOTPATH="$(ROOTPATH)" \
		-DJPEG_LIBRARY=$(LIB)/libjpeg/libjpeg.a \
		-DJPEG_INCLUDE_DIR=$(LIB)/libjpeg \
		-DLIBXML2_LIBRARY=$(LIB)/libxml2/instdir/lib/libxml2.a \
		-DLIBXML2_INCLUDE_DIR="$(LIB)/libxml2/instdir/include/libxml2/" \
		-DPNG_LIBRARIES=$(GEN)/libpng16/lib/libpng16.a \
		-DPNG_PNG_INCLUDE_DIR=$(GEN)/libpng16/include/libpng16/ \
		-DPNG_INCLUDE_DIRS=$(GEN)/libpng16/include/libpng16/ \
		-DLIBXSLT_LIBRARIES=$(LIB)/libxslt/instdir/lib/libxslt.a \
		-DLIBXSLT_INCLUDE_DIR=$(LIB)/libxslt/instdir/include \
		-DSQLITE_LIBRARIES=$(LIB)/sqlite/instdir/lib/libsqlite3.a \
		-DSQLITE_INCLUDE_DIR=$(LIB)/sqlite/instdir/include \
		-DSQLite3_LIBRARY=$(LIB)/sqlite/instdir/include \
		-DSQLite3_INCLUDE_DIR=$(LIB)/sqlite/instdir/include \
		-DCAIRO_INCLUDE_DIRS=$(ROOTPATH)/morphoswb/libs/cairo/MorphOS/os-include/cairo \
		-DCAIRO_LIBRARIES="$(ROOTPATH)/morphoswb/libs/cairo/MorphOS/lib/libnix/libcairo.a" \
		-DCairo_INCLUDE_DIR=$(ROOTPATH)/morphoswb/libs/cairo/MorphOS/os-include/cairo \
		-DCairo_LIBRARY="$(ROOTPATH)/morphoswb/libs/cairo/MorphOS/lib/libnix/libcairo.a" \
		-DHarfBuzz_INCLUDE_DIR="$(realpath Dummy)"\
		-DHarfBuzz_LIBRARY=$(GEN)/lib/libnghttp2.a \
		-DICU_ROOT="$(LIB)/libicu70/instdir/" \
		-DICU_UC_LIBRARY_RELEASE="$(LIB)/libicu70/instdir/lib/libicuuc.a" \
		-DICU_DATA_LIBRARY_RELEASE="$(LIB)/libicu70/instdir/lib/libicudata.a" \
		-DICU_I18N_LIBRARY_RELEASE="$(LIB)/libicu70/instdir/lib/libicui18n.a" \
		-DHarfBuzz_ICU_LIBRARY="$(realpath Dummy)/libdummy.a" \
		-DFREETYPE_INCLUDE_DIRS="$(ROOTPATH)/morphoswb/libs/freetype/include" \
		-DFREETYPE_LIBRARY="$(ROOTPATH)/morphoswb/libs/freetype/library/lib/libfreetype.a" \
		-DFontconfig_LIBRARY="$(ROOTPATH)/morphoswb/libs/fontconfig/MorphOS/libfontconfig-glue.a" \
		-DFontconfig_INCLUDE_DIR="$(ROOTPATH)/morphoswb/libs/fontconfig/src" \
		-DOpenJPEG_INCLUDE_DIR="$(GEN)/include/openjpeg-2.5" \
		-DWebP_INCLUDE_DIR="$(GEN)/include" -DWebP_LIBRARY="$(GEN)/lib/libwebp.a" -DWebP_DEMUX_LIBRARY="$(GEN)/lib/libwebpdemux.a"\
		-DAVFORMAT_LIBRARY="ffmpeg/instdir/lib/libavformat.a" -DAVFORMAT_INCLUDE_DIR="$(realpath ffmpeg/instdir/include)" \
		-DAVCODEC_LIBRARY="ffmpeg/instdir/lib/libavcodec.a" -DAVCODEC_INCLUDE_DIR="$(realpath ffmpeg/instdir/include)" \
		-DAVUTIL_LIBRARY="ffmpeg/instdir/lib/libavutil.a" -DAVUTIL_INCLUDE_DIR="$(realpath ffmpeg/instdir/include)" \
		-DSWSCALE_LIBRARY="ffmpeg/instdir/lib/libswscale.a" -DSWSCALE_INCLUDE_DIR="$(realpath ffmpeg/instdir/include)" \
		-DWOFF2_LIBRARY="$(GEN)/lib/libwoff2dec.a $(GEN)/lib/libwoff2common.a" -DWOFF2_INCLUDE_DIR="$(GEN)/include/" \
		-DOBJC_INCLUDE="$(OBJC)" \
		-DCMAKE_MODULE_PATH=$(realpath Source/cmake) $(realpath ./))

configure-mini: morphos.cmake link.sh CMakeLists.txt Dummy/libdummy.a ffmpeg/.buildstamp
	rm -rf cross-build-mini
	mkdir cross-build-mini
	(cd cross-build-mini && PKG_CONFIG_PATH=$(PKG) \
		cmake -DCMAKE_CROSSCOMPILING=ON -DCMAKE_BUILD_TYPE=RelWithDebugInfo -DCMAKE_TOOLCHAIN_FILE=$(realpath morphos.cmake) -DCMAKE_DL_LIBS="syscall" \
		-DBUILD_SHARED_LIBS=NO -DPORT=MorphOS -DENABLE_WEBCORE=1 -DENABLE_WEBKIT_LEGACY=1 -DLOG_DISABLED=0 -DMORPHOS_MINIMAL=1 -DROOTPATH="$(ROOTPATH)" \
		-DJPEG_LIBRARY=$(LIB)/libjpeg/libjpeg.a \
		-DJPEG_INCLUDE_DIR=$(LIB)/libjpeg \
		-DLIBXML2_LIBRARY=$(LIB)/libxml2/instdir/lib/libxml2.a \
		-DLIBXML2_INCLUDE_DIR="$(LIB)/libxml2/instdir/include/libxml2/" \
		-DPNG_LIBRARIES=$(GEN)/libpng16/lib/libpng16.a \
		-DPNG_PNG_INCLUDE_DIR=$(GEN)/libpng16/include/libpng16/ \
		-DPNG_INCLUDE_DIRS=$(GEN)/libpng16/include/libpng16/ \
		-DLIBXSLT_LIBRARIES=$(LIB)/libxslt/instdir/lib/libxslt.a \
		-DLIBXSLT_INCLUDE_DIR=$(LIB)/libxslt/instdir/include \
		-DSQLITE_LIBRARIES=$(LIB)/sqlite/instdir/lib/libsqlite3.a \
		-DSQLITE_INCLUDE_DIR=$(LIB)/sqlite/instdir/include \
		-DSQLite3_LIBRARY=$(LIB)/sqlite/instdir/include \
		-DSQLite3_INCLUDE_DIR=$(LIB)/sqlite/instdir/include \
		-DCAIRO_INCLUDE_DIRS=$(ROOTPATH)/morphoswb/libs/cairo/MorphOS/os-include/cairo \
		-DCAIRO_LIBRARIES="$(ROOTPATH)/morphoswb/libs/cairo/MorphOS/lib/libnix/libcairo.a" \
		-DCairo_INCLUDE_DIR=$(ROOTPATH)/morphoswb/libs/cairo/MorphOS/os-include/cairo \
		-DCairo_LIBRARY="$(ROOTPATH)/morphoswb/libs/cairo/MorphOS/lib/libnix/libcairo.a" \
		-DHarfBuzz_INCLUDE_DIR="$(realpath Dummy)"\
		-DHarfBuzz_LIBRARY=$(GEN)/lib/libnghttp2.a \
		-DICU_ROOT="$(LIB)/libicu70/instdir/" \
		-DICU_UC_LIBRARY_RELEASE="$(LIB)/libicu70/instdir/lib/libicuuc.a" \
		-DICU_DATA_LIBRARY_RELEASE="$(LIB)/libicu70/instdir/lib/libicudata.a" \
		-DICU_I18N_LIBRARY_RELEASE="$(LIB)/libicu70/instdir/lib/libicui18n.a" \
		-DHarfBuzz_ICU_LIBRARY="$(realpath Dummy)/libdummy.a" \
		-DFREETYPE_INCLUDE_DIRS="$(ROOTPATH)/morphoswb/libs/freetype/include" \
		-DFREETYPE_LIBRARY="$(ROOTPATH)/morphoswb/libs/freetype/library/lib/libfreetype.a" \
		-DFontconfig_LIBRARY="$(ROOTPATH)/morphoswb/libs/fontconfig/MorphOS/libfontconfig-glue.a" \
		-DFontconfig_INCLUDE_DIR="$(ROOTPATH)/morphoswb/libs/fontconfig/src" \
		-DOpenJPEG_INCLUDE_DIR="$(GEN)/include/openjpeg-2.5" \
		-DWebP_INCLUDE_DIR="$(GEN)/include" -DWebP_LIBRARY="$(GEN)/lib/libwebp.a" -DWebP_DEMUX_LIBRARY="$(GEN)/lib/libwebpdemux.a"\
		-DAVFORMAT_LIBRARY="ffmpeg/instdir/lib/libavformat.a" -DAVFORMAT_INCLUDE_DIR="$(realpath ffmpeg/instdir/include)" \
		-DAVCODEC_LIBRARY="ffmpeg/instdir/lib/libavcodec.a" -DAVCODEC_INCLUDE_DIR="$(realpath ffmpeg/instdir/include)" \
		-DAVUTIL_LIBRARY="ffmpeg/instdir/lib/libavutil.a" -DAVUTIL_INCLUDE_DIR="$(realpath ffmpeg/instdir/include)" \
		-DSWSCALE_LIBRARY="ffmpeg/instdir/lib/libswscale.a" -DSWSCALE_INCLUDE_DIR="$(realpath ffmpeg/instdir/include)" \
		-DOBJC_INCLUDE="$(OBJC)" \
		-DCMAKE_MODULE_PATH=$(realpath Source/cmake) $(realpath ./))

build:
	(cd cross-build && make -j$(shell nproc))
	echo "Link done"
	ppc-morphos-strip cross-build/Tools/morphos/MiniBrowser.db -o cross-build/Tools/morphos/MiniBrowser
	echo "Stripped binary in cross-build/Tools/morphos/MiniBrowser"

#		-Wdev --debug-output --trace --trace-expand \

build-mini:
	(cd cross-build-mini && make -j$(shell nproc))

cross-build:
	make configure

.build: cross-build build

cross-build-mini:
	make configure-mini

.build-mini: cross-build-mini build-mini

morphos.cmake: morphos.cmake.in
	gcc -xc -E -P -C -o$@ -nostdinc $@.in -D_IN_ROOTPATH=$(ROOTPATH) -D_IN_DUMMYPATH=$(realpath Dummy)

link.sh: link.sh.in
	gcc -xc -E -P -C -o$@ -nostdinc $@.in -D_IN_ROOTPATH=$(ROOTPATH)
	chmod u+x $@

libwebkit.a:
	(cd cross-build/Source/WebKitLegacy && make)

clean:
	rm -rf morphos.cmake cross-build cross-build-mini WebKitBuild build link.sh

install:

install-iso:

source:

sdk:

Dummy/libdummy.a:
	-mkdir Dummy
	echo "//nothing" >Dummy/dummy.c
	ppc-morphos-gcc-9 -c -o Dummy/dummy.o Dummy/dummy.c
	ppc-morphos-ar rc Dummy/libdummy.a Dummy/dummy.o
	ppc-morphos-ranlib Dummy/libdummy.a
	cp Dummy/libdummy.a Dummy/libdl.a

ffmpeg/.buildstamp:
	cd ffmpeg && make

miniscp:
	scp cross-build/Tools/morphos/MiniBrowser jaca@192.168.2.5:/Users/jaca

minidump:
	@read -p "Address:" address; \
	ppc-morphos-objdump --demangle --disassemble -l --source cross-build/Tools/morphos/MiniBrowser.db --start-address $$address | less

minirelease:
	rm -rf WebKitty webkitty.tar webkitty.tar.xz webkitty.lha
	mkdir -p WebKitty/MOSSYS/Data/ICU
	cp cross-build/Tools/morphos/MiniBrowser WebKitty/
	cp -a Source/WebCore/Resources WebKitty/Resources
	cp -a $(ROOTPATH)/lib/libicu/instdir/icu/54.2/icudt54b WebKitty/MOSSYS/Data/ICU/icudt54b
#	( cd WebKitty/Resources && wget https://easylist.to/easylist/easylist.txt )
	cp easylist/easylist.dat WebKitty/Resources
	mkdir WebKitty/MiniResources
	cp Tools/morphos/MiniResources/*.png WebKitty/MiniResources
	cp Tools/morphos/MiniResources/MiniBrowser.info WebKitty/
	cp MUSTREAD.txt WebKitty/
	lha ao5 webkitty.lha WebKitty
	rm -rf WebKitty

putrelease: minirelease
	scp webkitty.lha jaca@tunkki.dk:/home/jaca/public_html

LINKFILES := \
	cross-build-mini/lib/libWebKit.a \
	cross-build-mini/lib/libWebCore.a \
	cross-build-mini/lib/libPAL.a \
	cross-build-mini/lib/libJavaScriptCore.a \
	cross-build-mini/lib/libWTF.a \
	$(ROOTPATH)/lib/libxml2/instdir/lib/libxml2.a \
	$(ROOTPATH)/lib/libxslt/instdir/lib/libxslt.a \
	$(ROOTPATH)/lib/sqlite/instdir/lib/libsqlite3.a \
	$(ROOTPATH)/gen/host/libnix/lib/libz.a \
	$(ROOTPATH)/morphoswb/libs/cairo/MorphOS/lib/libnix/libcairo.a \
	$(ROOTPATH)/gen/host/libnix/lib/libcurl.a \
	$(ROOTPATH)/gen/host/libnix/lib/libssl.a \
	$(ROOTPATH)/morphoswb/libs/freetype/library/lib/libfreetype.a \
	$(ROOTPATH)/gen/host/libnix/lib/libnghttp2.a \
	$(ROOTPATH)/lib/libjpeg/libjpeg.a \
	$(ROOTPATH)/gen/host/libnix/lib/liblcms2.a \
	$(ROOTPATH)/gen/host/libnix/lib/libpsl.a \
	$(ROOTPATH)/gen/host/libnix/libpng16/lib/libpng16.a  \
	$(ROOTPATH)/gen/host/libnix/lib/libhyphen.a \
	$(ROOTPATH)/gen/host/libnix/lib/libcrypto.a \
	$(ROOTPATH)/lib/libicu70/instdir/lib/libicui18n.a \
	$(ROOTPATH)/lib/libicu70/instdir/lib/libicuuc.a \
	$(ROOTPATH)/lib/libicu70/instdir/lib/libicudata.a \
	$(ROOTPATH)/lib/libwebp/objects/host-libnix/tmpinstalldir/lib/libwebp.a \
	$(ROOTPATH)/lib/libwebp/objects/host-libnix/tmpinstalldir/lib/libwebpdemux.a \
	$(ROOTPATH)/gen/host/libnix/lib/libopenjp2.a \
	$(ROOTPATH)/gen/host/libnix/lib/libtasn1.a \
	$(ROOTPATH)/gen/host/libnix/lib/libgcrypt.a \
	$(ROOTPATH)/gen/host/libnix/lib/libgpg-error.a \
	$(ROOTPATH)/gen/host/libnix/lib/libnghttp3.a \
	$(ROOTPATH)/gen/host/libnix/lib/libngtcp2.a \
	$(ROOTPATH)/gen/host/libnix/lib/libngtcp2_crypto_ossl.a

.PHONY: linkpackage
linkpackage:
	@rm -rf linkpackage
	@mkdir linkpackage
	@for i in $(LINKFILES); \
	do echo -n "ppc-morphos-strip --strip-debug $$i -o linkpackage/">.run.sh; \
	echo $$i | rev | cut -d'/' -f-1 | rev >>.run.sh ; \
	echo "Copying and stripping $$i"; \
	bash ./.run.sh; \
	done
	@rm ./.run.sh

sdk: linkpackage
	rm -rf /tmp/webkittysdk
	mkdir -p /tmp/webkittysdk/linklibs /tmp/webkittysdk/includes/webkitty /tmp/webkittysdk/Resources
	cp linkpackage/* /tmp/webkittysdk/linklibs
	cp Source/WebCore/Resources/* /tmp/webkittysdk/Resources
	cp Source/WebKitLegacy/morphos/Wk*.h /tmp/webkittysdk/includes/webkitty
	rm -f /tmp/webkittysdk/includes/webkitty/*_private.h
