/* gcompris - gameutil.c
 *
 * Time-stamp: <2006/08/20 12:41:31 bruno>
 *
 * Copyright (C) 2000-2006 Bruno Coudoin
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */



#include <math.h>
#include <string.h>
#include <time.h>

/* g_mkdir */
#define G_STDIO_NO_WRAP_ON_UNIX
#include <glib/gstdio.h>

/* libxml includes */
#include <libxml/parserInternals.h>

#include "gcompris.h"

extern GooCanvas *canvas;

typedef void (*sighandler_t)(int);

/* GdkPixbuf RGBA C-Source image dump for a NULL image*/
#ifdef __SUNPRO_C
#pragma align 4 (null_img)
#endif
#ifdef __GNUC__
static const guint8 null_img[] __attribute__ ((__aligned__ (4))) =
#else
static const guint8 null_img[] =
#endif
{ ""
  /* Pixbuf magic (0x47646b50) */
  "GdkP"
  /* length: header (24) + pixel_data (4) */
  "\0\0\0\34"
  /* pixdata_type (0x1010002) */
  "\1\1\0\2"
  /* rowstride (4) */
  "\0\0\0\4"
  /* width (1) */
  "\0\0\0\1"
  /* height (1) */
  "\0\0\0\1"
  /* pixel_data: */
  "%%%\0"};

/** load a pixmap from the filesystem
 *
 * \param format: If format contains $LOCALE, it will be first replaced by the current long locale
 *                and if not found the short locale name. It support printf formating.
 * \param ...:    additional params for the format (printf like)
 *
 * \return a new allocated pixbuf or NULL
 */
GdkPixbuf *gc_pixmap_load(const gchar *format, ...)
{
  va_list args;
  gchar *filename;
  gchar *pixmapfile;
  GdkPixbuf *pixmap=NULL;

  if (!format)
    return NULL;

  va_start (args, format);
  pixmapfile = g_strdup_vprintf (format, args);
  va_end (args);

  /* Search */
  filename = gc_file_find_absolute(pixmapfile);

  if(filename)
    pixmap = gc_net_load_pixmap(filename);

  if (!filename || !pixmap)
    {
      char *str;

      if(!pixmap)
	g_warning("Loading image '%s' returned a null pointer", pixmapfile);
      else
	g_warning ("Couldn't find file %s !", pixmapfile);

      str = g_strdup_printf("%s\n%s\n%s\n%s",
			    _("Couldn't find or load the file"),
			    pixmapfile,
			    _("This activity is incomplete."),
			    _("Exit it and report\nthe problem to the authors."));
      gc_dialog (str, NULL);
      g_free(pixmapfile);
      g_free(str);

      /* Create an empty pixmap because activities does not manage loading error */
      return gdk_pixbuf_new_from_inline(-1, null_img, FALSE, NULL);
    }

  g_free(pixmapfile);
  g_free(filename);


  return(pixmap);
}

/*************************************************************
 * colorshift a pixbuf
 * code taken from the gnome-panel of gnome-core
 */
static void
do_colorshift (GdkPixbuf *dest, GdkPixbuf *src, int shift)
{
  gint i, j;
  gint width, height, has_alpha, srcrowstride, destrowstride;
  guchar *target_pixels;
  guchar *original_pixels;
  guchar *pixsrc;
  guchar *pixdest;
  int val;
  guchar r,g,b;

  has_alpha = gdk_pixbuf_get_has_alpha (src);
  width = gdk_pixbuf_get_width (src);
  height = gdk_pixbuf_get_height (src);
  srcrowstride = gdk_pixbuf_get_rowstride (src);
  destrowstride = gdk_pixbuf_get_rowstride (dest);
  target_pixels = gdk_pixbuf_get_pixels (dest);
  original_pixels = gdk_pixbuf_get_pixels (src);

  for (i = 0; i < height; i++) {
    pixdest = target_pixels + i*destrowstride;
    pixsrc = original_pixels + i*srcrowstride;
    for (j = 0; j < width; j++) {
      r = *(pixsrc++);
      g = *(pixsrc++);
      b = *(pixsrc++);
      val = r + shift;
      *(pixdest++) = CLAMP(val, 0, 255);
      val = g + shift;
      *(pixdest++) = CLAMP(val, 0, 255);
      val = b + shift;
      *(pixdest++) = CLAMP(val, 0, 255);
      if (has_alpha)
	*(pixdest++) = *(pixsrc++);
    }
  }
}



GdkPixbuf *
make_hc_pixbuf(GdkPixbuf *pb, gint val)
{
  GdkPixbuf *new;
  if(!pb)
    return NULL;

  new = gdk_pixbuf_new(gdk_pixbuf_get_colorspace(pb),
		       gdk_pixbuf_get_has_alpha(pb),
		       gdk_pixbuf_get_bits_per_sample(pb),
		       gdk_pixbuf_get_width(pb),
		       gdk_pixbuf_get_height(pb));
  do_colorshift(new, pb, val);
  /*do_saturate_darken (new, pb, (int)(1.00*255), (int)(1.15*255));*/

  return new;
}

