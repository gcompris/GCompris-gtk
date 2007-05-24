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
	
	/* mathematical bounds are theorics: width = 0 */
	CcDRect math_bounds;
	
	/* indexes of points for the mathematical bounds */
	/* these are the indexes giving the index for bounds */
	/* index 0 = CC_SEGMENT_MOVE, 0, 0 */
	guint index_x_min;
	guint index_y_min;
	guint index_x_max;
	guint index_y_max;

	gboolean has_bounds : 1;
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

	g_array_set_size(P(self)->segments, 1);
	P(self)->has_bounds = FALSE;
	cc_item_update_bounds(CC_ITEM(self), NULL);
}

#define GET_SEGMENT(line, index) (&g_array_index(P(line)->segments, struct CcSegment, index))

static void
update_mathematicals_bounds (CcItem *self, guint modified_index)
{
	guint i=1;

	gboolean x_min_changed = FALSE;
	gboolean y_min_changed = FALSE;
	gboolean x_max_changed = FALSE;
	gboolean y_max_changed = FALSE;
	
	CcDRect *bounds = &P(self)->math_bounds;
	struct CcSegment *segment    = GET_SEGMENT(self, modified_index);
	struct CcSegment *next       = (modified_index==(P(self)->segments->len-1))? segment : GET_SEGMENT(self, modified_index + 1);
	
	if ((segment->type == CC_SEGMENT_MOVE) && (next->type == CC_SEGMENT_MOVE))
		return;
	
	while (GET_SEGMENT(self, i)->type == CC_SEGMENT_MOVE) i++;
	
	if (i == P(self)->segments->len-1) { /* INITIALISE */
		if (GET_SEGMENT(self, i-1)->x < GET_SEGMENT(self, i)->x) {
			P(self)->index_x_min = i-1;
			bounds->x1 = GET_SEGMENT(self, i-1)->x;
			P(self)->index_x_max = i;
			bounds->x2 = GET_SEGMENT(self, i)->x;
		} else {
			P(self)->index_x_min = i;
			bounds->x1 = GET_SEGMENT(self, i)->x;
			P(self)->index_x_max = i-1;
			bounds->x2 = GET_SEGMENT(self, i-1)->x;
		}
		if (GET_SEGMENT(self, i-1)->y < GET_SEGMENT(self, i)->y) {
			P(self)->index_y_min = i-1;
			bounds->y1 = GET_SEGMENT(self, i-1)->y;
			P(self)->index_y_max = i;
			bounds->y2 = GET_SEGMENT(self, i)->y;
		} else {
			P(self)->index_y_min = i;
			bounds->y1 = GET_SEGMENT(self, i)->y;
			P(self)->index_y_max = i-1;
			bounds->y2 = GET_SEGMENT(self, i-1)->y;
		}
		P(self)->has_bounds = TRUE;
		return;
	} else {
		if ((modified_index == P(self)->index_x_min) && (segment->x > bounds->x1)) {
			x_min_changed = TRUE;
			bounds->x1 = segment->x;
		} else if (segment->x < bounds->x1) {
			bounds->x1 = segment->x;
			P(self)->index_x_min =  modified_index;
		}
	
		if ((modified_index == P(self)->index_y_min) && (segment->y > bounds->y1)) {
			y_min_changed = TRUE;
			bounds->y1 = segment->y;
		} else if (segment->y < bounds->y1) {
			bounds->y1 = segment->y;
			P(self)->index_y_min =  modified_index;
		}
		
		if ((modified_index == P(self)->index_x_max) && (segment->x < bounds->x2)) {
			x_max_changed = TRUE;
			bounds->x2 = segment->x;
		} else if (segment->x > bounds->x2) {
			bounds->x2 = segment->x;
			P(self)->index_x_max =  modified_index;
		}
		
		if ((modified_index == P(self)->index_y_max) && (segment->y < bounds->y2)) {
			y_max_changed = TRUE;
			bounds->y2 = segment->y;
		}  else if (segment->y > bounds->y2) {
			bounds->y2 = segment->y;
			P(self)->index_y_max =  modified_index;
		}
		
		if (G_UNLIKELY(x_min_changed || y_min_changed || x_max_changed || y_max_changed)) {
			for (i=i-1; i < P(self)->segments->len; i++) {
				if (G_UNLIKELY(GET_SEGMENT(self, i)->type == CC_SEGMENT_MOVE) && 
				    ((i == P(self)->segments->len-1 ) || (GET_SEGMENT(self, i)->type == CC_SEGMENT_MOVE)))
					continue;
				
				if (G_UNLIKELY(x_min_changed && (GET_SEGMENT(self, i)->x < bounds->x1))) {
					P(self)->index_x_min = i;
					bounds->x1 = GET_SEGMENT(self, i)->x;
				}
				
				if (G_UNLIKELY(y_min_changed && (GET_SEGMENT(self, i)->y < bounds->y1))) {
					P(self)->index_y_min = i;
					bounds->y1 = GET_SEGMENT(self, i)->y;
				}
				
				if (G_UNLIKELY(x_max_changed && (GET_SEGMENT(self, i)->x > bounds->x2))) {
					P(self)->index_x_max = i;
					bounds->x2 = GET_SEGMENT(self, i)->x;
				}
				
				if (G_UNLIKELY(y_max_changed && (GET_SEGMENT(self, i)->y > bounds->y2))) {
					P(self)->index_y_max = i;
					bounds->y2 = GET_SEGMENT(self, i)->y;
				}
				
			}
		}	
	}
}

