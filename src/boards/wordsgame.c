/* gcompris - wordsgame.c
 *
 * Time-stamp: <2004/11/14 15:13:11 bruno>
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

#include <errno.h>
#include <stdio.h>

#include "gcompris/gcompris.h"
#include "gcompris/gameutil.h"


#define SOUNDLISTFILE PACKAGE
#define MAXWORDSLENGTH 50
static GPtrArray *words=NULL;

GStaticRWLock items_lock = G_STATIC_RW_LOCK_INIT;
GStaticRWLock items2del_lock = G_STATIC_RW_LOCK_INIT;

/*
word - word to type
overword - part of word allready typed 
count - number of allready typed letters in word
pos - pointer to current position in word
letter - current expected letter to type
*/
typedef struct {
  GnomeCanvasItem *rootitem;
  GnomeCanvasItem *overwriteItem;
  gchar *word;
  gchar *overword;
  gint  count;
  gchar *pos;
  gchar *letter;
} LettersItem;

/*
items - array of displayed items 
items2del - array of items where moved offscreen
item_on_focus -  item on focus in array items. NULL - not set.
*/

static GPtrArray 	*items=NULL;
static GPtrArray 	*items2del=NULL;
static LettersItem 	*item_on_focus=NULL;


static GcomprisBoard *gcomprisBoard = NULL;

static gint dummy_id = 0;
static gint drop_items_id = 0;


static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void 		 end_board (void);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static void		 set_level (guint level);
static gint		 key_press(guint keyval);

static gboolean  	 wordsgame_read_wordfile();
static GnomeCanvasItem	 *wordsgame_create_item(GnomeCanvasGroup *parent);
static gint		 wordsgame_drop_items (GtkWidget *widget, gpointer data);
static gint		 wordsgame_move_items (GtkWidget *widget, gpointer data);
static void		 wordsgame_destroy_item(LettersItem *item);
static gboolean		 wordsgame_destroy_items(GPtrArray *items);
static void		 wordsgame_destroy_all_items(void);
static void		 wordsgame_next_level(void);
static void		 wordsgame_add_new_item(void);

static void		 player_win(LettersItem *item);
static void		 player_loose(void);


#define MAX_FALLSPEED  7000
#define MAX_SPEED  150
#define MIN_FALLSPEED  3000
#define MIN_SPEED  50
#define DEFAULT_FALLSPEED  7000
#define DEFAULT_SPEED  150

#define INCREMENT_FALLSPEED  1000
#define INCREMENT_SPEED  10


static  guint32              fallSpeed = 0;
static  double               speed = 0.0;

/* Description of this plugin */
static BoardPlugin menu_bp =
{
   NULL,
   NULL,
   N_("Falling Words"),
   N_("Fully type the falling words before they reach the ground"),
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
	g_source_remove (dummy_id);
	dummy_id = 0;
      }
      if (drop_items_id) {
	g_source_remove (drop_items_id);
	drop_items_id = 0;
      }
    }
  else
    {
      if(!drop_items_id) {
	drop_items_id = g_timeout_add (0,
					 (GtkFunction) wordsgame_drop_items, NULL);
      }
      if(!dummy_id) {
	dummy_id = g_timeout_add (10, (GtkFunction) wordsgame_move_items, NULL);
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

    gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "images/scenery_background.png");


    gcomprisBoard->level = 1;
    gcomprisBoard->maxlevel = 6;
    gcomprisBoard->sublevel = 0;
    gcompris_bar_set(GCOMPRIS_BAR_LEVEL);

    /* Default speed */
    speed=DEFAULT_SPEED;
    fallSpeed=DEFAULT_FALLSPEED;

    wordsgame_next_level();
    }
}

static void
end_board ()
{

  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      gcompris_score_end();
      wordsgame_destroy_all_items();
      gcomprisBoard = NULL;
    }
}

static void
set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      wordsgame_next_level();
    }
}


