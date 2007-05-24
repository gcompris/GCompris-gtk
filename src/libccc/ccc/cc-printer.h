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

#ifndef CC_PRINTER_H
#define CC_PRINTER_H

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _CcPrinter      CcPrinter;
typedef struct _CcPrinterClass CcPrinterClass;

#define CC_TYPE_PRINTER         (cc_printer_get_type())
#define CC_PRINTER(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_PRINTER, CcPrinter))
#define CC_PRINTER_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_PRINTER, CcPrinterClass))
#define CC_IS_PRINTER(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_PRINTER))
#define CC_IS_PRINTER_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_PRINTER))
#define CC_PRINTER_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_PRINTER, CcPrinterClass))

GType      cc_printer_get_type  (void);

CcPrinter* cc_printer_new       (gchar const* filename);
void       cc_printer_print_page(CcPrinter* self);

struct _CcPrinter {
	GInitiallyUnowned base_instance;

	gchar* filename;
};

struct _CcPrinterClass {
	GInitiallyUnownedClass base_class;
};

G_END_DECLS

#endif /* !CC_PRINTER_H */
