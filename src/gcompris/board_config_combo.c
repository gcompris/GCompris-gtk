/* gcompris - board_config_combo.c
 *
 * Copyright (C) 2009 Miguel De Izarra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */


#include "board_config.h"
#include "board_config_common.h"
#include <string.h>

/***********************************************/
/* L10n                                        */
/***********************************************/

/** \brief return the list of locales in which GCompris has been translated
 *         even partialy.
 *
 * \note The list is calculated at the first call and must not be freed.
 *       Uppon next call, the same list is returned.
 *
 * \return a list containing the locales we suport
 */
GList*
gc_locale_gets_list(){

  static GList *gcompris_locales_list = NULL;

  GcomprisProperties *properties = gc_prop_get();
  GDir   *textdomain_dir;
  GError **error = NULL;
  GList  *locales = NULL;

  if(gcompris_locales_list)
    return(gcompris_locales_list);

  /* There is no english locale but it exists anyway */
  locales = g_list_append(locales, g_strdup("en"));

  textdomain_dir = g_dir_open (properties->package_locale_dir, 0, error);
  const gchar *fname;
  gchar *fname_abs;
  gchar *catalog;

  if(textdomain_dir) {
    while ((fname = g_dir_read_name(textdomain_dir))) {
      fname_abs = g_strdup_printf("%s/%s", properties->package_locale_dir, fname);
      if (!g_file_test(fname_abs, G_FILE_TEST_IS_DIR))
        continue;

      catalog = g_strdup_printf("%s/LC_MESSAGES/gcompris.mo", fname_abs);

      if (g_file_test(catalog, G_FILE_TEST_EXISTS)){
      locales = g_list_append(locales, g_strdup(fname));
      }
      g_free (fname_abs);
      g_free(catalog);
    }
  g_dir_close (textdomain_dir);
  }

#if DEBUG	/* to find locales list when run from src directory */
  if(g_file_test("po", G_FILE_TEST_IS_DIR))
  {
  	textdomain_dir= g_dir_open("po",0, error);
	while((fname = g_dir_read_name(textdomain_dir)))
	{
		if(g_str_has_suffix(fname,".po"))
			locales = g_list_append(locales, g_strndup(fname,2));
	}
	g_dir_close(textdomain_dir);

  }
#endif
  /* Save it for next call */
  gcompris_locales_list = locales;

  return locales;
}


void
gc_board_config_combo_locales_changed(GtkComboBox *combobox,
			       gpointer data)
{
  _gc_boardconf_key *u = (_gc_boardconf_key*)data;
  gchar *the_key = g_strdup((gchar *)u->key);
  gchar *value;
  gint index = gtk_combo_box_get_active (combobox);

  if (index == 0)
    /* Default value of gcompris selected */
    value = g_strdup ("NULL");
  else
    value = _get_active_text (combobox);

  g_hash_table_replace(u->config->hash_conf, (gpointer) the_key, (gpointer) value);
}

