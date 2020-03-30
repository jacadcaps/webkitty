#!/bin/sh
/gg/bin/ppc-morphos-g++-9 -Wl,--relax -g1 -feliminate-unused-debug-symbols -noixemul -mlongcall -Wno-ignored-attributes -O3 -DNDEBUG -o MiniBrowser.db -Wl,--whole-archive \
../../Source/WebKitLegacy/libWebKit.a \
../../Source/WebCore/libWebCore.a \
../../Source/WebCore/PAL/pal/libPAL.a \
../../Source/JavaScriptCore/libJavaScriptCore.a \
../../Source/WTF/wtf/libWTF.a \
-Wl,--no-whole-archive \
/home/jaca/morphos/lib/libxml2/instdir/lib/libxml2.a \
/home/jaca/morphos/lib/libxslt/instdir/lib/libxslt.a \
/home/jaca/morphos/lib/sqlite/instdir/lib/libsqlite3.a \
/home/jaca/morphos/gen/host/libnix/lib/libz.a \
-Wl,--whole-archive \
/home/jaca/morphos/morphoswb/libs/cairo/MorphOS/lib/libnix/libcairo.a \
-Wl,--no-whole-archive \
/home/jaca/morphos/gen/host/libnix/lib/libcurl.a \
/home/jaca/morphos/gen/host/libnix/lib/libssl.a \
/home/jaca/morphos/morphoswb/libs/freetype/library/lib/libfreetype.a \
/home/jaca/morphos/gen/host/libnix/lib/libnghttp2.a \
/home/jaca/morphos/lib/libjpeg/libjpeg.a \
/home/jaca/morphos/gen/host/libnix/libpng16/lib/libpng.a  \
/home/jaca/morphos/gen/host/libnix/lib/libhyphen.a \
/home/jaca/morphos/gen/host/libnix/lib/libcrypto.a \
/home/jaca/morphos/lib/libicu/instdir/lib/libicui18n.a \
/home/jaca/morphos/lib/libicu/instdir/lib/libicuuc.a \
/home/jaca/morphos/lib/libicu/instdir/lib/libicudata.a \
-lsyscall -lpthread \
CMakeFiles/MiniBrowser.db.dir/minibrowser.mm.obj \
-lob.framework -lmui.framework \
-lobjc -lc -lgcc

