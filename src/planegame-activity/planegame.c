/* gcompris - planegame.c
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

static GcomprisBoard *gcomprisBoard = NULL;

static gint planemove_id = 0;
static gint drop_items_id = 0;

static GooCanvasItem *rootitem = NULL;
static GooCanvasItem *planeitem;
static GooCanvasItem *clouds_rootitem;
static gint plane_x, plane_y;
static gint planespeed_x, planespeed_y;

#define MAXSPEED 7

/* These are the index for managing the game rule */
static gint plane_target, plane_last_target;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void set_level (guint level);
static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str);

static GooCanvasItem *planegame_create_item(GooCanvasItem *parent);
static gint planegame_drop_items (GtkWidget *widget, gpointer data);
static gint planegame_move_items (GtkWidget *widget, gpointer data);
static void planegame_destroy_all_items(void);
static void planegame_next_level(void);

static  guint32              fallSpeed = 0;
static  double               speed = 0.0;
static  double               imageZoom = 0.0;

/* Description of this plugin */
static BoardPlugin menu_bp =
{
   NULL,
   NULL,
   N_("Numbers in Order"),
   N_("Move the plane to catch the clouds in the correct order"),
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

GET_BPLUGIN_INFO(planegame)

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
      if (planemove_id) {
	g_source_remove (planemove_id);
	planemove_id = 0;
      }
      if (drop_items_id) {
	g_source_remove (drop_items_id);
	drop_items_id = 0;
      }
    }
  else
    {
      if(!drop_items_id) {
	drop_items_id = g_timeout_add (1000,
				       (GtkFunction) planegame_drop_items,
				       NULL);
      }
      if(!planemove_id) {
	planemove_id = g_timeout_add (1000,
				      (GtkFunction) planegame_move_items,
				      NULL);
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

      /* disable im_context */
      gcomprisBoard->disable_im_context = TRUE;

      gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			"planegame/scenery3_background.png");


      /* set initial values for this level */
      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel = 2;
      gc_bar_set(GC_BAR_LEVEL);

      planegame_next_level();

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
      planegame_destroy_all_items();
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
      planegame_next_level();
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
      if(planespeed_x < MAXSPEED)
	planespeed_x++;
      return TRUE;
    case GDK_Left:
      if(planespeed_x > -MAXSPEED)
	planespeed_x--;
      return TRUE;
    case GDK_Up:
      if(planespeed_y > -MAXSPEED)
      planespeed_y--;
      return TRUE;
    case GDK_Down:
      if(planespeed_y < MAXSPEED)
      planespeed_y++;
      return TRUE;
    }
  return TRUE;
}

gboolean
is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "planegame")==0)
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
static void planegame_next_level()
{
  RsvgHandle *svg_handle = NULL;
  GooCanvasItem *item;

  gc_bar_set_level(gcomprisBoard);

  planegame_destroy_all_items();

  rootitem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
				   NULL);

  clouds_rootitem = goo_canvas_group_new (rootitem, NULL);

  /* Try the next level */
  speed=100+(40/(gcomprisBoard->level));
  fallSpeed=10000-gcomprisBoard->level*200;
  /* Make the images tend to 0.5 ratio */
  imageZoom=0.3+(0.5/(gcomprisBoard->level));

  /* Setup and Display the plane */
  planespeed_y = 0;
  planespeed_x = 0;
  svg_handle = gc_rsvg_load("planegame/tuxhelico.svgz");
  plane_x = 50;
  plane_y = 300;

  planeitem = goo_canvas_group_new (rootitem,
				   NULL);

  goo_canvas_item_translate(planeitem, plane_x, plane_y);

  item = goo_svg_item_new (planeitem,
			   svg_handle,
			   NULL);

  goo_canvas_item_scale(item,
  			0.4 * imageZoom,
  			0.4 * imageZoom);

  g_object_unref(svg_handle);

  /* Game rules */
  plane_target = 1;
  plane_last_target = 10;

  gcomprisBoard->number_of_sublevel=plane_last_target;

  gcomprisBoard->sublevel=plane_target;

  if(gcomprisBoard->level>1)
    {
      /* No scoring after level 1 */
      gc_score_end();
    }
  else
    {
      gc_score_start(SCORESTYLE_NOTE,
			   BOARDWIDTH - 220,
			   BOARDHEIGHT - 50,
			   gcomprisBoard->number_of_sublevel);
      gc_score_set(gcomprisBoard->sublevel);
    }

}

#define ISIN(x1, y1, px1, py1, px2, py2) \
  (x1>px1 && x1<px2 && y1>py1 && y1<py2 ? TRUE : FALSE)

