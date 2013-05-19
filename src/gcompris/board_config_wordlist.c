/* gcompris - board_config_wordlist.c
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

#include "gcompris.h"
#include "board_config_common.h"
#include <string.h>

typedef struct {
  GtkComboBox *combo_lang, *combo_level;
  GtkTextView *textview;
  GtkButton *button;
  const gchar *files;
  GcomprisWordlist *wordlist;
} user_param_type_wordlist;

static void _combo_level_changed(GtkComboBox *combo_level, gpointer user_data)
{
	LevelWordlist *lw;
	GSList *list;
	gchar **wordsArray, *text;
	guint level;
	int i;
	user_param_type_wordlist *w = (user_param_type_wordlist*)user_data;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (w->textview));

	level = gtk_combo_box_get_active(combo_level)+1;

	lw = gc_wordlist_get_levelwordlist(w->wordlist, level);
	if(!lw)
	{
		gtk_text_buffer_set_text(buffer, "", -1);
		gtk_widget_set_sensitive(GTK_WIDGET(w->button), FALSE);
		return;
	}

	wordsArray = g_malloc0(sizeof(gpointer)*(g_slist_length(lw->words)+1));

	for(i=0, list = lw->words; list; list=list->next)
	{
		wordsArray[i]=(gchar*)list->data;
		i++;
	}
	text = g_strjoinv(" ", wordsArray);
	g_free(wordsArray);

	gtk_text_buffer_set_text (buffer, g_strdup(text), -1);
	g_free(text);
	gtk_widget_set_sensitive(GTK_WIDGET(w->button), FALSE);
}

static void
_combo_lang_changed(GtkComboBox *combo_lang, gpointer user_data)
{
	user_param_type_wordlist *w = (user_param_type_wordlist*)user_data;
	const gchar *locale;
	gchar *filename, *temp, **tmp;
	int i;

	if(gtk_combo_box_get_active(combo_lang)<0)
		return;
	/* get the filename of the xml */
	locale = gc_locale_get_locale( _get_active_text(combo_lang) );
	if((tmp = g_strsplit(w->files, "$LOCALE", -1)))
	{
	  // Try the short locale
	  gchar *lang_short = gc_locale_short(locale);
	  filename = g_strjoinv(lang_short, tmp);
	  gchar *f_short = gc_file_find_absolute(filename);
	  if ( ! f_short )
	    {
	      // Try the long locale
	      g_free(filename);
	      gchar *lang_long = gc_locale_long(locale);
	      filename = g_strjoinv(lang_long, tmp);
	      f_short = gc_file_find_absolute(filename);
	    }
	  g_free(f_short);

	  g_strfreev(tmp);
	}
	else
		filename = g_strdup(w->files);

	/* clear combo level entry */
	if(w->wordlist)
	{
		gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model (w->combo_level)));
		gc_wordlist_free(w->wordlist);
	}
	/* load wordlist */
	w->wordlist = gc_wordlist_get_from_file(filename);
	g_free(filename);

	if ( ! w->wordlist )
	  g_error("Failed to find the wordlist file '%s'", filename);

	/* combo level */
	for(i=1; i <= w->wordlist->number_of_level; i++)
	{
		if((temp = g_strdup_printf("%d", i)))
		{
			gtk_combo_box_append_text(w->combo_level,temp);
			g_free(temp);
		}
	}

	/* add a new level*/
	if((temp = g_strdup_printf(D_(GETTEXT_GUI,"%d (New level)"), i)))
	{
		gtk_combo_box_append_text(w->combo_level,temp);
		g_free(temp);
	}
	gtk_combo_box_set_active(w->combo_level, 0);
	_combo_level_changed(w->combo_level, (gpointer)w);
}

static void _textview_changed(GtkWidget *w, gpointer data)
{
	user_param_type_wordlist *u = (user_param_type_wordlist*)data;
	gtk_widget_set_sensitive(GTK_WIDGET(u->button), TRUE);
}

static void _return_clicked(GtkWidget *w, gpointer data)
{
	int level;
	user_param_type_wordlist *u = (user_param_type_wordlist*)data;
	gchar *filename;

	filename = gc_file_find_absolute_writeable(u->wordlist->filename);
	g_free(filename);

	level = gtk_combo_box_get_active(u->combo_level)+1;
	_combo_lang_changed(u->combo_lang, u);
	gtk_combo_box_set_active(u->combo_level, level-1);
	_combo_level_changed(u->combo_level, u);
}

static void _button_clicked(GtkWidget *w, gpointer data)
{
	user_param_type_wordlist *u = (user_param_type_wordlist*)data;
	int level;
	gchar *text;
	GtkTextBuffer *buffer;
	GtkTextIter start_iter, end_iter;

	level = gtk_combo_box_get_active(u->combo_level)+1;
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (u->textview));

	gtk_text_buffer_get_start_iter(buffer, &start_iter);
	gtk_text_buffer_get_end_iter(buffer, &end_iter);
	text = gtk_text_buffer_get_slice(buffer, &start_iter, &end_iter, TRUE);
	gc_wordlist_set_wordlist(u->wordlist, level, -1,-1,-1, text);
	g_free(text);
	gc_wordlist_save(u->wordlist);
	_combo_lang_changed(u->combo_lang, u);
	gtk_combo_box_set_active(u->combo_level, level-1);
	_combo_level_changed(u->combo_level, u);
}