/**
 * Free the highlight image from our image_focus system
 *
 * It must be called before assigning a new image to an item that
 * already has a focus enabled with gc_item_focus_event().
 */
void
gc_item_focus_free(GooCanvasItem *item, void *none)
{
  GdkPixbuf *pixbuf;

  pixbuf = (GdkPixbuf *)g_object_get_data (G_OBJECT (item), "pixbuf_ref");
  if(pixbuf)
    {
      g_object_set_data (G_OBJECT (item), "pixbuf_ref", NULL);
      gdk_pixbuf_unref(pixbuf);
    }
}

/**
 * Set the focus of the given image (highlight or not)
 *
 */
void gc_item_focus_set(GooCanvasItem *item, gboolean focus)
{
  GdkPixbuf *dest = NULL;
  GdkPixbuf *pixbuf;
  GdkPixbuf *pixbuf_ref;

  g_object_get (item, "pixbuf", &pixbuf, NULL);
  g_return_if_fail (pixbuf != NULL);
  gdk_pixbuf_unref(pixbuf);

  /* Store the first pixbuf */
  pixbuf_ref = (GdkPixbuf *)g_object_get_data (G_OBJECT (item), "pixbuf_ref");
  if(!pixbuf_ref)
    {
      g_object_set_data (G_OBJECT (item), "pixbuf_ref", pixbuf);
      pixbuf_ref = pixbuf;
      gdk_pixbuf_ref(pixbuf);
      g_signal_connect (item, "destroy",
 			G_CALLBACK (gc_item_focus_free),
 			NULL);

    }


  switch (focus)
    {
    case TRUE:
      dest = make_hc_pixbuf(pixbuf, 30);
      g_object_set_data (G_OBJECT(item),
			 "pixbuf", dest);

      break;
    case FALSE:
      g_object_set_data (G_OBJECT(item),
			 "pixbuf", pixbuf_ref);
      break;
    default:
      break;
    }

  if(dest!=NULL)
    gdk_pixbuf_unref (dest);

}

/**
 * Callback over a canvas item, this function will highlight the focussed item
 * or the given one
 *
 */
gint
gc_item_focus_event(GooCanvasItem *item, GdkEvent *event,
		    GooCanvasItem *dest_item)
{

  if(dest_item!=NULL)
    item = dest_item;

  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
      gc_item_focus_set(item, TRUE);
      break;
    case GDK_LEAVE_NOTIFY:
      gc_item_focus_set(item, FALSE);
      break;
    default:
      break;
    }

  return FALSE;
}

/*
 * Return a new copy of the given string in which it has
 * changes '\''n' to '\n'.
 * The recognized sequences are \b
 * \f \n \r \t \\ \" and the octal format.
 *
 */
gchar *reactivate_newline(char *str)
{
  gchar *newstr;

  if(str==NULL)
    return NULL;

  xmlParserCtxtPtr ctxt = xmlNewParserCtxt();

  newstr =  (gchar *)xmlStringDecodeEntities(ctxt,
					     BAD_CAST str,
					     XML_SUBSTITUTE_REF,
					     0,
					     0,
					     0);

  xmlFreeParserCtxt(ctxt);

  return newstr;
}

/* ======================================= */
/* Any previous transformation are reseted first. */
void gc_item_absolute_move(GooCanvasItem *item, int x, int y)
{
  GooCanvasBounds bounds;

  goo_canvas_item_set_transform(item, NULL);
  goo_canvas_item_get_bounds(item, &bounds);
  goo_canvas_item_translate(item, ((double)x)-bounds.x1, ((double)y)-bounds.y1);
}

/* ======================================= */
/** As gnome does not implement its own API : gc_item_rotate
   we have to do it ourselves ....
   rotation is clockwise if angle > 0
*/
void
gc_item_rotate(GooCanvasItem *item, double angle) {
  GooCanvasBounds bounds;
  goo_canvas_item_get_bounds (item, &bounds);
  goo_canvas_item_rotate(item, angle,
			 bounds.x1+(bounds.x2-bounds.x1)/2,
			 bounds.y1+(bounds.y2-bounds.y1)/2);
}

/* As gnome does not implement its own API : gc_item_rotate_relative
   we have to do it ourselves ....
   IMPORTANT NOTE : This is designed for an item with "anchor" =  GTK_ANCHOR_CENTER
   rotation is clockwise if angle > 0
 */
