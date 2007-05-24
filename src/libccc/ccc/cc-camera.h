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

#ifndef CC_CAMERA_H
#define CC_CAMERA_H

#include <ccc/cc-item.h>

G_BEGIN_DECLS

typedef CcItem      CcCamera;
typedef CcItemClass CcCameraClass;

#define CC_TYPE_CAMERA         (cc_camera_get_type())
#define CC_CAMERA(i)           (G_TYPE_CHECK_INSTANCE_CAST((i), CC_TYPE_CAMERA, CcCamera))
#define CC_CAMERA_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST((c), CC_TYPE_CAMERA, CcCameraClass))
#define CC_IS_CAMERA(i)        (G_TYPE_CHECK_INSTANCE_TYPE((i), CC_TYPE_CAMERA))
#define CC_IS_CAMERA_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE((c), CC_TYPE_CAMERA))
#define CC_CAMERA_GET_CLASS(i) (G_TYPE_INSTANCE_GET_CLASS((i), CC_TYPE_CAMERA, CcCameraClass))

GType   cc_camera_get_type(void);
CcItem* cc_camera_new     (void);
CcItem* cc_camera_new_root(CcItem      * root);
void    cc_camera_set_root(CcCamera    * self,
			   CcItem      * root);
CcItem* cc_camera_get_root(CcItem const* self);

G_END_DECLS

#endif /* !CC_CAMERA_H */
