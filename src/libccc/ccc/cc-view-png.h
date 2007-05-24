/* This file is part of libccc, criawips' cairo based canvas
 *
 * AUTHORS
 *     Sven Herzberg  <herzi@gnome-de.org>
 *
 * Copyright (C) 2006  Sven Herzberg <herzi@gnome-de.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifndef CC_VIEW_PNG_H
#define CC_VIEW_PNG_H

#include <ccc/cc-view.h>

G_BEGIN_DECLS

typedef struct _CcViewPNG      CcViewPNG;
typedef struct _CcViewPNGClass CcViewPNGClass;

#define CC_TYPE_VIEW_PNG         (cc_view_png_get_type())
#define CC_VIEW_PNG(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_VIEW_PNG, CcViewPNG))
#define CC_VIEW_PNG_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_VIEW_PNG, CcViewPNGClass))
#define CC_IS_VIEW_PNG(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_VIEW_PNG))
#define CC_IS_VIEW_PNG_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_VIEW_PNG))
#define CC_VIEW_PNG_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_VIEW_PNG, CcViewPNGClass))

GType   cc_view_png_get_type(void);

CcView* cc_view_png_new(gchar const* filename);

struct _CcViewPNG {
	GInitiallyUnowned          base_instance;

	gchar          * filename;
	cairo_surface_t* surface;
};

struct _CcViewPNGClass {
	GInitiallyUnownedClass base_class;
};

G_END_DECLS

#endif /* !CC_VIEW_PNG_H */