static gint key_press(guint keyval)
{
    gchar *letter; 
    gint i;
    LettersItem *item;

    if(!gcomprisBoard)
	return TRUE;

    if(!g_unichar_isalnum (gdk_keyval_to_unicode (keyval)))
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
          keyval=GDK_0;
          break;
        case GDK_KP_1:
          keyval=GDK_1;
          break;
        case GDK_KP_2:
          keyval=GDK_2;
          break;
        case GDK_KP_3:
          keyval=GDK_3;
          break;
        case GDK_KP_4:
          keyval=GDK_4;
          break;
        case GDK_KP_5:
          keyval=GDK_5;
          break;
        case GDK_KP_6:
          keyval=GDK_6;
	  break;
        case GDK_KP_7:
	  keyval=GDK_7;
          break;
        case GDK_KP_8:
	  keyval=GDK_8;
          break;
        case GDK_KP_9:
          keyval=GDK_9;
          break;
        }

    letter=g_strnfill(6,'\0');
    g_unichar_to_utf8 (gdk_keyval_to_unicode(keyval), letter);

    
    if(item_on_focus==NULL) 
	{
	g_static_rw_lock_reader_lock (&items_lock);
	gint count=items->len;
	g_static_rw_lock_reader_unlock (&items_lock);

	for (i=0;i<count;i++)
	    { 
	    g_static_rw_lock_reader_lock (&items_lock);
	    item=g_ptr_array_index(items,i);
	    g_static_rw_lock_reader_unlock (&items_lock);
	    assert (item!=NULL);
	    if (strcmp(item->letter,letter)==0) 
		{
		item_on_focus=item;
	    	break;	     
		}
	    }
	}


    if(item_on_focus!=NULL) 
	{
	
	if(strcmp(item_on_focus->letter, letter)==0)
	    {
	    item_on_focus->count++;
	    g_free(item_on_focus->overword);
	    item_on_focus->overword=g_utf8_strndup(item_on_focus->word,item_on_focus->count);
	    gnome_canvas_item_set (item_on_focus->overwriteItem,
				     "text", item_on_focus->overword,
				     NULL);
		

	    if (item_on_focus->count<g_utf8_strlen(item_on_focus->word,-1))
	        {
	        g_free(item_on_focus->letter);
	        item_on_focus->letter=g_utf8_strndup(item_on_focus->pos,1);
		item_on_focus->pos=g_utf8_find_next_char(item_on_focus->pos,NULL);
	        }
	    else
	        {
	        player_win(item_on_focus);
	        item_on_focus=NULL;
	        }
	    }
	else
	    {
	     /* It is a loose : unselect the word and defocus */
	    g_free(item_on_focus->overword);
	    item_on_focus->overword=g_strdup(" ");
	    item_on_focus->count=0;
	    g_free(item_on_focus->letter);
	    item_on_focus->letter=g_utf8_strndup(item_on_focus->word,1);
	    
	    item_on_focus->pos=g_utf8_find_next_char(item_on_focus->word,NULL);

	    gnome_canvas_item_set (item_on_focus->overwriteItem,
			     "text", item_on_focus->overword,
				     NULL);
 	    item_on_focus=NULL;
	    player_loose();
	    }	
	}
    else
	{
        /* Anyway kid you clicked on the wrong key */
        player_loose();
	}

  g_free(letter);
  return FALSE;
}

static gboolean
is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "wordsgame")==0)
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
static void wordsgame_next_level() 
{


  gcomprisBoard->number_of_sublevel = 10 + 
    ((gcomprisBoard->level-1) * 5);
  gcompris_score_start(SCORESTYLE_NOTE, 
		       gcomprisBoard->width - 220, 
		       gcomprisBoard->height - 50, 
		       gcomprisBoard->number_of_sublevel);
  
  gcompris_bar_set_level(gcomprisBoard);
  gcompris_score_set(gcomprisBoard->sublevel);


  wordsgame_destroy_all_items();
  wordsgame_read_wordfile();
  items=g_ptr_array_new();
  items2del=g_ptr_array_new();

  
  /* Increase speed only after 5 levels */
  if(gcomprisBoard->level>5)
    {
    gint temp=fallSpeed-gcomprisBoard->level*200;
    if (temp > MIN_FALLSPEED)	fallSpeed=temp;
    }

  pause_board(FALSE);
}


static void wordsgame_move_item(LettersItem *item)
{
    double x1, y1, x2, y2;


    gnome_canvas_item_move(item->rootitem, 0, 2.0);

    gnome_canvas_item_get_bounds    (item->rootitem,
				   &x1,
				   &y1,
				   &x2,
				   &y2);
  
      if(y1>gcomprisBoard->height) {

	if (item == item_on_focus)   
	    item_on_focus = NULL;

	g_static_rw_lock_writer_lock (&items_lock);
	g_ptr_array_remove (items, item);
	g_static_rw_lock_writer_unlock (&items_lock);

	g_static_rw_lock_writer_lock (&items2del_lock);
	g_ptr_array_add (items2del, item);
	g_static_rw_lock_writer_unlock (&items2del_lock);

        g_timeout_add (100,(GtkFunction) wordsgame_destroy_items, items2del);

	player_loose();
    }
}

/*
 * This does the moves of the game items on the play canvas
 *
 */
