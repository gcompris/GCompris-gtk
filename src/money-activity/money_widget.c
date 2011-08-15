/*
 * money_widget.c
 *
 * Copyright (C) 2002, 2008  Robert Wilhelm
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
 *
 * Authors: Bruno Coudoin
 *
 */

#include <stdlib.h>
#include <string.h>
#include "money_widget.h"

/* From money.c */
void	 moneyactivity_process_ok(void);

struct _MoneyWidgetPrivate {
  GooCanvasItem	*rootItem;	/* The canvas to display our euros in             */
  double		 x1;		/* Coordinate of the widget                       */
  double		 y1;
  double		 x2;
  double		 y2;
  guint			 columns;	/* Number of columns				  */
  guint			 lines;		/* Number of lines				  */
  guint			 next_spot;	/* Next spot to display a money item		  */
  float			 total;		/* The number of euro in this pocket              */
  MoneyWidget		*targetWidget;	/* Target money widget to add when remove here	  */
  gboolean		 display_total;	/* Display or not the total of this pocket        */

  GooCanvasItem		*item_total;	/* Item to display the total                      */
  GList			*moneyItemList;	/* List of all the items			  */
};


typedef struct _MoneyList        MoneyList;

struct _MoneyList {
  const gchar		*image;
  const double		 value;
};

// List of images to use in the game
static const MoneyList euroList[] =
{
  { "money/c1c.svgz",		0.01  },
  { "money/c2c.svgz",		0.02  },
  { "money/c5c.svgz",		0.05  },
  { "money/c10c.svgz",		0.1   },
  { "money/c20c.svgz",		0.20  },
  { "money/c50c.svgz",		0.5   },
  { "money/c1e.svgz",		1.0   },
  { "money/c2e.svgz",		2.0   },
  { "money/n5e.svgz",		5.0   },
  { "money/n10e.svgz",		10.0  },
  { "money/n20e.svgz",		20.0  },
  { "money/n50e.svgz",		50.0  },
  { NULL,			-1.0  }
};

typedef struct {
  MoneyWidget		*moneyWidget;
  GooCanvasItem		*item;
  MoneyEuroType		 value;
  gboolean		 inPocket;
} MoneyItem;

#define BORDER_GAP	6

/* Prototypes */
static void money_display_total(MoneyWidget *moneyWidget);
static gboolean item_event (GooCanvasItem  *item,
			    GooCanvasItem  *target,
			    GdkEventButton *event,
			    MoneyItem *moneyItem);

#define MONEY_WIDGET_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_MONEY_WIDGET, MoneyWidgetPrivate))


G_DEFINE_TYPE(MoneyWidget, money_widget, G_TYPE_OBJECT);

#if 0
static void
finalize (GObject *object)
{
  MoneyItem	  *moneyitem;

  MoneyWidget *moneyWidget = (MoneyWidget *) object;

  printf("ERROR : Finalize is NEVER CALLED\n");
  /* FIXME: CLEANUP CODE UNTESTED */
  while(g_list_length(moneyWidget->priv->moneyItemList)>0)
    {
      moneyitem = g_list_nth_data(moneyWidget->priv->moneyItemList, 0);
      moneyWidget->priv->moneyItemList = g_list_remove (moneyWidget->priv->moneyItemList,
							moneyitem);
      g_free(moneyitem);
    }

  g_free (moneyWidget->priv);

  moneyWidget->priv = NULL;

}
#endif

static GObject *
money_widget_constructor (GType                  gtype,
			  guint                  n_properties,
			  GObjectConstructParam *properties)
{
  GObject *obj;

  {
    /* Always chain up to the parent constructor */
    obj = G_OBJECT_CLASS (money_widget_parent_class)->constructor (gtype, n_properties, properties);
  }

  /* update the object state depending on constructor properties */

  return obj;
}

static void
money_widget_class_init (MoneyWidgetClass *klass)
{
  g_type_class_add_private (klass, sizeof (MoneyWidgetPrivate));
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->constructor = money_widget_constructor;

}

static void
money_widget_init (MoneyWidget *self)
{
  MoneyWidgetPrivate *priv;
  self->priv = priv = MONEY_WIDGET_GET_PRIVATE (self);

  priv->total = 0;

}

GObject *
money_widget_new ()
{
  return G_OBJECT (g_object_new (money_widget_get_type (), NULL));
}

MoneyWidget *
money_widget_copy (MoneyWidget *pos)
{
	MoneyWidget *cpPos;

	cpPos = MONEY_WIDGET (money_widget_new ());

	memcpy (cpPos->priv, pos->priv, sizeof (MoneyWidgetPrivate));

	return cpPos;
}

void
money_widget_set_target (MoneyWidget *moneyWidget,
			 MoneyWidget *targetWidget)
{
  moneyWidget->priv->targetWidget = targetWidget;
}

