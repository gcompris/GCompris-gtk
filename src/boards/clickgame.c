/* gcompris - clickgame.c
 *
 * Time-stamp: <2006/08/21 23:30:07 bruno>
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

#ifdef __APPLE__
#   include <sys/types.h>
#endif
#include <string.h>

#include "gcompris/gcompris.h"
#include "gcompris/pixbuf_util.h"

#include <libart_lgpl/art_affine.h>

#define SOUNDLISTFILE PACKAGE

static gboolean board_paused = TRUE;

static GList *item_list = NULL;
static GList *item2del_list = NULL;

static GcomprisBoard *gcomprisBoard = NULL;

static gint dummy_id = 0;
static gint animate_id = 0;
static gint drop_items_id = 0;

static  GList *imagelist = NULL;

typedef struct {
  double speed;
  gint currentItem;
  GnomeCanvasItem *rootitem;
  GList *item_list;
} FishItem;

static void	 clickgame_start (GcomprisBoard *agcomprisBoard);
static void	 clickgame_pause (gboolean pause);
static void	 clickgame_end (void);
static gboolean	 clickgame_is_our_board (GcomprisBoard *gcomprisBoard);
static void	 clickgame_set_level (guint level);
static void	 clickgame_config(void);

static FishItem *clickgame_create_item(GnomeCanvasGroup *parent);
static gint	 clickgame_drop_items (GtkWidget *widget, gpointer data);
static gint	 clickgame_move_items (GtkWidget *widget, gpointer data);
static gint	 clickgame_animate_items (GtkWidget *widget, gpointer data);
static void	 clickgame_destroy_item(FishItem *fishitem);
static void	 clickgame_destroy_items(void);
static void	 clickgame_destroy_all_items(void);
static void	 setup_item(FishItem *fishitem);
static void	 load_random_pixmap(void);
static void	 clickgame_next_level(void);

static int gamewon;
static void	 game_won(void);

static  guint32              fallSpeed = 0;
static  double               speed = 0.0;
static  double               imageZoom = 0.0;

// List of images to use in the game
static gchar *pixmapList[] =
  {
    "fishes/blueking2_%d.png",
    "fishes/butfish_%d.png",
    "fishes/cichlid1_%d.png",
    "fishes/cichlid4_%d.png",
    "fishes/collaris_%d.png",
    "fishes/discus2_%d.png",
    "fishes/discus3_%d.png",
    "fishes/eel_%d.png",
    "fishes/f00_%d.png",
    "fishes/f01_%d.png",
    "fishes/f02_%d.png",
    "fishes/f03_%d.png",
    "fishes/f04_%d.png",
    "fishes/f05_%d.png",
    "fishes/f06_%d.png",
    "fishes/f07_%d.png",
    "fishes/f08_%d.png",
    "fishes/f09_%d.png",
    "fishes/f10_%d.png",
    "fishes/f11_%d.png",
    "fishes/f12_%d.png",
    "fishes/f13_%d.png",
    "fishes/manta_%d.png",
    "fishes/newf1_%d.png",
    "fishes/QueenAngel_%d.png",
    "fishes/shark1_%d.png",
    "fishes/six_barred_%d.png",
    "fishes/teeth_%d.png",
  };
#define NUMBER_OF_IMAGES G_N_ELEMENTS(pixmapList)

/* Description of this plugin */
static BoardPlugin menu_bp =
{
   NULL,
   NULL,
   "Click On Me",
   "Left-Click with the mouse on all swimming fish before they leave the fishtank",
   "Bruno Coudoin <bruno.coudoin@free.fr>",
   NULL,
   NULL,
   NULL,
   NULL,
   clickgame_start,
   clickgame_pause,
   clickgame_end,
   clickgame_is_our_board,
   NULL,
   NULL,
   clickgame_set_level,
   clickgame_config,
   NULL,
   NULL,
   NULL
};

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(clickgame)

/*
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 *
 */
static void clickgame_pause (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  if(gamewon == TRUE && pause == FALSE) /* the game is won */
    {
      game_won();
    }

  if(pause)
    {
      if (dummy_id) {
	gtk_timeout_remove (dummy_id);
	dummy_id = 0;
      }
      if (animate_id) {
	gtk_timeout_remove (animate_id);
	animate_id = 0;
      }
      if (drop_items_id) {
	gtk_timeout_remove (drop_items_id);
	drop_items_id = 0;
      }
    }
  else
    {
      if(!drop_items_id) {
	drop_items_id = gtk_timeout_add (200,
					 (GtkFunction) clickgame_drop_items, NULL);
      }
      if(!dummy_id) {
	dummy_id = gtk_timeout_add (200, (GtkFunction) clickgame_move_items, NULL);
      }
      if(!animate_id) {
	animate_id = gtk_timeout_add (200, (GtkFunction) clickgame_animate_items, NULL);
      }
    }

  board_paused = pause;
}

