/* gcompris - paratrooper.c
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

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE

static int gamewon;

static GcomprisBoard *gcomprisBoard = NULL;

static gint drop_tux_id = 0;

static GooCanvasItem *boardRootItem = NULL;
static gint boat_x, boat_y, boat_landarea_y, boat_width;

static gint plane_x, plane_y;
static gint planespeed_x;
static gdouble windspeed;

static GooCanvasItem *planeitem;

typedef enum
{
  TUX_INPLANE		= 1 << 0,
  TUX_DROPPING		= 1 << 1,
  TUX_FLYING		= 1 << 2,
  TUX_LANDED		= 1 << 3,
  TUX_CRASHED		= 1 << 4
} ParaStatus;


typedef struct {
  ParaStatus status;
  double speed;
  double drift;
  gboolean speed_override;
  GooCanvasItem *rootitem;
  GooCanvasItem *paratrooper;
  GooCanvasItem *instruct;
} ParatrooperItem;

static  ParatrooperItem paratrooperItem;


static void	 	 start_board (GcomprisBoard *agcomprisBoard);
static void	 	 pause_board (gboolean pause);
static void	 	 end_board (void);
static gboolean	 	 is_our_board (GcomprisBoard *gcomprisBoard);
static void	 	 set_level (guint level);
static gint	 	 key_press(guint keyval, gchar *commit_str, gchar *preedit_str);

static void		 paratrooper_create_cloud(GooCanvasItem *parent);
static gint		 paratrooper_drop_clouds (GtkWidget *widget, gpointer data);
static gint		 paratrooper_move_tux (GtkWidget *widget, gpointer data);
static void		 paratrooper_destroy_all_items(void);
static void		 paratrooper_next_level(void);
static gboolean		 item_event (GooCanvasItem  *item,
				     GooCanvasItem  *target,
				     GdkEventButton *event,
				     gpointer data);
static void		 next_state(void);


static double            speed = 0.0;
static double            imageZoom = 0.0;

/* Description of this plugin */
static BoardPlugin menu_bp =
{
   NULL,
   NULL,
   "Parachutist",
   "Direct the parachutist to help him or her land safely",
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
   NULL,
   NULL
};

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(paratrooper)

/*
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 *
 */
static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  if(pause)
    {
      if (drop_tux_id) {
	gtk_timeout_remove (drop_tux_id);
	drop_tux_id = 0;
      }
    }
  else
    {
      if(gamewon == TRUE) /* the game is won */
	{
	  gcomprisBoard->level++;
	  if(gcomprisBoard->level>gcomprisBoard->maxlevel)
	    gcomprisBoard->level = gcomprisBoard->maxlevel;
	}

      // Unpause code
      if(paratrooperItem.status!=TUX_INPLANE && paratrooperItem.status!=TUX_LANDED) {
	drop_tux_id = gtk_timeout_add (1000, (GtkFunction) paratrooper_move_tux, NULL);
      }

      if(gamewon == TRUE) /* the game is won */
	  paratrooper_next_level();
    }
}

/*
 */
static void start_board (GcomprisBoard *agcomprisBoard)
{

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;

      /* disable im_context */
      gcomprisBoard->disable_im_context = TRUE;

      gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			"paratrooper/scenery3_background.png");


      /* set initial values for this level */
      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel = 6;
      gc_bar_set(GC_BAR_LEVEL);

      /* Init of paratrooper struct */
      paratrooperItem.rootitem = NULL;

      paratrooper_next_level();

      pause_board(FALSE);

    }

}

static void
end_board ()
{

  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      paratrooper_destroy_all_items();
      gcomprisBoard->level = 1;       // Restart this game to zero
    }
  gcomprisBoard = NULL;
}

static void
set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      paratrooper_next_level();
    }
}