static void 
set_dirty_rect_for_view (CcItem *item, CcView *view, gpointer data)
{
        gdouble half_width = ceil(cc_shape_get_width(CC_SHAPE(item), view)/2.0);

	CcDRect dirty_view, *dirty = (CcDRect *)data;

	dirty_view.x1 = dirty->x1 - half_width;
	dirty_view.y1 = dirty->y1 - half_width;
	dirty_view.x2 =  dirty->x2 + half_width;
	dirty_view.y2 =  dirty->y2 + half_width ;
	
	cc_item_dirty (item, view, dirty_view);
}

void
cc_line_line(CcLine* self, gdouble x, gdouble y) {
	struct CcSegment segment = {CC_SEGMENT_LINE, x, y};
	g_array_append_val(P(self)->segments, segment);

	update_mathematicals_bounds(CC_ITEM(self), P(self)->segments->len-1);

	cc_item_update_bounds(CC_ITEM(self), NULL);

	struct CcSegment seg = g_array_index(P(self)->segments, struct CcSegment, P(self)->segments->len - 2); // the formerly last one

	CcDRect dirty = {
	  MIN(seg.x, segment.x), MIN(seg.y, segment.y),
	  MAX(seg.x, segment.x), MAX(seg.y, segment.y) 
	};

	cc_item_foreach_view (CC_ITEM(self), (CcItemFunc) set_dirty_rect_for_view, &dirty);
}

void
cc_line_move(CcLine* self, gdouble x, gdouble y) {
	struct CcSegment segment = {CC_SEGMENT_MOVE, x, y};
	g_array_append_val(P(self)->segments, segment);

	update_mathematicals_bounds(CC_ITEM(self), P(self)->segments->len-1);
	cc_item_update_bounds(CC_ITEM(self), NULL);
}

guint   
cc_line_get_num_data(CcLine * self)
{
  return P(self)->segments->len - 1;
}

void    cc_line_data_set_position (CcLine *self,
				   guint index,
				   gdouble x,
				   gdouble y)
{
        guint real_index = index + 1;
        g_return_if_fail(CC_IS_LINE(self));
	g_return_if_fail(real_index < P(self)->segments->len);

	struct CcSegment *segment = &g_array_index (P(self)->segments, struct CcSegment, real_index);

	if ((segment->x == x) && segment->y == y)
	  return;

	/* We need to redraw old position, previous and next segment too */
	gdouble old_x, old_y;

	struct CcSegment previous = (real_index == 0 ? *segment : g_array_index (P(self)->segments, struct CcSegment, real_index - 1));
	struct CcSegment next = ( (real_index == (P(self)->segments->len - 1)) ? *segment : g_array_index (P(self)->segments, struct CcSegment,real_index+1));

	old_x = segment->x;
	old_x = segment->y;
	segment->x = x;
	segment->y = y;

	update_mathematicals_bounds(CC_ITEM(self), real_index);
	cc_item_update_bounds(CC_ITEM(self), NULL);

	CcDRect dirty = { MIN( MIN(old_x, x), MIN(previous.x, next.x)),
			  MIN( MIN(old_y, y), MIN(previous.y, next.y)),
			  MAX( MAX(old_x, x), MAX(previous.x, next.x)),
			  MAX( MAX(old_y, y), MAX(previous.y, next.y))
	};

	cc_item_foreach_view (CC_ITEM(self), (CcItemFunc) set_dirty_rect_for_view, &dirty);
}

void    cc_line_data_get_position (CcLine *self,
				   guint index,
				   gdouble *x,
				   gdouble *y)

