/* this file is part of libccc
 *
 * AUTHORS
 *       Sven Herzberg        <herzi@gnome-de.org>
 *
 * Copyright (C) 2005,2007 Sven Herzberg
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

#include <ccc/cc-text.h>

#include <stdlib.h>
#include <string.h>
#include <pango/pangocairo.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <ccc/cc-brush-color.h>
#include <ccc/cc-caret.h>
#include <ccc/cc-line.h>
#include <ccc/cc-utils.h>

#include "glib-helpers.h"
#include "pango-helpers.h"

#define CDEBUG_TYPE cc_text_get_type
#include <cdebug.h>

#include <config.h>
#include <glib/gi18n-lib.h>

static GQuark ct_quark = 0;

struct CcTextPrivate {
	CcItem* caret;
};
#define P(i) (G_TYPE_INSTANCE_GET_PRIVATE((i), CC_TYPE_TEXT, struct CcTextPrivate))

typedef struct {
	GtkIMContext* im_context;
} CcTextViewData;
#define V(i,v) (CC_ITEM_GET_VIEW_DATA ((i), (v), CC_TYPE_TEXT, CcTextViewData))

CcItem*
cc_text_new(gchar const* text) {
	return g_object_new(CC_TYPE_TEXT, "text", text, NULL);
}

gchar const*
cc_text_get_text(CcText const* self) {
	g_return_val_if_fail(CC_IS_TEXT(self), "");

	return pango_layout_get_text(self->layout);
}

gboolean
cc_text_is_editable(CcText const* self) {
	g_return_val_if_fail(CC_IS_TEXT(self), FALSE);

	return self->editable;
}

static gdouble
ct_get_x_offset(CcText* self, gdouble x, gdouble width) {
	switch(self->anchor) {
	case GTK_ANCHOR_NE:
	case GTK_ANCHOR_E:
	case GTK_ANCHOR_SE:
		return x - width;
	case GTK_ANCHOR_N:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_S:
		return x - 0.5 * width;
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_W:
	case GTK_ANCHOR_SW:
	default:
		return x;
	}
}

static gdouble
ct_get_y_offset(CcText* self, gdouble y, gdouble height) {
	switch(self->anchor) {
	case GTK_ANCHOR_SW:
	case GTK_ANCHOR_S:
	case GTK_ANCHOR_SE:
		return y - height;
	case GTK_ANCHOR_W:
	case GTK_ANCHOR_CENTER:
	case GTK_ANCHOR_E:
		return y - 0.5 * height;
	case GTK_ANCHOR_NW:
	case GTK_ANCHOR_N:
	case GTK_ANCHOR_NE:
	default:
		return y;
	}
}

/**
 * cc_text_set_anchor:
 * @self: a #CcText
 * @x: the horizontal position of the anchor point
 * @y: the vertical position of the anchor point
 *
 * Specify the location of the anchor point of the text item.
 */
void
cc_text_set_anchor(CcText * self,
		   gdouble  x,
		   gdouble  y)
{
	g_return_if_fail(CC_IS_TEXT(self));

	// FIXME: add some necessity check

	self->x = x;
	self->y = y;

	cc_item_update_bounds(CC_ITEM(self), NULL);
	// FIXME: add some notification
}

/**
 * cc_text_set_anchor_type:
 * @self: a #CcText
 * @anchor: the anchor type to be specified
 *
 * Set the anchor type of the text item. It specifies the location of the
 * anchor point relative to the text (so that north-west would become the
 * top-left corner).
 */
void
cc_text_set_anchor_type(CcText       * self,
			GtkAnchorType  anchor)
{
	g_return_if_fail(CC_IS_TEXT(self));
	// FIXME: check enum value

	if(self->anchor == anchor) {
		return;
	}

	self->anchor = anchor;
	cc_item_update_bounds(CC_ITEM(self), NULL);

	g_object_notify(G_OBJECT(self), "anchor");
}