static void _destroy(GtkWidget *w, gpointer data)
{
	user_param_type_wordlist *u = (user_param_type_wordlist*)data;

	gc_wordlist_free(u->wordlist);
	g_free(u);
}

/**
 * Create a combo that let the user select the language
 */
static GtkWidget*
_create_lang_combo(const gchar *files) {
  GtkWidget *combo_lang = gtk_combo_box_new_text();
  GList *file_list;
  GList *list;
  const gchar *locale_name = gc_locale_get_name(  gc_locale_get() );

  file_list = gc_locale_gets_asset_list(files);
  for(list = file_list; list; list = list->next)
    {
      gtk_combo_box_append_text(GTK_COMBO_BOX(combo_lang), list->data);
      if(g_strcmp0( list->data, locale_name)==0)
	{
	  gtk_combo_box_set_active(GTK_COMBO_BOX(combo_lang),
				   g_list_position(file_list, list));
	}
    }
  if (g_list_length(file_list) > COMBOBOX_COL_MAX)
    gtk_combo_box_set_wrap_width    (GTK_COMBO_BOX(combo_lang),
				     g_list_length(file_list) / COMBOBOX_COL_MAX +1 );

  g_list_free(file_list);

  gtk_widget_show(combo_lang);

  return combo_lang;
}

/* wordlist edit */
GtkWidget *gc_board_config_wordlist(GcomprisBoardConf *config, const gchar *files)
{
	g_return_val_if_fail(config, NULL);
	GtkWidget *combo_lang, *combo_level, *label;
	GtkWidget *hbox, *vbox, *frame;
	GtkWidget *textview;
	GtkWidget *button;
	user_param_type_wordlist *user_data;

	/* frame */
	frame = gtk_frame_new(D_(GETTEXT_GUI,"Configure the list of words"));
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(config->main_conf_box), frame, FALSE, FALSE, 8);

	vbox = gtk_vbox_new(FALSE, 8);
	gtk_widget_show(vbox);

	gtk_container_add(GTK_CONTAINER(frame), vbox);
	/* Combo_box lang */
	combo_lang = _create_lang_combo(files);
	hbox = gtk_hbox_new(FALSE, 8);
	label = gtk_label_new(D_(GETTEXT_GUI,"Choose a language"));
	gtk_widget_show(label);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 8);
	gtk_box_pack_start(GTK_BOX(hbox), combo_lang, FALSE, FALSE, 8);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 8);

	/* Combo_box level */
	combo_level = gtk_combo_box_new_text();

	gtk_widget_show(combo_level);
	hbox = gtk_hbox_new(FALSE, 8);
	label = gtk_label_new(D_(GETTEXT_GUI,"Choice of the level"));
	gtk_widget_show(label);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 8);
	gtk_box_pack_start(GTK_BOX(hbox), combo_level, FALSE, FALSE, 8);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 8);

	/* scroll and Textview */
	GtkWidget *scroll = gtk_scrolled_window_new ( NULL, NULL);
	gtk_scrolled_window_set_policy  (GTK_SCROLLED_WINDOW(scroll),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(scroll, -1, 100);
	gtk_widget_show( scroll);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, FALSE, FALSE, 8);

	textview = gtk_text_view_new();
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview), GTK_WRAP_WORD_CHAR);
	gtk_widget_show(textview);
	gtk_container_add (GTK_CONTAINER(scroll), textview);

	/* valid button */
	hbox = gtk_hbox_new(FALSE, 8);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 8);

	GtkWidget * b_default = gtk_button_new_with_label(D_(GETTEXT_GUI,"Back to default"));
	gtk_widget_show(b_default);
	gtk_box_pack_start(GTK_BOX(hbox), b_default, FALSE, FALSE, 8);

	button = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	gtk_widget_show(button);
	gtk_widget_set_sensitive(button, FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 8);

	/* user_data */
	user_data = g_malloc0(sizeof(user_param_type_wordlist));
	user_data -> combo_lang = GTK_COMBO_BOX(combo_lang);
	user_data -> combo_level = GTK_COMBO_BOX(combo_level);
	user_data -> textview = GTK_TEXT_VIEW(textview);
	user_data -> button = GTK_BUTTON(button);
	user_data -> files = files;

	g_signal_connect(G_OBJECT(combo_lang), "changed",
		G_CALLBACK(_combo_lang_changed), (gpointer)user_data);
	g_signal_connect(G_OBJECT(combo_level), "changed",
		G_CALLBACK(_combo_level_changed), (gpointer)user_data);
	g_signal_connect(G_OBJECT(combo_lang), "destroy",
		G_CALLBACK(_destroy), (gpointer)user_data);
	g_signal_connect(G_OBJECT(gtk_text_view_get_buffer(user_data->textview)), "changed",
		G_CALLBACK(_textview_changed), (gpointer)user_data);
	g_signal_connect(G_OBJECT(button), "clicked",
		G_CALLBACK(_button_clicked), (gpointer)user_data);
	g_signal_connect(G_OBJECT(b_default), "clicked",
		G_CALLBACK(_return_clicked), (gpointer)user_data);


	_combo_lang_changed(GTK_COMBO_BOX(combo_lang), user_data);
	return NULL;
}

