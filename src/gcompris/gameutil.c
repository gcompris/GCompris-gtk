/* gcompris - gameutil.c
 *
 * Copyright (C) 2000, 2008 Bruno Coudoin
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
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

/** load a pixmap from the filesystem, return NULL if
 *  not found and do not display an error message.
 *
 * \param format: If format contains $LOCALE, it will be first replaced by the current long locale
 *                and if not found the short locale name. It support printf formating.
 * \param ...:    additional params for the format (printf like)
 *
 * \return a new allocated pixbuf or NULL
 */
GdkPixbuf *gc_pixmap_load_or_null(const gchar *format, ...)
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
     pixmap = gdk_pixbuf_new_from_file(filename,NULL);

  g_free(pixmapfile);
  g_free(filename);


  return(pixmap);
}

/** load a pixmap from the filesystem, if not found display
 *  an error message and return an small 1x1 pixmap.
 *
 * \param format: If format contains $LOCALE, it will be first replaced by the current long locale
 *                and if not found the short locale name. It support printf formating.
 * \param ...:    additional params for the format (printf like)
 *
 * \return a new allocated pixbuf or a 1x1 pixmap
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
    pixmap = gc_pixmap_load_or_null(filename);

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

/** load an svg image from the filesystem
 *
 * \param format: If format contains $LOCALE, it will be first replaced by the current long locale
 *                and if not found the short locale name. It support printf formating.
 * \param ...:    additional params for the format (printf like)
 *
 * \return a new allocated pixbuf or NULL
 */
RsvgHandle *gc_rsvg_load(const gchar *format, ...)
{
  va_list args;
  gchar *filename;
  gchar *rsvghandlefile;
  RsvgHandle *rsvghandle = NULL;
  GError *error = NULL;

  if (!format)
    return NULL;

  va_start (args, format);
  rsvghandlefile = g_strdup_vprintf (format, args);
  va_end (args);

  /* Search */
  filename = gc_file_find_absolute(rsvghandlefile);

  if(filename)
    rsvghandle = rsvg_handle_new_from_file (filename, &error);

  if (!filename || !rsvghandle)
    {
      char *str;

      if(!rsvghandle)
	g_warning("Loading image '%s' returned a null pointer", rsvghandlefile);
      else
	g_warning ("Couldn't find file %s !", rsvghandlefile);

      str = g_strdup_printf("%s\n%s\n%s\n%s",
			    _("Couldn't find or load the file"),
			    rsvghandlefile,
			    _("This activity is incomplete."),
			    _("Exit it and report\nthe problem to the authors."));
      gc_dialog (str, NULL);
      g_free(rsvghandlefile);
      g_free(str);

      /* Create an empty rsvghandle because activities does not manage loading error */
      return rsvg_handle_new();
    }

  g_free(rsvghandlefile);
  g_free(filename);

  return(rsvghandle);
}

/**
 * Set the focus of the given item (highlight or not)
 *
 */
static void
gc_item_focus_set(GooCanvasItem *item, gboolean focus)
{
  GooCanvasItem *highlight_item;

  highlight_item = g_object_get_data (G_OBJECT(item),
				      "highlight_item");
  g_assert(highlight_item);

  switch (focus)
    {
    case TRUE:
      g_object_set (highlight_item,
		    "visibility", GOO_CANVAS_ITEM_VISIBLE,
		    NULL);
      break;
    case FALSE:
      g_object_set (highlight_item,
		    "visibility", GOO_CANVAS_ITEM_INVISIBLE,
		    NULL);
      break;
    default:
      break;
    }

}

/**
 * Callback over a canvas item, this function will highlight the focussed item
 * or the given one
 *
 */