void
gc_item_rotate_relative(GooCanvasItem *item, double angle)
{
  double x1, x2, y1, y2;
  double tx1, tx2, ty1, ty2;
  double cx, cy;

  //  goo_canvas_item_get_bounds( item, &x1, &y1, &x2, &y2 );
  /* WARNING: Do not use goo_canvas_item_get_bounds which gives unpredictable results */
  if(GOO_IS_CANVAS_POLYLINE(item)) {
    GooCanvasPoints	*points;
    g_object_get (item, "points", &points, NULL);
    x1 = points->coords[0];
    y1 = points->coords[1];
    x2 = points->coords[2];
    y2 = points->coords[3];
  } else if(GOO_IS_CANVAS_IMAGE(item) || GOO_IS_CANVAS_RECT(item)) {
    g_object_get (item, "x", &x1, NULL);
    g_object_get (item, "y", &y1, NULL);
    g_object_get (item, "width",  &x2, NULL);
    g_object_get (item, "height", &y2, NULL);
    x2 += x1;
    y2 += y1;
  } else if(GOO_IS_CANVAS_GROUP(item)){
    g_object_get (item, "x", &x1, NULL);
    g_object_get (item, "y", &y1, NULL);
    x2 = x1;
    y2 = y1;
  } else {
    g_object_get (item, "x1", &x1, NULL);
    g_object_get (item, "y1", &y1, NULL);
    g_object_get (item, "x2", &x2, NULL);
    g_object_get (item, "y2", &y2, NULL);
  }

  tx1 = x1;
  ty1 = y1;
  tx2 = x2;
  ty2 = y2;

  x1 = MIN(tx1,tx2);
  y1 = MIN(ty1,ty2);
  x2 = MAX(tx1,tx2);
  y2 = MAX(ty1,ty2);


  cx = (x2+x1)/2;
  cy = (y2+y1)/2;

  goo_canvas_item_rotate(item, angle,
			 x1+(x2-x1)/2,
			 y1+(y2-y1)/2);

}

/** rotates an item around the center (x,y), relative to the widget's coordinates
 * Any previous transformation are reseted first.
 */
void
gc_item_rotate_with_center(GooCanvasItem *item, double angle, int x, int y)
{
  GooCanvasBounds bounds;

  goo_canvas_item_set_transform(item, NULL);
  goo_canvas_item_get_bounds( item, &bounds );
  goo_canvas_item_rotate(item, angle, bounds.x1+x, bounds.y1+y);
}

/** rotates an item around the center (x,y), relative to the widget's coordinates
 *  The rotation is relative to the previous rotation
 */
void
gc_item_rotate_relative_with_center(GooCanvasItem *item, double angle, int x, int y)
{
  double x1, x2, y1, y2;

  if(GOO_IS_CANVAS_POLYLINE(item)) {
    GooCanvasPoints *points;
    g_object_get (item, "points", &points, NULL);
    x1 = points->coords[0];
    y1 = points->coords[1];
    x2 = points->coords[2];
    y2 = points->coords[3];
  } else if(GOO_IS_CANVAS_IMAGE(item) || GOO_IS_CANVAS_RECT(item)) {
    g_object_get (item, "x", &x1, NULL);
    g_object_get (item, "y", &y1, NULL);
    g_object_get (item, "width",  &x2, NULL);
    g_object_get (item, "height", &y2, NULL);
    x2 += x1 + (x2 - x1);
    y2 += y1 + (y2 - y1);
  } else if(GOO_IS_CANVAS_GROUP(item)){
    g_object_get (item, "x", &x1, NULL);
    g_object_get (item, "y", &y1, NULL);
    x2 = x1;
    y2 = y1;
  } else {
    g_object_get (item, "x1", &x1, NULL);
    g_object_get (item, "y1", &y1, NULL);
    g_object_get (item, "x2", &x2, NULL);
    g_object_get (item, "y2", &y2, NULL);
  }

  goo_canvas_item_rotate(item, angle, x1+x, y1+y);
}

/**
 * Display the number of stars representing the difficulty level at the x,y location
 * The stars are created in a group 'parent'
 * The new group in which the stars are created is returned.
 * This is only usefull for the menu plugin and the configuration dialog box.
 */
GooCanvasItem *gc_difficulty_display(GooCanvasItem *parent,
				     double x, double y,
				     double ratio,
				     gint difficulty)
{
  GdkPixbuf *pixmap = NULL;
  GooCanvasItem *stars_group = NULL;
  GooCanvasItem *item = NULL;
  gchar *filename = NULL;

  if(difficulty==0 || difficulty>6)
    return NULL;

  filename = g_strdup_printf("difficulty_star%d.png", difficulty);
  pixmap   = gc_skin_pixmap_load(filename);
  g_free(filename);

  if(!pixmap)
    return NULL;

  stars_group = goo_canvas_group_new (parent, NULL);

  item = goo_canvas_image_new (stars_group,
			       pixmap,
			       x,
			       y,
			       "width", (double) gdk_pixbuf_get_width(pixmap) * ratio,
			       "height", (double) gdk_pixbuf_get_height(pixmap) * ratio,
			       NULL);

  g_signal_connect(item, "button_press_event",
		   (GtkSignalFunc) gc_item_focus_event,
		   NULL);

  gdk_pixbuf_unref(pixmap);

  return(stars_group);
}

