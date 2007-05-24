/* this file is part of libccc, criawips' cairo-based canvas
 *
 * AUTHORS
 *       Sven Herzberg        <herzi@gnome-de.org>
 *
 * Copyright (C) 2005,2006 Sven Herzberg
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

#ifndef CC_VIEW_H
#define CC_VIEW_H

#include <glib/gmacros.h>

G_BEGIN_DECLS

typedef struct _CcView      CcView;
typedef struct _CcViewIface CcViewIface;

G_END_DECLS

#include <ccc/cc-item.h>

G_BEGIN_DECLS

#define CC_TYPE_VIEW         (cc_view_get_type())
#define CC_VIEW(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_VIEW, CcView))
#define CC_IS_VIEW(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_VIEW))
#define CC_VIEW_GET_CLASS(i) (G_TYPE_INSTANCE_GET_INTERFACE((i), CC_TYPE_VIEW, CcViewIface))

GType cc_view_get_type(void);
void  _cc_view_install_property(GObjectClass* go_class,
				guint         root_id,
				guint         scroll_id,
				guint         zoom_id,
				guint         mode_id,
				guint         focus_id);

CcItem* cc_view_get_focus               (const CcView* self);
CcItem* cc_view_get_root                (CcView      * self);
CcDRect*cc_view_get_scrolled_region     (CcView      * self);
gdouble cc_view_get_zoom                (CcView      * self);
void    cc_view_set_focus               (CcView      * self,
					 CcItem      * focus);
void    cc_view_set_root                (CcView * self,
			                 CcItem * root);
void    cc_view_set_scrolled_region     (CcView      * self,
					 CcDRect     * scrolled_region);
void    cc_view_set_zoom                (CcView      * self,
				         gdouble       zoom);
gint    cc_view_grab_item               (CcView      * self,
					 CcItem      * item,
					 GdkEventMask  mask,
					 GdkCursor   * cursor,
					 guint32       time);
void    cc_view_ungrab_item             (CcView      * self,
					 CcItem      * item,
					 guint32       time);
void    cc_view_window_to_world         (CcView      * self,
				         gdouble     * x,
				         gdouble     * y);
void    cc_view_world_to_window         (CcView * self,
				         gdouble* x,
				         gdouble* y);
void    cc_view_world_to_window_distance(CcView * self,
					 gdouble* x,
					 gdouble* y);

struct _CcViewIface {
	GTypeInterface base_iface;

	/* vtable */
	GdkGrabStatus (*grab_item)               (CcView*       self,
				                  CcItem*       item,
					          GdkEventMask  mask,
					          GdkCursor   * cursor,
					          guint32       time);
	void          (*ungrab_item)             (CcView      * self,
					          CcItem      * item,
					          guint32       time);
	void          (*window_to_world)         (CcView      * self,
				                  gdouble     * x,
				                  gdouble     * y);
	void          (*world_to_window)         (CcView      * self,
				                  gdouble     * x,
				                  gdouble     * y);
	void          (*world_to_window_distance)(CcView      * self,
					          gdouble     * x,
					          gdouble     * y);
};

G_END_DECLS

#endif /* !CC_VIEW_H */

