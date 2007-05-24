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

#ifndef CC_HASH_MAP_H
#define CC_HASH_MAP_H

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _CCHashMap      CCHashMap;
typedef struct _CCHashMapClass CCHashMapClass;
typedef struct _CCHashMap      CcHashMap;

#define CC_TYPE_HASH_MAP         (cc_hash_map_get_type())
#define CC_HASH_MAP(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_HASH_MAP, CCHashMap))
#define CC_HASH_MAP_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_HASH_MAP, CCHashMapClass))
#define CC_IS_HASH_MAP(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_HASH_MAP))
#define CC_IS_HASH_MAP_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_HASH_MAP))
#define CC_HASH_MAP_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_HASH_MAP, CCHashMapClass))

GType cc_hash_map_get_type(void);

CcHashMap* cc_hash_map_new   (GType content_type);
void              cc_hash_map_insert(CCHashMap* self,
					    gpointer          key,
					    gpointer          data);
gpointer          cc_hash_map_lookup(CcHashMap    * self,
				     gconstpointer  key);
void              cc_hash_map_remove(CCHashMap* self,
					    gconstpointer     key);
void              cc_hash_map_foreach(CcHashMap* self,
				      GHFunc     func,
				      gpointer   user_data);

struct _CCHashMap {
	GObject     base_instance;
	GType       fundamental;
	GType       content;
	GHashTable* hash_table;
};

struct _CCHashMapClass {
	GObjectClass base_class;
};

G_END_DECLS

#endif /* !CC_HASH_MAP_H */