static gint wordsgame_move_items (GtkWidget *widget, gpointer data)
{
    assert (items!=NULL);
    gint i;
    LettersItem *item;
    
    for (i=items->len-1;i>=0;i--)
	{
	
	g_static_rw_lock_reader_lock (&items_lock);
	item=g_ptr_array_index(items,i);
	g_static_rw_lock_reader_unlock (&items_lock);
	wordsgame_move_item(item);
	}
    dummy_id = g_timeout_add (speed,(GtkFunction) wordsgame_move_items, NULL);
    return (FALSE);
}



static void wordsgame_destroy_item(LettersItem *item)
{
 
  /* The items are freed by player_win */
  gtk_object_destroy (GTK_OBJECT(item->rootitem));
  g_free(item->word);
  g_free(item->overword);
  g_free(item->letter);
  g_free(item);
}

/* Destroy items that falls out of the canvas */
static gboolean wordsgame_destroy_items(GPtrArray *item_list)
{
    LettersItem *item;

    assert(item_list!=NULL);
    


    if  (item_list==items) {
	g_static_rw_lock_writer_lock (&items_lock);
	while (item_list->len>0) 
	    {
	    item = g_ptr_array_index(item_list,0);
    	    g_ptr_array_remove_index_fast(item_list,0);
	    wordsgame_destroy_item(item);
	    }
	g_static_rw_lock_writer_unlock (&items_lock);
	}

    if  (item_list==items2del) {
	g_static_rw_lock_writer_lock (&items2del_lock);
	while (item_list->len>0) 
	    {
	    item = g_ptr_array_index(item_list,0);
    	    g_ptr_array_remove_index_fast(item_list,0);
	    wordsgame_destroy_item(item);
	    }
	g_static_rw_lock_writer_unlock (&items2del_lock);
	}

    return (FALSE);
}

/* Destroy all the items */
static void wordsgame_destroy_all_items()
{
    
  if (items!=NULL){
    if(items->len > 0) {
	wordsgame_destroy_items(items);
	}
    g_ptr_array_free (items, TRUE);
    items=NULL;
    }

  if (items2del!=NULL){
    if(items2del->len > 0) {
	wordsgame_destroy_items(items2del);
	}
    g_ptr_array_free (items2del, TRUE);
    items2del=NULL;
    }
    
  if (words!=NULL){
    g_ptr_array_free (words, TRUE);
    words=NULL;
    }

}


static GnomeCanvasItem *wordsgame_create_item(GnomeCanvasGroup *parent)
{
 

    GnomeCanvasItem *item2;
    LettersItem *item;

    // create and init item
    item = g_new(LettersItem,1);
    item->word = g_strdup(g_ptr_array_index(words,rand()%words->len));
    item->overword=g_strdup("");
    item->count=0;
    item->letter=g_utf8_strndup(item->word,1);
    item->pos=g_utf8_find_next_char(item->word,NULL);
     
    item->rootitem = \
	gnome_canvas_item_new (parent,
			   gnome_canvas_group_get_type (),
			   "x", (double) 0,
			   "y", (double) -12,
			   NULL);

  /* To 'erase' words, I create 2 times the text item. One is empty now */
  /* It will be filled each time the user enters the right key         */  
    item2 = \
	gnome_canvas_item_new (GNOME_CANVAS_GROUP(item->rootitem),
			   gnome_canvas_text_get_type (),
			   "text", item->word,
			   "font", gcompris_skin_font_board_huge_bold,
			   "x", (double) 0,
			   "y", (double) 0,
			   "anchor", GTK_ANCHOR_NW,
			   "fill_color_rgba", 0xba00ffff,
			   NULL);

    item->overwriteItem = \
	gnome_canvas_item_new (GNOME_CANVAS_GROUP(item->rootitem),
			   gnome_canvas_text_get_type (),
			   "text", item->overword,
			   "font", gcompris_skin_font_board_huge_bold,
			   "x", (double) 0,
			   "y", (double) 0,
			   "anchor", GTK_ANCHOR_NW,
			   "fill_color", "blue",
			   NULL);
			   
   /*set right x position */

    double x1, y1, x2, y2;
                                                                                                          
                                                                                                                          
    gnome_canvas_item_get_bounds    (item->rootitem,
                                   &x1,
                                   &y1,
                                   &x2,
                                   &y2);
                                                                                                                      
    gnome_canvas_item_move (item->rootitem,(double) (rand()%(gcomprisBoard->width-(gint)(x2))),(double) 0);


    g_static_rw_lock_writer_lock (&items_lock);
    g_ptr_array_add(items, item);
    g_static_rw_lock_writer_unlock (&items_lock);

    return (item->rootitem);
}

static void wordsgame_add_new_item() 
{

 assert(gcomprisBoard->canvas!=NULL); 
  wordsgame_create_item(gnome_canvas_root(gcomprisBoard->canvas));

}

