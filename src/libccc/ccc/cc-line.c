/* This file is part of libccc, criawips' cairo-based canvas
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

#include "cc-line.h"

#include <math.h>
#include <cairo.h>
#include <ccc/cc-utils.h>

struct CcLinePrivate {
	GArray * segments;
};
#define P(i) (G_TYPE_INSTANCE_GET_PRIVATE((i), CC_TYPE_LINE, struct CcLinePrivate))

enum CcSegmentType {
	CC_SEGMENT_STOP = 0,
	CC_SEGMENT_MOVE,
	CC_SEGMENT_LINE
};

struct CcSegment {
	enum CcSegmentType type;
	gdouble x;
	gdouble y;
};

CcItem*
cc_line_new(void) {
	return g_object_new(CC_TYPE_LINE, NULL);
}

void
cc_line_clear(CcLine* self) {
	g_return_if_fail(CC_IS_LINE(self));

	g_array_set_size(P(self)->segments, 0);
	cc_item_update_bounds(CC_ITEM(self), NULL);
}

void
cc_line_line(CcLine* self, gdouble x, gdouble y) {
	struct CcSegment segment = {CC_SEGMENT_LINE, x, y};
	g_array_append_val(P(self)->segments, segment);

	cc_item_update_bounds(CC_ITEM(self), &segment);
}

void
cc_line_move(CcLine* self, gdouble x, gdouble y) {
	struct CcSegment segment = {CC_SEGMENT_MOVE, x, y};
	g_array_append_val(P(self)->segments, segment);

	cc_item_update_bounds(CC_ITEM(self), &segment);
}

/* GType */
G_DEFINE_TYPE(CcLine, cc_line, CC_TYPE_SHAPE);

static void
cc_line_init(CcLine* self) {
	P(self)->segments = g_array_new(TRUE, TRUE, sizeof(struct CcSegment));
}

static void
cl_finalize(GObject* object) {
	CcLine* self = CC_LINE(object);

	if(P(self)->segments) {
		g_array_free(P(self)->segments, TRUE);
		P(self)->segments = NULL;
	}

	G_OBJECT_CLASS(cc_line_parent_class)->finalize(object);
}

static void
cl_update_bounds(CcItem* item, CcView const* view, gpointer data) {
	struct CcSegment* segment = data;
	CcLine* self = CC_LINE(item);
	CcDRect* bounds = cc_hash_map_lookup(item->bounds, view);

	if(!bounds && !data && !P(self)->segments->len) {
		return;
	}

	gdouble half_width = ceil(/* FIXME: cc_shape_get_width(CC_SHAPE(item), view)*/ CC_SHAPE(item)->width / 2.0);
	if(G_LIKELY(segment && bounds)) {
		struct CcSegment seg = g_array_index(P(self)->segments, struct CcSegment, P(self)->segments->len - 2); // the formerly last one
		CcDRect dirty = {
			MIN(seg.x, segment->x) - half_width, MIN(seg.y, segment->y) - half_width,
			MAX(seg.x, segment->x) + half_width, MAX(seg.y, segment->y) + half_width
		};

		bounds->x1 = MIN(bounds->x1, segment->x - half_width);
		bounds->y1 = MIN(bounds->y1, segment->y - half_width);
		bounds->x2 = MAX(bounds->x2, segment->x + half_width);
		bounds->y2 = MAX(bounds->y2, segment->y + half_width);
		cc_item_dirty(item, view, dirty);

		cc_item_bounds_changed(item, view);
	} else if(G_UNLIKELY(!bounds && P(self)->segments->len)) {
		guint i = 0;
		CcDRect rect;
		segment = &g_array_index(P(self)->segments, struct CcSegment, i);
		rect.x1 = segment->x - half_width;
		rect.y1 = segment->y - half_width;
		rect.x2 = segment->x + half_width;
		rect.y2 = segment->y + half_width;

		for(i++; i < P(self)->segments->len; i++) {
			segment = &g_array_index(P(self)->segments, struct CcSegment, i);
			rect.x1 = MIN(rect.x1, segment->x - half_width);
			rect.y1 = MIN(rect.y1, segment->y - half_width);
			rect.x2 = MAX(rect.x2, segment->x + half_width);
			rect.y2 = MAX(rect.y2, segment->y + half_width);
		}
		cc_hash_map_insert(item->bounds, (gpointer)view, cc_d_rect_copy(&rect));
		cc_item_dirty(item, view, rect);

		cc_item_bounds_changed(item, view);
	}
}

static void
cl_path(CcShape* shape, CcView* view, cairo_t* cr) {
	CcLine* self = CC_LINE(shape);
	struct CcSegment* segment,
		        * last = &g_array_index(P(self)->segments, struct CcSegment, P(self)->segments->len - 1);
	gdouble x, y;
	gdouble w = cc_shape_get_width(shape, view);

	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

	for(segment = &g_array_index(P(self)->segments, struct CcSegment, 0); segment <= last; segment++) {
		x = segment->x;
		y = segment->y;

		cc_view_world_to_window(view, &x, &y);

		if(CC_ITEM_GRID_ALIGNED(shape)) {
			cc_point_grid_align(&x, &y, &w);
		}

		switch(segment->type) {
		case CC_SEGMENT_STOP:
			segment = last + sizeof(struct CcSegment);
			break;
		case CC_SEGMENT_LINE:
			cairo_line_to(cr, x, y);
			break;
		case CC_SEGMENT_MOVE:
			cairo_move_to(cr, x, y);
			break;
		}
	}
}

static void
cc_line_class_init(CcLineClass* self_class) {
	GObjectClass* go_class;
	CcItemClass * ci_class;
	CcShapeClass* cs_class;

	/* GObjectClass */
	go_class = G_OBJECT_CLASS(self_class);
	go_class->finalize = cl_finalize;

	/* CcItemClass */
	ci_class = CC_ITEM_CLASS(self_class);
	ci_class->update_bounds = cl_update_bounds;

	/* CcShapeClass */
	cs_class = CC_SHAPE_CLASS(self_class);
	cs_class->path = cl_path;

	/* CcLineClass */
	g_type_class_add_private(self_class, sizeof(struct CcLinePrivate));
}

