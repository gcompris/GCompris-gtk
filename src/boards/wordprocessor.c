/* gcompris - wordprocessor.c
 *
 * Copyright (C) 2006 Bruno Coudoin
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#include "gcompris/gcompris.h"

/*
 * Predefined styles
 * -----------------
 */
typedef struct {
  gchar *name;
  gchar *font;
  PangoWeight weight;
  GtkJustification justification;
  gint indent;
  gint pixels_above_lines;
  gint pixels_below_lines;
  gchar *foreground;
} style_t;

static style_t doctype_default[] =
  {
    { "H0", "Serif 30", PANGO_WEIGHT_ULTRABOLD,  GTK_JUSTIFY_CENTER, 0,  40, 20, "black"},
    { "H1", "Serif 26", PANGO_WEIGHT_BOLD,       GTK_JUSTIFY_LEFT,   0,  30, 15, "black" },
    { "H2", "Serif 20", PANGO_WEIGHT_SEMIBOLD,   GTK_JUSTIFY_LEFT,   0,  20, 12, "black" },
    { "P",  "Serif 16", PANGO_WEIGHT_NORMAL,     GTK_JUSTIFY_LEFT,   30, 3,  3,  "black" }
  };
#define NUMBER_OF_STYLE G_N_ELEMENTS(doctype_default)

static style_t doctype_love_letter[] =
  {
    { "Hx0", "Serif 30", PANGO_WEIGHT_ULTRABOLD,  GTK_JUSTIFY_CENTER, 0,  40, 20, "DeepPink" },
    { "Hx1", "Serif 26", PANGO_WEIGHT_BOLD,       GTK_JUSTIFY_LEFT,   0,  30, 15, "HotPink" },
    { "Hx2", "Serif 20", PANGO_WEIGHT_SEMIBOLD,   GTK_JUSTIFY_LEFT,   0,  20, 12, "MediumOrchid" },
    { "Px",  "Serif 16", PANGO_WEIGHT_NORMAL,     GTK_JUSTIFY_LEFT,   30, 3,  3,  "black" }
  };

static style_t *doctype_list[] =
  {
    doctype_default,
    doctype_love_letter
  };

static GcomprisBoard	*gcomprisBoard = NULL;
static gboolean		 board_paused = TRUE;
static GtkWidget	*gtk_combo_filetypes = NULL;

static void	 start_board (GcomprisBoard *agcomprisBoard);
static void	 pause_board (gboolean pause);
static void	 end_board (void);
static gboolean	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 set_level (guint level);
static gboolean  key_press_event (GtkWidget *text_view,
				  GdkEventKey *event);

static GnomeCanvasGroup *boardRootItem = NULL;

static GnomeCanvasItem	*wordprocessor_create(void);
static void		 wordprocessor_destroy_all_items(void);
static gint		 item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static void		 display_style_buttons(GnomeCanvasGroup *boardRootItem, 
					       int x,
					       int y);
static void		 create_tags (GtkTextBuffer *buffer, style_t style[]);
static void		 set_default_style (GtkTextBuffer *buffer, style_t *style);
static void		 display_style_selector(GnomeCanvasGroup *boardRootItem);
static void		 item_event_style_selection (GtkComboBox *widget, void *styles_tab[]);

#define word_area_x1 120
#define word_area_y1 80
#define word_area_width 580
#define word_area_height 420

#define combo_style_x1 500
#define combo_style_y1 20

static style_t *current_style_default;
static style_t **current_doctype_default;
static GtkTextBuffer *buffer;
static GtkWidget *view;
static GHashTable* styles_hash = NULL;

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Wordprocessor",
    "A basic word processor",
    "Bruno Coudoin <bruno.coudoin@free.fr>",
    NULL,
    NULL,
    NULL,
    NULL,
    start_board,
    pause_board,
    end_board,
    is_our_board,
    NULL,
    NULL,
    set_level,
    NULL,
    NULL,
    NULL,
    NULL
  };

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(wordprocessor)

/*
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 *
 */
static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  board_paused = pause;
}

/*
 */
