/* this file is part of libccc, criawips' cairo-based canvas
 *
 * AUTHORS
 *       Sven Herzberg        <herzi@gnome-de.org>
 *
 * Copyright (C) 2005 Sven Herzberg
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

#ifndef CC_TEXT_H
#define CC_TEXT_H

#include <ccc/cc-shape.h>
#include <pango/pango-layout.h>
#include <gtk/gtkenums.h>
#include <gtk/gtkimcontext.h>

G_BEGIN_DECLS

typedef struct _CcText      CcText;
typedef struct _CcTextClass CcTextClass;

#define CC_TYPE_TEXT         (cc_text_get_type())
#define CC_TEXT(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_TEXT, CcText))
#define CC_TEXT_CLASS(c)     (G_TYPE_ECHCK_CLASS_CAST((c), CC_TYPE_TEXT, CcTextClass))
#define CC_IS_TEXT(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_TEXT))
#define CC_IS_TEXT_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_TEXT))
#define CC_TEXT_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_TEXT, CcTextClass))

GType cc_text_get_type(void);

CcItem*      cc_text_new                 (gchar const         * text);
gchar const* cc_text_get_text            (CcText const        * self);
gboolean     cc_text_is_editable         (CcText const        * self);
void         cc_text_set_anchor          (CcText              * self,
			                  gdouble               x,
			                  gdouble               y);
void         cc_text_set_anchor_type     (CcText              * self,
				          GtkAnchorType         anchor);
void         cc_text_set_editable        (CcText              * self,
				          gboolean              editable);
void         cc_text_set_font_description(CcText              * self,
				          PangoFontDescription* desc);
void         cc_text_set_markup          (CcText              * self,
				          gchar const         * markup);
void         cc_text_set_size_pixels     (CcText              * self,
				          gboolean              size_pixels);
void         cc_text_set_text            (CcText              * self,
				          gchar const         * text);

struct _CcText {
	CcShape        base_instance;

	GtkAnchorType  anchor;
	gdouble        x;
	gdouble        y;

	gboolean       size_pixels;

	PangoLayout         * layout;
	PangoFontDescription* font_description;

	gboolean       editable;
	gint           cursor,
		       cursor_trail;
	CcHashMap    * im_contexts;
};

struct _CcTextClass {
	CcShapeClass base_class;

	/* vtable */
	void (*remove) (CcText     * self,
			guint        offset,
			gsize        length);
	void (*insert) (CcText     * self,
			gchar const* text,
			gsize        offset);
};

G_END_DECLS

#endif /* !CC_TEXT_H */