/* key = "locale" */
GtkComboBox*
gc_board_config_combo_locales(GcomprisBoardConf *config, gchar *init)
{
  g_return_val_if_fail(config, NULL);
  GtkWidget *combobox;
  GtkWidget *hbox = gtk_hbox_new (FALSE, 8);
  GList *list, *strings;
  GtkWidget *label_combo;
  gint init_index = 0;

  strings = gc_locale_gets_list();

  strings = g_list_prepend( strings, _("Default"));

  if (init)
    init_index = g_list_position(strings,
				 g_list_find_custom(strings,
						    (gconstpointer) init,
						    (GCompareFunc) strcmp));

  if (init_index < 0)
    init_index=0;

  gtk_widget_show(hbox);

  gtk_box_pack_start (GTK_BOX(config->main_conf_box),
		      hbox,
		      FALSE,
		      FALSE,
		      0);

  /* Label */
  label_combo = gtk_label_new ((gchar *)NULL);
  gtk_widget_show(label_combo);
  gtk_box_pack_start (GTK_BOX(hbox),
		      label_combo,
		      FALSE,
		      FALSE,
		      0);

  gtk_label_set_justify (GTK_LABEL(label_combo),
			 GTK_JUSTIFY_RIGHT);

  gtk_label_set_markup (GTK_LABEL(label_combo),
                        _("Select the language\n to use in the board"));

  combobox = GTK_WIDGET(gtk_combo_box_new_text());

  gtk_widget_show(combobox);

  gtk_box_pack_start (GTK_BOX(hbox),
		      combobox,
		      FALSE,
		      FALSE,
		      0);


  for (list = strings; list != NULL; list = list->next)
    gtk_combo_box_append_text       (GTK_COMBO_BOX(combobox),
				     list->data);

  if (g_list_length(strings) > COMBOBOX_COL_MAX)
    gtk_combo_box_set_wrap_width    (GTK_COMBO_BOX(combobox),
  	     g_list_length(strings) / COMBOBOX_COL_MAX +1 );

  gtk_combo_box_set_active (GTK_COMBO_BOX(combobox),
			    init_index);
  _gc_boardconf_key *u = g_malloc0(sizeof(_gc_boardconf_key));
  u -> key = g_strdup("locale");
  u -> config = config;
  g_signal_connect(G_OBJECT(combobox),
		   "changed",
		   G_CALLBACK(gc_board_config_combo_locales_changed),
		   u);
  g_signal_connect(G_OBJECT(combobox),
  	"destroy",
	G_CALLBACK(_gc_destroy_boardconf_key),
	u);

  return GTK_COMBO_BOX(combobox);

}

void
gc_board_config_combo_drag_changed(GtkComboBox *combobox,
			       gpointer data)
{
  _gc_boardconf_key *u = (_gc_boardconf_key*)data;
  gchar *the_key = g_strdup((gchar *)u->key);
  gchar *value;
  gint index = gtk_combo_box_get_active (combobox);

  if (index == 0)
    /* Default value of gcompris selected */
    value = g_strdup ("NULL");
  else
    value = g_strdup_printf("%d", index);

  g_hash_table_replace(u->config->hash_conf, (gpointer) the_key, (gpointer) value);
}

/* key = "locale" */
GtkComboBox*
gc_board_config_combo_drag(GcomprisBoardConf *config, gint init)
{
  g_return_val_if_fail(config, NULL);
  GtkWidget *combobox;
  GtkWidget *hbox = gtk_hbox_new (FALSE, 8);
  GList *list, *strings;
  GtkWidget *label_combo;
  gint init_index;

  strings = NULL;

  strings = g_list_prepend( strings, _("Global GCompris mode"));
  strings = g_list_append( strings, _("Normal"));
  strings = g_list_append( strings, _("2 clicks"));
  strings = g_list_append( strings, _("both modes"));

  if (init < 0)
    init_index =0;
  else
    init_index = init;

  gtk_widget_show(hbox);

  gtk_box_pack_start (GTK_BOX(config->main_conf_box),
		      hbox,
		      FALSE,
		      FALSE,
		      0);

  /* Label */
  label_combo = gtk_label_new ((gchar *)NULL);
  gtk_widget_show(label_combo);
  gtk_box_pack_start (GTK_BOX(hbox),
		      label_combo,
		      FALSE,
		      FALSE,
		      0);

  gtk_label_set_justify (GTK_LABEL(label_combo),
			 GTK_JUSTIFY_RIGHT);

  gtk_label_set_markup (GTK_LABEL(label_combo),
                        _("Select the drag and drop mode\n to use in the board"));

  combobox = GTK_WIDGET(gtk_combo_box_new_text());

  gtk_widget_show(combobox);

  gtk_box_pack_start (GTK_BOX(hbox),
		      combobox,
		      FALSE,
		      FALSE,
		      0);


  for (list = strings; list != NULL; list = list->next)
    gtk_combo_box_append_text       (GTK_COMBO_BOX(combobox),
				     list->data);

  if (g_list_length(strings) > COMBOBOX_COL_MAX)
    gtk_combo_box_set_wrap_width    (GTK_COMBO_BOX(combobox),
  	     g_list_length(strings) / COMBOBOX_COL_MAX +1 );

  gtk_combo_box_set_active (GTK_COMBO_BOX(combobox),
			    init_index);
  _gc_boardconf_key *u = g_malloc0(sizeof(_gc_boardconf_key));
  u->key = g_strdup("drag_mode");
  u->config = config;

  g_signal_connect(G_OBJECT(combobox),
		   "changed",
		   G_CALLBACK(gc_board_config_combo_drag_changed),
		   u);
  g_signal_connect(G_OBJECT(combobox),
  	"destroy",
	G_CALLBACK(_gc_destroy_boardconf_key),
	u);
  return GTK_COMBO_BOX(combobox);

}

