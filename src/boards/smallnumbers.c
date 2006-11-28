/* gcompris - smallnumbers.c
 *
 * Time-stamp: <2006/08/21 23:36:29 bruno>
 *
 * Copyright (C) 2000 Bruno Coudoin
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
#include <string.h>

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;

static gint dummy_id = 0;
static gint drop_items_id = 0;

static char *numbers = "123456";
static  int  gamewon;
static guint number_of_dices=1;

static GnomeCanvasGroup *boardRootItem = NULL;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void set_level (guint level);
static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str);

static void smallnumbers_create_item(GnomeCanvasGroup *parent);
static gint smallnumbers_drop_items (GtkWidget *widget, gpointer data);
static gint smallnumbers_move_items (GtkWidget *widget, gpointer data);
static void smallnumbers_destroy_all_items(void);
static void smallnumbers_next_level(void);
static void smallnumbers_gotkey_item(GnomeCanvasItem *item, guint key);

static void		 smallnumber_config_start(GcomprisBoard *agcomprisBoard,
					     GcomprisProfile *aProfile);
static void		 smallnumber_config_stop(void);

static void player_win(GnomeCanvasItem *item);
static void player_loose(void);

static  guint32              fallSpeed = 0;
static  double               speed = 0.0;
static  double               imageZoom = 0.0;

/* if board has alternate locale */
static gchar *locale_sound = NULL;
static gboolean with_sound = FALSE;

/* Description of this plugin */
static BoardPlugin menu_bp =
{
   NULL,
   NULL,
   "Numbers With Dice",
   "Type the keyboard number 1 to 9 before the dice reaches the ground",
   "Bruno Coudoin <bruno.coudoin@free.fr>",
   NULL,
   NULL,
   NULL,
   NULL,
   start_board,
   pause_board,
   end_board,
   is_our_board,
   key_press,
   NULL,
   set_level,
   NULL,
   NULL,
   smallnumber_config_start,
   smallnumber_config_stop
};

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(smallnumbers)

/*
 * in : boolean TRUE = PAUSE : FALSE = UNPAUSE
 *
 */
static void pause_board (gboolean pause)
{

  if(gcomprisBoard==NULL)
    return;

  if(pause)
    {
      if (dummy_id) {
	gtk_timeout_remove (dummy_id);
	dummy_id = 0;
      }
      if (drop_items_id) {
	gtk_timeout_remove (drop_items_id);
	drop_items_id = 0;
      }
    }
  else
    {
      if(gamewon == TRUE) /* the game is won */
	{
	  smallnumbers_next_level();
	}

      if(!drop_items_id) {
	drop_items_id = gtk_timeout_add (1000,
					 (GtkFunction) smallnumbers_drop_items, NULL);
      }
      if(!dummy_id) {
	dummy_id = gtk_timeout_add (1000, (GtkFunction) smallnumbers_move_items, NULL);
      }
    }
}

/*
 */
static void start_board (GcomprisBoard *agcomprisBoard)
{
  GHashTable *config = gc_db_get_board_conf();

  locale_sound = g_strdup(g_hash_table_lookup( config, "locale_sound"));

  gchar *control_sound = g_hash_table_lookup( config, "with_sound");

  if (control_sound && strcmp(g_hash_table_lookup( config, "with_sound"),"True")==0)
    with_sound = TRUE;
  else
    with_sound = FALSE;

  g_hash_table_destroy(config);

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;

      /* disable im_context */
      gcomprisBoard->disable_im_context = TRUE;

      gc_set_background(gnome_canvas_root(gcomprisBoard->canvas), "images/scenery7_background.png");

      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel = 9;
      gcomprisBoard->number_of_sublevel=10;
      gc_score_start(SCORESTYLE_NOTE,
			   gcomprisBoard->width - 220,
			   gcomprisBoard->height - 50,
			   gcomprisBoard->number_of_sublevel);
      gc_bar_set(GC_BAR_CONFIG|GC_BAR_LEVEL);


      number_of_dices=1;
      if(!gcomprisBoard->mode)
	number_of_dices=1;
      else if(g_strncasecmp(gcomprisBoard->mode, "2_DICES", 7)==0) {
	/* 2 Dices mode */
	number_of_dices=2;
      }

      smallnumbers_next_level();

      gamewon = FALSE;
      pause_board(FALSE);
    }
}

static void
end_board ()
{
  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      gc_score_end();
      smallnumbers_destroy_all_items();
    }
  gcomprisBoard = NULL;
}

static void
set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      smallnumbers_next_level();
    }
}

