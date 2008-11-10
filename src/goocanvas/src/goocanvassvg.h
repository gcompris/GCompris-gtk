/*
 * GooCanvas Demo. Copyright (C) 2006 Damon Chaplin.
 * Released under the GNU LGPL license. See COPYING for details.
 *
 * goocanvassvg.c - a simple svg item.
 */
#ifndef __GOO_CANVAS_SVG_H__
#define __GOO_CANVAS_SVG_H__

#include <gtk/gtk.h>
#include "goocanvasitemsimple.h"
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>

G_BEGIN_DECLS


#define GOO_TYPE_CANVAS_SVG            (goo_canvas_svg_get_type ())
#define GOO_CANVAS_SVG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_CANVAS_SVG, GooCanvasSvg))
#define GOO_CANVAS_SVG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_CANVAS_SVG, GooCanvasSvgClass))
#define GOO_IS_CANVAS_SVG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_CANVAS_SVG))
#define GOO_IS_CANVAS_SVG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_CANVAS_SVG))
#define GOO_CANVAS_SVG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_CANVAS_SVG, GooCanvasSvgClass))


typedef struct _GooCanvasSvg       GooCanvasSvg;
typedef struct _GooCanvasSvgClass  GooCanvasSvgClass;

struct _GooCanvasSvg
{
  GooCanvasItemSimple parent_object;

  gdouble width, height;
  gchar *id;
  cairo_t *cr;
  cairo_pattern_t *pattern;
};

struct _GooCanvasSvgClass
{
  GooCanvasItemSimpleClass parent_class;
};


GType               goo_canvas_svg_get_type  (void) G_GNUC_CONST;

GooCanvasItem*      goo_canvas_svg_new       (GooCanvasItem      *parent,
					      RsvgHandle         *svg_handle,
					      ...);


G_END_DECLS

#endif /* __GOO_CANVAS_SVG_H__ */
