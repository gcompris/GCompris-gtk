/* gcompris - main.c
 *
 * Time-stamp: <2002/06/02 00:25:49 bruno>
 *
 * Copyright (C) 2000,2001 Bruno Coudoin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <signal.h>

#include "gcompris.h"

int
gcompris_init (int argc, char *argv[]);

int
main (int argc, char *argv[])
{
  gcompris_init(argc, argv);
}