static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str)
{
  char str[2];

  if(!gcomprisBoard || !boardRootItem)
    return FALSE;

  /* Add some filter for control and shift key */
  switch (keyval)
    {
    case GDK_Shift_L:
    case GDK_Shift_R:
    case GDK_Control_L:
    case GDK_Control_R:
    case GDK_Caps_Lock:
    case GDK_Shift_Lock:
    case GDK_Meta_L:
    case GDK_Meta_R:
    case GDK_Alt_L:
    case GDK_Alt_R:
    case GDK_Super_L:
    case GDK_Super_R:
    case GDK_Hyper_L:
    case GDK_Hyper_R:
    case GDK_Mode_switch:
    case GDK_dead_circumflex:
    case GDK_Num_Lock:
      return FALSE;
    }

  sprintf(str, "%c", keyval);

  keyval = atoi(str);

  g_list_foreach(GNOME_CANVAS_GROUP(boardRootItem)->item_list,
		 (GFunc) smallnumbers_gotkey_item,
		 GINT_TO_POINTER(keyval));

  return TRUE;
}

gboolean
is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "smallnumbers")==0)
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
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

/* set initial values for the next level */
static void smallnumbers_next_level()
{

  gamewon = FALSE;
  gc_bar_set_level(gcomprisBoard);

  smallnumbers_destroy_all_items();

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  /* Try the next level */
  speed=100+(40/gcomprisBoard->level);
  fallSpeed=5000-gcomprisBoard->level*200;
  imageZoom=0.9+(0.5/gcomprisBoard->level);
  gcomprisBoard->sublevel=1;
  gc_score_set(gcomprisBoard->sublevel);
}


/* Destroy all the items */
static void smallnumbers_destroy_all_items()
{

  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem = NULL;

}
static void smallnumbers_gotkey_item(GnomeCanvasItem *item, guint key)
{
  guint number;

  if(G_OBJECT (item)) {
    number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (item), "dice_number"));

    if(number==key) {
      player_win(item);
    }
  }
}

static void smallnumbers_move_item(GnomeCanvasItem *item)
{
  double x1, y1, x2, y2;
  gnome_canvas_item_move(item, 0, 2.0);

  gnome_canvas_item_get_bounds (item,
				&x1,
				&y1,
				&x2,
				&y2);

  if(y1>gcomprisBoard->height) {
    player_loose();
    gtk_object_destroy (GTK_OBJECT(item));
  }
}

/*
 * This does the moves of the game items on the play canvas
 *
 */
static gint smallnumbers_move_items (GtkWidget *widget, gpointer data)
{

  /* For each item we need to move */
  g_list_foreach(GNOME_CANVAS_GROUP(boardRootItem)->item_list, (GFunc) smallnumbers_move_item, NULL);

  dummy_id = gtk_timeout_add (speed,
			      (GtkFunction) smallnumbers_move_items, NULL);

  return(FALSE);
}

static void smallnumbers_create_item(GnomeCanvasGroup *parent)
{
  GdkPixbuf *smallnumbers_pixmap = NULL;
  GnomeCanvasItem *item;
  GnomeCanvasGroup *group_item;
  guint i;
  guint total_number = 0;
  double x = 0.0;
  guint number_of_dice = number_of_dices;

  group_item = GNOME_CANVAS_GROUP(
				  gnome_canvas_item_new (parent,
							 gnome_canvas_group_get_type (),
							 "x", (double) 0,
							 "y", (double) 40,
							 NULL));

  while(number_of_dice-- > 0) {
    gchar *str1 = NULL;
    gchar *str2 = NULL;

    /* Take care not to go above 9 anyway */
    if(total_number==0) {
      i=g_random_int()%6;
    } else {
      int rando = g_random_int()%(9-total_number);
      i=MIN(rando, 5);
    }

    total_number += i + 1;

    /*
     * Play the sound
     */

    if (with_sound)
      {
	gunichar *unichar_letterItem;
	char *lettersItem;
	gchar *letter;

	lettersItem = g_malloc (2);

	sprintf(lettersItem, "%c", numbers[i]);
	lettersItem[1] = '\0';

	unichar_letterItem = g_new(gunichar,1);

	*unichar_letterItem = g_utf8_get_char (lettersItem);

	letter = g_new0(gchar, 6);

	g_unichar_to_utf8(*unichar_letterItem, letter);

	str1 = gc_sound_alphabet(letter);

	g_free(letter);
	g_free(lettersItem);
	g_free(unichar_letterItem);

	str2 = g_strdup_printf("sounds/$LOCALE/alphabet/%s", str1);

	gc_sound_play_ogg(str2, NULL);

	g_free(str1);
	g_free(str2);
      }

    /*
     * Now the images
     */
    str1 = g_strdup_printf("level%c.png", numbers[i]);
    str2 = gc_skin_image_get(str1);

    smallnumbers_pixmap = gc_pixmap_load(str2);

    g_free(str1);
    g_free(str2);

    if(x==0.0)
      {
	x = (double)(g_random_int()%(gcomprisBoard->width-
			     (guint)(gdk_pixbuf_get_width(smallnumbers_pixmap)* imageZoom)*2));
      }
    else
      {
	x += ((gdk_pixbuf_get_width(smallnumbers_pixmap)-10)*imageZoom);
      }

    item = gnome_canvas_item_new (group_item,
				  gnome_canvas_pixbuf_get_type (),
				  "pixbuf", smallnumbers_pixmap,
				  "x", x,
				  "y", (double) -gdk_pixbuf_get_height(smallnumbers_pixmap)*imageZoom,
				  "width", (double) gdk_pixbuf_get_width(smallnumbers_pixmap)*imageZoom,
				  "height", (double) gdk_pixbuf_get_height(smallnumbers_pixmap)*imageZoom,
				  "width_set", TRUE,
				  "height_set", TRUE,
				  NULL);
    gdk_pixbuf_unref(smallnumbers_pixmap);
  }
  g_object_set_data (G_OBJECT (group_item), "dice_number", GINT_TO_POINTER (total_number));

}