static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str)
{

  if(!gcomprisBoard)
    return FALSE;


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
    case GDK_KP_Enter:
    case GDK_Return:
      return TRUE;
    case GDK_Right:
      break;
    case GDK_Left:
      break;
    case GDK_Up:
      paratrooperItem.speed_override = 1;
      if(paratrooperItem.status == TUX_FLYING && paratrooperItem.speed >= 3)
	paratrooperItem.speed--;
      return TRUE;
    case GDK_Down:
      paratrooperItem.speed_override = 1;
      if(paratrooperItem.status == TUX_FLYING && paratrooperItem.speed <= 6)
	paratrooperItem.speed++;
      return TRUE;
    }

  next_state();

  return TRUE;
}

static gboolean
is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "paratrooper")==0)
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
static void paratrooper_next_level()
{
  RsvgHandle *svg_handle;
  RsvgDimensionData rsvg_dimension;
  GooCanvasItem *item;
  GooCanvasItem *planeroot;
  GooCanvasBounds bounds;

  gamewon = FALSE;

  gc_bar_set_level(gcomprisBoard);

  paratrooper_destroy_all_items();

  boardRootItem = \
    goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
			  NULL);

  /* Try the next level */
  speed=10+(30/(gcomprisBoard->level));
  gcomprisBoard->number_of_sublevel=0;
  /* Make the images tend to 0.5 ratio */
  imageZoom=0.4+(0.2 * (2 - (gcomprisBoard->level-1) % 3));

  /* Setup and Display the plane */
  svg_handle = gc_rsvg_load("paratrooper/tuxplane.svgz");
  rsvg_handle_get_dimensions (svg_handle, &rsvg_dimension);

  planeroot = \
    goo_canvas_group_new (boardRootItem,
			  NULL);
  planespeed_x = 4 + gcomprisBoard->level;

  plane_x = -rsvg_dimension.width;
  plane_y = 10;
  planeitem = goo_svg_item_new (planeroot,
				svg_handle,
				NULL);

  goo_canvas_item_set_simple_transform(planeitem,
				       plane_x,
				       plane_y,
				       imageZoom,
				       0);
  goo_canvas_item_animate(planeroot,
			  BOARDWIDTH + rsvg_dimension.width,
			  plane_y,
			  1,
			  0,
			  FALSE,
			  BOARDWIDTH * speed,
			  speed*1.5,
			  GOO_CANVAS_ANIMATE_RESTART);

  g_signal_connect(planeitem, "button-press-event",
		     (GtkSignalFunc) item_event,
		     NULL);
  gc_item_focus_init(planeitem, NULL);
  g_object_unref(svg_handle);

  windspeed = (3 + rand() % (100 * gcomprisBoard->level) / 100);
  if(rand()%2==0)
    windspeed *= -1;
  if (gcomprisBoard->level >= 4)
    windspeed *= 2;

  /* Drop a cloud */
  gtk_timeout_add (200,
		   (GtkFunction) paratrooper_drop_clouds, NULL);

  /* Display the target */
  svg_handle = gc_rsvg_load("paratrooper/fishingboat.svgz");
  rsvg_handle_get_dimensions (svg_handle, &rsvg_dimension);
  boat_x = (BOARDWIDTH - rsvg_dimension.width) / 2;
  boat_y = BOARDHEIGHT - 100;
  boat_landarea_y = boat_y + 20;
  item = \
    goo_svg_item_new (boardRootItem,
		      svg_handle,
		      NULL);
  goo_canvas_item_translate(item,
			    -rsvg_dimension.width/2,
			    boat_y - 50);

  goo_canvas_item_get_bounds(item, &bounds);
  boat_width = bounds.x2 - bounds.x1;

  goo_canvas_item_animate(item,
			  boat_x,
			  boat_y,
			  1,
			  0,
			  TRUE,
			  BOARDWIDTH/2 * 30,
			  40*2,
			  GOO_CANVAS_ANIMATE_FREEZE);

  g_object_unref(svg_handle);

  /* Prepare the parachute */
  if (drop_tux_id) {
    gtk_timeout_remove (drop_tux_id);
    drop_tux_id = 0;
  }

  paratrooperItem.status	= TUX_INPLANE;
  paratrooperItem.speed		= 3;

  paratrooperItem.rootitem = \
    goo_canvas_group_new (boardRootItem,
			  NULL);

  paratrooperItem.paratrooper = \
    goo_svg_item_new (paratrooperItem.rootitem,
		      NULL,
		      NULL);

  g_object_set (paratrooperItem.paratrooper, "visibility",
		GOO_CANVAS_ITEM_INVISIBLE, NULL);

  g_signal_connect(paratrooperItem.paratrooper, "button-press-event",
		     (GtkSignalFunc) item_event,
		     NULL);

  paratrooperItem.instruct = \
    goo_canvas_text_new (boardRootItem,
			 _("Control fall speed with up and down arrow keys."),
			 (double) BOARDWIDTH / 2.0,
			 (double) 130,
			 -1,
			 GTK_ANCHOR_CENTER,
			 "font", gc_skin_font_board_medium,
			 "fill_color_rgba", gc_skin_color_title,
			 NULL);
  g_object_set (paratrooperItem.instruct, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
}


