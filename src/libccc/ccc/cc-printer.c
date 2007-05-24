/* This file is part of libccc
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

#include "cc-printer.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */

#include <glib/gi18n-lib.h>
#include <ccc/cc-view.h>

CcPrinter*
cc_printer_new(gchar const* filename) {
	return g_object_new(CC_TYPE_PRINTER, "filename", filename, NULL);
}

void
cc_printer_print_page(CcPrinter* self) {
}

/* GType */
static void cp_init_cc_view(CcViewIface* iface);
G_DEFINE_TYPE_WITH_CODE(CcPrinter, cc_printer, G_TYPE_INITIALLY_UNOWNED,
			G_IMPLEMENT_INTERFACE(CC_TYPE_VIEW, cp_init_cc_view));

enum {
	PROP_0,
	PROP_FILENAME,
	PROP_FOCUS,
	PROP_ROOT,
	PROP_SCROLLED_REGION,
	PROP_ZOOM,
	PROP_ZOOM_MODE
};

static void
cc_printer_init(CcPrinter* self) {
	/* TRANSLATORS: this is for "%s.pdf" as the default file name of the
	 * printer, you might want to have a lowercase only word here, even if
	 * the correct translation would have been a capitalized word in your
	 * language. */
	self->filename = g_strdup_printf("%s.pdf", _("output"));
}

static void
cp_finalize(GObject* object) {
	CcPrinter* self = CC_PRINTER(object);

	g_free(self->filename);
	self->filename = NULL;

	G_OBJECT_CLASS(cc_printer_parent_class)->finalize(object);
}

static void
cp_get_property() {}

static void
cp_set_property() {}

static void
cc_printer_class_init(CcPrinterClass* self_class) {
	GObjectClass* go_class;

	/* GObjectClass */
	go_class = G_OBJECT_CLASS(self_class);
	go_class->finalize     = cp_finalize;
	go_class->get_property = cp_get_property;
	go_class->set_property = cp_set_property;

	g_object_class_install_property(go_class,
					PROP_FILENAME,
					g_param_spec_string("filename",
							    "File name",
							    "The name of the output file",
							    "output.pdf",
							    G_PARAM_READWRITE));

	/* CcViewIface */
	_cc_view_install_property(go_class,
				  PROP_ROOT,
				  PROP_SCROLLED_REGION,
				  PROP_ZOOM,
				  PROP_ZOOM_MODE,
				  PROP_FOCUS);
}

/* CcViewIface */
static void
cp_init_cc_view(CcViewIface* iface) {}

