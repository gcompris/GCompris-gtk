/* gcompris - menu2.c
 *
 * Copyright (C) 2000, 2008 Bruno Coudoin, Yves Combe
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

/**
 * The menu display icons of each boards and let the user select one
 *
 * Seen from gcompris, the menu is a board like another.
 */
#include <string.h>

#include "gcompris/gcompris.h"
#include "gcompris/gcompris_config.h"

#define SOUNDLISTFILE PACKAGE
#define MENU_PER_LINE 5

typedef struct {
  GooCanvasItem *bg; /* Background */
  GooCanvasItem *boardname_item;
  GooCanvasItem *description_item;
  GooCanvasItem *author_item;
} MenuItems;

static MenuItems *menuitems;

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static gchar *menu_position = NULL;

/* We don't wan't the callback on boards to be accepted until the menu is fully displayed */
static gboolean menu_displayed = FALSE;

static void		 menu_start (GcomprisBoard *agcomprisBoard);
static void		 menu_pause (gboolean pause);
static void		 menu_end (void);
static gboolean		 menu_is_our_board (GcomprisBoard *gcomprisBoard);
static void		 menu_config(void);

static void              menu_config_start(GcomprisBoard *agcomprisBoard,
					   GcomprisProfile *aProfile);

static void              menu_config_stop(void);

static void		 menu_create_item(GooCanvasItem *parent, MenuItems *menuitems, GcomprisBoard *board);
static gboolean		 next_spot();
static void		 create_info_area(GooCanvasItem *parent, MenuItems *menuitems);
static gboolean		 on_enter_notify (GooCanvasItem *item,
					  GooCanvasItem *target,
					  GdkEventCrossing *event,
					  MenuItems *menuitems);
static gboolean		 on_leave_notify (GooCanvasItem *item,
					  GooCanvasItem *target,
					  GdkEventCrossing *event,
					  MenuItems *menuitems);
static gint		 item_event(GooCanvasItem *item, GdkEvent *event, MenuItems *menuitems);
static void		 display_board_icon(GcomprisBoard *board, MenuItems *menuitems);

static void              display_section (gchar *path);
static void              display_welcome (MenuItems *menuitems);
static void		 create_panel(GooCanvasItem *parent);
static void		 create_top(GooCanvasItem *parent, gchar *path);
static GooCanvasItem	*menu_difficulty_display(GooCanvasItem *parent,
						 double x, double y,
						 double ratio,
						 gint difficulty);
static GooCanvasItem	*menu_demo_display(GooCanvasItem *parent,
					   gdouble x, gdouble y);

static double current_x = 0.0;
static double current_y = 0.0;

static double current_top_x = 0.0;
static double current_top_y = 0.0;

static double panel_x, panel_y, panel_w, panel_h;
static double top_x, top_y, top_w, top_h, top_int_x;
static double display_x, display_y, display_w, display_h, display_int_x, display_int_y;
static double info_x, info_y, info_w, info_h;

static double icon_size, icon_size_panel, icon_size_top, top_arrow_size;

static gdouble get_ratio(GdkPixbuf *pixmap, gdouble size);

#define P_X 8
#define P_Y 35
#define P_W 80
#define P_H 480

#define T_X 175
#define T_Y 10
#define T_W 550
#define T_H 55
#define T_INT_X 5
#define T_ARROW_SIZE 20

#define D_X 175
#define D_Y 70
#define D_W 550
#define D_H 320
#define D_INT_X 50
#define D_INT_Y 5

#define I_X 140
#define I_Y 390
#define I_W 600
#define I_H 125

#define ICON_SIZE 100
#define ICON_SIZE_PANEL 50
#define ICON_SIZE_TOP 50