void
money_widget_set_position (MoneyWidget *moneyWidget,
			   GooCanvasItem *rootItem,
			   double x1,
			   double y1,
			   double x2,
			   double y2,
			   guint  columns,
			   guint  lines,
			   gboolean display_total)
{
  g_return_if_fail (moneyWidget != NULL);

  moneyWidget->priv->rootItem = rootItem;
  moneyWidget->priv->x1 = x1;
  moneyWidget->priv->y1 = y1;
  moneyWidget->priv->x2 = x2;
  moneyWidget->priv->y2 = y2;
  moneyWidget->priv->columns = columns;
  moneyWidget->priv->lines = lines;
  moneyWidget->priv->next_spot = 0;
  moneyWidget->priv->display_total = display_total;

  /* Uncomment to display the limits
    goo_canvas_item_new (GOO_CANVAS_GROUP(rootItem),
			 goo_canvas_rect_get_type (),
			 "x1", (double) x1,
			 "y1", (double) y1,
			 "x2", (double) x2,
			 "y2", (double) y2,
			 "stroke-color", "red",
			 "width_pixels", 2,
			 NULL);
  */

  moneyWidget->priv->item_total =  \
    goo_canvas_text_new(rootItem,
			"",
			x1+(x2-x1)/2,
			y2 + 10,
			-1,
			GTK_ANCHOR_CENTER,
			"font", gc_skin_font_board_big,
			"fill-color", "white",
			NULL);

}

static void money_display_total(MoneyWidget *moneyWidget)
{
  gchar *tmpstr;
  g_return_if_fail (moneyWidget != NULL);

  tmpstr = g_strdup_printf("%.2f", moneyWidget->priv->total);
  if(moneyWidget->priv->display_total)
    g_object_set (moneyWidget->priv->item_total,
			   "text", tmpstr,
			   NULL);
  g_free(tmpstr);

}

void
money_widget_add (MoneyWidget *moneyWidget, MoneyEuroType value)
{
  GooCanvasItem *item;
  RsvgHandle    *svg_handle;
  RsvgDimensionData dimension;
  double	 xratio, yratio, ratio;
  double	 block_width, block_height;
  MoneyItem	*moneyitem;
  guint		 i, length;

  g_return_if_fail (moneyWidget != NULL);

  /* Search for an hidden item with the same value */
  length = g_list_length(moneyWidget->priv->moneyItemList);
  for(i=0; i<length; i++)
    {
      moneyitem = \
	(MoneyItem *)g_list_nth_data(moneyWidget->priv->moneyItemList,
				     i);

      if(moneyitem && !moneyitem->inPocket && moneyitem->value == value)
	{
	  g_object_set (moneyitem->item,
			"visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
	  moneyitem->inPocket = TRUE;
	  moneyWidget->priv->total += euroList[value].value;
	  money_display_total(moneyWidget);
	  return;
	}
    }

  /* There is no already suitable item create, create a new one */

  if(moneyWidget->priv->next_spot
     > moneyWidget->priv->columns * moneyWidget->priv->lines)
    g_message("More money items requested than the pocket size\n");

  block_width  = \
    (moneyWidget->priv->x2 - moneyWidget->priv->x1)
    / moneyWidget->priv->columns;
  block_height = \
    (moneyWidget->priv->y2 - moneyWidget->priv->y1) / moneyWidget->priv->lines;

  svg_handle = gc_rsvg_load(euroList[value].image);

  rsvg_handle_get_dimensions(svg_handle, &dimension);

  xratio =  block_width  / (dimension.width + BORDER_GAP);
  yratio =  block_height / (dimension.height + BORDER_GAP);
  ratio = yratio = MIN(xratio, yratio);

  item =  goo_canvas_svg_new(moneyWidget->priv->rootItem,
			   svg_handle,
			   NULL);

  goo_canvas_item_translate(item,
			    moneyWidget->priv->x1 +
			    (moneyWidget->priv->next_spot % moneyWidget->priv->columns) * block_width
			    +  block_width/2 - (dimension.width * ratio)/2,
			    moneyWidget->priv->y1 +
			    (moneyWidget->priv->next_spot / moneyWidget->priv->columns)
			    * (block_height + BORDER_GAP)
			    + block_height/2 - (dimension.height * ratio)/2);
  goo_canvas_item_scale(item,
			ratio,
  			ratio);

  moneyitem = g_malloc(sizeof(MoneyItem));
  moneyitem->moneyWidget = moneyWidget;
  moneyitem->item   = item;
  moneyitem->value  = value;
  moneyitem->inPocket = TRUE;

  moneyWidget->priv->moneyItemList = \
    g_list_append (moneyWidget->priv->moneyItemList,
		   moneyitem);

  g_signal_connect(item,
		   "button_press_event", (GCallback) item_event,
		   moneyitem);

  g_object_unref(svg_handle);

  moneyWidget->priv->next_spot++;

  moneyWidget->priv->total += euroList[value].value;

  money_display_total(moneyWidget);
}

void
money_widget_remove(MoneyWidget *moneyWidget, MoneyEuroType value)
{
  g_return_if_fail (moneyWidget != NULL);

  moneyWidget->priv->total -= euroList[value].value;

  money_display_total(moneyWidget);
}

float
money_widget_get_total (MoneyWidget *moneyWidget)
{
  if(moneyWidget == NULL)
    return 0;

  return moneyWidget->priv->total;
}


static gboolean
item_event (GooCanvasItem  *item,
	    GooCanvasItem  *target,
	    GdkEventButton *event,
	    MoneyItem *moneyItem)
{

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      switch(event->button)
	{
	case 1:
	  g_object_set (item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
	  moneyItem->inPocket = FALSE;
	  money_widget_remove(moneyItem->moneyWidget, moneyItem->value);

	  if(moneyItem->moneyWidget->priv->targetWidget != NULL)
	    money_widget_add(moneyItem->moneyWidget->priv->targetWidget,
			     moneyItem->value);

	  break;
	default:
	  break;
	}
    default:
      break;
    }
  return FALSE;
}