/*
 */
static void clickgame_start (GcomprisBoard *agcomprisBoard)
{
  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard = agcomprisBoard;

      /* set initial values for this level */
      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel=6;
      gcomprisBoard->number_of_sublevel=10; /* Go to next level after this number of 'play' */
      gc_score_start(SCORESTYLE_NOTE,
			   gcomprisBoard->width - 220,
			   gcomprisBoard->height - 50,
			   gcomprisBoard->number_of_sublevel);
      gc_bar_set(GC_BAR_LEVEL);

      clickgame_next_level();

      clickgame_pause(FALSE);

    }

}

static void
clickgame_end ()
{
  if(gcomprisBoard!=NULL)
    {
      clickgame_pause(TRUE);
      gc_score_end();
      clickgame_destroy_all_items();
      gcomprisBoard->level = 1;       // Restart this game to zero
    }
  gcomprisBoard = NULL;
}

static void
clickgame_set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      clickgame_next_level();
    }
}

static gboolean
clickgame_is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "clickgame")==0)
	{
	  /* Set the plugin entry */
	  gcomprisBoard->plugin=&menu_bp;

	  return TRUE;
	}
    }
  return FALSE;
}

static void
clickgame_config ()
{
  if(gcomprisBoard!=NULL)
    {
      clickgame_pause(TRUE);
    }
}

/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

/* set initial values for the next level */
static void clickgame_next_level()
{

  switch(gcomprisBoard->level)
    {
    case 1:
      gc_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			      "clickgame/nur00523.jpg");
      break;
    case 2:
      gc_set_background(gnome_canvas_root(gcomprisBoard->canvas),
      			      "clickgame/nur03006.jpg");
      break;
    case 3:
      gc_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			      "clickgame/nur03011.jpg");
      break;
    case 4:
      gc_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			      "clickgame/nur03010.jpg");
      break;
    case 5:
      gc_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			      "clickgame/nur03013.jpg");
      break;
    default:
      gc_set_background(gnome_canvas_root(gcomprisBoard->canvas),
			      "clickgame/nur03505.jpg");
    }

  gc_bar_set_level(gcomprisBoard);

  /* Try the next level */
  speed=100+(40/(gcomprisBoard->level));
  fallSpeed=5000-gcomprisBoard->level*200;
  /* Make the images tend to 0.5 ratio */
  imageZoom=0.5+(0.5/(gcomprisBoard->level));
  gcomprisBoard->sublevel=0;
  gc_score_set(gcomprisBoard->sublevel);

}


static void clickgame_animate_item(FishItem *fishitem)
{
  gint currentItem;

  /* Manage the animation */
  currentItem = fishitem->currentItem;
  fishitem->currentItem++;
  if(fishitem->currentItem >= g_list_length(fishitem->item_list))
    fishitem->currentItem=0;

  gnome_canvas_item_show((GnomeCanvasItem *)g_list_nth_data(fishitem->item_list,
							    fishitem->currentItem));

  gnome_canvas_item_hide((GnomeCanvasItem *)g_list_nth_data(fishitem->item_list,
							    currentItem));
}

static void clickgame_move_item(FishItem *fishitem)
{
  double x1, y1, x2, y2;

  GnomeCanvasItem *item = fishitem->rootitem;

  gnome_canvas_item_move(item, fishitem->speed, 0.0);


  gnome_canvas_item_get_bounds    (item,
				   &x1,
				   &y1,
				   &x2,
				   &y2);

  if(fishitem->speed>0)
    {
      if(x1>gcomprisBoard->width) {
	item2del_list = g_list_append (item2del_list, fishitem);
	gc_sound_play_ogg ("crash", NULL);
      }
    }
  else
    {
      if(x2<0) {
	item2del_list = g_list_append (item2del_list, fishitem);
	gc_sound_play_ogg ("crash", NULL);
      }
    }

}

static void clickgame_destroy_item(FishItem *fishitem)
{
  GnomeCanvasItem *item = fishitem->rootitem;

  item_list = g_list_remove (item_list, fishitem);
  item2del_list = g_list_remove (item2del_list, fishitem);
  gtk_object_destroy (GTK_OBJECT(item));

  g_list_free(fishitem->item_list);
  g_free(fishitem);
}