static void start_board (GcomprisBoard *agcomprisBoard)
{

  if(agcomprisBoard!=NULL)
    {
      gchar *img;

      gcomprisBoard=agcomprisBoard;
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=1;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=1; /* Go to next level after this number of 'play' */
      gc_bar_set(0);

      img = gcompris_image_to_skin("gcompris-shapebg.jpg");
      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			      img);
      g_free(img);

      wordprocessor_create();

      pause_board(FALSE);

    }
}
/* ======================================= */
static void end_board ()
{
  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      wordprocessor_destroy_all_items();
    }
  gcomprisBoard = NULL;
}

/* ======================================= */
static void set_level (guint level)
{
}
/* ======================================= */
static gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "wordprocessor")==0)
	{
	  /* Set the plugin entry */
	  gcomprisBoard->plugin=&menu_bp;

	  return TRUE;
	}
    }
  return FALSE;
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/* ==================================== */
/* Destroy all the items */
static void wordprocessor_destroy_all_items()
{
  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;
}
/* ==================================== */
static GnomeCanvasItem *wordprocessor_create()
{
  GnomeCanvasItem *item = NULL;
  GtkWidget *sw;

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  view = gtk_text_view_new ();
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
  g_signal_connect (view, "key-release-event", 
		    G_CALLBACK (key_press_event), NULL);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
      

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);
  gtk_container_add (GTK_CONTAINER (sw), view);

  item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(boardRootItem),
				gnome_canvas_widget_get_type (),
				"widget", GTK_WIDGET(sw),
				"x", (double) word_area_x1,
				"y", (double) word_area_y1,
				"width", (double) word_area_width,
				"height", (double) word_area_height,
				"anchor", GTK_ANCHOR_NW,
				"size_pixels", FALSE,
				NULL);
  gtk_widget_show(GTK_WIDGET(view));
  gtk_widget_show(GTK_WIDGET(sw));

  /*
   * Create the default style tags
   */
  current_doctype_default = &doctype_list[0];
  create_tags(buffer, *current_doctype_default);

  /*
   * Display the style buttons
   */
  display_style_buttons(boardRootItem,
			word_area_x1 + word_area_width + 10,
			word_area_y1);

  display_style_selector(boardRootItem);

  return NULL;
}

/*
 * Display the style buttons
 */
static void display_style_buttons(GnomeCanvasGroup *boardRootItem, 
				  int x,
				  int y)
{
  GdkPixbuf *pixmap;
  int offset_y, text_x, text_y;
  int i = 0;
  gchar *styles_tab[] = { _("TITLE"), "H0",
			  _("TITLE 1"), "H1",
			  _( "TITLE 2"), "H2",
			  _("TEXT"), "P",
			  NULL, NULL };

  pixmap = gcompris_load_skin_pixmap("button_small.png");

  offset_y = gdk_pixbuf_get_height(pixmap) + 10;
  text_x   = gdk_pixbuf_get_width(pixmap) / 2;
  text_y   = gdk_pixbuf_get_height(pixmap) / 2;

  while(styles_tab[i])
    {
      GnomeCanvasItem *item;

      item = gnome_canvas_item_new (boardRootItem,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap,
				    "x", (double) x,
				    "y", (double) y,
				    "anchor", GTK_ANCHOR_NW,
				    NULL);

      gtk_signal_connect(GTK_OBJECT(item), "event", (GtkSignalFunc) item_event, styles_tab[i+1] );
      gtk_signal_connect(GTK_OBJECT(item), "event", (GtkSignalFunc) gc_item_focus_event, NULL);

      item = gnome_canvas_item_new (boardRootItem,
				    gnome_canvas_text_get_type (),
				    "text", styles_tab[i],
				    "font", gcompris_skin_font_board_medium,
				    "x", (double) x + text_x + 1,
				    "y", (double) y + text_y + 1,
				    "anchor", GTK_ANCHOR_CENTER,
				    "fill_color_rgba", gcompris_skin_color_shadow,
				    NULL);
      gtk_signal_connect(GTK_OBJECT(item), "event", (GtkSignalFunc) item_event, styles_tab[i+1] );

      item = gnome_canvas_item_new (boardRootItem,
				    gnome_canvas_text_get_type (),
				    "text", styles_tab[i],
				    "font", gcompris_skin_font_board_medium,
				    "x", (double) x + text_x,
				    "y", (double) y + text_y,
				    "anchor", GTK_ANCHOR_CENTER,
				    "fill_color_rgba", gcompris_skin_color_text_button,
				    NULL);
      gtk_signal_connect(GTK_OBJECT(item), "event", (GtkSignalFunc) item_event, styles_tab[i+1] );

      y += offset_y;

      i += 2;
    }

  gdk_pixbuf_unref(pixmap);
}