static GooCanvasItem *boardRootItem = NULL;
static GooCanvasItem *actualSectionItem = NULL;
static GList *panelBoards = NULL;

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Main Menu Second Version"),
    N_("Select a Board"),
    "Bruno Coudoin <bruno.coudoin@free.fr>",
    NULL,
    NULL,
    NULL,
    NULL,
    menu_start,
    menu_pause,
    menu_end,
    menu_is_our_board,
    NULL,
    NULL,
    NULL,
    menu_config,
    NULL,
    menu_config_start,
    menu_config_stop
  };

/*
 * Main entry point mandatory for each Gcompris's board
 * ---------------------------------------------------
 *
 */


GET_BPLUGIN_INFO(menu)

/*
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 *
 */
static void menu_pause (gboolean pause)
{

  if(gcomprisBoard==NULL)
    return;

  board_paused = pause;
}

/*
 */
static void menu_start (GcomprisBoard *agcomprisBoard)
{

  current_x = 0.0;
  current_y = 0.0;

  /* initialisations */
  /* in case we will make this parametrable */

  panel_x = P_X;
  panel_y = P_Y;
  panel_w = P_W;
  panel_h = P_H;

  top_x = T_X;
  top_y = T_Y;
  top_w = T_W;
  top_h = T_H;
  top_int_x = T_INT_X;
  top_arrow_size = T_ARROW_SIZE;

  display_x = D_X;
  display_y = D_Y;
  display_w = D_W;
  display_h = D_H;
  display_int_x = D_INT_X;
  display_int_y = D_INT_Y;

  info_x = I_X;
  info_y = I_Y;
  info_w = I_W;
  info_h = I_H;

  icon_size = ICON_SIZE;
  icon_size_panel = ICON_SIZE_PANEL;
  icon_size_top = ICON_SIZE_TOP;

  if(agcomprisBoard != NULL)
    {
      RsvgHandle *svg_handle;

      gcomprisBoard=agcomprisBoard;

      /* set initial values for this level */
      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel=1;

      /* Set back the bar to it's original location */
      gc_bar_set(GC_BAR_CONFIG|GC_BAR_ABOUT);

      menuitems = g_new(MenuItems, 1);

      svg_handle = gc_skin_rsvg_get();

      gc_set_background_by_id (goo_canvas_get_root_item(gcomprisBoard->canvas),
			       svg_handle,
			       "#BACKGROUND");

      boardRootItem = \
	goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
			      NULL);

      g_object_set_data_full(G_OBJECT (boardRootItem),
			     "menuitems", menuitems, g_free);

      goo_canvas_svg_new (boardRootItem,
			  svg_handle,
			  "svg-id", "#SELECTOR",
			  "pointer-events", GOO_CANVAS_EVENTS_NONE,
			  NULL);

      goo_canvas_svg_new (boardRootItem,
			  svg_handle,
			  "svg-id", "#BUTTON_HORIZONTAL",
			  "pointer-events", GOO_CANVAS_EVENTS_NONE,
			  NULL);

      create_info_area(boardRootItem, menuitems);

      create_panel(boardRootItem);

      if (menu_position)
	display_section(menu_position);
      else
	display_welcome(menuitems);

      {
	gchar *text = g_strdup_printf(_("Number of activities: %d"),
				      gc_board_get_number_of_activity());
	goo_canvas_text_new (boardRootItem,
			     text,
			     BOARDWIDTH - 10,
			     BOARDHEIGHT - 10,
			   -1,
			   GTK_ANCHOR_EAST,
			   "font", gc_skin_font_board_tiny,
			   "fill-color-rgba", gc_skin_get_color("menu/text"),
			   "alignment", PANGO_ALIGN_RIGHT,
			   NULL);
	g_free(text);
      }

      menu_pause(FALSE);

    }

}


