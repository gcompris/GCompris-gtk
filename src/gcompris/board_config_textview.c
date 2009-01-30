/* gcompris - board_config_textview.c
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

/****************************************/
/* TextView                             */

typedef struct {
  gchar *key;
  GcomprisTextCallback callback;
  GtkLabel *feedback;
  GtkTextBuffer *TextBuffer;
  GcomprisBoardConf *config;
} user_param_type;

static void *
_textview_destroy (GtkButton *button,
		   gpointer user_data)
{
  g_free(((user_param_type *)user_data)->key);
  g_free(user_data);

  return NULL;
}


static void *
_textbuffer_changed (GtkTextBuffer *buffer,
		     gpointer user_data)
{
  gtk_widget_set_sensitive        (GTK_WIDGET(user_data),
				   TRUE);

  return NULL;
}

static void *
_textview_yes (GtkButton *button,
	       gpointer user_data)
{

  user_param_type *params= (user_param_type *) user_data;

  gchar *key = params->key;
  GcomprisTextCallback validate = params->callback;
  GtkLabel *label = params->feedback;
  GtkTextBuffer *text_buffer = params->TextBuffer;

  GtkTextIter start_iter;
  GtkTextIter end_iter;

  gtk_text_buffer_get_start_iter  (text_buffer,
                                   &start_iter);

  gtk_text_buffer_get_end_iter  (text_buffer,
				 &end_iter);

  /* has this to be freed ? */
  gchar *text = gtk_text_buffer_get_slice (text_buffer,
					   &start_iter,
					   &end_iter,
					   TRUE);



  gchar *in_memoriam_text = g_strdup (text);
  gchar *in_memoriam_key = g_strdup (key);

  if (validate( key, text, label)){
    g_hash_table_replace (params->config->hash_conf, (gpointer) in_memoriam_key, (gpointer) in_memoriam_text);
    gtk_widget_set_sensitive        (GTK_WIDGET(button),
				     FALSE);
  }
  else {
    g_free (in_memoriam_text);
    g_free (in_memoriam_key);
  }
  g_free(text);

  return NULL;
}

GtkTextView *
gc_board_config_textview(GcomprisBoardConf *config,
		  const gchar *label,
		  gchar *key,
		  const gchar*description,
		  gchar *init_text,
		  GcomprisTextCallback validate)
{
  g_return_val_if_fail(config, NULL);
  GtkWidget*frame =  gtk_frame_new ("GCompris text tool");
  gtk_widget_show(frame);

  gtk_box_pack_start (GTK_BOX(config->main_conf_box),
		      frame,
		      FALSE,
		      FALSE,
		      8);



  /* Main vbox for all our widegt */
  GtkWidget *textVbox = gtk_vbox_new ( FALSE, 8);
  gtk_widget_show(textVbox);

  gtk_container_add(GTK_CONTAINER(frame),
		    textVbox);
  /* Title */
  GtkWidget *title = gtk_label_new ((gchar *)NULL);
  gtk_widget_show(title);

  gtk_box_pack_start (GTK_BOX(textVbox),
		      title,
		      FALSE,
		      FALSE,
		      8);

  gtk_label_set_justify (GTK_LABEL(title),
			 GTK_JUSTIFY_CENTER);

  gchar *title_text = g_strdup(label);
  gtk_label_set_markup (GTK_LABEL(title),
                        (const gchar *)title_text);

  GtkWidget *separator = gtk_hseparator_new ();

  gtk_widget_show(separator);

  gtk_box_pack_start (GTK_BOX(textVbox),
		      separator,
		      FALSE,
		      FALSE,
		      0);

  /* Description */
  GtkWidget *desc = gtk_label_new ((gchar *)NULL);
  gtk_widget_show(desc);

  gtk_box_pack_start (GTK_BOX(textVbox),
		      desc,
		      FALSE,
		      FALSE,
		      0);

  //gtk_label_set_justify (GTK_LABEL(title),
  //		 GTK_JUSTIFY_CENTER);

  gtk_label_set_line_wrap(GTK_LABEL(desc), TRUE);

  gchar *desc_text = g_strdup(description);
  gtk_label_set_markup (GTK_LABEL(desc),
                        (const gchar *)desc_text);

  GtkWidget *scroll = gtk_scrolled_window_new ( NULL, NULL);


  gtk_scrolled_window_set_policy  (GTK_SCROLLED_WINDOW(scroll),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC);

  gtk_widget_set_size_request     (scroll,
				   -1,
				   100);

  gtk_widget_show( scroll);

  gtk_box_pack_start (GTK_BOX(textVbox),
		      scroll,
		      FALSE,
		      FALSE,
		      0);

  /* TextView */
  GtkWidget *textView = gtk_text_view_new ();
  gtk_widget_show(textView);

  gtk_container_add (GTK_CONTAINER(scroll),
		     textView);

  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textView), GTK_WRAP_WORD_CHAR);


  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textView));

  gtk_text_buffer_set_text (buffer, g_strdup(init_text), -1);


  /* hbox for feedback and validation button */
  GtkWidget *validationHbox = gtk_vbox_new ( FALSE, 8);
  gtk_widget_show(validationHbox);

  gtk_box_pack_start (GTK_BOX(textVbox),
		      validationHbox,
		      FALSE,
		      FALSE,
		      0);

  /* Feedback */
  GtkWidget *feedback = gtk_label_new ((gchar *)NULL);
  gtk_widget_show(feedback);

  gtk_box_pack_start (GTK_BOX(validationHbox),
		      feedback,
		      FALSE,
		      FALSE,
		      0);

  gtk_label_set_justify (GTK_LABEL(title),
			 GTK_JUSTIFY_FILL);

  gtk_label_set_line_wrap(GTK_LABEL(feedback), TRUE);

  user_param_type *user_param = g_malloc0(sizeof(user_param_type));

  user_param->key = g_strdup(key);
  user_param->callback = validate;
  user_param->feedback = GTK_LABEL(feedback);
  user_param->TextBuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(textView));
  user_param->config = config;


  /* vbox for feedback and validation button */
  GtkWidget *validationVbox = gtk_hbox_new ( FALSE, 8);
  gtk_widget_show(validationVbox);

  gtk_box_pack_end (GTK_BOX(validationHbox),
		    validationVbox,
		    FALSE,
		    FALSE,
		    0);


  /* Validate button */
  GtkWidget *button =  gtk_button_new_from_stock (GTK_STOCK_YES);
  gtk_widget_show(button);
  gtk_box_pack_end (GTK_BOX(validationVbox),
		    button,
		    FALSE,
		    FALSE,
		    0);

  g_signal_connect(G_OBJECT(button),
		   "clicked",
		   G_CALLBACK(_textview_yes),
		   (gpointer) user_param);


  g_signal_connect(G_OBJECT(button),
		   "destroy",
		   G_CALLBACK(_textview_destroy),
		   (gpointer) user_param);

  g_signal_connect(G_OBJECT(user_param->TextBuffer),
		   "changed",
		   G_CALLBACK(_textbuffer_changed),
		   (gpointer) button);

  gtk_widget_set_sensitive (button,
			    FALSE);

  return GTK_TEXT_VIEW(textView);
}

