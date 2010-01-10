#
# global.mak
#
# This file should be included by all Makefile.mingw files for project
# wide definitions.
#

OS:=$(OS)

ifeq ($(OS),)
	CROSSCOMPILER=i586-mingw32msvc-
else
	CROSSCOMPILER=
endif

CC = $(CROSSCOMPILER)gcc
AR = $(CROSSCOMPILER)ar
RANLIB = $(CROSSCOMPILER)ranlib
WINDRES = $(CROSSCOMPILER)windres

ifneq ($(CROSSCOMPILER),)
	GLIB_GENMARSHAL := glib-genmarshal
else
	GLIB_GENMARSHAL := $(GTK_TOP)/bin/glib-genmarshal.exe
endif

# Use -g flag when building debug version of Gcompris (including plugins).
# Use -fnative-struct instead of -mms-bitfields when using mingw 1.1
# (gcc 2.95)
#CFLAGS += -O2 -Wall -mno-cygwin -mms-bitfields
CFLAGS += -g -Wall -mno-cygwin -mms-bitfields

# If not specified, dlls are built with the default base address of 0x10000000.
# When loaded into a process address space a dll will be rebased if its base
# address colides with the base address of an existing dll.  To avoid rebasing
# we do the following.  Rebasing can slow down the load time of dlls and it
# also renders debug info useless.
DLL_LD_FLAGS += -Wl,--enable-auto-image-base

DEFINES += 	-DHAVE_CONFIG_H

ifneq ($(CROSSCOMPILER),)
	# CROSS COMPILATION
	LOCAL_PREFIX:=/home/bruno/Projets/gcompris/windows
	MSVCR71_DLL:=$(LOCAL_PREFIX)/msvcr71.dll
	MAKENSIS:=makensis
else
	# MSYS CASE
	LOCAL_PREFIX:=
	MSVCR71_DLL:=/c/WINDOWS/system32/msvcr71.dll
	MAKENSIS:="/c/Program Files/NSIS/makensis.exe"
endif

GTK_TOP :=		$(LOCAL_PREFIX)/gtk
GLIB_TOP :=		$(LOCAL_PREFIX)/gtk
CAIRO_TOP :=		$(LOCAL_PREFIX)/gtk
RSVG_TOP :=		$(LOCAL_PREFIX)/rsvg
LIBXML2_TOP :=		$(LOCAL_PREFIX)/libxml2
SDL_TOP :=		$(LOCAL_PREFIX)/sdl
GNUCHESS_TOP :=		$(LOCAL_PREFIX)/gnuchess
PYTHON_TOP :=		$(LOCAL_PREFIX)/Python26
GNUCAP_TOP :=		$(LOCAL_PREFIX)/gnucap
SQLITE_TOP :=		$(LOCAL_PREFIX)/sqlite
FONTCONFIG_TOP :=	$(LOCAL_PREFIX)/fontconfig
ICONV_TOP :=		$(LOCAL_PREFIX)/iconv
FREETYPE_TOP :=		$(LOCAL_PREFIX)/freetype
