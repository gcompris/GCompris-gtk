/* gcompris - properties.h
 *
 * Time-stamp: <2001/12/27 00:58:20 bruno>
 *
 * Copyright (C) 2000 Bruno Coudoin
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

#ifndef _PROPERTIES_H_
#define _PROPERTIES_H_

typedef struct {
  gint		music;
  gint		fx;
  gint		fullscreen;
  gchar	       *locale;
} GcomprisProperties;

GcomprisProperties	*gcompris_get_properties ();
GcomprisProperties	*gcompris_properties_new ();
void			 gcompris_properties_destroy (GcomprisProperties *props);
GcomprisProperties	*gcompris_properties_copy (GcomprisProperties *props);
void			 gcompris_properties_save (GcomprisProperties *props);

#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
