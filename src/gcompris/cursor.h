/* gcompris - cursor.h
 *
 * Copyright (C) 2002 Pascal Georges
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
#ifndef CURSOR_H
#define CURSOR_H

#include "gcompris.h"
#include <gtk/gtk.h>
#include <gdk_imlib.h>

#include <stdio.h>

// cursor defines must be over the last gnome cursor defined in gdkcursors.h
#define FIRST_CUSTOM_CURSOR 1000
#define BIG_RED_ARROW_CURSOR 1001
#define BIRD_CURSOR 1002

/* Bitmap data of cursor */
static const gchar *big_red_arrow_cursor_bits[] = {
"1111111111111111111111111               ",
"111111111111111111111111                ",
"1100000000000000000111                  ",
"11000000000000000111                    ",
"110000000000000111                      ",
"1100000000000111                        ",
"11000000000111                          ",
"110000000011                            ",
"1100000000011                           ",
"11000000000011                          ",
"110000110000011                         ",
"1100001110000011                        ",
"1100011 110000011                       ",
"1100011  110000011                      ",
"110011    110000011                     ",
"110011     110000011                    ",
"11011       110000011                   ",
"11011        110000011                  ",
"1111          110000011                 ",
"1111           110000011                ",
"111             110000011               ",
"111              110000011              ",
"11                110000011             ",
"1                  110000011            ",
"                    110000011           ",
"                     110000011          ",
"                      110000011         ",
"                       110000011        ",
"                        110000011       ",
"                         110000011      ",
"                          110000011     ",
"                           110000011    ",
"                            110000011   ",
"                             110000011  ",
"                              110000011 ",
"                               110000011",
"                                11000011",
"                                 1100011",
"                                  111111",
"                                   11111"};

static const gchar *bird_cursor_bits[] = {
"        111                             ",
"     11000001                           ",
" 1110001000001                          ",
"110000000000001                         ",
"  11100000000001                        ",
"     0000000000011                      ",
"      100000000011111                   ",
"       000000010000111                  ",
"       0000000000000111                 ",
"       00000010000000011                ", // 10
"       100000000000000011               ",
"       0000000000000000011              ",
"       10000000100000000011             ",
"        0000000000000000011             ",
"        10000001000000000011            ",
"         0000000000000000001            ",
"         10000000000000000011           ",
"          00000000000000000011          ",
"           0000000000000000011          ",
"            000000000000000011          ", // 20
"             1000000000000000011        ",
"              10000000000000011         ",
"               1000000000000001         ",
"                 100000000000011        ",
"                  11000000111111        ",
"                    100000000011        ",
"                      11000000001       ",
"                         10000001       ",
"                            10001       ",
"                              00001     ", // 30
"                              100001    ",
"                               00000    ",
"                               100001   ",
"                               1000001  ",
"                                1000001 ",
"                                 0000001",
"                                 0000000",
"                                 1000001",
"                                  10000 ",
"                                   01   "}; // 40

//void init_cursor();
GdkCursor *gdk_cursor_new_from_data(const gchar *bits[],
				    gint width, gint height,
				    GdkColor *fg, GdkColor *bg,
				    gint hot_x, gint hot_y);

#endif