void
cc_text_set_attributes(CcText       * self,
		       PangoAttrList* attrs)
{
	g_return_if_fail(CC_IS_TEXT(self));

	cdebug("set_attributes()", "pango_layout_set_attributes()");
	pango_layout_set_attributes(self->layout, attrs);
	cc_item_update_bounds(CC_ITEM(self), NULL);
	// FIXME: mark dirty

	g_object_notify(G_OBJECT(self), "attributes");

}

void
cc_text_set_editable(CcText* self, gboolean editable) {
	g_return_if_fail(CC_IS_TEXT(self));

	if(self->editable == editable) {
		return;
	}

	self->editable = editable;

	cc_item_update_bounds(CC_ITEM(self), NULL);
	g_object_notify(G_OBJECT(self), "editable");
}

/**
 * cc_text_set_font_description:
 * @self: a #CcText
 * @desc: a #PangoFontDescription
 *
 * Sets the font description to be used for rendering.
 */
void
cc_text_set_font_description(CcText* self, PangoFontDescription* desc) {
	cdebug("set_font_description()", "pango_layout_set_font_description()");
	pango_layout_set_font_description(self->layout, desc);
	// FIXME: forward the description-changed signal
	cc_item_update_bounds(CC_ITEM(self), NULL);
}

/**
 * cc_text_set_markup:
 * @self: a #CcText
 * @markup: the markup to be set
 *
 * Set the markup displayed by this item.
 */
void
cc_text_set_markup(CcText* self, gchar const* markup) {
	g_warning("FIXME: implement markup");
	cc_text_set_text(self, markup);
}

/**
 * cc_text_set_size_pixels:
 * @self: a #Cctext
 * @size_pixels: the value to be set
 *
 * Specify whether the text size is given in pixels. If it is, the text won't
 * scale with the zoom level.
 */
void
cc_text_set_size_pixels(CcText* self, gboolean size_pixels) {
	g_return_if_fail(CC_IS_TEXT(self));

	if(self->size_pixels == size_pixels) {
		return;
	}

	self->size_pixels = size_pixels;

	g_object_notify(G_OBJECT(self), "size-pixels");
	cc_item_update_bounds(CC_ITEM(self), NULL);
}

static void
text_update_caret(CcText* self)
{
	// render cursor
	PangoRectangle cursor, cursor2, ink;
	CcDRect dr;
	pango_layout_get_extents(self->layout, &ink, NULL);
	pango_layout_get_cursor_pos(self->layout, self->cursor + self->cursor_trail, &cursor, &cursor2);
	dr.x1 = /*x +*/ (ink.x + cursor.x) / PANGO_SCALE;
	dr.y1 = /*y +*/ (ink.y + cursor.y) / PANGO_SCALE;
	dr.x2 = dr.x1 + cursor.width / PANGO_SCALE;
	dr.y2 = dr.y1 + cursor.height / PANGO_SCALE;
	cc_caret_set_position(CC_CARET(P(self)->caret), dr.x1, dr.y1, dr.x2, dr.y2);
	cc_item_update_bounds(CC_ITEM(self), NULL);
}

static void
ct_move_cursor(CcText* self, CcView* view, gint direction) {
	PangoLayout* layout = self->layout;
	gint         new_cursor, new_trail;

	pango_layout_move_cursor_visually(layout, TRUE,
					  self->cursor, self->cursor_trail,
					  direction,
					  &new_cursor, &new_trail);

	if(new_cursor == G_MAXINT) {
		return;
	}

	if((new_cursor != self->cursor && new_cursor >= 0) || (new_trail != self->cursor_trail)) {
		if(new_cursor != self->cursor && new_cursor >= 0) {
			self->cursor = new_cursor;
		}

		if(new_trail != self->cursor_trail) {
			self->cursor_trail = new_trail;
		}

		g_object_notify(G_OBJECT(self), "cursor");
		text_update_caret(self);
		cc_item_dirty(CC_ITEM(self), view, *cc_item_get_all_bounds(CC_ITEM(self), view));
	}
}

/**
 * cc_text_set_text:
 * @self: a #CcText
 * @text: the text to be set
 *
 * Set the text to be displayed by a #CcText item.
 */