/* Destroy items that falls out of the canvas */
static void clickgame_destroy_items()
{
  FishItem *fishitem;

  while(g_list_length(item2del_list)>0)
    {
      fishitem = g_list_nth_data(item2del_list, 0);
      clickgame_destroy_item(fishitem);
    }
}

/* Destroy all the items */
static void clickgame_destroy_all_items()
{
  FishItem *fishitem;

  while(g_list_length(item_list)>0)
    {
      fishitem = g_list_nth_data(item_list, 0);
      clickgame_destroy_item(fishitem);
    }
  if(item_list) {
    g_list_free(item_list);
    item_list = NULL;
  }

  if(imagelist) {
    g_list_free(imagelist);
    imagelist = NULL;
  }
}

/*
 * This does the moves of the game items on the play canvas
 *
 */
static gint clickgame_move_items (GtkWidget *widget, gpointer data)
{
  g_list_foreach (item_list, (GFunc) clickgame_move_item, NULL);

  /* Destroy items that falls out of the canvas */
  clickgame_destroy_items();

  dummy_id = gtk_timeout_add (speed,
			      (GtkFunction) clickgame_move_items, NULL);

  return(FALSE);
}

/*
 * This does the icon animation
 *
 */
static gint clickgame_animate_items (GtkWidget *widget, gpointer data)
{
  g_list_foreach (item_list, (GFunc) clickgame_animate_item, NULL);

  animate_id = gtk_timeout_add (1000,
			      (GtkFunction) clickgame_animate_items, NULL);

  return(FALSE);
}

static FishItem *clickgame_create_item(GnomeCanvasGroup *parent)
{
  GdkPixbuf *pixmap = NULL;
  GdkPixbuf *pixmap2 = NULL;
  GnomeCanvasItem *rootitem, *item;
  FishItem *fishitem;
  double x;
  gint i, length;

  /* Avoid to have too much items displayed */
  if(g_list_length(item_list)>5)
    return NULL;

  load_random_pixmap();

  fishitem = g_malloc(sizeof(FishItem));
  fishitem->currentItem   = 0;
  fishitem->speed = (double)(rand()%(60))/10 - 3;
  fishitem->item_list = NULL;

  pixmap = (GdkPixbuf *)g_list_nth_data(imagelist, 0);

  if(pixmap==NULL)
    return NULL;

  if(fishitem->speed<0)
    {
      x = (double) gcomprisBoard->width;
      fishitem->speed=MIN(fishitem->speed, -1);
    }
  else
    {
      x = (double) -gdk_pixbuf_get_width(pixmap)*imageZoom;
      fishitem->speed=MAX(fishitem->speed, 1);
    }

  rootitem = \
    gnome_canvas_item_new (parent,
			   gnome_canvas_group_get_type (),
			   "x", x,
			   "y", (double)(rand()%(gcomprisBoard->height-
						 (guint)(gdk_pixbuf_get_height(pixmap)*
							 imageZoom))),
			   NULL);


  fishitem->rootitem = rootitem;

  length = g_list_length(imagelist);
  for(i=0; i<length; i++)
    {
      pixmap = (GdkPixbuf *)g_list_nth_data(imagelist, i);

      pixmap2 = pixbuf_copy_mirror(pixmap, (fishitem->speed<0?TRUE:FALSE), FALSE);

      gdk_pixbuf_unref(pixmap);

      item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(rootitem),
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap2,
				    "x", 0.0,
				    "y", 0.0,
				    "width", (double) gdk_pixbuf_get_width(pixmap)*imageZoom,
				    "height", (double) gdk_pixbuf_get_height(pixmap)*imageZoom,
				    "width_set", TRUE,
				    "height_set", TRUE,
				    NULL);
      gdk_pixbuf_unref(pixmap2);

      if(i==fishitem->currentItem)
	gnome_canvas_item_show(item);
      else
	gnome_canvas_item_hide(item);

      fishitem->item_list = g_list_append (fishitem->item_list, item);
    }

  for(i=0; i<length; i++)
    {
      pixmap = (GdkPixbuf *)g_list_nth_data(imagelist, 0);
      imagelist = g_list_remove(imagelist, pixmap);
    }
  item_list = g_list_append (item_list, fishitem);

  return (fishitem);
}

static void clickgame_add_new_item()
{
  setup_item (clickgame_create_item(gnome_canvas_root(gcomprisBoard->canvas)));
}

/*
 * This is called on a low frequency and is used to drop new items
 *
 */