static gchar *current_locale = NULL;
void
gc_locale_change(gchar *locale)
{
  if (!locale)
    return;

  if (strcmp(locale, "NULL") == 0){
    gc_locale_reset();
    return;
  }

  current_locale = g_strdup(gc_locale_get());

  gc_locale_set(locale);
}

void
gc_locale_reset()
{
  if (current_locale == NULL)
    return;

  gc_locale_change(current_locale);

  g_free(current_locale);
  current_locale = NULL;
}


/** \brief Search the given file for each locale and returns the locale list
 *
 * \param file: the file to search. In order to work, you need to provide a
 *              filename that includes a $LOCALE in it like:
 *              voices/$LOCALE/colors/blue.ogg
 *
 * \return a list of locale
 */
GList*
gc_locale_gets_asset_list(const gchar *filename)
{
  GList *locales, *list, *locales_asset = NULL;
  gchar *abs_filename;

  locales = gc_locale_gets_list();

  for (list = locales; list != NULL; list = list->next)
    {
      gchar **tmp;

      /* Check there is a $LOCALE to replace */
      if((tmp = g_strsplit(filename, "$LOCALE", -1)))
	{
	  gchar locale[16];
	  gchar *filename2;

	  /* try with the locale */
	  g_strlcpy(locale, list->data, sizeof(locale));
	  filename2 = g_strjoinv(locale, tmp);
	  g_warning("trying locale file '%s'\n", filename2);
	  abs_filename = gc_file_find_absolute(filename2);
	  g_free(filename2);

	  g_strfreev(tmp);
	}
      else
	{
	  abs_filename = gc_file_find_absolute(filename);
	}

      if(abs_filename)
      {
	/* It would be cleaner to provide the real locale name but then we need a way
	 * to get back the locale code from it's name and from the boards
	 *
	 * locales_asset = g_list_append(locales_asset, gc_locale_get_name(list->data));
	 *
	 */
	locales_asset = g_list_append(locales_asset, list->data);
	g_free(abs_filename);
	}
    }


  return locales_asset;
}

/* key = "locale_sound" */
GtkComboBox *gc_board_config_combo_locales_asset(GcomprisBoardConf *config, const gchar *label,
					  gchar *init,
					  const gchar *file)
{
  g_return_val_if_fail(config, NULL);
  GtkWidget *combobox;
  GtkWidget *hbox = gtk_hbox_new (FALSE, 8);
  GList *list, *strings;
  GtkWidget *label_combo;
  gint init_index = 0;

  strings = gc_locale_gets_asset_list(file);

  strings = g_list_prepend( strings, _("Default"));

  if (init)
    {
      init_index =  g_list_position(strings,
				    g_list_find_custom(strings,
						       (gconstpointer)init,
						       (GCompareFunc) strcmp));
    }

  if (init_index < 0)
    init_index=0;

  gtk_widget_show(hbox);

  gtk_box_pack_start (GTK_BOX(config->main_conf_box),
		      hbox,
		      FALSE,
		      FALSE,
		      0);

  /* Label */
  label_combo = gtk_label_new ((gchar *)NULL);
  gtk_widget_show(label_combo);
  gtk_box_pack_start (GTK_BOX(hbox),
		      label_combo,
		      FALSE,
		      FALSE,
		      0);

  gtk_label_set_justify (GTK_LABEL(label_combo),
			 GTK_JUSTIFY_RIGHT);

  gtk_label_set_markup (GTK_LABEL(label_combo),
                        label);

  combobox = GTK_WIDGET(gtk_combo_box_new_text());

  gtk_widget_show(combobox);

  gtk_box_pack_start (GTK_BOX(hbox),
		      combobox,
		      FALSE,
		      FALSE,
		      0);


  for (list = strings; list != NULL; list = list->next)
    gtk_combo_box_append_text(GTK_COMBO_BOX(combobox),
			      list->data);

  if (g_list_length(strings) > COMBOBOX_COL_MAX)
    gtk_combo_box_set_wrap_width(GTK_COMBO_BOX(combobox),
				 g_list_length(strings) / COMBOBOX_COL_MAX +1 );

  gtk_combo_box_set_active (GTK_COMBO_BOX(combobox),
			    init_index);

  _gc_boardconf_key *u = g_malloc0(sizeof(_gc_boardconf_key));
  u -> key = g_strdup("locale_sound");
  u -> config = config;

  g_signal_connect(G_OBJECT(combobox),
		   "destroy",
		   G_CALLBACK(_gc_destroy_boardconf_key),
		   u);
  g_signal_connect(G_OBJECT(combobox),
		   "changed",
		   G_CALLBACK(gc_board_config_combo_locales_changed),
		   u);

  return GTK_COMBO_BOX(combobox);
}

