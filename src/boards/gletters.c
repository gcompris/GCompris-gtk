/* gcompris - gletters.c
 *
 * Time-stamp: <2002/02/03 09:00:52 bruno>
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

#include <ctype.h>

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

static GList *item_list = NULL;
static GList *item2del_list = NULL;

static GcomprisBoard *gcomprisBoard = NULL;

static gint dummy_id = 0;
static gint drop_items_id = 0;

#define LETTERS_ARRAY_LENGTH 3
static char *letters_array[LETTERS_ARRAY_LENGTH] = {
  "0123456789",
  G_CSET_A_2_Z,
  G_CSET_a_2_z
};

/* Hash table of all displayed letters  */
static GHashTable *letters_table= NULL;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void set_level (guint level);
gint key_press(guint keyval);

static GnomeCanvasItem *gletters_create_item(GnomeCanvasGroup *parent);
static gint gletters_drop_items (GtkWidget *widget, gpointer data);
static gint gletters_move_items (GtkWidget *widget, gpointer data);
static void gletters_destroy_item(GnomeCanvasItem *item);
static void gletters_destroy_items(void);
static void gletters_destroy_all_items(void);
static void gletters_next_level(void);
static void gletters_add_new_item(void);

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
   N_("Simple Letters"),
   N_("Type the falling letters before they reach the ground"),
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
					 (GtkFunction) gletters_drop_items, NULL);
      }
      if(!dummy_id) {
	dummy_id = gtk_timeout_add (1000, (GtkFunction) gletters_move_items, NULL);
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
      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "gcompris/gcompris-bg.jpg");

      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel=6;
      gcomprisBoard->number_of_sublevel=10; /* Go to next level after this number of 'play' */
      gcompris_score_start(SCORESTYLE_NOTE, 
			   gcomprisBoard->width - 220, 
			   gcomprisBoard->height - 50, 
			   gcomprisBoard->number_of_sublevel);
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL);

      gletters_next_level();

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
      gletters_destroy_all_items();
    }
  gcomprisBoard = NULL;
}

static void
set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      gletters_next_level();
    }
}

gint key_press(guint keyval)
{
  gchar *old_value;
  gchar *old_name;
  guint c;

  if(!gcomprisBoard)
    return TRUE;

  /* Add some filter for control and shift key */
  switch (keyval)
    {
      /* Avoid all this keys to be interpreted by this game */
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

  c = tolower(keyval); 
  if (g_hash_table_lookup_extended (letters_table,
				    (char *)&c,
				    (gpointer) &old_name,
				    (gpointer) &old_value))
    {

      player_win(item_find_by_title((char *)&c));

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
      if(g_strcasecmp(gcomprisBoard->type, "gletters")==0)
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
static void gletters_next_level() 
{

  gcompris_bar_set_level(gcomprisBoard);

  gletters_destroy_all_items();

  /* Try the next level */
  speed=100+(40/gcomprisBoard->level);
  fallSpeed=5000-gcomprisBoard->level*200;
  imageZoom=1.0;

  gcomprisBoard->sublevel=1;
  gcompris_score_set(gcomprisBoard->sublevel);
}


static void gletters_move_item(GnomeCanvasItem *item)
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

static void gletters_destroy_item(GnomeCanvasItem *item)
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
static void gletters_destroy_items()
{
  GnomeCanvasItem *item;

  while(g_list_length(item2del_list)>0) 
    {
      item = g_list_nth_data(item2del_list, 0);
      gletters_destroy_item(item);
    }
}

/* Destroy all the items */
static void gletters_destroy_all_items()
{
  GnomeCanvasItem *item;

  if(item_list)
    while(g_list_length(item_list)>0) 
      {
	item = g_list_nth_data(item_list, 0);
	gletters_destroy_item(item);
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
static gint gletters_move_items (GtkWidget *widget, gpointer data)
{
  g_list_foreach (item_list, (GFunc) gletters_move_item, NULL);

  /* Destroy items that falls out of the canvas */
  gletters_destroy_items();

  dummy_id = gtk_timeout_add (speed, 
			      (GtkFunction) gletters_move_items, NULL);

  return(FALSE);
}

static GnomeCanvasItem *gletters_create_item(GnomeCanvasGroup *parent)
{
  GdkPixbuf *gletters_pixmap = NULL;
  GnomeCanvasItem *item;
  char *str = NULL;
  int i;
  guint c;
  char *lettersItem;

  if (!letters_table)
    {
      letters_table= g_hash_table_new (g_str_hash, g_str_equal);
    }

  lettersItem = g_malloc (2);

  /* Beware, since we put the letters in a hash table, we do not allow the same
     letter to be displayed two times 
     WARNING : This can cause an infinite loop if there is not enough
     element to choose
  */
  do {
    i=rand()%strlen(letters_array[gcomprisBoard->level%LETTERS_ARRAY_LENGTH]);
    sprintf(lettersItem, "%c",
	    letters_array[gcomprisBoard->level%LETTERS_ARRAY_LENGTH][i]);

    /* Changing the letter to lower case : should be easier but these works */
    c = tolower(lettersItem[0]); 
    sprintf(lettersItem, "%c", c);

  } while(item_find_by_title(lettersItem)!=NULL);

  lettersItem[1] = '\0';
  gcompris_play_ogg(lettersItem, NULL);

  str = g_strdup_printf("gcompris/letters/%c.png",
			letters_array[gcomprisBoard->level%LETTERS_ARRAY_LENGTH][i]);

  gletters_pixmap = gcompris_load_pixmap(str);
  item = gnome_canvas_item_new (parent,
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", gletters_pixmap, 
				"x", (double)(rand()%(gcomprisBoard->width-
						      (guint)(gdk_pixbuf_get_width(gletters_pixmap)*
						       imageZoom))),
				"y", (double) -gdk_pixbuf_get_height(gletters_pixmap)*imageZoom,
				"width", (double) gdk_pixbuf_get_width(gletters_pixmap)*imageZoom,
				"height", (double) gdk_pixbuf_get_height(gletters_pixmap)*imageZoom,
				"width_set", TRUE, 
				"height_set", TRUE,
				NULL);
  gdk_pixbuf_unref(gletters_pixmap);
  item_list = g_list_append (item_list, item);

  /* Add letter to hash table of all falling letters. */
  g_hash_table_insert (letters_table, lettersItem, item);

  g_free(str);

  return (item);
}

static void gletters_add_new_item() 
{
  gletters_create_item(gnome_canvas_root(gcomprisBoard->canvas));
}

/*
 * This is called on a low frequency and is used to drop new items
 *
 */
static gint gletters_drop_items (GtkWidget *widget, gpointer data)
{
  gletters_add_new_item();

  drop_items_id = gtk_timeout_add (fallSpeed,
				   (GtkFunction) gletters_drop_items, NULL);
  return (FALSE);
}

static void player_win(GnomeCanvasItem *item)
{
  gletters_destroy_item(item);
  gcompris_play_sound (SOUNDLISTFILE, "gobble");

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
      gletters_next_level();
      gcompris_play_sound (SOUNDLISTFILE, "bonus");
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
					     (GtkFunction) gletters_drop_items, NULL);
	  }
	}
    }
}

static void player_loose()
{
  gcompris_play_sound (SOUNDLISTFILE, "crash");
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