gchar *g_utf8_strndup(gchar* utf8text, gint n)
{
 gchar* result;

 gint len = g_utf8_strlen(utf8text, -1);

 if( n < len && n > 0 )
   len = n;

 result = g_strndup(utf8text, g_utf8_offset_to_pointer(utf8text, len) - utf8text);

 return result;
}

/** \brief search a given relative file in all gcompris dir it could be found
 *
 * \param format: If format contains $LOCALE, it will be first replaced by the current long locale
 *                and if not found the short locale name. It support printf formating.
 * \param ...:    additional params for the format (printf like)
 *
 * \return NULL or a new gchar* with the absolute_filename of the given filename or a full url to it.
 *
 */
gchar*
gc_file_find_absolute(const gchar *format, ...)
{
  va_list		 args;
  int			 i = 0;
  gchar			*filename;
  gchar			*absolute_filename;
  gchar			*dir_to_search[4];
  GcomprisProperties	*properties = gc_prop_get();

  if (!format)
    return NULL;

  va_start (args, format);
  filename = g_strdup_vprintf (format, args);
  va_end (args);

  /* Check it's already found */
  if( g_file_test (filename, G_FILE_TEST_EXISTS)
      || gc_net_is_url(filename) )
    {
      return filename;
    }

  /*
   * Search it on the file system
   */

  dir_to_search[i++] = properties->package_data_dir;
  dir_to_search[i++] = NULL;

  absolute_filename = g_strdup(filename);
  i = 0;

  while (dir_to_search[i])
    {
      gchar **tmp;
      g_free(absolute_filename);

      /* Check there is a $LOCALE to replace */
      if((tmp = g_strsplit(filename, "$LOCALE", -1)))
	{
	  gchar locale[6];
	  gchar *filename2;

	  /* First try with the long locale */
	  g_strlcpy(locale, gc_locale_get(), sizeof(locale));
	  filename2 = g_strjoinv(locale, tmp);
	  absolute_filename = g_strdup_printf("%s/%s", dir_to_search[i], filename2);
	  if(g_file_test (absolute_filename, G_FILE_TEST_EXISTS))
	    {
	      g_strfreev(tmp);
	      g_free(filename2);
	      goto FOUND;
	    }
      g_free(absolute_filename);
	  /* Now check if this file is on the net */
	  if((absolute_filename = gc_net_get_url_from_file(filename2, NULL)))
	    {
	      g_strfreev(tmp);
	      g_free(filename2);
	      goto FOUND;
	    }

      g_free(filename2);
      g_free(absolute_filename);
	  /* Try the short locale */
	  if(g_strv_length(tmp)>1)
	    {
	      locale[2] = '\0';
	      filename2 = g_strjoinv(locale, tmp);
	      g_strfreev(tmp);
	      absolute_filename = g_strdup_printf("%s/%s", dir_to_search[i], filename2);
	      if(g_file_test (absolute_filename, G_FILE_TEST_EXISTS))
		{
		  g_free(filename2);
		  goto FOUND;
		}

	      /* Now check if this file is on the net */
	      if((absolute_filename = gc_net_get_url_from_file(filename2, NULL)))
		{
		  g_free(filename2);
		  goto FOUND;
		}
        g_free(filename2);

	    }
      else
          g_strfreev(tmp);
	}
      else
	{
	  absolute_filename = g_strdup_printf("%s/%s", dir_to_search[i], filename);

	  if(g_file_test (absolute_filename, G_FILE_TEST_EXISTS))
	    goto FOUND;
    g_free(absolute_filename);
	  /* Now check if this file is on the net */
	  if((absolute_filename = gc_net_get_url_from_file(filename, NULL)))
	    goto FOUND;
      g_free(absolute_filename);
	}

      i++;
    }

  g_free(filename);
  g_free(absolute_filename);
  return NULL;

 FOUND:
  g_free(filename);
  char *abs_name = realpath(absolute_filename, NULL);
  g_free(absolute_filename);
  return abs_name;
}

/** Create a directory if needed.
 *
 * \param rootdir: the directory to create
 *
 * return 0 if OK, -1 if ERROR
 */
int
gc_util_create_rootdir (gchar *rootdir)
{

  if(g_file_test(rootdir, G_FILE_TEST_IS_DIR)) {
    return 0;
  }

  return(g_mkdir(rootdir, 0755));
}
