/* gcompris - planegame.c
 *
 * Time-stamp: <2002/01/13 17:57:09 bruno>
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

static GnomeCanvasItem *planeitem = NULL;
static gint plane_x, plane_y;
static gint planespeed_x, planespeed_y;

/* These are the index for managing the game rule */
static gint plane_target, plane_last_target;

typedef struct {
  gint number;
  GnomeCanvasItem *rootitem;
} CloudItem;


/* default gnome pixmap directory in which this game tales the icon */
static char *pixmapsdir = "gcompris/misc/";

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void set_level (guint level);
gint key_press(guint keyval);

static GnomeCanvasItem *planegame_create_item(GnomeCanvasGroup *parent);
static gint planegame_drop_items (GtkWidget *widget, gpointer data);
static gint planegame_move_items (GtkWidget *widget, gpointer data);
static void planegame_destroy_item(CloudItem *clouditem);
static void planegame_destroy_items(void);
static void planegame_destroy_all_items(void);
static void setup_item(GnomeCanvasItem *item);
static void planegame_next_level(void);

static  guint32              fallSpeed = 0;
static  double               speed = 0.0;
static  double               imageZoom = 0.0;

/* Description of this plugin */
BoardPlugin menu_bp =
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
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
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
					 (GtkFunction) planegame_drop_items, NULL);
      }
      if(!dummy_id) {
	dummy_id = gtk_timeout_add (1000, (GtkFunction) planegame_move_items, NULL);
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


      /* set initial values for this level */
      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel = 6;
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL);

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
      gcompris_score_end();
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

gint key_press(guint keyval)
{

   if(!get_board_playing())
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
    case GDK_Right:
      if(planespeed_x < 3)
	planespeed_x++;
      return TRUE;
    case GDK_Left:
      if(planespeed_x > -3)
	planespeed_x--;
      return TRUE;
    case GDK_Up:
      if(planespeed_y > -3)
      planespeed_y--;
      return TRUE;
    case GDK_Down:
      if(planespeed_y < 3)
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
  GdkPixbuf *pixmap = NULL;
  char *str = NULL;

  gcompris_bar_set_level(gcomprisBoard);

  planegame_destroy_all_items();

  /* Try the next level */
  speed=100+(40/(gcomprisBoard->level));
  fallSpeed=7000-gcomprisBoard->level*200;
  /* Make the images tend to 0.5 ratio */
  imageZoom=0.5+(0.5/(gcomprisBoard->level));

  /* Setup and Display the plane */
  planespeed_y = 0;
  planespeed_x = 0;
  str = g_strdup_printf("%s%s", pixmapsdir, "tuxhelico.png");
  pixmap = gcompris_load_pixmap(str);
  plane_x = 50;
  plane_y = 300;
  planeitem = gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
				     gnome_canvas_pixbuf_get_type (),
				     "pixbuf", pixmap, 
				     "x", (double) plane_x,
				     "y", (double) plane_y,
				     "width", (double) gdk_pixbuf_get_width(pixmap)*imageZoom,
				     "height", (double) gdk_pixbuf_get_height(pixmap)*imageZoom,
				     "width_set", TRUE, 
				     "height_set", TRUE,
				     NULL);
  gdk_pixbuf_unref(pixmap);

  /* Game rules */
  plane_target = 1;
  plane_last_target = gcomprisBoard->level*10;

  gcomprisBoard->number_of_sublevel=plane_last_target;
  gcompris_score_start(SCORESTYLE_NOTE, 
		       gcomprisBoard->width - 220, 
		       gcomprisBoard->height - 50, 
		       gcomprisBoard->number_of_sublevel);
  gcomprisBoard->sublevel=plane_target;
  gcompris_score_set(gcomprisBoard->sublevel);

  g_free (str);

}