{
       guint real_index = index + 1;
       g_return_if_fail(CC_IS_LINE(self));
	g_return_if_fail(real_index < P(self)->segments->len);

	struct CcSegment segment = g_array_index (P(self)->segments, struct CcSegment, real_index);
	*x = segment.x;
	*y = segment.y;
}



/* GType */
G_DEFINE_TYPE(CcLine, cc_line, CC_TYPE_SHAPE);

static void
cc_line_init(CcLine* self) {
	P(self)->segments = g_array_new(TRUE, TRUE, sizeof(struct CcSegment));

	struct CcSegment segment = {CC_SEGMENT_MOVE, 0.0, 0.0};
	g_array_append_val(P(self)->segments, segment);
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

static gdouble
cl_distance (CcItem      * item,
	    CcView const* view,
	    gdouble       x,
	    gdouble       y,
	    CcItem      **found)
{
	CcLine* self = CC_LINE(item);
	gdouble      distance = G_MAXDOUBLE;
	gdouble new_dist;
	guint i=0;
	
	g_return_val_if_fail(found, distance);
	g_return_val_if_fail(!CC_IS_ITEM(*found), distance);

	// search the children first
	distance = CC_ITEM_CLASS(cc_line_parent_class)->distance(item, view, x, y, found);

	if(CC_IS_ITEM(*found)) {
		return distance;
	}

	while ((g_array_index(P(self)->segments, struct CcSegment, i).type == CC_SEGMENT_MOVE) && (i < P(self)->segments->len)) i++;

	if (i == P(self)->segments->len)
		return MIN (distance, G_MAXDOUBLE);

	new_dist = G_MAXDOUBLE;
	gdouble half_width = ceil(cc_shape_get_width(CC_SHAPE(item), view) / 2.0);
	for (; i < P(self)->segments->len; i++) {
		if (g_array_index(P(self)->segments, struct CcSegment, i).type == CC_SEGMENT_MOVE)
			continue;
		gdouble dist = math_distance_point_segment (x, y,
							g_array_index(P(self)->segments, struct CcSegment, i-1).x,
							g_array_index(P(self)->segments, struct CcSegment, i-1).y,
							g_array_index(P(self)->segments, struct CcSegment, i).x,
							    g_array_index(P(self)->segments, struct CcSegment, i).y) - half_width;
		new_dist = MIN (dist, new_dist);
		if (new_dist <= 0.0) {
			new_dist = 0.0;
			break;
		}
	}

	distance = MIN (new_dist, distance);
	if (distance == 0.0)
		if (found)
			*found = item;

	return distance;
}

static void
cl_update_bounds(CcItem* item, CcView const* view, gpointer data) {
	CcLine* self = CC_LINE(item);
	CcDRect *bounds = cc_hash_map_lookup(item->bounds, view);
	CcDRect *m_bounds = &P(self)->math_bounds;

	if(!bounds && !P(self)->has_bounds) {
	  return;
	}

	gdouble half_width = ceil(cc_shape_get_width(CC_SHAPE(item), view) / 2.0);
	 
	if (bounds){
	  if (G_UNLIKELY(!P(self)->has_bounds))
	    cc_hash_map_remove(item->bounds, view);
	  else {
	    if ((bounds->x1 == m_bounds->x1 - half_width) &&
		(bounds->y1 == m_bounds->y1 - half_width) &&
		(bounds->x2 == m_bounds->x2 + half_width) &&
		(bounds->y2 == m_bounds->y2 + half_width))
	      return;
	    else {
	      bounds->x1 = m_bounds->x1 - half_width;
	      bounds->y1 = m_bounds->y1 - half_width;
	      bounds->x2 = m_bounds->x2 + half_width;
	      bounds->y2 = m_bounds->y2 + half_width;
	      
	      cc_item_bounds_changed(item, view);
	      return;
	    }
	  }
	} else { /* NO BOUNDS */
	  CcDRect rect;
	  rect.x1 = m_bounds->x1 - half_width;
	  rect.y1 = m_bounds->y1 - half_width;
	  rect.x2 = m_bounds->x2 + half_width;
	  rect.y2 = m_bounds->y2 + half_width;

	  cc_hash_map_insert(item->bounds, (gpointer)view, cc_d_rect_copy(&rect));	  cc_item_bounds_changed(item, view);
	  return;
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
	ci_class->distance = cl_distance;

	/* CcShapeClass */
	cs_class = CC_SHAPE_CLASS(self_class);
	cs_class->path = cl_path;

	/* CcLineClass */
	g_type_class_add_private(self_class, sizeof(struct CcLinePrivate));
}