static gint clickgame_drop_items (GtkWidget *widget, gpointer data)
{
  clickgame_add_new_item();

  drop_items_id = gtk_timeout_add (fallSpeed,
				   (GtkFunction) clickgame_drop_items, NULL);
  return (FALSE);
}

/* ==================================== */
static void game_won()
{
  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel>=gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->sublevel=0;
    gcomprisBoard->level++;
    if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
      gc_bonus_end_display(BOARD_FINISHED_RANDOM);
      return;
    }
    gc_sound_play_ogg ("sounds/bonus.ogg", NULL);
  }
  clickgame_next_level();
}

static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, FishItem *fishitem)
{
   static double x, y;
   double new_x, new_y;
   GdkCursor *fleur;
   static int dragging;
   double item_x, item_y;

   if(!gcomprisBoard)
     return FALSE;

   if(board_paused)
     return FALSE;

   item_x = event->button.x;
   item_y = event->button.y;
   gnome_canvas_item_w2i(item->parent, &item_x, &item_y);

   switch (event->type)
     {
     case GDK_BUTTON_PRESS:
       switch(event->button.button)
         {
         case 1:
           if (event->button.state & GDK_SHIFT_MASK)
             {
               x = item_x;
               y = item_y;

               fleur = gdk_cursor_new(GDK_FLEUR);
               gc_canvas_item_grab(item,
                                      GDK_POINTER_MOTION_MASK |
                                      GDK_BUTTON_RELEASE_MASK,
                                      fleur,
                                      event->button.time);
               gdk_cursor_destroy(fleur);
               dragging = TRUE;
             }
           else
             {
	       clickgame_destroy_item(fishitem);
	       gc_sound_play_ogg ("sounds/gobble.ogg", NULL);

	       gcomprisBoard->sublevel++;
	       gc_score_set(gcomprisBoard->sublevel);

	       if(gcomprisBoard->sublevel>=gcomprisBoard->number_of_sublevel) {
		 gamewon = TRUE;
		 clickgame_destroy_all_items();
		 gc_bonus_display(gamewon, BONUS_FLOWER);
		 return FALSE;
	       }
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
						      (GtkFunction) clickgame_drop_items,
						      NULL);
		   }
		 }
             }
           break;

         case 3:
	   /* Speed up the fish */
	   if(fishitem->speed>0)
	     fishitem->speed = MAX(fishitem->speed+1 , 5);
	   else
	     fishitem->speed = MIN(fishitem->speed-1, -5);
	   break;

         case 2:
	   /* Slow the fish */
	   if(fishitem->speed>0)
	     fishitem->speed = MAX(fishitem->speed-1, 1);
	   else
	     fishitem->speed = MIN(fishitem->speed+1, -1);
	   break;

         case 4:
	   /* fish up */
	   gnome_canvas_item_move(item, 0.0, -3.0);
	   break;

         case 5:
	   /* fish down */
	   gnome_canvas_item_move(item, 0.0, 3.0);
	   break;

         default:
           break;
         }
       break;

     case GDK_MOTION_NOTIFY:
       if (dragging && (event->motion.state & GDK_BUTTON1_MASK))
         {
           new_x = item_x;
           new_y = item_y;

           gnome_canvas_item_move(item, new_x - x, new_y - y);
           x = new_x;
           y = new_y;
         }
       break;

     case GDK_BUTTON_RELEASE:
       if(dragging)
	 {
	   gc_canvas_item_ungrab(item, event->button.time);
	   dragging = FALSE;
	 }
       break;

     default:
       break;
     }

   return FALSE;
 }

static void
setup_item(FishItem *fishitem)
{
  if(fishitem)
      gtk_signal_connect(GTK_OBJECT(fishitem->rootitem), "event",
			 (GtkSignalFunc) item_event,
			 fishitem);
}

/*
 * Return the list of pixmap in the global
 * imagelist
 */
static void load_random_pixmap()
{
  GdkPixbuf *pixmap;
  int i, j;
  gchar *str = NULL;
  gboolean cont = TRUE;

  i=rand()%(NUMBER_OF_IMAGES);

  /* First image */
  j=0;

  while(cont)
    {
      gchar *url;
      str = g_strdup(pixmapList[i]);

      url = gc_file_find_absolute(str, j++);

      if(!url)
	{
	  cont = FALSE;
	}
      else
	{
	  pixmap = gc_pixmap_load (str, j-1);

	  if(pixmap) {
	    imagelist = g_list_append (imagelist, pixmap);
	  } else {
	    cont = FALSE;
	  }
	}
      g_free(str);
    }

}