static gint
gc_item_focus_event(GooCanvasItem *item, GooCanvasItem *target,
		    GdkEvent *event,
		    GooCanvasItem *target_item)
{
  if(target_item != NULL)
    item = target_item;

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

/**
 * Init an item so that it has a focus
 * Optionnaly, provide a target_item that will be focused
 * by events on source_item.
 *
 * Warning: You must call it each time the size of the focused
 *          item size changes.
 *
 */
#define GAP 4
void gc_item_focus_init(GooCanvasItem *source_item,
			GooCanvasItem *target_item)
{
  GooCanvasItem *highlight_item;
  GooCanvasBounds bounds;

  if(!target_item)
    target_item = source_item;

  goo_canvas_item_get_bounds(target_item, &bounds);
  goo_canvas_convert_to_item_space(goo_canvas_item_get_canvas(target_item),
				   goo_canvas_item_get_parent(target_item),
				   &bounds.x1, &bounds.y1);
  goo_canvas_convert_to_item_space(goo_canvas_item_get_canvas(target_item),
				   goo_canvas_item_get_parent(target_item),
				   &bounds.x2, &bounds.y2);
  highlight_item = g_object_get_data (G_OBJECT(target_item),
				      "highlight_item");

  if(highlight_item)
    {
      goo_canvas_item_remove(highlight_item);
    }

  /* Create the highlight_item */
  highlight_item =
    goo_canvas_rect_new (goo_canvas_item_get_parent(target_item),
			 bounds.x1 - GAP,
			 bounds.y1 - GAP,
			 bounds.x2 - bounds.x1 + GAP*2,
			 bounds.y2 - bounds.y1 + GAP*2,
			 "stroke_color_rgba", 0xFFFFFFFFL,
			 "fill_color_rgba", 0xFF000090L,
			 "line-width", (double) 2,
			 "radius-x", (double) 10,
			 "radius-y", (double) 10,
			 NULL);

  g_object_set_data (G_OBJECT(target_item), "highlight_item",
		     highlight_item);
  goo_canvas_item_lower(highlight_item, target_item);
  g_object_set (highlight_item,
  		"visibility", GOO_CANVAS_ITEM_INVISIBLE,
  		NULL);

  /* Avoid double connection */
  g_signal_handlers_disconnect_by_func(source_item,
				       (GCallback) gc_item_focus_event,
				       target_item);

  /* connect source to target */
  g_signal_connect(source_item, "enter_notify_event",
		   (GCallback) gc_item_focus_event,
		   target_item);
  g_signal_connect(source_item, "leave_notify_event",
		   (GCallback) gc_item_focus_event,
		   target_item);
}

/**
 * gc_item_focus_remove
 *
 * Remove a previously set of item focus
 *
 * @param[in] source_item is the same as the one passed to
 *            gc_item_focus_init()
 * @param[in] target_itemis the same as the one passed to
 *            gc_item_focus_init()
 */
void gc_item_focus_remove(GooCanvasItem *source_item,
			  GooCanvasItem *target_item)
{
  GooCanvasItem *highlight_item;

  if(!target_item)
    target_item = source_item;

  g_signal_handlers_disconnect_by_func(source_item,
				       (GCallback) gc_item_focus_event,
				       target_item);

  highlight_item = g_object_get_data (G_OBJECT(target_item),
				      "highlight_item");

  if(highlight_item)
    goo_canvas_item_remove(highlight_item);

  g_object_set_data (G_OBJECT(target_item), "highlight_item",
		     NULL);
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

  //  goo_canvas_item_get_bounds( item, &x1, &y1, &x2, &y2 );
  /* WARNING: Do not use goo_canvas_item_get_bounds which gives unpredictable results */
  if(GOO_IS_CANVAS_POLYLINE(item)) {
    GooCanvasPoints	*points;
    g_object_get (item, "points", &points, NULL);
    x1 = points->coords[0];
    y1 = points->coords[1];
    x2 = points->coords[2];
    y2 = points->coords[3];
  } else if(GOO_IS_CANVAS_IMAGE(item)
	    || GOO_IS_CANVAS_RECT(item)) {
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
    g_assert(FALSE);
  }

  tx1 = x1;
  ty1 = y1;
  tx2 = x2;
  ty2 = y2;

  x1 = MIN(tx1,tx2);
  y1 = MIN(ty1,ty2);
  x2 = MAX(tx1,tx2);
  y2 = MAX(ty1,ty2);

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
  gchar			*dir_to_search[5];
  GcomprisProperties	*properties = gc_prop_get();

  if (!format)
    return NULL;

  va_start (args, format);
  filename = g_strdup_vprintf (format, args);
  va_end (args);

  /* Check it's already found */
  if( g_file_test (filename, G_FILE_TEST_EXISTS) )
    return filename;

  /*
   * Search it on the file system
   */

  if(properties->server)
  	dir_to_search[i++] = "";
  dir_to_search[i++] = properties->user_dir;
  dir_to_search[i++] = properties->package_data_dir;
  dir_to_search[i++] = properties->package_skin_dir;
  dir_to_search[i++] = NULL;

  absolute_filename = g_strdup(filename);
  i = 0;

  while (dir_to_search[i])
    {
      gchar **filesplit;
      gchar **locale;
      gchar *filename2;
      g_free(absolute_filename);

      /* Maybe there is a $LOCALE to replace */
      filesplit = g_strsplit(filename, "$LOCALE", -1);
      if(g_strv_length(filesplit) == 1)
	{
	  g_strfreev(filesplit);
	  absolute_filename = \
	    g_strdup_printf("%s/%s", dir_to_search[i], filename);
	  if(g_file_test (absolute_filename, G_FILE_TEST_EXISTS))
	    goto FOUND;
	}
      else
	{
	  /* First try with the long locale */
	  locale = g_strsplit_set(gc_locale_get(), ".", 2);
	  filename2 = g_strjoinv(locale[0], filesplit);
	  absolute_filename = g_strdup_printf("%s/%s", dir_to_search[i],
					      filename2);
	  g_strfreev(locale);
	  if(g_file_test (absolute_filename, G_FILE_TEST_EXISTS))
	    {
	      g_strfreev(filesplit);
	      g_free(filename2);
	      goto FOUND;
	    }
	  g_free(filename2);
	  /* Try the short locale */
	  if(g_strv_length(filesplit)>1)
	    {
	      locale = g_strsplit_set(gc_locale_get(), "_", 2);
	      if(g_strv_length(locale) < 1)
		goto NOT_FOUND;
	      filename2 = g_strjoinv(locale[0], filesplit);
	      g_strfreev(locale);
	      g_strfreev(filesplit);
	      g_free(absolute_filename);
	      absolute_filename = g_strdup_printf("%s/%s", dir_to_search[i],
						  filename2);
	      if(g_file_test (absolute_filename, G_FILE_TEST_EXISTS))
		{
		  g_free(filename2);
		  goto FOUND;
		}
	      g_free(filename2);

	    }
	  else
	    g_strfreev(filesplit);
	}

      i++;
    }

 NOT_FOUND:
  g_debug("absolute_filename '%s' NOT FOUND\n", filename);
  g_free(filename);
  g_free(absolute_filename);
  return NULL;

 FOUND:
  g_free(filename);
  //  printf("absolute_filename=%s\n", absolute_filename);
  return absolute_filename;
}

gchar*
gc_file_find_absolute_writeable(const gchar *format, ...)
{
  gchar *filename, *absolute_filename, *dirname;
  GcomprisProperties *prop;
  va_list args;

  va_start (args, format);
  filename = g_strdup_vprintf (format, args);
  va_end (args);

  prop = gc_prop_get();
  absolute_filename = g_build_filename(prop->user_dir, filename,NULL);
  g_free(filename);
  dirname = g_path_get_dirname(absolute_filename);
  if(!g_file_test(dirname, G_FILE_TEST_IS_DIR))
    {
      if(g_mkdir_with_parents(dirname, 0755))
      {
      	g_free(absolute_filename);
	absolute_filename=NULL;
      }
    }
  g_free(dirname);
  if(absolute_filename)
  	gc_cache_add(absolute_filename);
  return absolute_filename;
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


/** Play the activity intro voice
 *
 * \param gcomprisboard
 *
 * return void
 */
void
gc_activity_intro_play (GcomprisBoard *gcomprisBoard)
{
  gchar *str;

  str = gc_file_find_absolute("voices/$LOCALE/intro/%s.ogg",
			      gcomprisBoard->name, NULL);
  gc_sound_play_ogg(str, NULL);
  g_free(str);
}

/** Display a button with the given text
 *
 * \param x the x coordinate of the button
 * \param y the y coordinate of the button
 * \param button_file the image file to use as the button
 * \param text is the text to display in the button
 * \param process is the callback function
 * \param data is the user data passed to the callback function
 *
 * \return void
 */
void
gc_util_button_text(GooCanvasItem *rootitem,
		    guint x, guint y,
		    char *button_file,
		    char *text,
		    GCallback process,
		    gpointer data)
{
  GdkPixbuf *pixmap;
  GooCanvasItem *item;
  GooCanvasItem *item_text;

  /* The Button */
  pixmap = gc_skin_pixmap_load(button_file);
  item = goo_canvas_image_new (rootitem,
			       pixmap,
			       x,
			       y,
			       NULL);
  g_signal_connect(item,
		   "button_press_event",
		   (GCallback) process, data);
  gc_item_focus_init(item, NULL);

  /* The Text */
  item_text =
    goo_canvas_text_new (rootitem,
			 text,
			 x + (double)gdk_pixbuf_get_width(pixmap)/2,
			 y + 24,
			 -1,
			 GTK_ANCHOR_CENTER,
			 "font", gc_skin_font_board_small,
			 "fill_color_rgba", gc_skin_color_text_button,
			 NULL);
  g_signal_connect(item_text,
		   "button_press_event",
		   process, data);
  gc_item_focus_init(item_text, item);
#if GDK_PIXBUF_MAJOR <= 2 && GDK_PIXBUF_MINOR <= 24
  gdk_pixbuf_unref(pixmap);
#else
  g_object_unref(pixmap);
#endif
}

/** Display a button with the given text (BY SKIN'S SVG ID)
 *
 * \param x the x coordinate of the button
 * \param y the y coordinate of the button
 * \param button_file the image file to use as the button
 * \param text is the text to display in the button
 * \param process is the callback function
 * \param data is the user data passed to the callback function
 *
 * \return void
 */
void
gc_util_button_text_svg(GooCanvasItem *rootitem,
			guint x, guint y,
			char *button_id,
			char *text,
			GCallback process,
			gpointer data)
{
  GooCanvasItem *item;
  GooCanvasItem *item_text;

  /* The Button */
  item = goo_canvas_svg_new (rootitem,
			     gc_skin_rsvg_get(),
			     "svg-id", button_id,
			     NULL);
  SET_ITEM_LOCATION_CENTER(item, x, y);

  g_signal_connect(item,
		   "button_press_event",
		   (GCallback) process, data);
  gc_item_focus_init(item, NULL);

  /* The Text */
  item_text =
    goo_canvas_text_new (rootitem,
			 text,
			 x,
			 y,
			 -1,
			 GTK_ANCHOR_CENTER,
			 "font", gc_skin_font_board_small,
			 "fill_color_rgba", gc_skin_color_text_button,
			 NULL);
  g_signal_connect(item_text,
		   "button_press_event",
		   process, data);
  gc_item_focus_init(item_text, item);
}