static void planegame_cloud_colision(CloudItem *clouditem)
{
  double px1, px2, py1, py2;
  double x1, x2, y1, y2;
  GnomeCanvasItem *item;
  static CloudItem *inCloudItem = NULL;


  if(clouditem==NULL)
    return;

  item = clouditem->rootitem;

  gnome_canvas_item_get_bounds(planeitem,  &px1, &py1, &px2, &py2); 
  gnome_canvas_item_get_bounds(item,  &x1, &y1, &x2, &y2); 

  if(((px1>x1 && px1<x2) ||
      (px2>x1 && px2<x2)) &&
      (((py1>y1 && py1<y2) ||
	(py2>y1 && py2<y2))))
    {
      if(plane_target == clouditem->number)
	{
	  gcompris_play_sound (SOUNDLISTFILE, "gobble");
	  item2del_list = g_list_append (item2del_list, clouditem);
	  plane_target++;
	  gcompris_score_set(plane_target);
	  
	  if(plane_target==plane_last_target)
	    {
	      /* Try the next level */
	      gcomprisBoard->level++;
	      if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
		board_finished(BOARD_FINISHED_RANDOM);
		return;
	      }
	      planegame_next_level();
	      gcompris_play_sound (SOUNDLISTFILE, "bonus");
	    }
	}
      else
	{
	  /* Oups, you get the wrong cloud */
	  if(inCloudItem!=clouditem)
	    gcompris_play_sound (SOUNDLISTFILE, "crash");
	}
      inCloudItem=clouditem;
    }
  else
    {
      /* OK, now we get out of the wrong cloud */
      if(inCloudItem==clouditem)
	inCloudItem=NULL;
    }
}

/* Move the plane */
static void planegame_move_plane(GnomeCanvasItem *item)
{
  if(plane_x>gcomprisBoard->width-150 && planespeed_x>0)
    planespeed_x=0;

  if(plane_x<0 && planespeed_x<0)
    planespeed_x=0;

  if(plane_y>gcomprisBoard->height-50 && planespeed_y>0)
    planespeed_y=0;

  if(plane_y<10 && planespeed_y<0)
    planespeed_y=0;

  gnome_canvas_item_move(item, (double)planespeed_x, (double)planespeed_y);
  plane_x+=planespeed_x;
  plane_y+=planespeed_y;
}

static void planegame_move_item(CloudItem *clouditem)
{
  double x1, y1, x2, y2;
  GnomeCanvasItem *item = clouditem->rootitem;

  gnome_canvas_item_move(item, -2.0, 0.0);

  gnome_canvas_item_get_bounds    (item,
				   &x1,
				   &y1,
				   &x2,
				   &y2);
  
  if(x2<0) {
    item2del_list = g_list_append (item2del_list, clouditem);
  }

}

static void planegame_destroy_item(CloudItem *clouditem)
{
  GnomeCanvasItem *item = clouditem->rootitem;

  item_list = g_list_remove (item_list, clouditem);
  item2del_list = g_list_remove (item2del_list, clouditem);
  gtk_object_destroy (GTK_OBJECT(item));

  g_free(clouditem);
}

/* Destroy items that falls out of the canvas */
static void planegame_destroy_items()
{
  CloudItem *clouditem;

  while(g_list_length(item2del_list)>0) 
    {
      clouditem = g_list_nth_data(item2del_list, 0);
      planegame_destroy_item(clouditem);
    }
}

/* Destroy all the items */
static void planegame_destroy_all_items()
{
  CloudItem *clouditem;

  while(g_list_length(item_list)>0) 
    {
      clouditem = g_list_nth_data(item_list, 0);
      planegame_destroy_item(clouditem);
    }

  if(planeitem)
    {
      gtk_object_destroy (GTK_OBJECT(planeitem));
      planeitem = NULL;
    }
}

/*
 * This does the moves of the game items on the play canvas
 *
 */
static gint planegame_move_items (GtkWidget *widget, gpointer data)
{
  g_list_foreach (item_list, (GFunc) planegame_move_item, NULL);
  g_list_foreach (item_list, (GFunc) planegame_cloud_colision, NULL);

  /* Destroy items that falls out of the canvas */
  planegame_destroy_items();

  /* move the plane */
  planegame_move_plane(planeitem);
  dummy_id = gtk_timeout_add (speed,
			      (GtkFunction) planegame_move_items, NULL);

  return(FALSE);
}