static void
_combo_box_changed(GtkComboBox *combobox,
		   gpointer data)
{
  _gc_boardconf_key *u = (_gc_boardconf_key*)data;
  gchar *the_key = g_strdup(u->key);

  gchar *value = g_strdup_printf("%s", _get_active_text (combobox));

  g_hash_table_replace(u->config->hash_conf, (gpointer) the_key, (gpointer) value);
}


GtkComboBox *gc_board_config_combo_box(GcomprisBoardConf *config, const gchar *label, GList *strings, gchar *key, gchar *init)
{
  g_return_val_if_fail(config, NULL);
  check_key(key);
  GtkWidget *combobox;
  GtkWidget *hbox = gtk_hbox_new (FALSE, 8);
  GList *list;
  GtkWidget *label_combo;
  gint init_index = 0;

  if (init)
    init_index =  g_list_position ( strings, g_list_find_custom ( strings,(gconstpointer)  init, (GCompareFunc) strcmp));

  if (init_index < 0)
    init_index=0;

  gtk_widget_show(hbox);

  gtk_box_pack_start (GTK_BOX(config->main_conf_box),
		      hbox,
		      FALSE,
		      FALSE,
		      0);

  /* Label */
  label_combo = gtk_label_new ((gchar *)NULL);
  gtk_widget_show(label_combo);
  gtk_box_pack_start (GTK_BOX(hbox),
		      label_combo,
		      FALSE,
		      FALSE,
		      0);

  gtk_label_set_justify (GTK_LABEL(label_combo),
			 GTK_JUSTIFY_RIGHT);

  gtk_label_set_markup (GTK_LABEL(label_combo),
                        (const gchar *)label);


  combobox = GTK_WIDGET(gtk_combo_box_new_text());

  gtk_widget_show(combobox);

  gtk_box_pack_start (GTK_BOX(hbox),
		      combobox,
		      FALSE,
		      FALSE,
		      0);


  for (list = strings; list != NULL; list = list->next)
    gtk_combo_box_append_text       (GTK_COMBO_BOX(combobox),
				     list->data);

  if (g_list_length(strings) > COMBOBOX_COL_MAX)
    gtk_combo_box_set_wrap_width    (GTK_COMBO_BOX(combobox),
  	     g_list_length(strings) / COMBOBOX_COL_MAX +1 );

  gtk_combo_box_set_active (GTK_COMBO_BOX(combobox),
			    init_index);
  _gc_boardconf_key *u = g_malloc0(sizeof(_gc_boardconf_key));
  u -> key = g_strdup(key);
  u -> config = config;
  g_signal_connect(G_OBJECT(combobox),
		   "changed",
		   G_CALLBACK(_combo_box_changed),
		   u);
  g_signal_connect(G_OBJECT(combobox),
  	"destroy",
	G_CALLBACK(_gc_destroy_boardconf_key),
	u);

  return GTK_COMBO_BOX(combobox);
}

