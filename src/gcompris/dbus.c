/* gcompris - timer.c
 *
 * Copyright (C) 2007 Bruno coudoin
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

#include "gcompris.h"
#include <gc_core.h>

#if USE_DBUS
#include <dbus/dbus-glib.h>
#include <glib-object.h>
#include <dbus/dbus.h>
#define	SUPPORT_OR_RETURN(rv)	{}
#else
#define	SUPPORT_OR_RETURN(rv)	{ return rv; }
#endif

#define _ACTIVITY_SERVICE_NAME	"org.laptop.Activity"
#define _ACTIVITY_SERVICE_PATH	"/org/laptop/Activity"
#define _ACTIVITY_INTERFACE	"org.laptop.Activity"

void _set_active(DBusGProxy *proxy, gint active, void *dummy)
{
  printf("_set_active %d\n", active);
  if(active)
    gc_sound_close();
  else
    gc_sound_reopen();

}

void
gc_dbus_init(gchar *sugarActivityId)
{
  SUPPORT_OR_RETURN();

#ifdef USE_DBUS
  DBusGConnection *connection;
  GError *error;
  DBusGProxy *proxy;

  g_type_init ();

  error = NULL;
  connection = dbus_g_bus_get (DBUS_BUS_SESSION,
                               &error);
  if (connection == NULL)
    {
      g_printerr ("Failed to open connection to bus: %s\n",
                  error->message);
      g_error_free (error);
      exit (1);
    }

  gchar *service_name = g_strdup_printf("%s%s",
					_ACTIVITY_SERVICE_NAME,
					sugarActivityId);
  gchar *object_path  = g_strdup_printf("%s/%s",
					_ACTIVITY_SERVICE_PATH,
					sugarActivityId);

  proxy = dbus_g_proxy_new_for_name (connection,
                                     service_name,
                                     object_path,
                                     _ACTIVITY_INTERFACE);

  dbus_g_proxy_add_signal(proxy, "set_active", G_TYPE_BOOLEAN);
  dbus_g_proxy_connect_signal(proxy, "set_active",
			      G_CALLBACK(_set_active), NULL, NULL);
#endif
}
