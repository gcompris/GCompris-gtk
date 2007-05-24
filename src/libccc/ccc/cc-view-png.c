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

#include "cc-view-png.h"

CcView*
cc_view_png_new(gchar const* filename) {
	return g_object_new(CC_TYPE_VIEW_PNG, "filename", filename, NULL);
}

/* GType */
G_DEFINE_TYPE_WITH_CODE(CcViewPNG, cc_view_png, G_TYPE_INITIALLY_UNOWNED,
			G_IMPLEMENT_INTERFACE(CC_TYPE_VIEW, NULL));

enum {
	PROP_0,
	PROP_FILENAME
};

static void
cc_view_png_init(CcViewPNG* self) {}

static GObject*
cvp_constructor(GType type, guint n_props, GObjectConstructParam* props) {
	GObject* retval = G_OBJECT_CLASS(cc_view_png_parent_class)->constructor(type, n_props, props);
	CcViewPNG* self = CC_VIEW_PNG(retval);
	self->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 128, 128); // FIXME: make size configurable
	return retval;
}

static void
cvp_finalize(GObject* object) {
	CcViewPNG* self = CC_VIEW_PNG(object);
	cairo_surface_write_to_png(self->surface, self->filename);
	cairo_surface_destroy(self->surface);
	self->surface = NULL;
	G_OBJECT_CLASS(cc_view_png_parent_class)->finalize(object);
}

static void
cvp_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	CcViewPNG* self = CC_VIEW_PNG(object);

	switch(prop_id) {
	case PROP_FILENAME:
		g_value_set_string(value, self->filename);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
cvp_set_property(GObject* object, guint prop_id, GValue const* value, GParamSpec* pspec) {
	CcViewPNG* self = CC_VIEW_PNG(object);

	switch(prop_id) {
	case PROP_FILENAME:
		g_return_if_fail(!self->filename);

		self->filename = g_strdup(g_value_get_string(value)); // FIXME: there is a nicer call for this
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
cc_view_png_class_init(CcViewPNGClass* self_class) {
	GObjectClass* go_class;

	/* GObjectClass */
	go_class = G_OBJECT_CLASS(self_class);
	go_class->constructor  = cvp_constructor;
	go_class->finalize     = cvp_finalize;
	go_class->get_property = cvp_get_property;
	go_class->set_property = cvp_set_property;

	g_object_class_install_property(go_class,
					PROP_FILENAME,
					g_param_spec_string("filename",
							    "Filename",
							    "Filename",
							    NULL,
							    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

}

