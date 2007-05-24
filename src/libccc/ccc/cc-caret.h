/* This file is part of CCC, a cairo-based canvas
 *
 * AUTHORS
 *     Sven Herzberg  <herzi@gnome-de.org>
 *
 * Copyright (C) 2006  Sven Herzberg
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

#ifndef CC_CARET_H
#define CC_CARET_H

#include <ccc/cc-line.h>

G_BEGIN_DECLS

typedef CcLine      CcCaret;
typedef CcLineClass CcCaretClass;

#define CC_TYPE_CARET         (cc_caret_get_type())
#define CC_CARET(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_CARET, CcCaret))
#define CC_CARET_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_CARET, CcCaretClass))
#define CC_IS_CARET(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_CARET))
#define CC_IS_CARET_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_CARET))
#define CC_CARET_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_CARET, CcCaretClass))

GType   cc_caret_get_type    (void);
CcItem* cc_caret_new         (void);
void    cc_caret_set_position(CcCaret* self,
			      gdouble x1,
			      gdouble y1,
			      gdouble x2,
			      gdouble y2);

G_END_DECLS

#endif /* !CC_CARET_H */
