/* gcompris - smallnumbers.c
 *
 * Time-stamp: <2003/01/06 22:03:41 bruno>
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

static GList *item_list = NULL;
static GList *item2del_list = NULL;

static GcomprisBoard *gcomprisBoard = NULL;

static gint dummy_id = 0;
static gint drop_items_id = 0;

static char *numbers = "123456789";

/* Hash table of all displayed letters  */
static GHashTable *letters_table= NULL;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void set_level (guint level);
gint key_press(guint keyval);

static GnomeCanvasItem *smallnumbers_create_item(GnomeCanvasGroup *parent);
static gint smallnumbers_drop_items (GtkWidget *widget, gpointer data);
static gint smallnumbers_move_items (GtkWidget *widget, gpointer data);
static void smallnumbers_destroy_item(GnomeCanvasItem *item);
static void smallnumbers_destroy_items(void);
static void smallnumbers_destroy_all_items(void);
static void smallnumbers_next_level(void);
static void smallnumbers_add_new_item(void);

static void player_win(GnomeCanvasItem *item);
static void player_loose(void);
static GnomeCanvasItem *item_find_by_title (const gchar *title);
static char *key_find_by_item (const GnomeCanvasItem *item);

static  guint32              fallSpeed = 0;
static  double               speed = 0.0;
static  double               imageZoom = 0.0;

/* Description of this plugin */
BoardPlugin menu_bp =
{
   NULL,
   NULL,
   N_("Numbers With Dices"),
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
   NULL
};

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

BoardPlugin 
*get_bplugin_info(void)
{
  return &menu_bp;
}

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

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;

      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "images/scenery3_background.jpg");

      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel = 6;
      gcomprisBoard->number_of_sublevel=10;
      gcompris_score_start(SCORESTYLE_NOTE, 
			   gcomprisBoard->width - 220, 
			   gcomprisBoard->height - 50, 
			   gcomprisBoard->number_of_sublevel);
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL);

      smallnumbers_next_level();

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

gint key_press(guint keyval)
{
  gchar *old_value;
  gchar *old_name;
  char str[2];

  if(!gcomprisBoard)
    return TRUE;

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
  if (g_hash_table_lookup_extended (letters_table,
				    str,
				    (gpointer) &old_name,
				    (gpointer) &old_value))
    {

      player_win(item_find_by_title(str));

    }
  else
    {
      player_loose();
    }

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

  gcompris_bar_set_level(gcomprisBoard);

  smallnumbers_destroy_all_items();

  /* Try the next level */
  speed=100+(40/gcomprisBoard->level);
  fallSpeed=5000-gcomprisBoard->level*200;
  imageZoom=0.9+(0.5/gcomprisBoard->level);
  gcomprisBoard->sublevel=1;
  gcompris_score_set(gcomprisBoard->sublevel);
}


static void smallnumbers_move_item(GnomeCanvasItem *item)
{
  double x1, y1, x2, y2;

  gnome_canvas_item_move(item, 0, 2.0);

  gnome_canvas_item_get_bounds    (item,
				   &x1,
				   &y1,
				   &x2,
				   &y2);
  
  if(y1>gcomprisBoard->height) {
    item2del_list = g_list_append (item2del_list, item);
    player_loose();
  }
}

static void smallnumbers_destroy_item(GnomeCanvasItem *item)
{
  char *key;

  key = key_find_by_item(item);
  /* Remove old letter  */
  g_hash_table_remove (letters_table, key);
  g_free (key);

  item_list = g_list_remove (item_list, item);
  item2del_list = g_list_remove (item2del_list, item);
  gtk_object_destroy (GTK_OBJECT(item));
}

/* Destroy items that falls out of the canvas */
static void smallnumbers_destroy_items()
{
  GnomeCanvasItem *item;

  while(g_list_length(item2del_list)>0) 
    {
      item = g_list_nth_data(item2del_list, 0);
      smallnumbers_destroy_item(item);
    }
}

/* Destroy all the items */
static void smallnumbers_destroy_all_items()
{
  GnomeCanvasItem *item;

  if(item_list)
    while(g_list_length(item_list)>0) 
      {
	item = g_list_nth_data(item_list, 0);
	smallnumbers_destroy_item(item);
      }

  /* Delete the letters_table */
  if(letters_table) {
    g_hash_table_destroy (letters_table);
    letters_table=NULL;
  }
}

/*
 * This does the moves of the game items on the play canvas
 *
 */
static gint smallnumbers_move_items (GtkWidget *widget, gpointer data)
{
  g_list_foreach (item_list, (GFunc) smallnumbers_move_item, NULL);

  /* Destroy iterm that falls out of the canvas */
  smallnumbers_destroy_items();

  dummy_id = gtk_timeout_add (speed, 
			      (GtkFunction) smallnumbers_move_items, NULL);

  return(FALSE);
}

static GnomeCanvasItem *smallnumbers_create_item(GnomeCanvasGroup *parent)
{
  GdkPixbuf *smallnumbers_pixmap = NULL;
  GnomeCanvasItem *item;
  char *str;
  int i;
  char *lettersItem;

  if (!letters_table)
    {
      letters_table= g_hash_table_new (g_str_hash, g_str_equal);
    }

  lettersItem = g_malloc (2);

  /* Beware, since we put the letters in a hash table, we do not allow the same
     letter to be displayed two times */
  i=rand()%strlen(numbers);
  sprintf(lettersItem, "%c", numbers[i]);
  if(item_find_by_title(lettersItem)!=NULL)
    {
      g_free(lettersItem);
      return NULL;
    }

  lettersItem[1] = '\0';
  gcompris_play_ogg(lettersItem, NULL);

  str = g_strdup_printf("gcompris/dice/gnome-dice%c.png", numbers[i]);

  smallnumbers_pixmap = gcompris_load_pixmap(str);
  g_free(str);

  item = gnome_canvas_item_new (parent,
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", smallnumbers_pixmap, 
				"x", (double)(rand()%(gcomprisBoard->width-
						      (guint)(gdk_pixbuf_get_width(smallnumbers_pixmap)*
						       imageZoom))),
				"y", (double) -gdk_pixbuf_get_height(smallnumbers_pixmap)*imageZoom,
				"width", (double) gdk_pixbuf_get_width(smallnumbers_pixmap)*imageZoom,
				"height", (double) gdk_pixbuf_get_height(smallnumbers_pixmap)*imageZoom,
				"width_set", TRUE, 
				"height_set", TRUE,
				NULL);
  gdk_pixbuf_unref(smallnumbers_pixmap);
  item_list = g_list_append (item_list, item);

  /* Add letter to hash table of all falling letters. */
  g_hash_table_insert (letters_table, lettersItem, item);

  return (item);
}

static void smallnumbers_add_new_item() 
{
  smallnumbers_create_item(gnome_canvas_root(gcomprisBoard->canvas));
}

/*
 * This is called on a low frequency and is used to drop new items
 *
 */
static gint smallnumbers_drop_items (GtkWidget *widget, gpointer data)
{
  smallnumbers_add_new_item();

  drop_items_id = gtk_timeout_add (fallSpeed,
				   (GtkFunction) smallnumbers_drop_items, NULL);
  return (FALSE);
}

static void player_win(GnomeCanvasItem *item)
{
  smallnumbers_destroy_item(item);
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
      smallnumbers_next_level();
      gcompris_play_ogg ("bonus", NULL);
    }
  else
    {
      /* Drop a new item now to speed up the game */
      if(g_list_length(item_list)==0)
	{
	  if (drop_items_id) {
	    /* Remove pending new item creation to sync the falls */
	    gtk_timeout_remove (drop_items_id);
	    drop_items_id = 0;
	  }
	  if(!drop_items_id) {
	    drop_items_id = gtk_timeout_add (0,
					     (GtkFunction) smallnumbers_drop_items, NULL);
	  }
	}
    }
}

static void player_loose()
{
  gcompris_play_ogg ("crash", NULL);
}


/* Return in item the key if the value equals the item */
static void get_item(char *key, char *value, char **item)
{
  if(value==*item) {
    *item=key;
  }
}

static char *
key_find_by_item (const GnomeCanvasItem *item)
{
  static char *key_found;

  key_found = (char *)item;

  if (!letters_table)
    return NULL;
  
  /* We have to loop to find this item's key */
  g_hash_table_foreach (letters_table,
			(GHFunc) get_item,
			&key_found);

  return key_found;
}

static GnomeCanvasItem *
item_find_by_title (const gchar *title)
{
  if (!letters_table)
    return NULL;
  
  return g_hash_table_lookup (letters_table, title);
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
