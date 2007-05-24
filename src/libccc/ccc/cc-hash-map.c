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

#include "cc-hash-map.h"
#include "glib-helpers.h"

CcHashMap*
cc_hash_map_new(GType content_type) {
	return g_object_new(CC_TYPE_HASH_MAP, "content", content_type, NULL);
}

void
cc_hash_map_foreach(CcHashMap* self, GHFunc func, gpointer user_data) {
	g_return_if_fail(CC_IS_HASH_MAP(self));

	g_hash_table_foreach(self->hash_table, func, user_data);
}

void
cc_hash_map_insert(CcHashMap* self, gpointer key, gpointer value) {
	g_return_if_fail(CC_IS_HASH_MAP(self));

	g_hash_table_insert(self->hash_table, key, value);
	// FIXME: add a destroy notification on the object
	// FIXME: require an onbject first
}

gpointer
cc_hash_map_lookup(CcHashMap* self, gconstpointer key) {
	g_return_val_if_fail(CC_IS_HASH_MAP(self), NULL);

	return g_hash_table_lookup(self->hash_table, key);
}

void
cc_hash_map_remove(CcHashMap* self, gconstpointer key) {
	g_return_if_fail(CC_IS_HASH_MAP(self));

	// FIXME: remove the destroy notification
	g_hash_table_remove(self->hash_table, key);
}

/* GType */
G_DEFINE_TYPE(CCHashMap, cc_hash_map, G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_CONTENT
};

static void
cc_hash_map_init(CCHashMap* self) {
	self->fundamental = G_TYPE_INVALID;
	self->content     = G_TYPE_INVALID;
}

static GObject*
mhm_constructor(GType type, guint n_params, GObjectConstructParam* params) {
	GObject* retval = G_OBJECT_CLASS(cc_hash_map_parent_class)->constructor(type, n_params, params);
	CCHashMap* self = CC_HASH_MAP(retval);
	self->fundamental = G_TYPE_FUNDAMENTAL(self->content);
	self->hash_table  = g_hash_table_new(g_direct_hash, g_direct_equal);
	return retval;
}

static void
mhm_finalize(GObject* object) {
	CCHashMap* self = CC_HASH_MAP(object);

	// FIXME: delete key/value pairs
	g_hash_table_destroy(self->hash_table);
	self->hash_table = NULL;

	G_OBJECT_CLASS(cc_hash_map_parent_class)->finalize(object);
}

static void
mhm_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	CCHashMap* self = CC_HASH_MAP(object);

	switch(prop_id) {
	case PROP_CONTENT:
		g_value_set_gtype(value, self->content);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
mhm_set_property(GObject* object, guint prop_id, GValue const* value, GParamSpec* pspec) {
	CCHashMap* self = CC_HASH_MAP(object);

	switch(prop_id) {
	case PROP_CONTENT:
		g_return_if_fail(self->content == G_TYPE_INVALID);
		self->content = g_value_get_gtype(value);
		g_object_notify(object, "content");
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
cc_hash_map_class_init(CCHashMapClass* self_class) {
	GObjectClass* go_class;

	/* GObjectClass */
	go_class = G_OBJECT_CLASS(self_class);
	go_class->constructor  = mhm_constructor;
	go_class->finalize     = mhm_finalize;
	go_class->get_property = mhm_get_property;
	go_class->set_property = mhm_set_property;

	g_object_class_install_property(go_class,
					PROP_CONTENT,
					g_param_spec_gtype("content",
							   "Content",
							   "The Type of the content",
							   G_TYPE_NONE,
							   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

