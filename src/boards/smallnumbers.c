/* gcompris - smallnumbers.c
 *
 * Time-stamp: <2005/09/21 23:10:56 yves>
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
   N_("Numbers With Dice"),
   N_("Type the keyboard number 1 to 9 before the dice reaches the ground"),
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
  GHashTable *config = gcompris_get_board_conf();

  locale_sound = g_hash_table_lookup( config, "locale_sound");

  gchar *control_sound = g_hash_table_lookup( config, "with_sound");

  if (control_sound && strcmp(g_hash_table_lookup( config, "with_sound"),"True")==0)
    with_sound = TRUE;
  else
    with_sound = FALSE;

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;

      /* disable im_context */
      gcomprisBoard->disable_im_context = TRUE;

      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "images/scenery3_background.png");

      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel = 9;
      gcomprisBoard->number_of_sublevel=10;
      gcompris_score_start(SCORESTYLE_NOTE, 
			   gcomprisBoard->width - 220, 
			   gcomprisBoard->height - 50, 
			   gcomprisBoard->number_of_sublevel);
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL);


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
      gcompris_score_end();
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
  gchar *old_value;
  gchar *old_name;
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
    case GDK_KP_0:
    case GDK_KP_Insert:
      keyval=GDK_0;
      break;
    case GDK_KP_1:
    case GDK_KP_End:
      keyval=GDK_1;
      break;
    case GDK_KP_2:
    case GDK_KP_Down:
      keyval=GDK_2;
      break;
    case GDK_KP_3:
    case GDK_KP_Page_Down:
      keyval=GDK_3;
      break;
    case GDK_KP_4:
    case GDK_KP_Left:
      keyval=GDK_4;
      break;
    case GDK_KP_5:
    case GDK_KP_Begin:
      keyval=GDK_5;
      break;
    case GDK_KP_6:
    case GDK_KP_Right:
      keyval=GDK_6;
      break;
    case GDK_KP_7:
    case GDK_KP_Home:
      keyval=GDK_7;
      break;
    case GDK_KP_8:
    case GDK_KP_Up:
      keyval=GDK_8;
      break;
    case GDK_KP_9:
    case GDK_KP_Page_Up:
      keyval=GDK_9;
      break;
    }

  sprintf(str, "%c", keyval);

  keyval = atoi(str);

  g_list_foreach(GNOME_CANVAS_GROUP(boardRootItem)->item_list, (GFunc) smallnumbers_gotkey_item, (void *)keyval);

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
  gcompris_bar_set_level(gcomprisBoard);

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
  gcompris_score_set(gcomprisBoard->sublevel);
}


/* Destroy all the items */
static void smallnumbers_destroy_all_items()
{
  GnomeCanvasItem *item;

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
  char *str;
  guint i;
  char *lettersItem;
  char *str1 = NULL;
  char *str2 = NULL;
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

    lettersItem = g_malloc (2);

    /* Take care not to go above 9 anyway */
    if(total_number==0) {
      i=rand()%6;
    } else {
      int rando = rand()%(9-total_number);
      i=MIN(rando, 5);
    }

    total_number += i + 1;
    sprintf(lettersItem, "%c", numbers[i]);

    lettersItem[1] = '\0';

    str1 = g_strdup_printf("%s%s", lettersItem, ".ogg");
    str2 = gcompris_get_asset_file_locale("gcompris alphabet", NULL, "audio/x-ogg", str1, locale_sound);

    if (with_sound)
      gcompris_play_ogg(str2, NULL);

    g_free(str1);
    g_free(str2);

    str = g_strdup_printf("gcompris/dice/gnome-dice%c.png", numbers[i]);

    smallnumbers_pixmap = gcompris_load_pixmap(str);

    g_free(str);

    if(x==0.0) {
      x = (double)(rand()%(gcomprisBoard->width-
			   (guint)(gdk_pixbuf_get_width(smallnumbers_pixmap)* imageZoom)*2));
    } else {
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
  smallnumbers_create_item(boardRootItem);

  drop_items_id = gtk_timeout_add (fallSpeed,
				   (GtkFunction) smallnumbers_drop_items, NULL);
  return (FALSE);
}

static void player_win(GnomeCanvasItem *item)
{
  gtk_object_destroy (GTK_OBJECT(item));
  gcompris_play_ogg ("gobble", NULL);

  gcomprisBoard->sublevel++;
  gcompris_score_set(gcomprisBoard->sublevel);

  if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel)
    {
      /* Try the next level */
      gcomprisBoard->level++;
      if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
	gcompris_score_end();
	board_finished(BOARD_FINISHED_RANDOM);
	return;
      }
      gamewon = TRUE;
      smallnumbers_destroy_all_items();
      gcompris_display_bonus(gamewon, BONUS_SMILEY);
    }
  else
    {
      gcompris_score_set(gcomprisBoard->sublevel);
    }
}

static void player_loose()
{
  gcompris_play_ogg ("crash", NULL);
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
  gcompris_set_board_conf ( profile_conf,
			    board_conf,
			    (gchar *) key, 
			    (gchar *) value);
}

static GcomprisConfCallback conf_ok(GHashTable *table)
{
  g_hash_table_foreach(table, (GHFunc) save_table, NULL);
  
  board_conf = NULL;
  profile_conf = NULL;
}

static gboolean check_text(gchar *key, gchar *text, GtkLabel *label){
  gtk_label_set_markup(label, text);

  return TRUE;
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

  gchar *label;
  
  label = g_strdup_printf("<b>%s</b> configuration\n for profile <b>%s</b>",
			  agcomprisBoard->name, aProfile->name);

  gcompris_configuration_window(label, (GcomprisConfCallback )conf_ok);

  g_free(label);

  /* init the combo to previously saved value */
  GHashTable *config = gcompris_get_conf( profile_conf, board_conf);

  locale_sound = g_hash_table_lookup( config, "locale_sound");

  gchar *control_sound = g_hash_table_lookup( config, "with_sound");
  if (control_sound && strcmp(g_hash_table_lookup( config, "with_sound"),"True")==0)
    with_sound = TRUE;
  else
    with_sound = FALSE;

  GtkCheckButton  *sound_control = gcompris_boolean_box("Enable sounds", "with_sound", with_sound);
  
  GtkComboBox *sound_box = gcompris_combo_locales_asset( "Select sound locale", locale_sound, "gcompris colors", NULL, "audio/x-ogg", "purple.ogg");

  gtk_widget_set_sensitive(GTK_WIDGET(sound_box), with_sound);

  g_signal_connect(G_OBJECT(sound_control), "toggled", 
		   G_CALLBACK(sound_control_box_toggled),
		   sound_box);

}  
/* ======================= */
/* = config_stop        = */
/* ======================= */
static void 
smallnumber_config_stop()
{
}




/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
