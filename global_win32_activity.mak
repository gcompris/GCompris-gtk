#
# This is the global makefile for mingw compilation of activities
#

#
# PATHS
#

CANVAS_TOP :=		$(GCOMPRIS_TOP)/src/goocanvas/src

##
## VARIABLE DEFINITIONS
##

# Compiler Options

DEFINES =

.SUFFIXES:
.SUFFIXES: .c .a

##
## INCLUDE  MAKEFILES
##

include $(GCOMPRIS_TOP)/global_win32.mak

##
## INCLUDE PATHS
##

INCLUDE_PATHS +=	-I. \
			-I$(GTK_TOP)/include \
			-I$(GTK_TOP)/include/gtk-2.0 \
			-I$(GLIB_TOP)/include/glib-2.0 \
			-I$(GTK_TOP)/include/pango-1.0 \
			-I$(GLIB_TOP)/lib/glib-2.0/include \
			-I$(GTK_TOP)/lib/gtk-2.0/include \
			-I$(GTK_TOP)/include/atk-1.0 \
			-I$(GTK_TOP)/include/freetype2 \
			-I$(LIBXML2_TOP)/include \
			-I$(GCOMPRIS_TOP) \
			-I$(GCOMPRIS_TOP)/src \
			-I$(CANVAS_TOP) \
			-I$(CAIRO_TOP)/include/cairo \
			-I$(PYTHON_TOP)/include \
			-I$(PYTHON_TOP)/include/pygtk-2.0 \
			-I$(PYTHON_TOP)/include/pycairo \
			-I$(SQLITE_TOP) \
			-I$(ICONV_TOP)/include \
			-I$(RSVG_TOP)/include/librsvg-2


##
## TARGET DEFINITIONS
##

.PHONY: all clean

# How to make a C file
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE_PATHS) $(DEFINES) -c $< -o $@

all: libactivity

#
# BUILD Activity staticlibrary
#

ACTIVITY_OBJECTS = $(ACTIVITY_C_SRC:%.c=%.o)

libactivity: \
		libactivity.a

libactivity.a: $(ACTIVITY_OBJECTS)
	-rm -f libactivity.a
	$(AR) cru libactivity.a $(ACTIVITY_OBJECTS)
	$(RANLIB) libactivity.a

clean:
	rm -rf *.o
	rm -rf *.a

install:

