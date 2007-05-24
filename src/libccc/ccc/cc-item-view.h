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

#ifndef CC_ITEM_VIEW_H
#define CC_ITEM_VIEW_H

#include <ccc/cc-item.h>

G_BEGIN_DECLS

typedef struct _CcItemView      CcItemView;
typedef struct _CcItemViewIface CcItemViewIface;

#define CC_TYPE_ITEM_VIEW         (cc_item_view_get_type())
#define CC_ITEM_VIEW(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_ITEM_VIEW, CcItemView))
#define CC_IS_ITEM_VIEW(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_ITEM_VIEW))
#define CC_ITEM_VIEW_GET_CLASS(i) (G_TYPE_INSTANCE_GET_INTERFACE((i), CC_TYPE_ITEM_VIEW, CcItemViewIface))

GType cc_item_view_get_type(void);

void  cc_item_view_register  (CcItemView* self,
			      CcItem    * item);
void  cc_item_view_unregister(CcItemView* self,
			      CcItem    * item);

struct _CcItemViewIface {
	GTypeInterface base_iface;

	/* vtable */
	void (*item_added)        (CcItemView   * self,
				   CcItem       * item,
				   gint           position,
				   CcItem       * child);
	void (*item_removed)      (CcItemView   * self,
				   CcItem       * item,
				   gint           position,
				   CcItem       * child);
	void (*item_dirty)        (CcItemView   * self,
				   CcItem       * item,
				   CcView const * view,
				   CcDRect const* dirty);
	void (*notify_all_bounds) (CcItemView   * self,
				   CcItem       * item,
				   CcView       * view,
				   CcDRect const* all_bounds);
};

G_END_DECLS

#endif /* !CC_ITEM_VIEW_H */

