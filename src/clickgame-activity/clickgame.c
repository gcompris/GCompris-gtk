/* gcompris - clickgame.c
 *
 * Copyright (C) 2000, 2008 Bruno Coudoin
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef __APPLE__
#   include <sys/types.h>
#endif
#include <string.h>

#include "gcompris/gcompris.h"
#include "gcompris/pixbuf_util.h"

#define SOUNDLISTFILE PACKAGE

static gboolean board_paused = TRUE;

static GList *item_list = NULL;     // onscreen fish
static GList *item2del_list = NULL;

static GcomprisBoard *gcomprisBoard = NULL;

static gint move_items_id = 0;
static gint animate_id = 0;
static gint drop_items_id = 0;

typedef struct {
  double speed;
  gint fright;
  gint stun;
  gint currentItem;
  GooCanvasItem *rootitem;
  GSList *fwd_frames;
  GSList *rev_frames;
  GSList *cur_frames;  // points to fwd_frames or rev_frames
} FishItem;

static void	 clickgame_start (GcomprisBoard *agcomprisBoard);
static void	 clickgame_pause (gboolean pause);
static void	 clickgame_end (void);
static gboolean	 clickgame_is_our_board (GcomprisBoard *gcomprisBoard);
static void	 clickgame_set_level (guint level);
static void	 clickgame_config(void);

static FishItem *clickgame_create_item();
static gboolean	 clickgame_drop_items (gpointer data);
static gboolean	 clickgame_move_items (gpointer data);
static gboolean  clickgame_animate_items (gpointer data);
static void	 clickgame_destroy_item(FishItem *fishitem);
static void	 clickgame_destroy_items(void);
static void	 clickgame_destroy_all_items(void);
static void	 clickgame_next_level(void);

static int gamewon;
static void	 game_won(void);

static  guint32              fallSpeed = 0;
static  double               moveSpeed = 0.0;
static  double               imageZoom = 0.0;


static const gchar *smallFish[] =
  {
    "%s/cichlid1_%d.png",
    "%s/cichlid4_%d.png",
    "%s/collaris_%d.png",
    "%s/discus3_%d.png",
    "%s/eel_%d.png",
    "%s/teeth_%d.png",
    "%s/f02_%d.png",
  };
#define NUM_SMALLFISH G_N_ELEMENTS(smallFish)

static const gchar *medFish[] =
  {
    "%s/f00_%d.png",
    "%s/f01_%d.png",
    "%s/f03_%d.png",
    "%s/f04_%d.png",
    "%s/f05_%d.png",
    "%s/f06_%d.png",
    "%s/f11_%d.png",
    "%s/f12_%d.png",
    "%s/f13_%d.png",
    "%s/QueenAngel_%d.png",
    "%s/six_barred_%d.png",
    "%s/shark1_%d.png",
  };
#define NUM_MEDFISH G_N_ELEMENTS(medFish)

static const gchar *bigFish[] =
  {
    "%s/blueking2_%d.png",
    "%s/butfish_%d.png",
    "%s/discus2_%d.png",
    "%s/f07_%d.png",
    "%s/f08_%d.png",
    "%s/f09_%d.png",
    "%s/f10_%d.png",
    "%s/manta_%d.png",
    "%s/newf1_%d.png",
  };
#define NUM_BIGFISH G_N_ELEMENTS(bigFish)

/* Description of this plugin */
static BoardPlugin menu_bp =
{
   NULL,
   NULL,
   "Click On Me",
   "Click with the mouse on all swimming fish before they leave the fishtank",
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
      if (move_items_id) {
	g_source_remove (move_items_id);
	move_items_id = 0;
      }
      if (animate_id) {
	g_source_remove (animate_id);
	animate_id = 0;
      }
      if (drop_items_id) {
	g_source_remove (drop_items_id);
	drop_items_id = 0;
      }
    }
  else
    {
      if(!drop_items_id) {
	drop_items_id = g_timeout_add (200,
				       clickgame_drop_items, NULL);
      }
      if(!move_items_id) {
	move_items_id = g_timeout_add (200, clickgame_move_items, NULL);
      }
      if(!animate_id) {
	animate_id = g_timeout_add (200, clickgame_animate_items, NULL);
      }
    }

  board_paused = pause;
}

