/*
 * GooCanvas Demo. Copyright (C) 2006 Damon Chaplin.
 * Released under the GNU LGPL license. See COPYING for details.
 *
 * svg-item.c - a simple svg item.
 */
#include "goocanvas.h"
#include "svg-item.h"

enum {
  PROP_0,

  /* Convenience properties. */
  PROP_SVGHANDLE
};

static void goo_svg_item_set_property (GObject            *object,
				       guint               param_id,
				       const GValue       *value,
				       GParamSpec         *pspec);


/* Use the GLib convenience macro to define the type. GooSvgItem is the
   class struct, goo_svg_item is the function prefix, and our class is a
   subclass of GOO_TYPE_CANVAS_ITEM_SIMPLE. */
G_DEFINE_TYPE (GooSvgItem, goo_svg_item, GOO_TYPE_CANVAS_ITEM_SIMPLE)

static void
goo_svg_item_install_common_properties (GObjectClass *gobject_class)
{
  g_object_class_install_property (gobject_class, PROP_SVGHANDLE,
				   g_param_spec_object ("rsvg-handle",
							"Rsvg Handle",
							"The RsvgHandle to display",
							RSVG_TYPE_HANDLE,
							G_PARAM_WRITABLE));

}

/* The standard object initialization function. */
static void
goo_svg_item_init (GooSvgItem *svg_item)
{
  svg_item->width = 0.0;
  svg_item->height = 0.0;
}


/* The convenience function to create new items. This should start with a
   parent argument and end with a variable list of object properties to fit
   in with the standard canvas items. */
GooCanvasItem*
goo_svg_item_new (GooCanvasItem      *parent,
		  RsvgHandle         *svg_handle,
		  ...)
{
  GooCanvasItem *item;
  GooSvgItem *svg_item;
  const char *first_property;
  va_list var_args;
  RsvgDimensionData dimension_data;

  item = g_object_new (GOO_TYPE_SVG_ITEM, NULL);

  svg_item = (GooSvgItem*) item;
  svg_item->svg_handle = svg_handle;
  if(svg_handle)
    {
      g_object_ref (svg_handle);
      rsvg_handle_get_dimensions (svg_handle, &dimension_data);
      svg_item->width = dimension_data.width;
      svg_item->height = dimension_data.height;
    }

  va_start (var_args, svg_handle);
  first_property = va_arg (var_args, char*);
  if (first_property)
    g_object_set_valist ((GObject*) item, first_property, var_args);
  va_end (var_args);

  if (parent)
    {
      goo_canvas_item_add_child (parent, item, -1);
      g_object_unref (item);
    }

  return item;
}


/* The update method. This is called when the canvas is initially shown and
   also whenever the object is updated and needs to change its size and/or
   shape. It should calculate its new bounds in its own coordinate space,
   storing them in simple->bounds. */
static void
goo_svg_item_update  (GooCanvasItemSimple *simple,
		      cairo_t             *cr)
{
  GooSvgItem *svg_item = (GooSvgItem*) simple;

  /* Compute the new bounds. */
  simple->bounds.x1 = 0;
  simple->bounds.y1 = 0;
  simple->bounds.x2 = svg_item->width;
  simple->bounds.y2 = svg_item->height;
}


/* The paint method. This should draw the item on the given cairo_t, using
   the item's own coordinate space. */
static void
goo_svg_item_paint (GooCanvasItemSimple   *simple,
		     cairo_t               *cr,
		     const GooCanvasBounds *bounds)
{
  GooSvgItem *svg_item = (GooSvgItem*) simple;

  if(svg_item->svg_handle)
    rsvg_handle_render_cairo (svg_item->svg_handle, cr);
}


static void
goo_svg_item_set_common_property (GObject              *object,
				  GooSvgItem           *svg_item,
				  guint                 prop_id,
				  const GValue         *value,
				  GParamSpec           *pspec)
{
  RsvgHandle *svg_handle;
  RsvgDimensionData dimension_data;

  switch (prop_id)
    {
    case PROP_SVGHANDLE:
      svg_handle = g_value_get_object (value);
      if(svg_item->svg_handle)
	g_object_unref (svg_item->svg_handle);
      g_object_ref (svg_handle);
      svg_item->svg_handle = svg_handle;
      rsvg_handle_get_dimensions (svg_handle, &dimension_data);
      svg_item->width = dimension_data.width;
      svg_item->height = dimension_data.height;
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static void
goo_svg_item_set_property (GObject              *object,
			   guint                 prop_id,
			   const GValue         *value,
			   GParamSpec           *pspec)
{
  GooCanvasItemSimple *simple = (GooCanvasItemSimple*) object;
  GooSvgItem *image = (GooSvgItem*) object;

  if (simple->model)
    {
      g_warning ("Can't set property of a canvas item with a model - set the model property instead");
      return;
    }

  goo_svg_item_set_common_property (object, image, prop_id,
				    value, pspec);
  goo_canvas_item_simple_changed (simple, TRUE);
}



/* Hit detection. This should check if the given coordinate (in the item's
   coordinate space) is within the item. If it is it should return TRUE,
   otherwise it should return FALSE. */
static gboolean
goo_svg_item_is_item_at (GooCanvasItemSimple *simple,
			  gdouble              x,
			  gdouble              y,
			  cairo_t             *cr,
			  gboolean             is_pointer_event)
{
  /* Don't do hit-detection for now. */
  return TRUE;
}


/* The class initialization function. Here we set the class' update(), paint()
   and is_item_at() methods. */
static void
goo_svg_item_class_init (GooSvgItemClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass*) klass;
  GooCanvasItemSimpleClass *simple_class = (GooCanvasItemSimpleClass*) klass;

  gobject_class->set_property = goo_svg_item_set_property;

  simple_class->simple_update        = goo_svg_item_update;
  simple_class->simple_paint         = goo_svg_item_paint;
  simple_class->simple_is_item_at    = goo_svg_item_is_item_at;

  goo_svg_item_install_common_properties (gobject_class);

}


