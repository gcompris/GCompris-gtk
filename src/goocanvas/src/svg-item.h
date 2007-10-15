/*
 * GooCanvas Demo. Copyright (C) 2006 Damon Chaplin.
 * Released under the GNU LGPL license. See COPYING for details.
 *
 * svg-item.c - a simple svg item.
 */
#ifndef __GOO_SVG_ITEM_H__
#define __GOO_SVG_ITEM_H__

#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>
#include <gtk/gtk.h>
#include "goocanvasitemsimple.h"

G_BEGIN_DECLS


#define GOO_TYPE_SVG_ITEM            (goo_svg_item_get_type ())
#define GOO_SVG_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOO_TYPE_SVG_ITEM, GooSvgItem))
#define GOO_SVG_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GOO_TYPE_SVG_ITEM, GooSvgItemClass))
#define GOO_IS_SVG_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOO_TYPE_SVG_ITEM))
#define GOO_IS_SVG_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GOO_TYPE_SVG_ITEM))
#define GOO_SVG_ITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GOO_TYPE_SVG_ITEM, GooSvgItemClass))


typedef struct _GooSvgItem       GooSvgItem;
typedef struct _GooSvgItemClass  GooSvgItemClass;

struct _GooSvgItem
{
  GooCanvasItemSimple parent_object;

  RsvgHandle *svg_handle;
  gdouble width, height;
};

struct _GooSvgItemClass
{
  GooCanvasItemSimpleClass parent_class;
};


GType               goo_svg_item_get_type  (void) G_GNUC_CONST;

GooCanvasItem*      goo_svg_item_new       (GooCanvasItem      *parent,
					    RsvgHandle         *svg_handle,
					    ...);


G_END_DECLS

#endif /* __GOO_SVG_ITEM_H__ */
