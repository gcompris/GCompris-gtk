/* gcompris - gc_core.h
 *
 * Copyright (C) 2006, 2008 Bruno Coudoin
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/** These are the functions shared in the GCompris core only, not available to
 *  activity code
 */
#ifndef _GC_CORE_H_
#define _GC_CORE_H_

double gc_zoom_factor_get();

/** The internal help  API */
void gc_help_start (GcomprisBoard *gcomprisBoard);
void gc_help_stop ();
gboolean gc_help_has_board (GcomprisBoard *gcomprisBoard);

/** about dialog */
void gc_about_start (void);
void gc_about_stop (void);

/** miscelaneous functions */
void gc_exit();
void gc_fullscreen_set(gboolean state);
int file_end_with_xml(const gchar *file);

/** menu */
void gc_menu_load();
void gc_menu_destroy(void);

/** log */
void gc_log_start (GcomprisBoard *gcomprisBoard);

/** dbus */
void gc_dbus_init(gchar *sugarActivityId);

#endif