/*
 * This is called on a low frequency and is used to drop new items
 *
 */
static gint smallnumbers_drop_items (GtkWidget *widget, gpointer data)
{
  gc_sound_play_ogg ("sounds/bleep.wav", NULL);
  smallnumbers_create_item(boardRootItem);

  drop_items_id = gtk_timeout_add (fallSpeed,
				   (GtkFunction) smallnumbers_drop_items, NULL);
  return (FALSE);
}

static void player_win(GnomeCanvasItem *item)
{
  gtk_object_destroy (GTK_OBJECT(item));
  gc_sound_play_ogg ("sounds/flip.wav", NULL);

  gcomprisBoard->sublevel++;
  gc_score_set(gcomprisBoard->sublevel);

  if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel)
    {
      /* Try the next level */
      gcomprisBoard->level++;
      if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
	gc_score_end();
	gc_bonus_end_display(BOARD_FINISHED_RANDOM);
	return;
      }
      gamewon = TRUE;
      smallnumbers_destroy_all_items();
      gc_bonus_display(gamewon, BONUS_SMILEY);
    }
  else
    {
      gc_score_set(gcomprisBoard->sublevel);
    }
}

static void player_loose()
{
  gc_sound_play_ogg ("crash", NULL);
}


/* ======================= */
/* = config_start        = */
/* ======================= */

static GcomprisProfile *profile_conf;
static GcomprisBoard   *board_conf;

static GHFunc save_table (gpointer key,
		    gpointer value,
		    gpointer user_data)
{
  gc_db_set_board_conf ( profile_conf,
			    board_conf,
			    (gchar *) key,
			    (gchar *) value);

  return NULL;
}

static void conf_ok(GHashTable *table)
{
  if (!table){
    if (gcomprisBoard)
      pause_board(FALSE);
    return;
  }

  g_hash_table_foreach(table, (GHFunc) save_table, NULL);

  if (gcomprisBoard){
    GHashTable *config;

    if (profile_conf)
      config = gc_db_get_board_conf();
    else
      config = table;

    if (locale_sound)
      g_free(locale_sound);

    locale_sound = g_strdup(g_hash_table_lookup( config, "locale_sound"));

    gchar *control_sound = g_hash_table_lookup( config, "with_sound");

    if (control_sound && strcmp(g_hash_table_lookup( config, "with_sound"),"True")==0)
      with_sound = TRUE;
    else
      with_sound = FALSE;

    if (profile_conf)
      g_hash_table_destroy(config);

    smallnumbers_next_level();

    gamewon = FALSE;

    pause_board(FALSE);
  }

  board_conf = NULL;
  profile_conf = NULL;
}

static void sound_control_box_toggled(GtkToggleButton *togglebutton,
				      gpointer combo)
{
  gtk_widget_set_sensitive(GTK_WIDGET(combo), gtk_toggle_button_get_active (togglebutton));
}


static void
smallnumber_config_start(GcomprisBoard *agcomprisBoard,
		    GcomprisProfile *aProfile)
{
  board_conf = agcomprisBoard;
  profile_conf = aProfile;

  if (gcomprisBoard)
    pause_board(TRUE);

  gchar *label;

  label = g_strdup_printf("<b>%s</b> configuration\n for profile <b>%s</b>",
			  agcomprisBoard->name, aProfile ? aProfile->name : "");

  gc_board_config_window_display(label, (GcomprisConfCallback )conf_ok);

  g_free(label);

  /* init the combo to previously saved value */
  GHashTable *config = gc_db_get_conf( profile_conf, board_conf);

  gchar *saved_locale_sound = g_hash_table_lookup( config, "locale_sound");

  gchar *control_sound = g_hash_table_lookup( config, "with_sound");
  if (control_sound && strcmp(g_hash_table_lookup( config, "with_sound"),"True")==0)
    with_sound = TRUE;
  else
    with_sound = FALSE;

  GtkCheckButton  *sound_control = gc_board_config_boolean_box("Enable sounds", "with_sound", with_sound);

  GtkComboBox *sound_box = gc_board_config_combo_locales_asset( "Select sound locale",
							 saved_locale_sound,
							 "sounds/$LOCALE/colors/purple.ogg");

  gtk_widget_set_sensitive(GTK_WIDGET(sound_box), with_sound);

  g_signal_connect(G_OBJECT(sound_control), "toggled",
		   G_CALLBACK(sound_control_box_toggled),
		   sound_box);

  g_hash_table_destroy(config);

}
/* ======================= */
/* = config_stop        = */
/* ======================= */
static void
smallnumber_config_stop()
{
}
