FFMPEG_VERSION=4.4
OPUS_VERSION=1.3.1

all: .buildstamp

.buildstamp: ffmpeg-$(FFMPEG_VERSION).tar.xz ffmpeg-$(FFMPEG_VERSION).diff opus-$(OPUS_VERSION).tar.gz
	mkdir -p instdir/include instdir/lib
	rm -rf ffmpeg-$(OPUS_VERSION)
	tar xf opus-$(OPUS_VERSION).tar.gz
	(cd opus-$(OPUS_VERSION) && CC="ppc-morphos-gcc-9 -noixemul" LD=ppc-morphos-ld ./configure --host=ppc-morphos --prefix="")
	(cd opus-$(OPUS_VERSION) && CC="ppc-morphos-gcc-9 -noixemul" LD=ppc-morphos-ld make -j$(shell nproc) )
	(cd opus-$(OPUS_VERSION) && make DESTDIR="$(abspath ./)/instdir" install)
	rm -rf ffmpeg-$(FFMPEG_VERSION)
	(../../../../aboxtools/xz/xz -dcf ffmpeg-$(FFMPEG_VERSION).tar.xz | tar -x)
	(cd ffmpeg-$(FFMPEG_VERSION) && patch -p1 <../ffmpeg-$(FFMPEG_VERSION).diff)
	(cd ffmpeg-$(FFMPEG_VERSION) && CFLAGS="-noixemul -O3 -I$(abspath instdir/include) -I$(abspath aom-build/instdir/include)" LDFLAGS="-noixemul -L$(abspath instdir/lib) -L$(abspath aom-build/instdir/lib) " \
		PKG_CONFIG_PATH="$(abspath instdir/lib/pkgconfig):$(abspath aom-build/instdir/lib/pkgconfig)" ./configure --prefix="$(abspath ./instdir)" --cc=/gg/bin/ppc-morphos-gcc-9 --ld=/gg/bin/ppc-morphos-gcc-9 \
		--enable-cross-compile --cross-prefix=/gg/bin/ --arch=powerpc --cpu=powerpc --target-os=linux --enable-runtime-cpudetect \
		--enable-pthreads --disable-shared --disable-indevs --disable-protocols --enable-protocol=file --disable-network --disable-encoders --disable-decoders \
		 --disable-hwaccels --disable-muxers --disable-demuxers --disable-parsers --disable-bsfs --disable-devices --disable-filters --enable-decoder=aac \
		--enable-decoder=aac_latm --enable-decoder=h264 --enable-decoder=mp3 --enable-decoder=theora --enable-decoder=vorbis --enable-demuxer=aac \
		 --enable-parser=aac --enable-parser=aac_latm --enable-demuxer=mp3 --enable-demuxer=mov --enable-demuxer=ogg --enable-parser=mpegaudio \
		--enable-bsf=h264_mp4toannexb --enable-decoder=flac --enable-demuxer=flac --enable-parser=flac \
		--enable-demuxer=flv --enable-decoder=flv --enable-decoder=h263 --enable-decoder=vp8 --enable-parser=vp8 \
		--enable-decoder=vp9 --enable-parser=vp9 --enable-bsf=aac_adtstoasc --enable-decoder=mpeg4 --enable-parser=mpeg4video \
		--enable-demuxer=matroska --enable-libopus --enable-decoder=opus --enable-demuxer=mpegts --enable-demuxer=mpegtsraw --enable-parser=h264 \
		--enable-demuxer=hevc --enable-decoder=hevc \
		--enable-decoder=pcm_s16le --enable-demuxer=wav \
		--disable-ffmpeg --disable-ffplay --pkg-config-flags="--static" --pkg-config=pkg-config)
#--enable-libaom --enable-decoder=libaom_av1 \
	# remove "-maltivec -mabi=altivec" from generic CFLAGS
	perl -pi -e 's/-maltivec -mabi=altivec//' ffmpeg-$(FFMPEG_VERSION)/ffbuild/config.mak
	# add "-maltivec -mabi=altivec" to CFLAGS for specific files only
	(for i in blockdsp h264chroma_init h264dsp fdctdsp fft_vsx fft_init h264qpel me_cmp idctdsp mpegaudiodsp_altivec mpegvideo_altivec hpeldsp_altivec pixblockdsp mpegvideodsp vp8dsp_altivec vp3dsp_altivec vorbisdsp_altivec hevcdsp hevcdsp_template; do echo -n "libavcodec/ppc/$$i.o "; done) >> ffmpeg-$(FFMPEG_VERSION)/ffbuild/config.mak
	(for i in hscale gamma alphablend hscale_fast_bilinear options input output rgb2rgb slice swscale_unscaled swscale utils vscale yuv2rgb ppc/swscale_altivec ppc/yuv2rgb_altivec ppc/swscale_vsx ppc/yuv2yuv_altivec; do echo -n "libswscale/$$i.o "; done) >> ffmpeg-$(FFMPEG_VERSION)/ffbuild/config.mak
	(for i in float_dsp_altivec; do echo -n "libavutil/ppc/$$i.o "; done) >> ffmpeg-$(FFMPEG_VERSION)/ffbuild/config.mak
	echo -e ": CFLAGS+= -maltivec -mabi=altivec\n" >> ffmpeg-$(FFMPEG_VERSION)/ffbuild/config.mak
	(cd ffmpeg-$(FFMPEG_VERSION) && $(MAKE) -j$(shell nproc))
	(cd ffmpeg-$(FFMPEG_VERSION) && $(MAKE) install)
	touch $@

clean:
	rm -rf instdir
	rm -rf ffmpeg-$(FFMPEG_VERSION) .buildstamp
	rm -rf aom aom-build .aom

.PHONY: clean

# disabled for now, does not appear to work
.aom:
	rm -rf aom aom-build
	tar xf aom.tar.xz
	(cd aom && patch -p1 <../aom.diff)
	mkdir -p aom-build
	(cd aom-build && cmake ../aom -DCMAKE_TOOLCHAIN_FILE=$(realpath ../morphos.cmake) -DCONFIG_AV1_DECODER=1 -DCONFIG_AV1_ENCODER=0 -DENABLE_TOOLS=0 \
		-DAOM_TARGET_CPU=ppc -DCONFIG_RUNTIME_CPU_DETECT=0 -DENABLE_EXAMPLES=0 -DENABLE_TESTS=0 -DCMAKE_INSTALL_PREFIX="instdir" -DENABLE_VSX=0 )
	(cd aom-build && $(MAKE) -j$(shell nproc))
	(cd aom-build && $(MAKE) install)
	cp aom-build/instdir/lib/pkgconfig/aom.pc aom-build/instdir/lib/pkgconfig/libaom.pc
	touch .aom