static void
create_panel(GooCanvasItem *parent)
{
  int n_sections;
  GList *list = NULL;
  GcomprisBoard *board;
  GdkPixbuf *pixmap = NULL;
  GooCanvasItem *item;

  gdouble x, y;
  gint int_y;
  GcomprisProperties *properties = gc_prop_get();
  g_assert(properties);

  /* In normal mode, we show all the sections in panel */
  /* in direct submenu access, we show the icon of the submenu */
  if (strcmp(properties->root_menu,"/")==0)
    panelBoards = gc_menu_getlist(properties->root_menu);
  else
    panelBoards = g_list_append(list, gcomprisBoard);

  n_sections = g_list_length(panelBoards);

  if (n_sections == 0)
    return;

  if ( (panel_h/n_sections) <= icon_size_panel)
    {
      icon_size_panel = panel_h / n_sections;
      int_y = 0;
    }
  else
    {
      int_y = (panel_h - n_sections*icon_size_panel)/n_sections;
    }

  x = panel_x + panel_w * 0.25;
  y = panel_y + int_y / 2.0 - 20;

  for (list = panelBoards; list != NULL; list = list->next)
    {
      board = (GcomprisBoard *) list->data;

      pixmap = gc_pixmap_load(board->icon_name);

      item = goo_canvas_image_new (parent,
				   pixmap,
				   x,
				   y,
				   NULL);

#if GDK_PIXBUF_MAJOR <= 2 && GDK_PIXBUF_MINOR <= 24
      gdk_pixbuf_unref(pixmap);
#else
      g_object_unref(pixmap);
#endif

      y += int_y + icon_size_panel;

      g_object_set_data (G_OBJECT (item), "board", board);

      g_signal_connect(item, "button_press_event",
		       (GCallback) item_event,
		       menuitems);
      g_signal_connect (item, "enter_notify_event",
			(GCallback) on_enter_notify, menuitems);
      g_signal_connect (item, "leave_notify_event",
			(GCallback) on_leave_notify, menuitems);

      if ( ! gc_board_is_demo_only(board) )
	gc_item_focus_init(item, NULL);
    }
}

static void
display_section (gchar *path)
{
  GList *boardlist;	/* List of Board */

  menu_displayed = FALSE;

  boardlist = gc_menu_getlist(path);

  if (actualSectionItem)
    goo_canvas_item_remove(actualSectionItem);

  current_x = 0.0;
  current_y = 0.0;
  current_top_x = 0.0;
  current_top_y = 0.0;

  actualSectionItem = goo_canvas_group_new (boardRootItem,
					    NULL);

  create_top (actualSectionItem, path);

  g_list_foreach (boardlist, (GFunc) display_board_icon, menuitems);

  if (strcmp(path,"home")!=0)
    g_list_free(boardlist);

  menu_displayed = TRUE;
}

static void
menu_end ()
{
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  boardRootItem     = NULL;
  actualSectionItem = NULL;

  g_list_free(panelBoards);

}

static gboolean
menu_is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_ascii_strcasecmp(gcomprisBoard->type, "menu")==0)
	{
	  /* Set the plugin entry */
	  gcomprisBoard->plugin=&menu_bp;

	  return TRUE;
	}
    }
  return FALSE;
}

static void
menu_config ()
{
  if(gcomprisBoard!=NULL)
    {
      menu_pause(TRUE);
      gc_config_start();
    }
}



/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------*/

static void display_board_icon(GcomprisBoard *board, MenuItems *menuitems)
{
  /* FIXME: Check board is available for the current profile */
  if (board!=NULL
      && gc_board_check_file(board))
    {
      menu_create_item(actualSectionItem, menuitems, board);
    }
}

/*
 * Calculate the next stop where to place an item
 * return false if there is no more space left
 */
static gboolean next_spot()
{
  if(current_x==0.0)
    {
      /* Initialisation case */
      current_x = display_x + icon_size/2.0;
	current_y = display_y + icon_size/2.0;
      return(TRUE);
    }

  current_x += icon_size + display_int_x;

  if(current_x > display_x + display_w - icon_size/2.0)
    {
      current_x =  display_x + icon_size/2.0;
      current_y += icon_size + display_int_y;
    }

  if ( current_y > display_y + display_h - icon_size/2.0 )
    return FALSE;
  else
    return TRUE;

}