/*
 * This is called on a low frequency and is used to drop new items
 *
 */
static gint wordsgame_drop_items (GtkWidget *widget, gpointer data)
{
  wordsgame_add_new_item();
  g_source_remove(drop_items_id);
  drop_items_id = g_timeout_add (fallSpeed,(GtkFunction) wordsgame_drop_items, NULL);
 
  return (FALSE);
}

static void player_win(LettersItem *item)
{

  gcompris_play_ogg ("gobble", NULL);

  assert(gcomprisBoard!=NULL);

  gcomprisBoard->sublevel++;
  gcompris_score_set(gcomprisBoard->sublevel);


    g_static_rw_lock_writer_lock (&items_lock);
    g_ptr_array_remove(items,item);
    g_static_rw_lock_writer_unlock (&items_lock);

    g_static_rw_lock_writer_lock (&items2del_lock);
    g_ptr_array_add(items2del,item);
    g_static_rw_lock_writer_unlock (&items2del_lock);

    gnome_canvas_item_hide(item->rootitem);
    g_timeout_add (500,(GtkFunction) wordsgame_destroy_items, items2del);


  if(gcomprisBoard->sublevel > gcomprisBoard->number_of_sublevel) 
    {

      /* Try the next level */
      gcomprisBoard->level++;
      gcomprisBoard->sublevel = 0;
      if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
	board_finished(BOARD_FINISHED_RANDOM);
	return;
      }
      wordsgame_next_level();
      gcompris_play_ogg ("bonus", NULL);
    }
  else
    {
                                                                                                                              
      /* Drop a new item now to speed up the game */
    g_static_rw_lock_reader_lock (&items_lock);
    gint count=items->len;
    g_static_rw_lock_reader_unlock (&items_lock);
                                                                                                                              
      if(count==0)
        {

	  if ((fallSpeed-=INCREMENT_FALLSPEED) < MIN_FALLSPEED) fallSpeed+=INCREMENT_FALLSPEED;

	  if ((speed-=INCREMENT_SPEED) < MIN_SPEED) speed+=INCREMENT_SPEED;
	  
          if (drop_items_id) {
            /* Remove pending new item creation to sync the falls */
            g_source_remove (drop_items_id);
            drop_items_id = 0;
          }
	  
          if(!drop_items_id) {
            drop_items_id = g_timeout_add (0,
                                             (GtkFunction) wordsgame_drop_items, NULL);
          }
  
        }
    }

}

static void player_loose()
{
  gcompris_play_ogg ("crash", NULL);
}


static FILE *get_wordfile(const char *locale)
{
  char *filename;
  FILE *wordsfd = NULL;

  /* First Try to find a file matching the level and the locale */
  filename = g_strdup_printf("%s%s%d.%.2s",  
			     PACKAGE_DATA_DIR, "/wordsgame/wordslevel", 
			     gcomprisBoard->level, locale);
  g_message("Trying to open file %s ", filename);
  wordsfd = fopen (filename, "r");

  if(wordsfd==NULL)
    {
      /* Second Try to find a file matching the 'max' and the locale */
      sprintf(filename, "%s%s%.2s",  
	      PACKAGE_DATA_DIR, "/wordsgame/wordslevelmax.", 
	      locale);
      g_message("Trying to open file %s ", filename);

      wordsfd = fopen (filename, "r");
    }

  g_free(filename);

  return wordsfd;
}
/*
 * Return a random word from a set of text file depending on 
 * the current level and language
 */
                                                                                                                              
static gboolean  wordsgame_read_wordfile()
{
                                                                                                                              
  FILE *wordsfd;
  gchar *buf;
  int len;
                                                                                                                              
  wordsfd = get_wordfile(gcompris_get_locale());
                                                                                                                              
  if(wordsfd==NULL)
    {
      /* Try to open the english locale by default */
      wordsfd = get_wordfile("en");
                                                                                                                              
      /* Too bad, even english is not there. Check your Install */
      if(wordsfd==NULL) {
        gcompris_dialog(_("Cannot open file of words for your locale"), gcompris_end_board);
        return FALSE;
      }
    }
                                                                                                                              
   words=g_ptr_array_new ();
   while (buf=fgets(g_new(gchar,MAXWORDSLENGTH), MAXWORDSLENGTH, wordsfd)) {
        assert(g_utf8_validate(buf,-1,NULL));
                                                                                                                              
        //remove \n from end of line
        len = strlen(buf);
        if((0 < len)&&('\n'==buf[len-1]))
            buf[len-1] = '\0';
                                                                                                                              
        g_ptr_array_add(words,buf);
        }
   fclose(wordsfd);
                                                                                                                              
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