static GnomeCanvasItem *planegame_create_item(GnomeCanvasGroup *parent)
{
  GdkFont *gdk_font;
  GdkPixbuf *pixmap = NULL;
  GnomeCanvasItem *itemgroup;
  char *str = NULL;
  char *number = NULL;
  int i, min;
  CloudItem *clouditem;

  gdk_font = gdk_font_load ("-adobe-times-medium-r-normal--*-240-*-*-*-*-*-*");

  /* Random cloud number */
  if(rand()%2==0)
    {
      /* Put the target */
      i = plane_target;
    }
  else
    {
      min = MAX(1, plane_target - 1);
      i   = min + rand()%(plane_target - min + 3);
    }
  number = g_strdup_printf("%d", i);

  str = g_strdup_printf("%s%s", pixmapsdir, "cloud.png");
  pixmap = gcompris_load_pixmap(str);

  itemgroup = \
    gnome_canvas_item_new (parent,
			   gnome_canvas_group_get_type (),
			   "x", (double) gcomprisBoard->width,
			   "y", (double)(rand()%(gcomprisBoard->height-
						 (guint)(gdk_pixbuf_get_height(pixmap)*
							 imageZoom))),
			   NULL);


  gnome_canvas_item_new (GNOME_CANVAS_GROUP(itemgroup),
			 gnome_canvas_pixbuf_get_type (),
			 "pixbuf", pixmap, 
			 "x", (double) -gdk_pixbuf_get_width(pixmap)*imageZoom/2,
			 "y", (double) -gdk_pixbuf_get_height(pixmap)*imageZoom/2,
			 "width", (double) gdk_pixbuf_get_width(pixmap)*imageZoom,
			 "height", (double) gdk_pixbuf_get_height(pixmap)*imageZoom,
			 "width_set", TRUE, 
			 "height_set", TRUE,
			 NULL);
  gdk_pixbuf_unref(pixmap);


  gnome_canvas_item_new (GNOME_CANVAS_GROUP(itemgroup),
			 gnome_canvas_text_get_type (),
			 "text", number,
			 "font_gdk", gdk_font,
			 "x", (double) 0,
			 "y", (double) 0,
			 "fill_color", "red",
			 NULL);

  /* The plane is always on top */
  gnome_canvas_item_raise_to_top(planeitem);

  clouditem = g_malloc(sizeof(CloudItem));
  clouditem->rootitem = itemgroup;
  clouditem->number   = i;

  item_list = g_list_append (item_list, clouditem);

  g_free (str);
  g_free (number);

  return (itemgroup);
}

static void planegame_add_new_item() 
{
  setup_item (planegame_create_item(gnome_canvas_root(gcomprisBoard->canvas)));
}

/*
 * This is called on a low frequency and is used to drop new items
 *
 */
static gint planegame_drop_items (GtkWidget *widget, gpointer data)
{
  planegame_add_new_item();

  //  drop_items_id = gtk_timeout_add (fallSpeed,
  //				   (GtkFunction) planegame_drop_items, NULL);
  return (FALSE);
}

static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
   static double x, y;
   double new_x, new_y;
   GdkCursor *fleur;
   static int dragging;
   double item_x, item_y;

   if(!get_board_playing())
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
               gnome_canvas_item_grab(item,
                                      GDK_POINTER_MOTION_MASK | 
                                      GDK_BUTTON_RELEASE_MASK,
                                      fleur,
                                      event->button.time);
               gdk_cursor_destroy(fleur);
               dragging = TRUE;
             }
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
	   gnome_canvas_item_ungrab(item, event->button.time);
	   dragging = FALSE;
	 }
       break;

     default:
       break;
     }
         
   return FALSE;
 }

static void
setup_item(GnomeCanvasItem *item)
{
  gtk_signal_connect(GTK_OBJECT(item), "event",
		     (GtkSignalFunc) item_event,
		     NULL);
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
