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

#ifndef CC_ITEM_H
#define CC_ITEM_H

#include <cairo.h>
#include <gdk/gdkevents.h>
#include <gtk/gtkenums.h>
#include <ccc/cc-d-rect.h>
#include <ccc/cc-hash-map.h>

G_BEGIN_DECLS

typedef struct _CcItem      CcItem;
typedef struct _CcItemClass CcItemClass;

G_END_DECLS

#include <ccc/cc-view.h>

G_BEGIN_DECLS

typedef enum {
	CC_CAN_FOCUS          = 1 << 1,
	CC_DISPOSED           = 1 << 2,
	CC_GRID_ALIGNED       = 1 << 3,
} CcItemFlags;

typedef void (*CcItemFunc) (CcItem* item, CcView* view, gpointer data);

#define CC_TYPE_ITEM         (cc_item_get_type())
#define CC_ITEM(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_ITEM, CcItem))
#define CC_ITEM_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_ITEM, CcItemClass))
#define CC_IS_ITEM(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_ITEM))
#define CC_IS_ITEM_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_ITEM))
#define CC_ITEM_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_ITEM, CcItemClass))

#define CC_ITEM_FLAGS(i)         (CC_ITEM(i)->flags)
#define CC_ITEM_SET_FLAGS(i,m)   G_STMT_START{ (CC_ITEM_FLAGS(i) |=  (m)); }G_STMT_END
#define CC_ITEM_UNSET_FLAGS(i,m) G_STMT_START{ (CC_ITEM_FLAGS(i) &= ~(m)); }G_STMT_END

#define CC_ITEM_CAN_FOCUS(i)          ((CC_ITEM_FLAGS(i) & CC_CAN_FOCUS) != 0)
#define CC_ITEM_DISPOSED(i)           ((CC_ITEM_FLAGS(i) & CC_DISPOSED) != 0)
#define CC_ITEM_GRID_ALIGNED(i)       ((CC_ITEM_FLAGS(i) & CC_GRID_ALIGNED) != 0)

#define CC_ITEM_FUNC(f) ((CcItemFunc)(f))

GType cc_item_get_type(void);

CcItem*       cc_item_new             (void);
void          cc_item_append          (CcItem      * self,
		                       CcItem      * child);
void          cc_item_bounds_changed  (CcItem      * self,
				       CcView const* view);
void          cc_item_dirty           (CcItem      * self,
				       CcView const* view,
				       CcDRect       dirty_region);
gdouble       cc_item_distance        (CcItem      * self,
				       gdouble       x,
				       gdouble       y,
				       CcItem      **found);
void          cc_item_foreach_view    (CcItem      * self,
				       CcItemFunc    func,
				       gpointer      data);
CcDRect const*cc_item_get_all_bounds  (CcItem const* self,
				       CcView const* view);
void          cc_item_grab_focus      (CcItem      * self,
				       CcView      * view);
void          cc_item_insert          (CcItem      * self,
		                       CcItem      * child,
		                       gint          position);
gboolean      cc_item_is_child_of     (CcItem const* child,
				       CcItem const* parent);
void          cc_item_remove          (CcItem  * self,
		                       CcItem  * child);
void          cc_item_render          (CcItem  * self,
		                       CcView  * view,
		                       cairo_t * cr);
void          cc_item_set_grid_aligned(CcItem  * self,
				       gboolean  grid_aligned);
void          cc_item_update_bounds   (CcItem      * self,
				       gpointer      data);
void          cc_item_update_bounds_for_view(CcItem* self,
					     CcView* view);

void          cc_item_add_view              (CcItem* self,
					     CcView* view);
void          cc_item_remove_view           (CcItem* self,
					     CcView* view);

struct _CcItem {
	GInitiallyUnowned      base_instance;
	CcItem     * parent;
	CcItemFlags  flags;

	CcHashMap  * all_bounds;
	CcHashMap  * bounds;
	CcHashMap  * children_bounds;

	GList      * children;
	GList      * views;
};

struct _CcItemClass {
	GInitiallyUnownedClass base_class;

	/* vtable */
	gdouble (*distance)             (CcItem         * self,
				         gdouble          x,
				         gdouble          y,
				         CcItem         **found);
	void    (*notify_child_bounds)  (CcItem         * self,
				         CcItem         * child,
					 CcView         * view,
				         CcDRect const  * all_bounds);
	void    (*render)               (CcItem         * self,
		                         CcView         * view,
			                 cairo_t        * cr);
	void    (*update_bounds)        (CcItem         * self,
					 CcView const   * view,
					 gpointer         user_data);

	/* signals */
	gboolean (*button_press_event)  (CcItem          * self,
					 CcView          * view,
					 GdkEventButton  * event);
	gboolean (*button_release_event)(CcItem          * self,
					 CcView          * view,
					 GdkEventButton  * event);
	gboolean (*enter_notify_event)  (CcItem          * self,
					 CcView          * view,
					 GdkEventCrossing* event);
	gboolean (*event)               (CcItem          * self,
					 CcView          * view,
					 GdkEvent        * event);
	gboolean (*focus)		(CcItem          * self,
					 CcView	         * view,
					 GtkDirectionType  dir);
	gboolean (*focus_enter)         (CcItem          * self,
					 CcView          * view,
					 GdkEventFocus   * event);
	gboolean (*focus_leave)         (CcItem          * self,
					 CcView          * view,
					 GdkEventFocus   * focus);
	gboolean (*key_press_event)     (CcItem          * item,
					 CcView          * view,
					 GdkEventKey     * event);
	gboolean (*key_release_event)   (CcItem          * item,
					 CcView          * view,
					 GdkEventKey     * event);
	gboolean (*leave_notify_event)  (CcItem          * self,
					 CcView          * view,
					 GdkEventCrossing* event);
	gboolean (*motion_notify_event) (CcItem          * self,
					 CcView          * view,
					 GdkEventMotion  * event);

	void     (*item_added)          (CcItem          * self,
					 gint              position,
					 CcItem          * child);
	void     (*item_removed)        (CcItem          * self,
					 gint              position,
					 CcItem          * child);

	void     (*view_register)       (CcItem          * self,
					 CcView          * view);
	void     (*view_unregister)     (CcItem          * self,
					 CcView          * view);
};

G_END_DECLS

#endif /* !CC_ITEM_H */

