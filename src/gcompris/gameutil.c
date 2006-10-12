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

/* for gc_util_create_rootdir */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

/* libxml includes */
#include <libxml/parserInternals.h>

#include "gcompris.h"

extern GnomeCanvas *canvas;

typedef void (*sighandler_t)(int);


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
	g_warning("Loading image '%s' returned a null pointer", filename);
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
      return NULL;
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



static GdkPixbuf *
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
 */
static void
free_image_focus (GnomeCanvasItem *item, void *none)
{
  GdkPixbuf *pixbuf;

  pixbuf = (GdkPixbuf *)g_object_get_data (G_OBJECT (item), "pixbuf_ref");
  gdk_pixbuf_unref(pixbuf);
}

/**
 * Set the focus of the given image (highlight or not)
 *
 */
void gc_item_focus_set(GnomeCanvasItem *item, gboolean focus)
{
  GdkPixbuf *dest = NULL;
  GdkPixbuf *pixbuf;
  GdkPixbuf *pixbuf_ref;

  gtk_object_get (GTK_OBJECT (item), "pixbuf", &pixbuf, NULL);
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
 			G_CALLBACK (free_image_focus),
 			NULL);

    }


  switch (focus)
    {
    case TRUE:
      dest = make_hc_pixbuf(pixbuf, 30);
      gnome_canvas_item_set (item,
			     "pixbuf", dest,
			     NULL);

      break;
    case FALSE:
      gnome_canvas_item_set (item,
			     "pixbuf", pixbuf_ref,
			     NULL);
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
gint gc_item_focus_event(GnomeCanvasItem *item, GdkEvent *event,
			       GnomeCanvasItem *dest_item)
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

  xmlParserCtxtPtr ctxt = xmlNewParserCtxt();

  if(str==NULL)
    return NULL;


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
void gc_item_absolute_move(GnomeCanvasItem *item, int x, int y) {
  double dx1, dy1, dx2, dy2;
  gnome_canvas_item_get_bounds(item, &dx1, &dy1, &dx2, &dy2);
  gnome_canvas_item_move(item, ((double)x)-dx1, ((double)y)-dy1);
}

/* ======================================= */
/** As gnome does not implement its own API : gc_item_rotate
   we have to do it ourselves ....
   IMPORTANT NOTE : This is designed for an item with "anchor" =  GTK_ANCHOR_CENTER
   rotation is clockwise if angle > 0
*/
void
gc_item_rotate(GnomeCanvasItem *item, double angle) {
  double r[6],t[6], x1, x2, y1, y2;

  gnome_canvas_item_get_bounds( item, &x1, &y1, &x2, &y2 );
  art_affine_translate( t , -(x2+x1)/2, -(y2+y1)/2 );
  art_affine_rotate( r, angle );
  art_affine_multiply( r, t, r);
  art_affine_translate( t , (x2+x1)/2, (y2+y1)/2 );
  art_affine_multiply( r, r, t);

  gnome_canvas_item_affine_absolute(item, r );
}

/* As gnome does not implement its own API : gc_item_rotate_relative
   we have to do it ourselves ....
   IMPORTANT NOTE : This is designed for an item with "anchor" =  GTK_ANCHOR_CENTER
   rotation is clockwise if angle > 0
 */
void
gc_item_rotate_relative(GnomeCanvasItem *item, double angle) {
  double x1, x2, y1, y2;
  double tx1, tx2, ty1, ty2;
  double cx, cy;
  double t;
  double r[6];

  //  gnome_canvas_item_get_bounds( item, &x1, &y1, &x2, &y2 );
  /* WARNING: Do not use gnome_canvas_item_get_bounds which gives unpredictable results */
  if(GNOME_IS_CANVAS_LINE(item)) {
    GnomeCanvasPoints	*points;
    gtk_object_get (GTK_OBJECT (item), "points", &points, NULL);
    x1 = points->coords[0];
    y1 = points->coords[1];
    x2 = points->coords[2];
    y2 = points->coords[3];
  } else if(GNOME_IS_CANVAS_PIXBUF(item)){
    gtk_object_get (GTK_OBJECT (item), "x", &x1, NULL);
    gtk_object_get (GTK_OBJECT (item), "y", &y1, NULL);
    gtk_object_get (GTK_OBJECT (item), "width",  &x2, NULL);
    gtk_object_get (GTK_OBJECT (item), "height", &y2, NULL);
    x2 += x1;
    y2 += y1;
  } else if(GNOME_IS_CANVAS_GROUP(item)){
    gtk_object_get (GTK_OBJECT (item), "x", &x1, NULL);
    gtk_object_get (GTK_OBJECT (item), "y", &y1, NULL);
    x2 = x1;
    y2 = y1;
  } else {
    gtk_object_get (GTK_OBJECT (item), "x1", &x1, NULL);
    gtk_object_get (GTK_OBJECT (item), "y1", &y1, NULL);
    gtk_object_get (GTK_OBJECT (item), "x2", &x2, NULL);
    gtk_object_get (GTK_OBJECT (item), "y2", &y2, NULL);
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

  /* Taken from anim by Yves Combe
   * This matrix rotate around ( cx, cy )
   * This is the result of the product:
   *            T_{-c}             Rot (t)                 T_c
   *
   *       1    0   cx       cos(t) -sin(t)    0        1    0  -cx
   *       0    1   cy  by   sin(t)  cos(t)    0   by   0    1  -cy
   *       0    0    1         0       0       1        0    0   1
   */

  t = M_PI*angle/180.0;

  r[0] = cos(t);
  r[1] = sin(t);
  r[2] = -sin(t);
  r[3] = cos(t);
  r[4] = (1-cos(t))*cx + sin(t)*cy;
  r[5] = -sin(t)*cx + (1 - cos(t))*cy;

  gnome_canvas_item_affine_relative(item, r );
}

/** rotates an item around the center (x,y), relative to the widget's coordinates
 */
void
gc_item_rotate_with_center(GnomeCanvasItem *item, double angle, int x, int y) {
  double r[6],t[6], x1, x2, y1, y2, tx, ty;

  gnome_canvas_item_get_bounds( item, &x1, &y1, &x2, &y2 );
  tx = x1 + x;
  ty = y1 + y;
  art_affine_translate( t , -tx, -ty );
  art_affine_rotate( r, angle );
  art_affine_multiply( r, t, r);
  art_affine_translate( t , tx, ty );
  art_affine_multiply( r, r, t);

  gnome_canvas_item_affine_absolute(item, r );
}

/** rotates an item around the center (x,y), relative to the widget's coordinates
 *  The rotation is relative to the previous rotation
 */
void
gc_item_rotate_relative_with_center(GnomeCanvasItem *item, double angle, int x, int y) {
  double r[6],t[6], x1, x2, y1, y2, tx, ty;

  gnome_canvas_item_get_bounds( item, &x1, &y1, &x2, &y2 );
  tx = x1 + x;
  ty = y1 + y;
  art_affine_translate( t , -tx, -ty );
  art_affine_rotate( r, angle );
  art_affine_multiply( r, t, r);
  art_affine_translate( t , tx, ty );
  art_affine_multiply( r, r, t);

  gnome_canvas_item_affine_relative(item, r );
}

/**
 * Display the number of stars representing the difficulty level at the x,y location
 * The stars are created in a group 'parent'
 * The new group in which the stars are created is returned.
 * This is only usefull for the menu plugin and the configuration dialog box.
 */
GnomeCanvasGroup *gc_difficulty_display(GnomeCanvasGroup *parent,
						    double x, double y,
						    double ratio,
						    gint difficulty)
{
  GdkPixbuf *pixmap = NULL;
  GnomeCanvasGroup *stars_group = NULL;
  GnomeCanvasPixbuf *item = NULL;
  gchar *filename = NULL;

  if(difficulty==0 || difficulty>6)
    return NULL;

  filename = g_strdup_printf("difficulty_star%d.png", difficulty);
  pixmap   = gc_skin_pixmap_load(filename);
  g_free(filename);

  if(!pixmap)
    return NULL;

  stars_group = GNOME_CANVAS_GROUP(
				  gnome_canvas_item_new (parent,
							 gnome_canvas_group_get_type (),
							 "x", (double) 0,
							 "y", (double) 0,
							 NULL));

  item = GNOME_CANVAS_PIXBUF(gnome_canvas_item_new (stars_group,
						    gnome_canvas_pixbuf_get_type (),
						    "pixbuf", pixmap,
						    "x", x,
						    "y", y,
						    "width", (double) gdk_pixbuf_get_width(pixmap) * ratio,
						    "height", (double) gdk_pixbuf_get_height(pixmap) * ratio,
						    "width_set", TRUE,
						    "height_set", TRUE,
						    NULL));

  gtk_signal_connect(GTK_OBJECT(item), "event",
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

  /* Check it's already an absolute file */
  if( ((g_path_is_absolute (filename) &&
	g_file_test (filename, G_FILE_TEST_EXISTS))
       || gc_net_is_url(filename)) )
    {
      return filename;
    }

  /*
   * Search it on the file system
   */

  dir_to_search[i++] = properties->package_data_dir;
  dir_to_search[i++] = properties->user_data_dir;
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
  return absolute_filename;
}

/** Create a directory if needed.
 *  If a file is given, it is removed and a directory is created instead.
 *
 * \param rootdir: the directory to create
 *
 * return 0 if OK, -1 if ERROR
 */
int
gc_util_create_rootdir (gchar *rootdir)
{

  /* Case where ~/.gcompris already exist as a file. We remove it */
  if(g_file_test(rootdir, G_FILE_TEST_IS_REGULAR)) {
    unlink(rootdir);
  }

  if(g_file_test(rootdir, G_FILE_TEST_IS_DIR)) {
    return 0;
  }

#if defined WIN32
  return(mkdir(rootdir));
#else
  return(mkdir(rootdir, 0755));
#endif
}