static void planegame_cloud_colision(GooCanvasItem *item)
{
  GooCanvasBounds ib, pb;

  goo_canvas_item_get_bounds(planeitem,  &pb);
  goo_canvas_item_get_bounds(item, &ib);

  if(ib.x2<0)
    {
      goo_canvas_item_remove(item);
      return;
    }

  if(
     ISIN(ib.x1, ib.y1, pb.x1, pb.y1, pb.x2, pb.y2) ||
     ISIN(ib.x2, ib.y1, pb.x1, pb.y1, pb.x2, pb.y2) ||
     ISIN(ib.x1, ib.y2, pb.x1, pb.y1, pb.x2, pb.y2) ||
     ISIN(ib.x2, ib.y2, pb.x1, pb.y1, pb.x2, pb.y2)
     )
    {
      gint number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (item),
							"cloud_number"));

      if(plane_target == number)
	{
	  gc_sound_play_ogg ("sounds/gobble.wav", NULL);
	  plane_target++;

	  goo_canvas_item_remove(item);

	  if(gcomprisBoard->level==1)
	    gc_score_set(plane_target);

	  if(plane_target == plane_last_target)
	    {
	      /* Try the next level */
	      gcomprisBoard->level++;
	      if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
		gc_bonus_end_display(GC_BOARD_FINISHED_RANDOM);
		return;
	      }
	      planegame_next_level();
	      gc_sound_play_ogg ("sounds/bonus.wav", NULL);
	    }
	}
    }
}

/* Move the plane */
static void planegame_move_plane(GooCanvasItem *item)
{
  GooCanvasBounds bounds;

  goo_canvas_item_get_bounds(item, &bounds);

  if(plane_x > BOARDWIDTH - (bounds.x2 - bounds.x1)
     && planespeed_x > 0)
    planespeed_x=0;

  if(plane_x < 0 && planespeed_x < 0)
    planespeed_x = 0;

  if(plane_y > BOARDHEIGHT - (bounds.y2 - bounds.y1)
     && planespeed_y > 0)
    planespeed_y = 0;

  if(plane_y < 0 && planespeed_y < 0)
    planespeed_y=0;

  goo_canvas_item_translate(item,
			    planespeed_x,
			    planespeed_y);
  plane_x += planespeed_x;
  plane_y += planespeed_y;
}

/* Destroy all the items */
static void planegame_destroy_all_items()
{
  if(rootitem)
    goo_canvas_item_remove(rootitem);

  rootitem = NULL;
}

/*
 * This does the moves of the game items on the play canvas
 *
 */
static gint planegame_move_items (GtkWidget *widget, gpointer data)
{
  int i;

  /* Check collision with each cloud */
  for(i = 0;
      i < goo_canvas_item_get_n_children(clouds_rootitem);
      i++)
    planegame_cloud_colision(goo_canvas_item_get_child(clouds_rootitem,
							 i));

  /* move the plane */
  planegame_move_plane(planeitem);
  planemove_id = g_timeout_add (speed,
				(GtkFunction) planegame_move_items, NULL);

  return(FALSE);
}

static GooCanvasItem *planegame_create_item(GooCanvasItem *parent)
{
  RsvgHandle *svg_handle;
  RsvgDimensionData dimension;
  GooCanvasItem *itemgroup;
  GooCanvasItem *item;
  char *number = NULL;
  int i, min;
  guint y;

  /* Random cloud number */
  if(g_random_int()%2==0)
    {
      /* Put the target */
      i = plane_target;
    }
  else
    {
      min = MAX(1, plane_target - 1);
      i   = min + g_random_int()%(plane_target - min + 3);
    }
  number = g_strdup_printf("%d", i);

  itemgroup = goo_canvas_group_new (parent, NULL);

  g_object_set_data (G_OBJECT (itemgroup),
		     "cloud_number", GINT_TO_POINTER (i));

  svg_handle = gc_rsvg_load("planegame/cloud.svgz");
  rsvg_handle_get_dimensions(svg_handle, &dimension);

  y = (g_random_int()%(BOARDHEIGHT -
		       (guint)(dimension.height * imageZoom)));

  goo_canvas_item_translate(itemgroup,
			    BOARDWIDTH,
			    y);

  item = goo_svg_item_new (itemgroup,
			   svg_handle,
			   NULL);
  goo_canvas_item_scale(item, imageZoom, imageZoom);

  g_object_unref(svg_handle);

  g_object_set_data (G_OBJECT (item),
		     "cloud_number", GINT_TO_POINTER (i));

  item = goo_canvas_text_new (itemgroup,
			      number,
			      dimension.width*imageZoom/2,
			      dimension.height*imageZoom/2,
			      -1,
			      GTK_ANCHOR_CENTER,
			      "font", gc_skin_font_board_big,
			      "fill-color", "red",
			      NULL);

  g_object_set_data (G_OBJECT (item),
		     "cloud_number", GINT_TO_POINTER (i));

  goo_canvas_item_animate(itemgroup,
			  -dimension.width*imageZoom,
			  y,
			  1.0,
			  0,
			  TRUE,
			  40*BOARDWIDTH,
			  40,
			  GOO_CANVAS_ANIMATE_FREEZE);

  /* The plane is always on top */
  goo_canvas_item_raise(itemgroup, NULL);
  goo_canvas_item_raise(planeitem, NULL);

  g_free (number);

  return (itemgroup);
}

static void planegame_add_new_item()
{
  planegame_create_item(clouds_rootitem);
}

/*
 * This is called on a low frequency and is used to drop new items
 *
 */
static gint planegame_drop_items (GtkWidget *widget, gpointer data)
{
  planegame_add_new_item();

  drop_items_id = g_timeout_add (fallSpeed,
				 (GtkFunction) planegame_drop_items, NULL);
  return (FALSE);
}

