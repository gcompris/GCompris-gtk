#!/bin/sh

CONFIG_SHELL=/bin/sh
export CONFIG_SHELL
PREFIX=/usr/local/cross-tools
TARGET=i586-mingw32msvc
CROSS_DEV=/home/ycombe/Win32/dev2
PATH="$PATH:$CROSS_DEV/bin"
export PATH
PKG_CONFIG_LIBDIR="$CROSS_DEV/lib/pkgconfig:$CROSS_DEV/Python24/Lib/pkgconfig"
export PKG_CONFIG_LIBDIR
export PKG_CONFIG_PATH
PYTHONHOME="$CROSS_DEV/Python24"
PYTHONPATH="$CROSS_DEV/Python24/Lib"
PYTHON_CFLAGS="-I$CROSS_DEV/Python24/include"
PYTHON_LIBS="-L$CROSS_DEV/Python24/libs -lpython24"
export PYTHONHOME
export PYTHONPATH
export PYTHON_CFLAGS
export PYTHON_LIBS

SDL_MIXER_CFLAGS="-I$CROSS_DEV/SDL_mixer-1.2.7/include"
SDL_MIXER_LDFLAGS="-L$CROSS_DEV/SDL_mixer-1.2.7/lib"
SDL_CFLAGS="-I$CROSS_DEV/SDL-1.2.11/include"
SDL_LIBS="-L$CROSS_DEV/SDL-1.2.11/lib -lSDLmain -lSDL"
export SDL_CFLAGS
export SDL_LIBS

if [ -f "$CROSS_DEV/SDL-1.2.11/bin/i386-mingw32msvc-sdl-config" ]; then
    SDL_CONFIG="$CROSS_DEV/SDL-1.2.11/bin/i386-mingw32msvc-sdl-config --prefix=$CROSS_DEV/SDL-1.2.11"
    export SDL_CONFIG
fi

cache=cross-config.cache
#CFLAGS="-I$CROSS_DEV/include -idirafter $CROSS_DEV/include/glibc" LDFLAGS=-L$CROSS_DEV/lib  LIBS="-lgw32c -luuid -lole32 -lwsock32 -mwindows" sh configure --cache-file="$cache" \
#CFLAGS="-I$CROSS_DEV/include -I$CROSS_DEV/include/glibc -I$CROSS_DEV/include/glibc -D__GW32__" LDFLAGS=-L$CROSS_DEV/lib LIBS=-lgw32c sh configure --cache-file="$cache" 
CFLAGS="-I$CROSS_DEV/include -idirafter $CROSS_DEV/include/glibc $SDL_MIXER_CFLAGS -D__GW32__"  LDFLAGS="-L$CROSS_DEV/lib $SDL_MIXER_LDFLAGS" LIBS=-lgw32c sh configure --cache-file="$cache" \
	--target=$TARGET --host=$TARGET --build=i386-linux \
        --enable-py-build-only \
        --with-sdl-prefix="$CROSS_DEV/SDL-1.2.11" \
        --prefix=/media/hda5/GCompris_test \
        --disable-binreloc \
        $*
status=$?
rm -f "$cache"
exit $status
