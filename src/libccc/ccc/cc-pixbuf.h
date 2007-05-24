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

#ifndef CC_PIXBUF_H
#define CC_PIXBUF_H

#include <ccc/cc-item.h>
#include <gdk/gdkpixbuf.h>

G_BEGIN_DECLS

typedef CcItem      CcPixbuf;
typedef CcItemClass CcPixbufClass;

#define CC_TYPE_PIXBUF         (cc_pixbuf_get_type())
#define CC_PIXBUF(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_PIXBUF, CcPixbuf))
#define CC_IS_PIXBUF(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_PIXBUF))

GType cc_pixbuf_get_type(void);

CcItem* cc_pixbuf_new       (void);
void    cc_pixbuf_set_pixbuf(CcPixbuf * self,
			     GdkPixbuf* pixbuf);
void    cc_pixbuf_set_position(CcPixbuf* self,
			       gdouble   x,
			       gdouble   y,
			       gdouble   w,
			       gdouble   h);
void    cc_pixbuf_set_position_x(CcPixbuf* self,
				 gdouble   pos_x);
void    cc_pixbuf_set_position_y(CcPixbuf* self,
				 gdouble   pos_y);

gdouble cc_pixbuf_get_rotation(CcPixbuf const* self);
void    cc_pixbuf_set_rotation(CcPixbuf      * self,
			       gdouble         rotation);

G_END_DECLS

#endif /* !CC_PIXBUF_H */
