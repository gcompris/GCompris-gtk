/*
 * GQview
 * (C) 2001 John Ellis
 *
 * Author: John Ellis
 *
 * This software is released under the GNU General Public License (GNU GPL).
 * Please read the included file COPYING for more information.
 * This software comes with no warranty of any kind, use at your own risk!
 */


#ifndef PIXBUF_UTIL_H
#define PIXBUF_UTIL_H

#include <gdk-pixbuf/gdk-pixbuf.h>

GdkPixbuf *pixbuf_copy_mirror(GdkPixbuf *src, gint mirror, gint flip);

void pixbuf_add_transparent (GdkPixbuf *pixbuf,guint alpha);

#endif
