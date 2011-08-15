/*
 * money_widget.h
 *
 * Copyright (C) 2002, 2008 Bruno Coudoin
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

#ifndef _MONEY_WIDGET_H_
#define _MONEY_WIDGET_H_

#include <gtk/gtk.h>
#include "gcompris/gcompris.h"

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */

#define TYPE_MONEY_WIDGET	   (money_widget_get_type ())
#define MONEY_WIDGET(obj)          (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_MONEY_WIDGET, MoneyWidget))
#define MONEY_WIDGET_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_MONEY_WIDGET, MoneyWidgetClass))
#define IS_MONEY_WIDGET(obj)       (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_MONEY_WIDGET))

typedef struct _MoneyWidget        MoneyWidget;
typedef struct _MoneyWidgetClass   MoneyWidgetClass;
typedef struct _MoneyWidgetPrivate MoneyWidgetPrivate;

struct _MoneyWidgetClass {
	GObjectClass parent_class;
};

struct _MoneyWidget {
	GObject        parent_instance;

	MoneyWidgetPrivate *priv;
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


GType          money_widget_get_type             (void);
GObject       *money_widget_new                  (void);
MoneyWidget   *money_widget_copy                 (MoneyWidget *moneyWidget);

void	       money_widget_set_target		 (MoneyWidget *moneyWidget,
					          MoneyWidget *targetWidget);
void           money_widget_set_position         (MoneyWidget *moneyWidget,
						  GooCanvasItem *rootItem,
						  double x1,
						  double y1,
						  double x2,
						  double y2,
						  guint  colomns,
						  guint  lines,
						  gboolean display_total);
void           money_widget_add		        (MoneyWidget *moneyWidget, MoneyEuroType value);
void           money_widget_remove	        (MoneyWidget *moneyWidget, MoneyEuroType value);

/* Misc. accessors */
float          money_widget_get_total           (MoneyWidget *moneyWidget);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MONEY_WIDGET_H_ */