/* Destroy all the items */
static void paratrooper_destroy_all_items()
{
  if(boardRootItem)
    goo_canvas_item_remove(boardRootItem);
  boardRootItem = NULL;
}

/*
 * This does the moves of the game's paratropper
 *
 */
static gint
paratrooper_move_tux (GtkWidget *widget, gpointer data)
{
  double offset;
  GooCanvasBounds bounds;
  guint center;

  goo_canvas_item_get_bounds(paratrooperItem.paratrooper, &bounds);

  center = bounds.x1 + (bounds.x2 - bounds.x1) / 2;

  /* Manage the wrapping */
  if(center < 0) {
    goo_canvas_item_translate(paratrooperItem.rootitem,
			      BOARDWIDTH,
			      0);
  }
  if(center > BOARDWIDTH) {
    goo_canvas_item_translate(paratrooperItem.rootitem,
			      -BOARDWIDTH,
			      0);
  }

  offset = (windspeed / 2 + 15 * paratrooperItem.drift) / 16;
  paratrooperItem.drift = offset;

  if (paratrooperItem.status == TUX_DROPPING && gcomprisBoard->level >= 4)
    paratrooperItem.speed *= 1.05;

  if (paratrooperItem.status == TUX_FLYING && paratrooperItem.speed > 3 &&
      !paratrooperItem.speed_override)
    paratrooperItem.speed /= 1.2;

  goo_canvas_item_translate(paratrooperItem.rootitem,
  			    offset, paratrooperItem.speed);

  /* Check we reached the target */
  if(bounds.y2 > boat_landarea_y)
    {
      if( center > boat_x &&
	  center < boat_x + boat_width &&
	  paratrooperItem.status == TUX_FLYING)
	{
	  paratrooperItem.status = TUX_LANDED;
	  next_state();
	}
      else
	{
	  if(bounds.y2 < BOARDHEIGHT - 20)
	    drop_tux_id = gtk_timeout_add (150,
					   (GtkFunction) paratrooper_move_tux,
					   NULL);
	  else
	    {
	      paratrooperItem.status = TUX_CRASHED;
	      next_state();
	    }
	}
    }
  else
    {
      drop_tux_id = gtk_timeout_add (150,
				     (GtkFunction) paratrooper_move_tux, NULL);
    }

  return(FALSE);
}

static void
paratrooper_create_cloud(GooCanvasItem *parent)
{
  RsvgHandle *svg_handle = NULL;
  GooCanvasItem *item;
  GooCanvasItem *root;
  int x, y;
  int x_end;
  RsvgDimensionData rsvg_dimension;

  svg_handle = gc_rsvg_load("paratrooper/cloud.svgz");
  rsvg_handle_get_dimensions (svg_handle, &rsvg_dimension);

  if(windspeed>0)
    {
      x = 0;
      x_end = BOARDWIDTH;
    }
  else
    {
      x = BOARDWIDTH - rsvg_dimension.width * imageZoom;
      x_end = 0;
    }

  y = 60;

  root = \
    goo_canvas_group_new (parent,
			  NULL);

  item = goo_svg_item_new (root,
			   svg_handle,
			   NULL);

  goo_canvas_item_scale(item,
			imageZoom, imageZoom);

  goo_canvas_item_translate(root, x, y);

  goo_canvas_item_animate(root,
			  x_end,
			  y,
			  1,
			  0,
			  TRUE,
			  BOARDWIDTH * (80 / ABS(windspeed)),
			  40,
			  GOO_CANVAS_ANIMATE_RESTART);
  g_object_unref(svg_handle);

  /* The plane is always on top */
  goo_canvas_item_raise(planeitem, NULL);
}

