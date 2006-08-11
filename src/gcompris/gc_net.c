/* gcompris - gc_net.c
 *
 * Time-stamp: <2006/07/10 01:24:04 bruno>
 *
 * Copyright (C) 2006 Bruno Coudoin
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "gc_net.h"

#ifdef USE_GNET
#include <gnet.h>
#endif

/*
 * Init the network library, must be called once before using it
 */
void gc_net_init()
{
}

/*
 * load a pixmap from the network
 * pixmapfile is given relative to PACKAGE_DATA_DIR
 */
GdkPixbuf *gc_net_load_pixmap(char *pixmapfile)
{
  return NULL;
}