void
cc_text_set_text(CcText* self, gchar const* text) {
	g_return_if_fail(CC_IS_TEXT(self));
#if 0
	while(P(self)->cursor, P(self)->cursor_trail) {
		ct_move_cursor();
	}
#endif
	cdebug("set_text()", "pango_layout_set_text()");
	pango_layout_set_text (self->layout, text, -1);
	cc_item_update_bounds (CC_ITEM(self), NULL);
	cc_item_repaint (CC_ITEM (self));

	g_object_notify(G_OBJECT(self), "text");
}

/* GType stuff */
G_DEFINE_TYPE(CcText, cc_text, CC_TYPE_SHAPE);

enum {
	PROP_0,
	PROP_ANCHOR,
	PROP_ATTRIBUTES,
	PROP_BRUSH_CARET,
	PROP_CURSOR,
	PROP_EDITABLE,
	PROP_SIZE_PIXELS,
	PROP_TEXT
};

static void
ct_insert_and_move_cursor(CcText* self, CcView* view, gchar const* text) {
	gsize i = self->cursor;

	g_return_if_fail(CC_TEXT_GET_CLASS(self)->insert);

	if(self->cursor_trail) {
		gchar const* current = pango_layout_get_text(self->layout);
		i += g_utf8_length_next(current + i);
	}

	CC_TEXT_GET_CLASS(self)->insert(self, text, i);

	/* move the cursor AFTER inserting text */
	for(i = text ? g_utf8_strlen(text, -1) : 0; 0 < i; i--) {
		ct_move_cursor(self, view, 1);
	}
}

static void
cc_text_init(CcText* self)
{
	CcBrush* brush;

	//cdebug("init()", "pango_layout_new_cairo()");
	self->layout = pango_layout_new_cairo();

	P(self)->caret = cc_caret_new();
	cc_item_set_grid_aligned(P(self)->caret, TRUE);
	brush = cc_brush_color_new(cc_color_new_rgb(0.0, 0.0, 0.0));
	cc_shape_set_brush_border(CC_SHAPE(P(self)->caret), brush);

	cc_shape_set_brush_content(CC_SHAPE(self), brush);
}

static void
ct_dispose (GObject* object)
{
	CcText* self = CC_TEXT(object);

	if(P(self)->caret) {
		g_object_unref(P(self)->caret);
		P(self)->caret = NULL;
	}

	if(self->layout) {
		g_object_unref(self->layout);
		self->layout = NULL;
	}

	G_OBJECT_CLASS(cc_text_parent_class)->dispose(object);
}

static void
ct_render(CcItem* item, CcView* view, cairo_t* cr) {
	CcText* self = CC_TEXT(item);
	CC_ITEM_CLASS(cc_text_parent_class)->render(item, view, cr);

	if(self->editable) {
		cc_item_render(P(item)->caret, view, cr);
	}
}

static void
ct_path(CcShape* shape,
	CcView * view,
	cairo_t* cr)
{
	CcText     * self   = CC_TEXT(shape);
	PangoLayout* layout = self->layout;
	CcDRect    * bounds;
	gdouble      x, y;
	PangoFontDescription const* desc_old = pango_layout_get_font_description(layout);
	PangoFontDescription      * backup   = pango_font_description_copy(desc_old ? desc_old : pango_context_get_font_description(pango_cairo_font_map_create_context(PANGO_CAIRO_FONT_MAP(pango_cairo_font_map_get_default()))));
	PangoFontDescription      * desc_new = pango_font_description_copy_static(backup);
	gboolean      size_absolute = FALSE;
	gdouble       old_size = 0.0,
		      new_size = 0.0;

	bounds = cc_hash_map_lookup(CC_ITEM(shape)->bounds, view);
	g_return_if_fail(bounds);

	x = ct_get_x_offset(self, self->x, bounds->x2 - bounds->x1);
	y = ct_get_y_offset(self, self->y, bounds->y2 - bounds->y1);

	cc_view_world_to_window(view, &x, &y);

	if(!self->size_pixels) {
		size_absolute = pango_font_description_get_size_is_absolute(backup);
		new_size = old_size = pango_font_description_get_size(backup);

		cc_view_world_to_window_distance(view, &new_size, NULL);

		if(size_absolute) {
			pango_font_description_set_absolute_size(desc_new, new_size);
		} else {
			pango_font_description_set_size(desc_new, new_size);
		}

		//cdebug("path()", "pango_layout_set_font_description()");
		pango_layout_set_font_description(layout, desc_new);
	}

	if(TRUE || CC_ITEM_GRID_ALIGNED(shape)) {
		gdouble width = 2.0;
		cc_point_grid_align(&x, &y, &width);
	}

	cairo_move_to(cr, x, y);
	//cdebug("path()", "pango_cairo_update_layout()");
	pango_cairo_update_layout(cr, layout);
	//cdebug("path()", "pango_cairo_layout_path()");
	pango_cairo_layout_path(cr, layout);

	if(!self->size_pixels) {
		pango_layout_set_font_description(layout, desc_old ? backup : desc_old);
	}

	pango_font_description_free(desc_new);
	pango_font_description_free(backup);
}

