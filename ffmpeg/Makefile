FFMPEG_VERSION=2.8.15

all: .buildstamp

.buildstamp: ffmpeg-$(FFMPEG_VERSION).tar.xz ffmpeg-$(FFMPEG_VERSION).diff
	mkdir -p instdir/include instdir/lib
	rm -rf ffmpeg-$(FFMPEG_VERSION)
	(../../../../aboxtools/xz/xz -dcf ffmpeg-$(FFMPEG_VERSION).tar.xz | tar -x)
	(cd ffmpeg-$(FFMPEG_VERSION) && patch -p1 <../ffmpeg-$(FFMPEG_VERSION).diff)
	(cd ffmpeg-$(FFMPEG_VERSION) && CFLAGS="-noixemul -Os" LDFLAGS="-noixemul" ./configure --prefix="$(abspath ./instdir)" --cc=/gg/bin/ppc-morphos-gcc-9 --ld=/gg/bin/ppc-morphos-gcc-9 --enable-cross-compile --cross-prefix=/gg/bin/ --arch=powerpc --cpu=powerpc --target-os=linux --enable-runtime-cpudetect --enable-memalign-hack --disable-pthreads --disable-shared --disable-indevs --disable-protocols --enable-protocol=file --disable-network --disable-encoders --disable-decoders --disable-hwaccels --disable-muxers --disable-demuxers --disable-parsers --disable-bsfs --disable-devices --disable-filters --enable-decoder=aac --enable-decoder=aac_latm --enable-decoder=h264 --enable-decoder=mp3 --enable-decoder=theora --enable-decoder=vorbis --enable-demuxer=aac --enable-demuxer=aac_latm --enable-parser=aac --enable-parser=aac_latm --enable-demuxer=mp3 --enable-demuxer=mov --enable-demuxer=ogg --enable-parser=mpegaudio --enable-bsf=h264_mp4toannexb --enable-decoder=pcm --enable-decoder=wav --enable-decoder=flac --enable-demuxer=flac --enable-parser=flac --enable-demuxer=flv --enable-decoder=flv --enable-decoder=h263 --enable-decoder=vp8 --enable-demuxer=vp8 --enable-parser=vp8 --enable-decoder=vp9 --enable-demuxer=vp9 --enable-parser=vp9 --enable-bsf=aac_adtstoasc --enable-decoder=mpeg4 --enable-parser=mpeg4video --enable-demuxer=matroska)
	# Disable the global enabling of Altivec
	cat ffmpeg-$(FFMPEG_VERSION)/config.mak | sed "s/-maltivec -mabi=altivec//" > ffmpeg-$(FFMPEG_VERSION)/config.mak.tmp && mv ffmpeg-$(FFMPEG_VERSION)/config.mak.tmp ffmpeg-$(FFMPEG_VERSION)/config.mak
	# And enable it again for certain files...
	(head -n -1 ffmpeg-$(FFMPEG_VERSION)/config.mak; echo 'include config.altivec'; echo 'endif # FFMPEG_CONFIG_MAK') >ffmpeg-$(FFMPEG_VERSION)/config.mak.tmp && mv ffmpeg-$(FFMPEG_VERSION)/config.mak.tmp ffmpeg-$(FFMPEG_VERSION)/config.mak
	(cd ffmpeg-$(FFMPEG_VERSION) && $(MAKE) -j$(shell nproc))
	(cd ffmpeg-$(FFMPEG_VERSION) && $(MAKE) install)
	# FFMPEG seems to forget to install these. Or maybe they're not meant for apps. In any case...
	cp ffmpeg-$(FFMPEG_VERSION)/libavformat/url.h instdir/include/libavformat
	cp ffmpeg-$(FFMPEG_VERSION)/libavcodec/audioconvert.h instdir/include/libavcodec
	touch $@

clean:
	rm -rf instdir
	rm -rf ffmpeg-$(FFMPEG_VERSION) .buildstamp

.PHONY: clean

