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


#include "pixbuf_util.h"


/*
 * Returns a copy of pixbuf mirrored and or flipped.
 * TO do a 180 degree rotations set both mirror and flipped TRUE
 * if mirror and flip are FALSE, result is a simple copy.
 */
GdkPixbuf *pixbuf_copy_mirror(GdkPixbuf *src, gint mirror, gint flip)
{
	GdkPixbuf *dest;
	gint has_alpha;
	gint w, h, srs;
	gint drs;
	guchar *s_pix;
        guchar *d_pix;
	guchar *sp;
        guchar *dp;
	gint i, j;
	gint a;

	if (!src) return NULL;

	w = gdk_pixbuf_get_width(src);
	h = gdk_pixbuf_get_height(src);
	has_alpha = gdk_pixbuf_get_has_alpha(src);
	srs = gdk_pixbuf_get_rowstride(src);
	s_pix = gdk_pixbuf_get_pixels(src);

	dest = gdk_pixbuf_new(GDK_COLORSPACE_RGB, has_alpha, 8, w, h);
	drs = gdk_pixbuf_get_rowstride(dest);
	d_pix = gdk_pixbuf_get_pixels(dest);

	a = has_alpha ? 4 : 3;

	for (i = 0; i < h; i++)
		{
		sp = s_pix + (i * srs);
		if (flip)
			{
			dp = d_pix + ((h - i - 1) * drs);
			}
		else
			{
			dp = d_pix + (i * drs);
			}
		if (mirror)
			{
			dp += (w - 1) * a;
			for (j = 0; j < w; j++)
				{
				*(dp++) = *(sp++);	/* r */
				*(dp++) = *(sp++);	/* g */
				*(dp++) = *(sp++);	/* b */
				if (has_alpha) *(dp) = *(sp++);	/* a */
				dp -= (a + 3);
				}
			}
		else
			{
			for (j = 0; j < w; j++)
				{
				*(dp++) = *(sp++);	/* r */
				*(dp++) = *(sp++);	/* g */
				*(dp++) = *(sp++);	/* b */
				if (has_alpha) *(dp++) = *(sp++);	/* a */
				}
			}
		}

	return dest;
}



/* Copyright 2006 Miguel DE IZARRA
 * Base on function gdk_pixbuf_fill from gdk_pixbuf
 */
void pixbuf_add_transparent (GdkPixbuf *pixbuf,guint alpha)
{
  guchar *pixels;
  guchar *p;
  guint w, h;

  g_return_if_fail(GDK_IS_PIXBUF (pixbuf));
  g_return_if_fail(gdk_pixbuf_get_has_alpha(pixbuf));

  if (gdk_pixbuf_get_width(pixbuf) == 0 || gdk_pixbuf_get_height(pixbuf) == 0)
    return;

  pixels = gdk_pixbuf_get_pixels(pixbuf);

  h = gdk_pixbuf_get_height(pixbuf);

  while (h--) {
    w = gdk_pixbuf_get_width(pixbuf);
    p = pixels;

    while (w--) {
      if(p[3] > alpha)
	p[3] = p[3] - alpha;
      else
	p[3] = 0;
      p += 4;
    }

    pixels += gdk_pixbuf_get_rowstride(pixbuf);
  }
}