static void menu_create_item(GooCanvasItem *parent, MenuItems *menuitems, GcomprisBoard *board)
{
  GdkPixbuf *menu_pixmap = NULL;
  GooCanvasItem *menu_button;
  int difficulty;
  gdouble ratio, pixmap_w, pixmap_h;

  menu_pixmap = gc_pixmap_load(board->icon_name);

  ratio = get_ratio (menu_pixmap, icon_size);

  pixmap_w = gdk_pixbuf_get_width(menu_pixmap) * ratio;
  pixmap_h = gdk_pixbuf_get_height(menu_pixmap) * ratio;

  next_spot();

  menu_button = goo_canvas_image_new (parent,
				      menu_pixmap,
				      current_x - pixmap_w/2,
				      current_y - pixmap_h/2,
				      NULL);
  goo_canvas_item_scale(menu_button, ratio, ratio);

  // display board availability due to sound voice not present
  if(board->mandatory_sound_file)
    {
      gchar *soundfile = NULL;

      /* We search a fixed path sound file */
      soundfile = gc_file_find_absolute(board->mandatory_sound_file);

      if (!soundfile || !gc_prop_get()->fx) {
	GooCanvasItem *item =			\
	  goo_canvas_svg_new (parent,
			      gc_skin_rsvg_get(),
			      "svg-id", "#SOUND_UNCHECKED",
			      "pointer-events", GOO_CANVAS_EVENTS_NONE,
			      NULL);

	GooCanvasBounds bounds;
	goo_canvas_item_get_bounds(item, &bounds);

	SET_ITEM_LOCATION(item,
			  current_x - pixmap_w/2 - 25 -
			  (bounds.x2 - bounds.x1)/2,
			  current_y - pixmap_h/2 + 28-
			  (bounds.y2 - bounds.y1)/2);
      }

      g_free(soundfile);
    }

  // display menu icon ========================== BEGIN
  if(g_ascii_strcasecmp(board->type, "menu") == 0)
    {
      GooCanvasItem *item = goo_canvas_svg_new (parent,
						gc_skin_rsvg_get(),
						"svg-id", "#MENUICON",
						"pointer-events", GOO_CANVAS_EVENTS_NONE,
						NULL);
      SET_ITEM_LOCATION(item,
			current_x - pixmap_w/2 - 25,
			current_y - pixmap_h/2);
    }
  else
    {
      // display difficulty stars
      if (board->difficulty != NULL)
	{
	  difficulty = atoi(board->difficulty);
	  menu_difficulty_display(parent,
				  (double)current_x - pixmap_w/2 - 25,
				  (double)current_y - pixmap_h/2,
				  (double) 0.6,
				  difficulty);
	}

      if ( gc_board_is_demo_only(board) )
	menu_demo_display(parent,
			  (gdouble)(current_x - pixmap_w/2 - 20),
			  (gdouble)(current_y - pixmap_h/2 + 60) );
    }


#if GDK_PIXBUF_MAJOR <= 2 && GDK_PIXBUF_MINOR <= 24
  gdk_pixbuf_unref(menu_pixmap);
#else
  g_object_unref(menu_pixmap);
#endif

  // display menu icon ========================== END

  /*
   * Now everything ready, map the events
   * -------------------------------------
   */
  g_object_set_data (G_OBJECT (menu_button), "board", board);

  g_signal_connect(menu_button, "button_press_event",
		   (GCallback) item_event,
		   menuitems);
  g_signal_connect (menu_button, "enter_notify_event",
		    (GCallback) on_enter_notify, menuitems);
  g_signal_connect (menu_button, "leave_notify_event",
		    (GCallback) on_leave_notify, menuitems);

  gc_item_focus_init(menu_button, NULL);

}

