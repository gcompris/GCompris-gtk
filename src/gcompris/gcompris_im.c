/* gcompris - gcompris_im.c
 *
 * Copyright (C) 2000, 2008 Bruno Coudoin
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

#include "gcompris.h"

static gint im_context_commit_callback (GtkIMContext *imcontext,
					gchar *arg1,
					gpointer user_data);

static gint im_context_preedit_callback (GtkIMContext *imcontext,
					 gpointer user_data);

static gint window_focus_callback (GtkWidget *widget,
				   GdkEventFocus *event,
				   gpointer user_data);


void
gc_im_init(GtkWidget *window)
{
  GcomprisProperties	*properties = gc_prop_get ();


  /* set IMContext */
  properties->context = gtk_im_multicontext_new ();

  if(gc_profile_get_current())
    {
      GHashTable *init_im =  g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
      init_im = gc_db_conf_with_table_get (gc_profile_get_current()->profile_id,
					      -1,
					      init_im );

      properties->default_context = g_hash_table_lookup ( init_im, "default_im");

      if (properties->default_context)
	g_setenv ("GTK_IM_MODULE",
		  properties->default_context,
		  TRUE);
    }

  gtk_im_context_set_client_window (properties->context,
                                    window->window);

  g_signal_connect (G_OBJECT (properties->context), "commit",
		    G_CALLBACK(im_context_commit_callback), NULL);

  g_signal_connect (G_OBJECT (properties->context), "preedit-changed",
		    G_CALLBACK(im_context_preedit_callback), NULL);

  g_signal_connect (GTK_WIDGET (window), "focus-in-event",
		    G_CALLBACK(window_focus_callback), NULL);

  g_signal_connect (GTK_WIDGET (window), "focus-out-event",
		    G_CALLBACK(window_focus_callback), NULL);

}

void
gc_im_reset()
{
  GcomprisProperties	*properties = gc_prop_get ();
  gtk_im_context_reset (properties->context);
}


static gint
im_context_commit_callback (GtkIMContext *imcontext,
			    gchar *arg1,
			    gpointer user_data)
{
  if (gc_board_get_current_board_plugin()!=NULL && gc_board_get_current_board_plugin()->key_press)
    {
      int result = gc_board_get_current_board_plugin()->key_press (0, arg1, NULL);
      //      g_free(arg1);
      return(result);
    }
  return FALSE;
}

static gint
im_context_preedit_callback (GtkIMContext *imcontext,
			     gpointer user_data)
{
  gchar *preedit_string;
  PangoAttrList *attrs;
  gint cursor_pos;

  gtk_im_context_get_preedit_string (imcontext,
				     &preedit_string,
				     &attrs,
				     &cursor_pos);

  if (gc_board_get_current_board_plugin()!=NULL && gc_board_get_current_board_plugin()->key_press)
    {
      int result = gc_board_get_current_board_plugin()->key_press (0, NULL, preedit_string);
      g_free(preedit_string);
      pango_attr_list_unref (attrs);
      return(result);
    }
  return FALSE;
}

static gint
window_focus_callback (GtkWidget *widget,
		       GdkEventFocus *event,
		       gpointer user_data)
{
  GcomprisProperties	*properties = gc_prop_get ();
  if (event->in)
    gtk_im_context_focus_in (properties->context);
  else
    gtk_im_context_focus_out (properties->context);

  return FALSE;
}