/* ==================================== */

/* ==================================== */
static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{

  if(board_paused)
    return FALSE;

  switch (event->type) 
    {
    case GDK_BUTTON_PRESS:
      switch(event->button.button) 
	{
	case 1:
	case 2:
	case 3:
	  {
	    GtkTextIter    iter_start, iter_end;
	    gchar *current_style_name;

	    current_style_name = (char *)data;

	    gtk_text_buffer_get_iter_at_mark(buffer,
					     &iter_start,
					     gtk_text_buffer_get_insert (buffer));
	    gtk_text_iter_set_line_offset(&iter_start, 0);

	    iter_end = iter_start;
	    gtk_text_iter_forward_to_line_end(&iter_end);

	    gtk_text_buffer_remove_all_tags(buffer,
					    &iter_start,
					    &iter_end);

	    gtk_text_buffer_apply_tag_by_name(buffer,
					      current_style_name,
					      &iter_start,
					      &iter_end);
	  }
	  break;
	default:
	  break;
	}
    default:
      break;
    }

  return FALSE;
}

/* Create a bunch of tags. Note that it's also possible to
 * create tags with gtk_text_tag_new() then add them to the
 * tag table for the buffer, gtk_text_buffer_create_tag() is
 * just a convenience function. Also note that you don't have
 * to give tags a name; pass NULL for the name to create an
 * anonymous tag.
 *
 * In any real app, another useful optimization would be to create
 * a GtkTextTagTable in advance, and reuse the same tag table for
 * all the buffers with the same tag set, instead of creating
 * new copies of the same tags for every buffer.
 *
 * Tags are assigned default priorities in order of addition to the
 * tag table.	 That is, tags created later that affect the same text
 * property affected by an earlier tag will override the earlier
 * tag.  You can modify tag priorities with
 * gtk_text_tag_set_priority().
 */

static void
create_tags (GtkTextBuffer *buffer, style_t style[])
{
  gint i;

  for(i=0; i<NUMBER_OF_STYLE; i++)
    {
      GtkTextTag *tag;

      tag = gtk_text_buffer_create_tag (buffer, style[i].name,
					"weight", style[i].weight,
					"font", style[i].font,
					"justification", style[i].justification,
					"indent", style[i].indent,
					"pixels-above-lines", style[i].pixels_above_lines,
					"pixels-below-lines", style[i].pixels_below_lines,
					"foreground", style[i].foreground,
					NULL);
      g_object_set_data (G_OBJECT (tag), "style", &style[i]);
    }

  /* Point to the last style */
  i--;

  current_style_default = &style[i];
  set_default_style(buffer, current_style_default);
}

/*
 * Set the default style
 */
static void
set_default_style (GtkTextBuffer *buffer, style_t *style)
{
  PangoFontDescription *font_desc;
  GdkColor color;

  printf("set_default_style %s\n", style->name);
  /* Change default font throughout the widget */
  font_desc = pango_font_description_from_string (style->font);
  gtk_widget_modify_font (view, font_desc);
  pango_font_description_free (font_desc);
	    
  /* Change default color throughout the widget */
  gdk_color_parse (style->foreground, &color);
  gtk_widget_modify_text (view, GTK_STATE_NORMAL, &color);

  /* Change left margin, justification, ... throughout the widget */
  gtk_text_view_set_indent (GTK_TEXT_VIEW (view), style->indent);
  gtk_text_view_set_justification(GTK_TEXT_VIEW (view), style->justification);
  gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW (view), style->pixels_below_lines);
  gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW (view), style->pixels_above_lines);
}