static gboolean
on_enter_notify (GooCanvasItem  *item,
		 GooCanvasItem  *target,
		 GdkEventCrossing *event,
		 MenuItems *menuitems)
{
  GcomprisBoard *board;

  board = g_object_get_data (G_OBJECT (item), "board");

  if(G_IS_OBJECT(menuitems->bg))
    g_object_set(menuitems->bg,
		 "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);

  if(board->title && G_IS_OBJECT(menuitems->boardname_item))
    g_object_set (menuitems->boardname_item,
		  "text", board->title,
		  NULL);

  if(board->description
     && G_IS_OBJECT(menuitems->description_item))
    g_object_set(menuitems->description_item,
		 "text", board->description,
		 NULL);

  if(board->author && G_IS_OBJECT(menuitems->author_item))
    g_object_set (menuitems->author_item,
		  "text", board->author,
		  NULL);

  return FALSE;
}

static gboolean
on_leave_notify (GooCanvasItem  *item,
		 GooCanvasItem  *target,
		 GdkEventCrossing *event,
		 MenuItems *menuitems)
{
  g_object_set(menuitems->bg,
	       "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

  g_object_set (menuitems->boardname_item,
		"text", "", NULL);

  g_object_set (menuitems->description_item,
		"text", "", NULL);

  g_object_set (menuitems->author_item,
		"text", "", NULL);

  return FALSE;
}

static gint
item_event(GooCanvasItem *item, GdkEvent *event,  MenuItems *menuitems)
{
  GcomprisBoard *board;

  if(board_paused)
    return FALSE;

  if(boardRootItem    == NULL)
    return FALSE;

  board = g_object_get_data (G_OBJECT (item), "board");

  if(!menu_displayed)
    return TRUE;


  if (strcmp(board->type,"menu")==0)
    {
      gchar *path = g_strdup_printf("%s/%s",board->section, board->name);

      gc_sound_play_ogg ("sounds/bleep.wav", NULL);
      display_section(path);

      if (menu_position)
	g_free(menu_position);

      menu_position = path;

    }
  else if ( ! gc_board_is_demo_only(board) )
    {
      gc_sound_play_ogg ("sounds/level.wav", NULL);
      gc_board_run_next (board);
    }

  return TRUE;
}

/** \brief create the area in which we display the board title and description
 *
 */
static void
create_info_area(GooCanvasItem *parent, MenuItems *menuitems)
{
  gint x = (double) info_x + info_w/2.0;
  gint y = info_y;

  g_assert(parent);

  /* The description background */
  menuitems->bg =
    goo_canvas_rect_new (parent,
			 info_x,
			 info_y,
			 info_w,
			 info_h,
			 "stroke_color_rgba", 0xFFFFFFFFL,
			 "fill_color_rgba",
			 gc_skin_get_color("menu/description_bg_color"),
			 "line-width", (double) 2,
			 "radius-x", (double) 10,
			 "radius-y", (double) 10,
			 NULL);

  g_object_set(menuitems->bg,
	       "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

  menuitems->boardname_item =			\
    goo_canvas_text_new (parent,
			 "",
			 x,
			 y + 20,
			 -1,
			 GTK_ANCHOR_CENTER,
			 "font", gc_skin_font_board_big,
			 "fill-color-rgba", gc_skin_get_color("menu/text"),
			 "alignment", PANGO_ALIGN_CENTER,
			 NULL);

  menuitems->description_item = \
    goo_canvas_text_new (parent,
			 "",
			 x,
			 y + 70,
			 info_w - 10,
			 GTK_ANCHOR_CENTER,
			 "font", gc_skin_font_board_medium,
			 "fill-color-rgba", gc_skin_get_color("menu/text"),
			 "alignment", PANGO_ALIGN_CENTER,
			 NULL);

  menuitems->author_item = \
    goo_canvas_text_new (parent,
			 "",
			 x,
			 y + 110,
			 -1,
			 GTK_ANCHOR_CENTER,
			 "font", gc_skin_font_board_tiny,
			 "fill-color-rgba", gc_skin_get_color("menu/text"),
			 "alignment", PANGO_ALIGN_CENTER,
			 NULL);
}

static gdouble
get_ratio(GdkPixbuf *pixmap, gdouble size)
{

  gdouble ratio = 1.0;
  gint pixmap_h, pixmap_w;

  pixmap_w = gdk_pixbuf_get_width(pixmap);
  pixmap_h = gdk_pixbuf_get_height(pixmap);

  if (pixmap_h <= pixmap_w){
    if (pixmap_w > size)
	ratio = size / pixmap_w;
    }
    else {
      if (pixmap_h > size)
	ratio = size / pixmap_h;
    }

  return ratio;

}

static void
create_top(GooCanvasItem *parent, gchar *path)
{
  gchar **splitted_section;
  gint i = 1;
  GdkPixbuf *pixmap = NULL;
  gdouble ratio;
  GcomprisBoard *board;
  gchar *path1, *path2;

  GooCanvasItem *item;

  GcomprisProperties *properties = gc_prop_get();

  if (!path)
    return;

  splitted_section = g_strsplit (path, "/", 0);

  path1 = g_strdup("");

  /* splitted_section[0] is always "" */
  i = 1;

  while (splitted_section[i] != NULL)
    {

      path2 = g_strdup_printf("%s/%s", path1, splitted_section[i]);

      g_free(path1);
      path1 = path2;

      if (strcmp(path1, properties->root_menu)<0){
	i++;
	continue;
      }

      if (current_top_x == 0.0)
	{
	  current_top_x = top_x;
	  current_top_y = top_y;
	}
      else
	{
	  item = \
	    goo_canvas_svg_new (parent,
				gc_skin_rsvg_get(),
				"svg-id", "#MENUICON",
				"pointer-events", GOO_CANVAS_EVENTS_NONE,
				NULL);

	  SET_ITEM_LOCATION(item,
			    current_top_x,
			    current_top_y + top_arrow_size);
	  //	  goo_canvas_item_scale(item, ratio, ratio);

	  current_top_x += top_arrow_size + top_int_x;
	}

      board = gc_menu_section_get(path1);

      pixmap = gc_pixmap_load(board->icon_name);

      ratio = get_ratio( pixmap, icon_size_top);

      item = goo_canvas_image_new (parent,
				   pixmap,
				   0, 0,
				   NULL);
      goo_canvas_item_translate(item,
				current_top_x,
				current_top_y);
      goo_canvas_item_scale(item, ratio, ratio);
#if GDK_PIXBUF_MAJOR <= 2 && GDK_PIXBUF_MINOR <= 24
      gdk_pixbuf_unref(pixmap);
#else
      g_object_unref(pixmap);
#endif

      current_top_x += top_int_x + icon_size_top;

      g_object_set_data (G_OBJECT (item), "board", board);

      g_signal_connect(item, "button_press_event",
		       (GCallback) item_event,
		       menuitems);

      gc_item_focus_init(item, NULL);

      i++;
    }

  g_strfreev(splitted_section);
  g_free(path1);

}

static gboolean
display_welcome_event (GooCanvasItem  *item,
		       GooCanvasItem  *target,
		       GdkEventCrossing *event,
		       MenuItems *menuitems)
{
  if(G_IS_OBJECT(menuitems->bg))
    g_object_set(menuitems->bg,
		 "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);

  if(G_IS_OBJECT(menuitems->boardname_item))
    g_object_set(menuitems->boardname_item,
		 "text", "GCompris V" VERSION,
		 NULL);

  if(G_IS_OBJECT(menuitems->description_item))
    g_object_set(menuitems->description_item,
		 "text",
		 _("GCompris is a collection of educational games that provides different activities for children aged 2 and up."),
		 NULL);

  if(G_IS_OBJECT(menuitems->author_item))
    g_object_set(menuitems->author_item,
		 "text", "", NULL);

  return FALSE;
}

static void display_sponsor(GooCanvasItem *rootitem)
{
  GooCanvasItem *item;
  GdkPixbuf   *pixmap = NULL;
  /* Location for a potential sponsor */
  gchar *sponsor_image = gc_file_find_absolute("sponsor.png");
  if(sponsor_image)
    {
      pixmap = gc_pixmap_load("sponsor.png");
      item = goo_canvas_image_new (rootitem,
				   pixmap,
				   0.0,
				   0.0,
				   NULL);
      SET_ITEM_LOCATION_CENTER(item,
			       display_x + display_w/2.0,
			       display_y + display_h/3.0 + 150);

#if GDK_PIXBUF_MAJOR <= 2 && GDK_PIXBUF_MINOR <= 24
      gdk_pixbuf_unref(pixmap);
#else
      g_object_unref(pixmap);
#endif
      g_free(sponsor_image);
    }
}

static void
display_welcome (MenuItems *menuitems)
{
  GooCanvasItem *item;
  RsvgHandle  *svg_handle;

  if (actualSectionItem)
    g_error("actualSectionItem exists in display_section !");


  actualSectionItem = goo_canvas_group_new(boardRootItem,
					   NULL);

  svg_handle = gc_skin_rsvg_get();
  item = goo_canvas_svg_new (actualSectionItem,
			     svg_handle,
			     "svg-id", "#LOGO",
			     NULL);
  SET_ITEM_LOCATION_CENTER(item,
			   display_x + display_w/2.0,
			   display_y + display_h/3.0);

  display_sponsor(actualSectionItem);

  g_signal_connect (item, "enter_notify_event",
		    (GCallback) display_welcome_event, menuitems);
  g_signal_connect (item, "leave_notify_event",
		    (GCallback) on_leave_notify, menuitems);

  menu_displayed = TRUE;
}

static void
menu_config_start(GcomprisBoard *agcomprisBoard,
		  GcomprisProfile *aProfile){
  if(gcomprisBoard!=NULL)
    {
      menu_pause(TRUE);
      gc_config_start();
    }
}

/* ======================= */
/* = config_stop        = */
/* ======================= */
static void
menu_config_stop()
{
}

/**
 * Display the number of stars representing the difficulty level at the x,y location
 * The stars are created in a group 'parent'
 * The new group in which the stars are created is returned.
 */
static GooCanvasItem *
menu_difficulty_display(GooCanvasItem *parent,
		      double x, double y,
		      double ratio,
		      gint difficulty)
{
  GooCanvasItem *stars_group = NULL;
  GooCanvasItem *item = NULL;
  gchar *svg_id = NULL;

  if(difficulty==0 || difficulty>6)
    return NULL;

  svg_id = g_strdup_printf("#DIFFICULTY%d", difficulty);

  stars_group = goo_canvas_group_new (parent, NULL);

  item = goo_canvas_svg_new (stars_group,
			     gc_skin_rsvg_get(),
			     "svg-id", svg_id,
			     "pointer-events", GOO_CANVAS_EVENTS_NONE,
			     NULL);
  SET_ITEM_LOCATION(item, x, y);

  g_free(svg_id);

  return(stars_group);
}

/**
 * Display the demo only icon
 */
static GooCanvasItem *
menu_demo_display(GooCanvasItem *parent,
		  gdouble x, gdouble y)
{
  GooCanvasItem *item = goo_canvas_svg_new (parent,
					    gc_skin_rsvg_get(),
					    "svg-id", "#UNCHECKED",
					    "pointer-events", GOO_CANVAS_EVENTS_NONE,
					    NULL);
  SET_ITEM_LOCATION(item, x, y);
  return(item);
}