static void
ct_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	CcText* self = CC_TEXT(object);

	switch(prop_id) {
	case PROP_ANCHOR:
		g_value_set_enum(value, self->anchor);
		break;
	case PROP_ATTRIBUTES:
		g_value_set_boxed(value, pango_layout_get_attributes(self->layout));
		break;
	case PROP_BRUSH_CARET:
		g_value_set_object(value, cc_shape_get_brush_border(CC_SHAPE(P(object)->caret)));
		break;
	case PROP_EDITABLE:
		g_value_set_boolean(value, cc_text_is_editable(self));
		break;
	case PROP_SIZE_PIXELS:
		g_value_set_boolean(value, self->size_pixels);
		break;
	case PROP_TEXT:
		g_value_set_string(value, pango_layout_get_text(self->layout));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
ct_set_property(GObject* object, guint prop_id, GValue const* value, GParamSpec* pspec) {
	CcText* self = CC_TEXT(object);

	switch(prop_id) {
	case PROP_ANCHOR:
		cc_text_set_anchor_type(self, g_value_get_enum(value));
		break;
	case PROP_ATTRIBUTES:
		cc_text_set_attributes(self, g_value_get_boxed(value));
		break;
	case PROP_BRUSH_CARET:
		cc_shape_set_brush_border(CC_SHAPE(P(object)->caret), g_value_get_object(value));
		break;
	case PROP_EDITABLE:
		cc_text_set_editable(self, g_value_get_boolean(value));
		break;
	case PROP_SIZE_PIXELS:
		cc_text_set_size_pixels(self, g_value_get_boolean(value));
		break;
	case PROP_TEXT:
		cc_text_set_text(self, g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
ct_update_bounds(CcItem* item, CcView const* view, gpointer data) {
	PangoRectangle rect;
	CcDRect new_bounds;
	CcText* self = CC_TEXT(item);
	CcDRect* bounds = cc_hash_map_lookup(item->bounds, view);
	CcDRect const* caret = cc_item_get_all_bounds(P(item)->caret, view);

	pango_layout_get_extents(self->layout, NULL, &rect);

	new_bounds.x1 = ct_get_x_offset(self, self->x, (1.0 * rect.width  / PANGO_SCALE)) + (1.0 * rect.x / PANGO_SCALE);
	new_bounds.y1 = ct_get_y_offset(self, self->y, (1.0 * rect.height / PANGO_SCALE)) + (1.0 * rect.y / PANGO_SCALE);
	new_bounds.x2 = new_bounds.x1         + (1.0 * rect.width  / PANGO_SCALE);
	new_bounds.y2 = new_bounds.y1         + (1.0 * rect.height / PANGO_SCALE);

	if(self->editable && caret) {
#if 0
		g_print("text (%.0fx%.0f)=>(%.0fx%.0f) + caret (%.0fx%.0f)=>(%.0fx%.0f) = ",
			new_bounds.x1, new_bounds.y1, new_bounds.x2, new_bounds.y2,
			caret->x1, caret->y1, caret->x2, caret->y2);
#endif
		cc_d_rect_union(new_bounds, *caret, &new_bounds);
#if 0
		g_print("result (%.0fx%.0f)=>(%.0fx%.0f)\n",
			new_bounds.x1, new_bounds.y1, new_bounds.x2, new_bounds.y2);
#endif
	}

	if(!bounds || !cc_d_rect_equal(new_bounds, *bounds)) {
		if(bounds) {
			cc_item_dirty(item, view, *bounds);
		}

		cc_hash_map_remove(item->bounds, view);
		cc_hash_map_insert(item->bounds, (gpointer)view, cc_d_rect_copy(&new_bounds));
		cc_item_dirty(item, view, new_bounds);
		cc_item_bounds_changed(item, view);
	}
}

static void
ct_move_backward_word(CcText* self, CcView* view) {
	PangoLayout* layout = self->layout;
	gchar const* text   = pango_layout_get_text(layout);
	gint  new_pos = g_utf8_pointer_to_offset(text, text + self->cursor + self->cursor_trail),
	      n_attrs;
	PangoLogAttr* attrs;

	pango_layout_get_log_attrs(layout, &attrs, &n_attrs);

	ct_move_cursor(self, view, -1);
	for(new_pos--; 0 < new_pos && !attrs[new_pos].is_word_start; new_pos--) {
		ct_move_cursor(self, view, -1);
	}

	g_free(attrs);
}

static void
ct_move_forward_word(CcText* self, CcView* view) {
	PangoLayout* layout = self->layout;
	gchar const* text   = pango_layout_get_text(layout);
	gint  new_pos = g_utf8_pointer_to_offset(text, text + self->cursor + self->cursor_trail),
	      n_attrs;
	PangoLogAttr* attrs;

	pango_layout_get_log_attrs(layout, &attrs, &n_attrs);

	ct_move_cursor(self, view, 1);
	for(new_pos++; new_pos < n_attrs && !attrs[new_pos].is_word_end; new_pos++) {
		ct_move_cursor(self, view, 1);
	}

	g_free(attrs);
}

static void
ct_delete_(CcText* self, CcView* view, gsize offset) {
	gchar const* text = NULL;
	gchar const* cursor_p = NULL;
	gsize        cursor = 0;
	gsize        length_of_deleted_char = 0;

	g_return_if_fail(CC_TEXT_GET_CLASS(self)->remove);

	text = pango_layout_get_text(self->layout);
	cursor = self->cursor;

	if(!text) {
		text = "";
	}

	cursor_p = text + self->cursor;

	if(G_UNLIKELY(self->cursor_trail)) {
		cursor += g_utf8_length_next(text + self->cursor);
	}

	if(offset) {
		length_of_deleted_char = (gsize)g_utf8_find_next_char(cursor_p, NULL);
	} else {
		length_of_deleted_char = (gsize)g_utf8_find_prev_char(text, cursor_p);
	}

	length_of_deleted_char = labs(length_of_deleted_char - (gsize)cursor_p);

	offset *= length_of_deleted_char;

	if(cursor >= offset && (cursor + 1 - offset) <= strlen(text)) {
		/* don't delete characters before or after the text */

		/* the cursor must be moved BEFORE the new text is set
		 * because the new position might be invalid within the new
		 * text */

		if(offset) {
			ct_move_cursor(self, view, -offset);
		}

		CC_TEXT_GET_CLASS(self)->remove(self, cursor - offset, length_of_deleted_char);
		g_object_notify(G_OBJECT(self), "cursor");
		cc_item_dirty(CC_ITEM(self), view, *cc_item_get_all_bounds(CC_ITEM(self), view));
	}
}

static void
ct_move_backward_home(CcText* self, CcView* view) {
	gchar const* text = pango_layout_get_text(self->layout);

	while(0 < self->cursor && text[self->cursor - 1] != '\n') {
		ct_move_cursor(self, view, 1);
	}
}

static void
ct_move_forward_end(CcText* self, CcView* view) {
	gchar const* text = pango_layout_get_text(self->layout);
	guint len = strlen(text);

	while((guint)(self->cursor + self->cursor_trail * g_utf8_length_next(text + self->cursor)) < len &&
	      text[(self->cursor + self->cursor_trail * g_utf8_length_next(text + self->cursor))] != '\n') {
		ct_move_cursor(self, view, 1);
	}
}

static gboolean
ct_key_press_event (CcItem     * item,
		    CcView     * view,
		    GdkEventKey* ev)
{
	CcTextViewData* view_data = V(item, view);
	CcText * self = CC_TEXT(item);
	gboolean retval = FALSE;

	if (gtk_im_context_filter_keypress (view_data->im_context, ev)) {
		return TRUE;
	}

	switch(ev->keyval) {
	case GDK_Left:
		if(ev->state & GDK_CONTROL_MASK) {
			ct_move_backward_word(self, view);
		} else {
			ct_move_cursor(self, view, -1);
		}
		retval = TRUE;
		break;
	case GDK_Right:
		if(ev->state & GDK_CONTROL_MASK) {
			ct_move_forward_word(self, view);
		} else {
			ct_move_cursor(self, view, 1);
		}
		retval = TRUE;
		break;
	case GDK_BackSpace:
		ct_delete_(self, view, 1);
		retval = TRUE;
		break;
	case GDK_Delete:
		ct_delete_(self, view, 0);
		retval = TRUE;
		break;
	case GDK_Return:
	case GDK_KP_Enter:
		ct_insert_and_move_cursor(self, view, "\n");
		retval = TRUE;
		break;
	case GDK_Home:
	case GDK_KP_Home:
		ct_move_backward_home(self, view);
		retval = TRUE;
		break;
	case GDK_End:
	case GDK_KP_End:
		ct_move_forward_end(self, view);
		retval = TRUE;
		break;
	}

	if(CC_ITEM_CLASS(cc_text_parent_class)->key_press_event) {
		retval |= CC_ITEM_CLASS(cc_text_parent_class)->key_press_event(item, view, ev);
	}

	return retval;
}

static gboolean
ct_button_press_event(CcItem* item, CcView* view, GdkEventButton* ev) {
	CcText  * self = CC_TEXT(item);
	gboolean  retval = FALSE;

	if(self->editable && cc_view_get_focus(view) != item) {
		cc_item_grab_focus(item, view);
	}

	if(CC_ITEM_CLASS(cc_text_parent_class)->button_press_event) {
		retval |= CC_ITEM_CLASS(cc_text_parent_class)->button_press_event(item, view, ev);
	}

	return retval;
}

static void
ct_im_context_commit(CcText* self, gchar const* text, GtkIMContext* context) {
	CcView* view = g_object_get_qdata(G_OBJECT(context), ct_quark);
	ct_insert_and_move_cursor(self, view, text);
}

static void
ct_view_register (CcItem* item,
		  CcView* view)
{
	CcTextViewData* view_data;
	CcText        * self = CC_TEXT(item);

	if(CC_ITEM_CLASS(cc_text_parent_class)->view_register) {
		CC_ITEM_CLASS(cc_text_parent_class)->view_register(item, view);
	}

	view_data = V(item, view);

	// set up the data for this view
	view_data->im_context = gtk_im_multicontext_new();
	g_object_set_qdata(G_OBJECT(view_data->im_context), ct_quark, view);
	// FIXME: maybe try to set the GdkWindow of the im_context
	// FIXME: set the view as a data pointer
	g_signal_connect_swapped(view_data->im_context, "commit",
				 G_CALLBACK(ct_im_context_commit), self);

	/* register caret */
	cc_item_add_view(P(item)->caret, view);
}

static void
ct_view_unregister (CcItem* item,
		    CcView* view)
{
	CcTextViewData* view_data;
	CcText      * self = CC_TEXT(item);

	view_data = V(item, view);

	/* caret cleanup */
	cc_item_remove_view(P(item)->caret, view);

	// remove the data for this view
	// FIXME: make sure the signal handler gets disconnected
	// FIXME: make sure the connected data gets deleted

	g_signal_handlers_disconnect_by_func(view_data->im_context, ct_im_context_commit, self);
	g_object_unref (view_data->im_context);
	view_data->im_context = NULL;

	if(CC_ITEM_CLASS(cc_text_parent_class)->view_unregister) {
		CC_ITEM_CLASS(cc_text_parent_class)->view_unregister(item, view);
	}
}

static void
ct_insert(CcText* self, gchar const* text, gsize position) {
	gchar const* old = pango_layout_get_text(self->layout);
	GString* str = g_string_new(old ? old : "");
	g_string_insert(str, position, text);
	cc_text_set_text(self, str->str);
	g_string_free(str, TRUE);
}

static void
ct_delete(CcText* self, gsize offset, gsize num_deleted) {
	gchar const* text = pango_layout_get_text(self->layout);
	GString* str = g_string_new(text ? text : "");
	g_string_erase(str, offset, num_deleted);
	cc_text_set_text(self, str->str);
	g_string_free(str, TRUE);
}

static void
cc_text_class_init(CcTextClass* self_class) {
	GObjectClass* go_class;
	CcItemClass * ci_class;
	CcShapeClass* cs_class;

	ct_quark = g_quark_from_static_string("CcText::view");

	/* GObjectClass */
	go_class = G_OBJECT_CLASS(self_class);
	go_class->dispose      = ct_dispose;
	go_class->get_property = ct_get_property;
	go_class->set_property = ct_set_property;

	g_object_class_install_property(go_class,
					PROP_ANCHOR,
					g_param_spec_enum("anchor",
							  _("Anchor"),
							  _("The location of the anchor point of the text element"),
							  GTK_TYPE_ANCHOR_TYPE,
							  GTK_ANCHOR_NW,
							  G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property(go_class,
					PROP_ATTRIBUTES,
					g_param_spec_boxed("attributes",
							   _("Attributes"),
							   _("The PangoAttrList that specifies the "),
							   PANGO_TYPE_ATTR_LIST,
							   G_PARAM_READWRITE));
	g_object_class_install_property(go_class,
					PROP_BRUSH_CARET,
					g_param_spec_object("brush-caret",
							    _("Caret Brush"),
							    _("The brush used to paint the caret (text cursor)"),
							    CC_TYPE_BRUSH,
							    G_PARAM_READWRITE));
	g_object_class_install_property(go_class,
					PROP_CURSOR,
					g_param_spec_int("cursor",
							 "cursor",
							 "cursor",
							 0, 0, 0,
							 0));
	g_object_class_install_property(go_class,
					PROP_EDITABLE,
					g_param_spec_boolean("editable",
							     "editable",
							     "editable",
							     FALSE,
							     G_PARAM_READWRITE));
	g_object_class_install_property(go_class,
					PROP_SIZE_PIXELS,
					g_param_spec_boolean("size-pixels",
							     "Size in Pixels",
							     "Specifies whether the given size is in pixels or in canvas units",
							     FALSE,
							     G_PARAM_READWRITE));
	g_object_class_install_property(go_class,
					PROP_TEXT,
					g_param_spec_string("text",
							    "Text",
							    "The displayed text",
							    NULL,
							    G_PARAM_READWRITE));

	/* CcItemClass */
	ci_class = CC_ITEM_CLASS(self_class);
	ci_class->render             = ct_render;
	ci_class->update_bounds      = ct_update_bounds;

	ci_class->key_press_event    = ct_key_press_event;
	ci_class->button_press_event = ct_button_press_event;

	ci_class->view_register      = ct_view_register;
	ci_class->view_unregister    = ct_view_unregister;

	/* CcShapeClass */
	cs_class = CC_SHAPE_CLASS(self_class);
	cs_class->path               = ct_path;

	/* CcTextClass */
	self_class->insert = ct_insert;
	self_class->remove = ct_delete;

	cc_item_class_add_view_data (self_class, sizeof (CcTextViewData));
	g_type_class_add_private(self_class, sizeof(struct CcTextPrivate));
}