/*
 * This is called on a low frequency and is used to drop new items
 *
 */
static gint paratrooper_drop_clouds (GtkWidget *widget, gpointer data)
{
  paratrooper_create_cloud(boardRootItem);

  return (FALSE);
}



/*
 * This is the state machine of the paratrooper
 */
void next_state()
{
  GooCanvasBounds bounds;

  switch(paratrooperItem.status)
    {
    case TUX_INPLANE:
      {
	RsvgHandle *svg_handle;

	gc_sound_play_ogg ("sounds/tuxok.wav", NULL);

	svg_handle = gc_rsvg_load("paratrooper/minitux.svgz");
	g_object_set (paratrooperItem.paratrooper,
		      "rsvg-handle", svg_handle,
		      NULL);
	g_object_unref(svg_handle);
	gc_item_focus_init(paratrooperItem.paratrooper, NULL);

	goo_canvas_item_get_bounds(planeitem, &bounds);

	g_object_set (paratrooperItem.paratrooper, "visibility",
		      GOO_CANVAS_ITEM_VISIBLE, NULL);
	paratrooperItem.status = TUX_DROPPING;
	paratrooperItem.drift = planespeed_x;

	goo_canvas_item_translate(paratrooperItem.rootitem,
				  (bounds.x1 > 0 ? bounds.x1 : 0),
				  bounds.y2);
	drop_tux_id = \
	  gtk_timeout_add (10, (GtkFunction) paratrooper_move_tux, NULL);
      }
      break;

    case TUX_DROPPING:
      {
	RsvgHandle *svg_handle;

	gc_sound_play_ogg ("sounds/eraser2.wav", NULL);

	svg_handle = gc_rsvg_load("paratrooper/parachute.svgz");

	g_object_set (paratrooperItem.paratrooper,
		      "rsvg-handle", svg_handle,
		      NULL);

	gc_item_focus_init(paratrooperItem.paratrooper, NULL);
	g_object_unref(svg_handle);

	paratrooperItem.status = TUX_FLYING;
	paratrooperItem.speed_override = 0;
	if (gcomprisBoard->level >= 2) {
	  goo_canvas_item_raise (paratrooperItem.instruct, NULL);
	  g_object_set (paratrooperItem.instruct, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
	}
      }
      break;

    case TUX_LANDED:
      gc_sound_play_ogg ("sounds/tuxok.wav", NULL);
      g_object_set (paratrooperItem.instruct, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      gamewon = TRUE;
      gc_bonus_display(gamewon, GC_BONUS_TUX);
      break;

    case TUX_CRASHED:
      /* Restart */
      gc_sound_play_ogg ("sounds/bubble.wav", NULL);
      g_object_set (paratrooperItem.instruct, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      goo_canvas_item_set_transform(paratrooperItem.rootitem, NULL);
      paratrooperItem.status	= TUX_INPLANE;
      paratrooperItem.speed	= 3;
      g_object_set (paratrooperItem.paratrooper, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      g_object_set (planeitem, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
      break;

    default:
      break;
    }
}

static gboolean
item_event (GooCanvasItem  *item,
	    GooCanvasItem  *target,
	    GdkEventButton *event,
	    gpointer data)
{

  if(!gcomprisBoard)
    return FALSE;

   switch (event->type)
     {
     case GDK_BUTTON_PRESS:
       next_state();
       break;

     default:
       break;
     }

   return FALSE;
 }