static void
fish_reverse_direction (FishItem *fish)
{
  fish->speed = -fish->speed;

  g_object_set(g_slist_nth_data(fish->cur_frames,
				fish->currentItem),
	       "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

  fish->cur_frames =
    fish->speed<0? fish->rev_frames : fish->fwd_frames;

  g_object_set(g_slist_nth_data(fish->cur_frames,
				fish->currentItem),
	       "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
}

static void
fish_gobble (FishItem *fishitem)
{
  clickgame_destroy_item(fishitem);
  gc_sound_play_ogg ("sounds/bubble.wav", NULL);

  gcomprisBoard->sublevel++;
  gc_score_set(gcomprisBoard->sublevel);

  if(gcomprisBoard->sublevel>=gcomprisBoard->number_of_sublevel) {
    gamewon = TRUE;
    clickgame_destroy_all_items();
    gc_bonus_display(gamewon, GC_BONUS_FLOWER);
    return;
  }
  /* Drop a new item now to speed up the game */
  if(g_list_length(item_list)==0)
    {
      /* Remove any pending new item creation to sync the falls */
      if (drop_items_id)
	g_source_remove (drop_items_id);
      drop_items_id =
	g_timeout_add (0, clickgame_drop_items, NULL);
    }
}

static gboolean
canvas_event (GooCanvasItem  *item,
	      GooCanvasItem  *target,
	      GdkEventButton *event,
	      gpointer data)
{
  FishItem *fish;
  gdouble mouse_x;
  gdouble mouse_y;
  int ii;

  mouse_x = event->x;
  mouse_y = event->y;

  if (gcomprisBoard->level >= 3 &&
      !(event->state & GDK_SHIFT_MASK)) {
    for (ii=0; (fish=g_list_nth_data(item_list, ii)); ii++) {
      double dx, dy, near;
      int react;
      GooCanvasBounds bounds;
      goo_canvas_item_get_bounds (fish->rootitem, &bounds);
      //printf("fish %d\n", ii);

      dy = (mouse_y - (bounds.y1 + (bounds.y2 - bounds.y1) / 2)) / ((bounds.y2 - bounds.y1) / 2);
      //printf("dy = %.2f\n", dy);
      if (fabs(dy) > 3) continue;

      dx = (mouse_x - (bounds.x1 + (bounds.x2 - bounds.x1) / 2)) / ((bounds.x2 - bounds.x1) / 2);
      //printf("dx = %.2f\n", dx);
      if (fabs(dx) > 3) continue;

      // 0 to .9
      near = (sqrt(2*3*3) - sqrt(dx*dx + dy*dy))/(1.11 * sqrt(2*3*3));
      //printf("near = %.2f\n", near);

      react = ((rand() % 1000 < near * 1000) +
	       (rand() % 1000 < near * 1000));
      if (react) {
	if (goo_canvas_get_item_at (goo_canvas_item_get_canvas (item),
				    mouse_x, mouse_y, FALSE) !=
	    g_slist_nth_data (fish->cur_frames, fish->currentItem) &&
	    (dx > 0) ^ (fish->speed < 0))
	  {
	    fish_reverse_direction (fish);
	    gc_sound_play_ogg ("sounds/drip.wav", NULL);
	  }
	else
	  react += 1;
      }
      if (react >= 2)
	fish->fright +=
	  (1000 + rand() % (int)(near * 2000)) / moveSpeed;
    }

  }
  return TRUE;
}

static gboolean
item_event (GooCanvasItem  *item,
	    GooCanvasItem  *target,
	    GdkEventButton *event,
	    FishItem *fishitem)
{
   double item_x, item_y;

   if(!gcomprisBoard || !event)
     return FALSE;

   if(board_paused)
     return FALSE;

   item_x = event->x;
   item_y = event->y;
   goo_canvas_convert_to_item_space( goo_canvas_item_get_canvas(item),
				     goo_canvas_item_get_parent(item), &item_x, &item_y);

   switch(event->button)
     {
     case 1:
     case 2:
     case 3:
       {
	   if (gcomprisBoard->level >= 5 && !fishitem->stun) {
	     fishitem->stun = 500 * (1 + gcomprisBoard->maxlevel - gcomprisBoard->level) / moveSpeed;
	     fishitem->fright += 500 / moveSpeed;
	   } else {
	     fish_gobble (fishitem);
	   }
	 }
       break;

     case 4:
       /* fish up */
       goo_canvas_item_translate(item, 0.0, -3.0);
       break;

     case 5:
       /* fish down */
       goo_canvas_item_translate(item, 0.0, 3.0);
       break;

     default:
       break;
     }

   return TRUE;
}

/*
 */
static void clickgame_start (GcomprisBoard *agcomprisBoard)
{
  if (!agcomprisBoard)
    return;

  gcomprisBoard = agcomprisBoard;

  /* set initial values for this level */
  gcomprisBoard->level = 1;
  gcomprisBoard->maxlevel=6;
  gcomprisBoard->number_of_sublevel=10;
  gc_score_start(SCORESTYLE_NOTE,
		 BOARDWIDTH - 195,
		 BOARDHEIGHT - 25,
		 gcomprisBoard->number_of_sublevel);
  gc_bar_set(GC_BAR_LEVEL);

  g_signal_connect(goo_canvas_get_root_item(gcomprisBoard->canvas),
		   "enter_notify_event",
		   (GCallback) canvas_event, NULL);
  clickgame_next_level();

  clickgame_pause(FALSE);
}

static void
clickgame_end ()
{
  if(gcomprisBoard!=NULL)
    {
      clickgame_pause(TRUE);
      gc_score_end();
      clickgame_destroy_all_items();
      g_signal_handlers_disconnect_by_func(goo_canvas_get_root_item(gcomprisBoard->canvas),
					   (GCallback) canvas_event, NULL);
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
clickgame_is_our_board (GcomprisBoard *board)
{
  if (board)
    {
      if(g_ascii_strcasecmp(board->type, "clickgame")==0)
	{
	  /* Set the plugin entry */
	  board->plugin=&menu_bp;

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
  static /*const*/ gchar *bglist[] =
    {
      "clickgame/sea1.jpg",
      "clickgame/sea2.jpg",
      "clickgame/sea3.jpg",
      "clickgame/sea4.jpg",
      "clickgame/sea5.jpg",
      "clickgame/sea6.jpg"
    };

  int bgx = gcomprisBoard->level - 1;

  if (bgx < 0 || G_N_ELEMENTS(bglist) <= bgx)
    bgx = G_N_ELEMENTS(bglist) - 1;

  gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
		    bglist[bgx]);

  gc_bar_set_level(gcomprisBoard);

  /* Try the next level */
  moveSpeed=100+(40/(gcomprisBoard->level));
  fallSpeed=5000-gcomprisBoard->level*200;
  imageZoom =
    0.75 + 0.25
    * ((double)(gcomprisBoard->maxlevel - gcomprisBoard->level + 1)
       / gcomprisBoard->maxlevel);
  gcomprisBoard->sublevel=0;
  gc_score_set(gcomprisBoard->sublevel);

  while (g_list_length (item_list) < 3) {
    FishItem *fish = clickgame_create_item();
    if (!fish) break;
    goo_canvas_item_translate(fish->rootitem,
                          fish->speed * (rand() % 200), 0.0);
  }
}


static void clickgame_animate_item(FishItem *fishitem)
{
  gint currentItem;

  /* Manage the animation */
  currentItem = fishitem->currentItem;
  fishitem->currentItem++;
  if(fishitem->currentItem >= g_slist_length(fishitem->cur_frames))
    fishitem->currentItem=0;

  g_object_set((GooCanvasItem *)g_slist_nth_data(fishitem->cur_frames,
						 fishitem->currentItem),
	       "visibility", GOO_CANVAS_ITEM_VISIBLE,
		NULL);

  g_object_set((GooCanvasItem *)g_slist_nth_data(fishitem->cur_frames,
						 currentItem),
	       "visibility", GOO_CANVAS_ITEM_INVISIBLE,
	       NULL);
}

static void
fish_escape (FishItem *fishitem)
{
  item2del_list = g_list_append (item2del_list, fishitem);
  gc_sound_play_ogg ("sounds/youcannot.wav", NULL);

  if (gcomprisBoard->sublevel) {
    --gcomprisBoard->sublevel;
    gc_score_set(gcomprisBoard->sublevel);
  }
}

static void clickgame_move_item(FishItem *fishitem)
{
  double sp = fishitem->speed;

  GooCanvasItem *item = fishitem->rootitem;

  if (fishitem->stun) {
    --fishitem->stun;
    sp *= .1 + (rand() % 100) / 1000.0;
  } else if (fishitem->fright) {
    --fishitem->fright;
    sp *= 3 + (rand() % 3000) / 1000.0;
  }
  goo_canvas_item_translate(item, sp, 0.0);

  GooCanvasBounds bounds;
  goo_canvas_item_get_bounds (item, &bounds);

  if(fishitem->speed>0)
    {
      if(bounds.x1>BOARDWIDTH)
	fish_escape (fishitem);
    }
  else
    {
      if(bounds.x2<0)
	fish_escape (fishitem);
    }

  while (g_list_length (item_list) < 3) {
    FishItem *fish = clickgame_create_item();
    if (!fish) break;
    goo_canvas_item_translate(fish->rootitem,
			      fish->speed * (rand() % 200), 0.0);
  }
}

static void clickgame_destroy_item(FishItem *fishitem)
{
  GooCanvasItem *item = fishitem->rootitem;

  item_list = g_list_remove (item_list, fishitem);
  item2del_list = g_list_remove (item2del_list, fishitem);
  goo_canvas_item_remove(item);

  g_slist_free (fishitem->fwd_frames);
  g_slist_free (fishitem->rev_frames);
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
  while(item_list)
    {
      FishItem *fishitem = g_list_nth_data(item_list, 0);
      clickgame_destroy_item(fishitem);
    }
}

/*
 * This does the moves of the game items on the play canvas
 *
 */
static gboolean clickgame_move_items (gpointer data)
{
  g_list_foreach (item_list, (GFunc) clickgame_move_item, NULL);

  /* Destroy items that falls out of the canvas */
  clickgame_destroy_items();

  move_items_id = g_timeout_add (moveSpeed,
				 clickgame_move_items, NULL);

  return(FALSE);
}

/*
 * This does the icon animation
 *
 */
static gboolean clickgame_animate_items (gpointer data)
{
  g_list_foreach (item_list, (GFunc) clickgame_animate_item, NULL);

  animate_id = g_timeout_add (1000,
			      clickgame_animate_items, NULL);

  return(FALSE);
}

static GSList *load_random_fish(gboolean smallish)
{
  int fish;
  const gchar **extraFish;
  const gchar **surpriseFish;
  int num_extra;
  int num_surprise;

  if (smallish) {
    extraFish = smallFish;
    num_extra = NUM_SMALLFISH;
    surpriseFish = bigFish;
    num_surprise = NUM_BIGFISH;
  } else {
    extraFish = bigFish;
    num_extra = NUM_BIGFISH;
    surpriseFish = smallFish;
    num_surprise = NUM_SMALLFISH;
  }

  fish = rand() % (NUM_MEDFISH + num_extra + 2);

  const gchar *path;
  if (fish < NUM_MEDFISH) {
    path = medFish[fish];
  } else if (fish < NUM_MEDFISH + num_extra) {
    path = extraFish[fish - NUM_MEDFISH];
  } else {
    fish = rand() % num_surprise;
    path = surpriseFish[fish];
  }

  int frame = 0;
  GSList *ilist = 0;
  while(1)
    {
      if (frame) {
	gchar *exists = gc_file_find_absolute (path, "clickgame", frame);
	g_free (exists);
	if (!exists) break;
      }

      GdkPixbuf *pixmap = gc_pixmap_load (path, "clickgame", frame);
      if (!pixmap) break;

      ilist = g_slist_prepend (ilist, pixmap);
      ++frame;
    }
  return g_slist_reverse (ilist);
}

static FishItem *
clickgame_create_item()
{
  GooCanvasItem *parent = \
    goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
			  NULL);

  GdkPixbuf *pixmap = NULL;
  GdkPixbuf *pixmap2 = NULL;
  GooCanvasItem *rootitem;
  FishItem *fishitem;
  double x, y;
  gint i, length;
  GSList *ilist;

  /* Avoid to have too much items displayed */
  if(g_list_length(item_list)>5)
    return NULL;

  // select smallish fish on even levels, also see imageZoom
  ilist = load_random_fish (!(gcomprisBoard->level & 1));

  fishitem = g_malloc(sizeof(FishItem));
  fishitem->currentItem   = 0;
  fishitem->speed = (double)(g_random_int()%(60))/10 - 3;
  fishitem->fright = 0;
  fishitem->stun = 0;
  fishitem->fwd_frames = NULL;
  fishitem->rev_frames = NULL;

  pixmap = (GdkPixbuf *)g_slist_nth_data(ilist, 0);

  if(pixmap==NULL)
    return NULL;

  if(fishitem->speed<0)
    {
      x = (double) BOARDWIDTH;
      fishitem->speed=MIN(fishitem->speed, -1);
    }
  else
    {
      x = (double) -gdk_pixbuf_get_width(pixmap)*imageZoom;
      fishitem->speed = MAX(fishitem->speed, 1);
    }

  rootitem = \
    goo_canvas_group_new (parent,
			   NULL);

  g_signal_connect(rootitem, "button_press_event",
		   (GCallback) item_event, fishitem);

  fishitem->rootitem = rootitem;

  length = g_slist_length(ilist);

  y = (g_random_int()%(BOARDHEIGHT - BARHEIGHT -
		       (guint)(gdk_pixbuf_get_height(pixmap)*
			       imageZoom)));

  for(i=0; i<length; i++)
    {
      GooCanvasItem *fwd, *rev;

      pixmap = (GdkPixbuf *)g_slist_nth_data(ilist, i);
      pixmap2 = pixbuf_copy_mirror(pixmap, TRUE, FALSE);

      fwd = goo_canvas_image_new (rootitem,
				  pixmap,
				  x,
				  y,
				  NULL);
      goo_canvas_item_scale(fwd, imageZoom, imageZoom);

      rev = goo_canvas_image_new (rootitem,
				  pixmap2,
				  x,
				  y,
				  NULL);
      goo_canvas_item_scale(rev, imageZoom, imageZoom);
#if GDK_PIXBUF_MAJOR <= 2 && GDK_PIXBUF_MINOR <= 24
      gdk_pixbuf_unref(pixmap);
      gdk_pixbuf_unref(pixmap2);
#else
      g_object_unref(pixmap);
      g_object_unref(pixmap2);
#endif

      fishitem->fwd_frames = g_slist_prepend (fishitem->fwd_frames, fwd);
      fishitem->rev_frames = g_slist_prepend (fishitem->rev_frames, rev);

      g_object_set (fwd,
		    "visibility", GOO_CANVAS_ITEM_INVISIBLE,
		    NULL);
      g_object_set (rev,
		    "visibility", GOO_CANVAS_ITEM_INVISIBLE,
		    NULL);
    }

  g_slist_free (ilist);

  fishitem->fwd_frames = g_slist_reverse (fishitem->fwd_frames);
  fishitem->rev_frames = g_slist_reverse (fishitem->rev_frames);

  fishitem->cur_frames =
    fishitem->speed<0? fishitem->rev_frames : fishitem->fwd_frames;

  g_object_set(g_slist_nth_data(fishitem->cur_frames,
				fishitem->currentItem),
	       "visibility", GOO_CANVAS_ITEM_VISIBLE,
	       NULL);

  item_list = g_list_append (item_list, fishitem);

  return (fishitem);
}

/*
 * This is called on a low frequency and is used to drop new items
 *
 */
static gboolean clickgame_drop_items (gpointer data)
{
  clickgame_create_item();

  drop_items_id = g_timeout_add (fallSpeed,
				 clickgame_drop_items, NULL);
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
    if(gcomprisBoard->level>gcomprisBoard->maxlevel)
      gcomprisBoard->level = gcomprisBoard->maxlevel;

    gc_sound_play_ogg ("sounds/bonus.wav", NULL);
  }
  clickgame_next_level();
}
