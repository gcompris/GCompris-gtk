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

/*
 * To test it:
 * run GCompris with  --sugarBundleId a --sugarActivityId a
 * dbus-send --dest='org.laptop.Activitya'
 *           /org/laptop/Activity/a org.laptop.Activity.SetActive boolean:true
 *
 */
#include "gcompris.h"
#include <gc_core.h>
#include "gc-marshal.h"

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

#if USE_DBUS
void _SetActive(DBusGProxy *proxy, gint active, void *dummy)
{
  printf("_SetActive %d\n", active);
  if(active)
    gc_sound_close();
  else
    gc_sound_reopen();

}
#endif

void
gc_dbus_init(gchar *sugarActivityId)
{
  SUPPORT_OR_RETURN();

#ifdef USE_DBUS
  DBusGConnection *connection;
  GError *error;
  DBusGProxy *proxy;

  g_type_init ();

  dbus_g_thread_init ();

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

  dbus_g_object_register_marshaller (gc_marshal_VOID__BOOLEAN,
				     G_TYPE_NONE, G_TYPE_BOOLEAN, G_TYPE_INVALID);

  proxy = dbus_g_proxy_new_for_name (connection,
                                     service_name,
                                     object_path,
                                     _ACTIVITY_INTERFACE);

  dbus_g_proxy_add_signal(proxy, "SetActive", G_TYPE_BOOLEAN, G_TYPE_INVALID);
  dbus_g_proxy_connect_signal(proxy, "SetActive",
			      G_CALLBACK(_SetActive), NULL, NULL);
#endif
}
