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

#include "cc-caret.h"

CcItem*
cc_caret_new(void)
{
	return g_object_new(CC_TYPE_CARET, NULL);
}

void
cc_caret_set_position(CcCaret* self,
		      gdouble  x1,
		      gdouble  y1,
		      gdouble  x2,
		      gdouble  y2)
{
	CcLine* line = CC_LINE(self);
	gdouble height = y2 - y1;
	gdouble delta = 0.125*height;
	gdouble coords[][2] = {
		{x1 - delta, y1},
		{x1 + delta, y1},
		{x1, y1 + delta},
		{x2, y2 - delta},
		{x2 + delta, y2},
		{x2 - delta, y2}
	};
	guint i;
	cc_line_clear(line);
	for(i = 0; i < G_N_ELEMENTS(coords); i++) {
		cc_line_line(line, coords[i][0], coords[i][1]);
	}
}

/* GType */
G_DEFINE_TYPE(CcCaret, cc_caret, CC_TYPE_LINE);

static void
cc_caret_init(CcCaret* self G_GNUC_UNUSED)
{}

static void
cc_caret_class_init(CcCaretClass* self_class G_GNUC_UNUSED)
{}