/*
 * Create the combo with the styles
 * --------------------------------
 */
static void
display_style_selector(GnomeCanvasGroup *boardRootItem)
{
  int i = 0;

  /* The list of all styles */
  gchar *doctype_names[] = { 
    _("Default"),
    _("Love letter"),
    NULL
  };

  styles_hash = g_hash_table_new (g_str_hash, g_str_equal);

  gtk_combo_filetypes = gtk_combo_box_new_text();

  while ( doctype_names[i] != NULL)
    {
      g_hash_table_insert(styles_hash, doctype_names[i], &doctype_list[i]);

      gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_combo_filetypes), (gchar *)doctype_names[i]);

      i++;
    }

  gnome_canvas_item_new (GNOME_CANVAS_GROUP(boardRootItem),
			 gnome_canvas_widget_get_type (),
			 "widget", GTK_WIDGET(gtk_combo_filetypes),
			 "x", (double) combo_style_x1,
			 "y", (double) combo_style_y1,
			 "width", (double) BOARDWIDTH - combo_style_x1 - 10,
			 "height", 35.0,
			 "anchor", GTK_ANCHOR_NW,
			 "size_pixels", FALSE,
			 NULL);

  gtk_widget_show(GTK_WIDGET(gtk_combo_filetypes));
  gtk_combo_box_set_active(GTK_COMBO_BOX(gtk_combo_filetypes), 0);

  g_signal_connect(G_OBJECT(gtk_combo_filetypes),
		   "changed",
		   G_CALLBACK(item_event_style_selection),
		   doctype_names);
}

/* Catch all typing events to apply the proper tags
 *
 */
static void
item_event_style_selection (GtkComboBox *widget,
			    void *doctype_names[])
{
  gchar *style_str;
  style_t *doctype[1];

  style_str = gtk_combo_box_get_active_text((GtkComboBox *)widget);

  printf("item_event_style_selection %s\n", style_str);

  /* Search the doctype */
  doctype[0] = (style_t *)(g_hash_table_lookup(styles_hash, style_str));
  if(doctype) {
    int i = 0;
    
    while(doctype[i])
      {
	printf("==>%s\n", doctype[i]->name);
	i++;
      }
  }
}


/* Catch all typing events to apply the proper tags
 *
 */
static gboolean
key_press_event (GtkWidget *text_view,
		 GdkEventKey *event)
{
  GtkTextIter iter_start, iter_end;
  GtkTextBuffer *buffer;

  {
    GSList *tags = NULL, *tagp = NULL;

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

    gtk_text_buffer_get_iter_at_mark(buffer,
				     &iter_start,
				     gtk_text_buffer_get_insert (buffer));
    gtk_text_iter_set_line_offset(&iter_start, 0);

    iter_end = iter_start;
    gtk_text_iter_forward_to_line_end(&iter_end);

    tags = gtk_text_iter_get_tags (&iter_start);
    printf("length %d\n", g_slist_length(tags));
    if(g_slist_length(tags) == 0)
      {
	gtk_text_iter_backward_char (&iter_end);
	tags = gtk_text_iter_get_tags (&iter_end);
	gtk_text_iter_forward_char (&iter_end);
	printf("2 length %d\n", g_slist_length(tags));
      }

    for (tagp = tags;  tagp != NULL;  tagp = tagp->next)
      {
	GtkTextTag *tag = tagp->data;
	gchar *name;
	g_object_get (G_OBJECT (tag), "name", &name, NULL);
	style_t *style = g_object_get_data (G_OBJECT (tag), "style");

	printf("name=%s  style=%s\n", name, style->name);
	set_default_style(buffer, style);
	gtk_text_buffer_apply_tag_by_name(buffer,
					  name,
					  &iter_start,
					  &iter_end);
      }
    
    if (tags) 
      g_slist_free (tags);
    else
      {
	/* Set the default style */
	set_default_style(buffer, current_style_default);
	gtk_text_buffer_apply_tag_by_name(buffer,
					  current_style_default->name,
					  &iter_start,
					  &iter_end);
      }
  }

  return FALSE;
}

