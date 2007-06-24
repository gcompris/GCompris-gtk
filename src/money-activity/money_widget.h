/*
 * money_pocket.h
 *
 * Copyright (C) 2002 Bruno Coudoin
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors: Bruno Coudoin
 *
 */

#ifndef _MONEY_WIDGET_H_
#define _MONEY_WIDGET_H_

#include <gtk/gtk.h>
#include "gcompris/gcompris.h"

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */

#define TYPE_MONEY_WIDGET	   (money_widget_get_type ())
#define MONEY_WIDGET(obj)          GTK_CHECK_CAST (obj, TYPE_MONEY_WIDGET, Money_Widget)
#define MONEY_WIDGET_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, TYPE_MONEY_WIDGET, Money_WidgetClass)
#define IS_MONEY_WIDGET(obj)       GTK_CHECK_TYPE (obj, TYPE_MONEY_WIDGET)

typedef struct _Money_Widget        Money_Widget;
typedef struct _Money_WidgetClass   Money_WidgetClass;
typedef struct _Money_WidgetPrivate Money_WidgetPrivate;

struct _Money_WidgetClass {
	GtkObjectClass parent_class;
};

struct _Money_Widget {
	GtkObject        object;

	Money_WidgetPrivate *priv;
};

typedef enum {
  MONEY_EURO_COIN_1C,
  MONEY_EURO_COIN_2C,
  MONEY_EURO_COIN_5C,
  MONEY_EURO_COIN_10C,
  MONEY_EURO_COIN_20C,
  MONEY_EURO_COIN_50C,
  MONEY_EURO_COIN_1E,
  MONEY_EURO_COIN_2E,
  MONEY_EURO_PAPER_5E,
  MONEY_EURO_PAPER_10E,
  MONEY_EURO_PAPER_20E,
  MONEY_EURO_PAPER_50E,
} MoneyEuroType;


GtkType        money_widget_get_type             (void);    
GtkObject     *money_widget_new                  (void);
Money_Widget  *money_widget_copy                 (Money_Widget *moneyWidget);

void	       money_widget_set_target		 (Money_Widget *moneyWidget,
					          Money_Widget *targetWidget);
void           money_widget_set_position         (Money_Widget *moneyWidget,
						  GnomeCanvasGroup *rootItem,
						  double x1,
						  double y1,
						  double x2,
						  double y2,
						  guint  colomns,
						  guint  lines,
						  gboolean display_total);
void           money_widget_add		        (Money_Widget *moneyWidget, MoneyEuroType value);
void           money_widget_remove	        (Money_Widget *moneyWidget, MoneyEuroType value);

/* Misc. accessors */
double         money_widget_get_total           (Money_Widget *moneyWidget);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MONEY_WIDGET_H_ */
