/* gcompris - menu2.c
 *
 * Time-stamp: <2006/08/11 18:58:38 bruno>
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
  /* Information items (_s are shadow) */
  GnomeCanvasItem *boardname_item;
  GnomeCanvasRichText *description_item;
  GnomeCanvasItem *author_item;
  GnomeCanvasItem *boardname_item_s;
  GnomeCanvasRichText *description_item_s;
  GnomeCanvasItem *author_item_s;
} MenuItems;

static MenuItems *menuitems;

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void		 menu_start (GcomprisBoard *agcomprisBoard);
static void		 menu_pause (gboolean pause);
static void		 menu_end (void);
static gboolean		 menu_is_our_board (GcomprisBoard *gcomprisBoard);
static void		 menu_config(void);

static void              menu_config_start(GcomprisBoard *agcomprisBoard,
					   GcomprisProfile *aProfile);

static void              menu_config_stop(void);

static void		 menu_create_item(GnomeCanvasGroup *parent, MenuItems *menuitems, GcomprisBoard *board);
static gboolean		 next_spot();
static void		 create_info_area(GnomeCanvasGroup *parent, MenuItems *menuitems);
static gint		 item_event(GnomeCanvasItem *item, GdkEvent *event, MenuItems *menuitems);
static void		 display_board_icon(GcomprisBoard *board, MenuItems *menuitems);
static void		 free_stuff (GtkObject *obj, gpointer data);
static void		 set_content(GnomeCanvasRichText *item_content,
				     GnomeCanvasRichText *item_content_s,
				     gchar *text);

static void              display_section (gchar *path);
static void              display_welcome (void);
static void		 create_panel(GnomeCanvasGroup *parent);
static void		 create_top(GnomeCanvasGroup *parent, gchar *path);

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

GList *homeBoards = NULL;

#define P_X 10
#define P_Y 10
#define P_W 90
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
#define I_Y 400
#define I_W 600
#define I_H 125

#define ICON_SIZE 100
#define ICON_SIZE_PANEL 50
#define ICON_SIZE_TOP 50

static GnomeCanvasGroup *boardRootItem = NULL;
static GnomeCanvasGroup *actualSectionItem = NULL;
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

  GcomprisProperties	*properties = gcompris_get_properties();

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

  g_warning ("menu2 : start  board");

  if(agcomprisBoard!=NULL)
    {
      gchar *img;

      gcomprisBoard=agcomprisBoard;

      menuitems = g_new(MenuItems, 1);

      img = gcompris_image_to_skin("gcompris-menu2bg.png");
      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), 
			      img);
      g_free(img);

      boardRootItem = GNOME_CANVAS_GROUP(
					 gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
								gnome_canvas_group_get_type (),
								"x", (double) 0,
								"y", (double) 0,
								NULL));

      g_object_set_data (G_OBJECT (boardRootItem), "menuitems", menuitems);
      g_signal_connect (boardRootItem, "destroy",
			G_CALLBACK (free_stuff),
			menuitems);

      create_info_area(boardRootItem, menuitems);

      create_panel(boardRootItem);

      if (properties->menu_position){
	display_section(properties->menu_position);
      }
      else
	display_welcome();

      /* set initial values for this level */
      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel=1;
      gcompris_bar_set(GCOMPRIS_BAR_CONFIG|GCOMPRIS_BAR_ABOUT);

      /* FIXME : Workaround for bugged canvas */
      //gnome_canvas_update_now(gcomprisBoard->canvas);

      menu_pause(FALSE);

    }

}


static void create_panel(GnomeCanvasGroup *parent)
{
  int n_sections;
  GList *list = NULL;
  GcomprisBoard *board;
  GdkPixbuf *pixmap = NULL;
  GnomeCanvasItem *item;
  gdouble ratio;

  gdouble x, y;
  gint int_y;
  GcomprisProperties	*properties = gcompris_get_properties();

  /* In normal mode, we show all the sections in panel */
  /* in direct submenu access, we show the icon of the submenu */
  if (strcmp(properties->root_menu,"/")==0)
    panelBoards = gcompris_get_menulist(properties->root_menu);
  else
    panelBoards = g_list_append(list, gcomprisBoard);

  n_sections = g_list_length(panelBoards);

  if (n_sections == 0)
    return;

  if ( (panel_h/n_sections) <= icon_size_panel){
    icon_size_panel = panel_h/n_sections;
    int_y = 0;
  }
  else {
    int_y = (panel_h - n_sections*icon_size_panel)/n_sections;
  }

  x = panel_x + panel_w/2.0;
  y = panel_y + int_y/2.0;

  for (list = panelBoards; list != NULL; list = list->next){
    board = (GcomprisBoard *) list->data;

    pixmap = gcompris_load_pixmap(board->icon_name);

    ratio = get_ratio( pixmap, icon_size_panel);
     
    item = gnome_canvas_item_new (parent,
				  gnome_canvas_pixbuf_get_type (),
				  "pixbuf", pixmap,
				  "x", x,
				  "y", y,
				  "width", (gdouble)  gdk_pixbuf_get_width(pixmap)*ratio,
				  "height", (gdouble) gdk_pixbuf_get_height(pixmap)*ratio,
				  "width-set", TRUE,
				  "height-set", TRUE,
				  "anchor", GTK_ANCHOR_NORTH,
				  NULL);

    gdk_pixbuf_unref(pixmap);

    y += int_y + icon_size_panel;

    g_object_set_data (G_OBJECT (item), "board", board);

    gtk_signal_connect(GTK_OBJECT(item), "event",
		       (GtkSignalFunc) item_event,
		       menuitems);
    
    gtk_signal_connect(GTK_OBJECT(item), "event",
		       (GtkSignalFunc) gcompris_item_event_focus,
		       NULL);
   
  }

  

}

static void display_section (gchar *path)
{
      GList		*boardlist;	/* List of Board */

      boardlist = gcompris_get_menulist(path);

      if (actualSectionItem)
	gtk_object_destroy (GTK_OBJECT(actualSectionItem));

      current_x = 0.0;
      current_y = 0.0;
      current_top_x = 0.0;
      current_top_y = 0.0;

      actualSectionItem = GNOME_CANVAS_GROUP(
					 gnome_canvas_item_new ( boardRootItem,
								 gnome_canvas_group_get_type (),
								"x", (double) 0,
								"y", (double) 0,
								NULL));
       

      create_top (actualSectionItem, path);

      g_list_foreach (boardlist, (GFunc) display_board_icon, menuitems);

      if (strcmp(path,"home")!=0)
	g_list_free(boardlist);

}

static void
menu_end ()
{
  if(boardRootItem!=NULL)
    {
      /* WORKAROUND: There is a bug in the richtex item and we need to remove it first */
      while (g_idle_remove_by_data (menuitems->description_item));
      gtk_object_destroy (GTK_OBJECT(menuitems->description_item));

      while (g_idle_remove_by_data (menuitems->description_item_s));
      gtk_object_destroy (GTK_OBJECT(menuitems->description_item_s));

      gtk_object_destroy (GTK_OBJECT(boardRootItem));
    }

  boardRootItem     = NULL;
  actualSectionItem = NULL;
}

static gboolean
menu_is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "menu")==0)
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
      gcompris_config_start();
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
      && board_check_file(board))
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

static void menu_create_item(GnomeCanvasGroup *parent, MenuItems *menuitems, GcomprisBoard *board)
{
  GdkPixbuf *menu_pixmap = NULL;
  GdkPixbuf *pixmap = NULL;
  GnomeCanvasItem *item, *menu_button;
  int difficulty;
  gchar *tmp_board_dir;
  gdouble ratio, pixmap_w, pixmap_h;

  /*
   * Take care to load the board->icon_name from the dir specified
   * in the board->board_dir
   * To make it simple and reuse the gcompris_load_pixmap, we overwrite temporarily
   * the gcomprisBoard->board_dir and board->board_dir
   */
  tmp_board_dir = gcomprisBoard->board_dir;
  gcomprisBoard->board_dir = board->board_dir;
  menu_pixmap = gcompris_load_pixmap(board->icon_name);
  gcomprisBoard->board_dir = tmp_board_dir;

  ratio = get_ratio (menu_pixmap, icon_size);

  if (ratio < 1.0)
    g_warning("Resize %s", board->icon_name);

  pixmap_w = gdk_pixbuf_get_width(menu_pixmap)*ratio;
  pixmap_h = gdk_pixbuf_get_height(menu_pixmap)*ratio;

  next_spot();

  menu_button = gnome_canvas_item_new (parent,
				       gnome_canvas_pixbuf_get_type (),
				       "pixbuf", menu_pixmap,
				       "x", (double)current_x - pixmap_w/2,
				       "y", (double)current_y - pixmap_h/2,
				       "width", (gdouble) pixmap_w,
				       "height", (gdouble) pixmap_h,
				       "width-set", TRUE,
				       "height-set", TRUE,
				       NULL);

  // display difficulty stars
  if (board->difficulty != NULL) {
    difficulty = atoi(board->difficulty);
    gcompris_display_difficulty_stars(parent,
				      (double)current_x - pixmap_w/2 - 25,
				      (double)current_y - pixmap_h/2,
				      (double) 0.6,
				      difficulty);
  }

  // display board availability due to sound voice not present
  if(board->mandatory_sound_file)
    {
      gchar *soundfile = NULL;

      /* We search a fixed path sound file */
      soundfile = gcompris_find_absolute_filename(board->mandatory_sound_file);
      g_warning("Checking mandatory_sound_file %s\n", soundfile);

      if (!soundfile || !gcompris_get_properties()->fx) 
	{
	  pixmap = gcompris_load_skin_pixmap("voice_bad.png");
	}
      else
	{
	  pixmap = gcompris_load_skin_pixmap("voice.png");
	}

      gnome_canvas_item_new (parent,
			     gnome_canvas_pixbuf_get_type (),
			     "pixbuf", pixmap,
			     "x", (double)current_x - pixmap_w/2 - 25,
			     "y", (double)current_y - pixmap_h/2 + 28,
			     NULL);
      gdk_pixbuf_unref(pixmap);
      g_free(soundfile);
    }

  // display menu icon ========================== BEGIN
  if(g_strcasecmp(board->type, "menu")==0)
    {
      pixmap = gcompris_load_skin_pixmap("menuicon.png");
      item =  gnome_canvas_item_new (parent,
				     gnome_canvas_pixbuf_get_type (),
				     "pixbuf", pixmap,
				     "x", (double)current_x - pixmap_w/2 - 25,
				     "y", (double)current_y - pixmap_h/2,
				     NULL);
      gdk_pixbuf_unref(pixmap);
    }

  gdk_pixbuf_unref(menu_pixmap);

  // display menu icon ========================== END

  /*
   * Now everything ready, map the events
   * -------------------------------------
   */
  g_object_set_data (G_OBJECT (menu_button), "board", board);

  gtk_signal_connect(GTK_OBJECT(menu_button), "event",
		     (GtkSignalFunc) item_event,
		     menuitems);
  
  gtk_signal_connect(GTK_OBJECT(menu_button), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);

}

static gint
item_event(GnomeCanvasItem *item, GdkEvent *event,  MenuItems *menuitems)
{
  GcomprisBoard *board;

  if(board_paused)
    return FALSE;

  if(boardRootItem    == NULL)
    return FALSE;

  board = g_object_get_data (G_OBJECT (item), "board");

  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
      if(board->title && G_IS_OBJECT(menuitems->boardname_item))
	gnome_canvas_item_set (menuitems->boardname_item,
			       "text", board->title,
			       NULL);

      if(board->description 
	 && G_IS_OBJECT(menuitems->description_item)
	 && G_IS_OBJECT(menuitems->description_item_s))
	set_content(menuitems->description_item,
		    menuitems->description_item_s,
		    board->description);

      if(board->author && G_IS_OBJECT(menuitems->author_item))
	gnome_canvas_item_set (menuitems->author_item,
			       "text",  board->author,
			       NULL);

      if(board->title && G_IS_OBJECT(menuitems->boardname_item_s))
	gnome_canvas_item_set (menuitems->boardname_item_s,
			       "text", board->title,
			       NULL);

      if(board->author && G_IS_OBJECT(menuitems->author_item_s))
	gnome_canvas_item_set (menuitems->author_item_s,
			       "text",  board->author,
			       NULL);

      break;
    case GDK_LEAVE_NOTIFY:
      gnome_canvas_item_set (menuitems->boardname_item,
			     "text", " ",
			     NULL);

      gnome_canvas_item_set (GNOME_CANVAS_ITEM(menuitems->description_item),
			     "text",  " ",
			     NULL);

      gnome_canvas_item_set (menuitems->author_item,
			     "text",  " ",
			     NULL);

      gnome_canvas_item_set (menuitems->boardname_item_s,
			     "text", " ",
			     NULL);

      gnome_canvas_item_set (GNOME_CANVAS_ITEM(menuitems->description_item_s),
			     "text",  " ",
			     NULL);

      gnome_canvas_item_set (menuitems->author_item_s,
			     "text",  " ",
			     NULL);

      break;
    case GDK_BUTTON_PRESS:
	gcompris_play_ogg ("gobble", NULL);
	
	if (strcmp(board->type,"menu")==0){
	  gchar *path = g_strdup_printf("%s/%s",board->section, board->name);
	  GcomprisProperties	*properties = gcompris_get_properties();

	  display_section(path);

	  if (properties->menu_position)
	    g_free(properties->menu_position);

	  properties->menu_position = path;
  
	}
	else
	  board_run_next (board);
	
      break;

    default:
      break;
    }
  
  return FALSE;
}

/* Apply the style to the given RichText item  */
static void
set_content(GnomeCanvasRichText *item_content, 
	    GnomeCanvasRichText *item_content_s,
	    gchar *text) {

  GtkTextIter    iter_start, iter_end;
  GtkTextBuffer *buffer;
  GtkTextTag    *txt_tag;
  gboolean success; 
  gchar *color_string;
  GdkColor *color_s = (GdkColor *)malloc(sizeof(GdkColor));
  GdkColor *color   = (GdkColor *)malloc(sizeof(GdkColor));

  /*
   * Set the new text in the 2 items
   */
  gnome_canvas_item_set(GNOME_CANVAS_ITEM(item_content),
			"text", text,
			NULL);

  gnome_canvas_item_set(GNOME_CANVAS_ITEM(item_content_s),
			"text", text,
			NULL);

  /*
   * Set the shadow
   */

  color_string = g_strdup_printf("#%x", gcompris_skin_color_shadow >> 8);
  gdk_color_parse(color_string, color_s);
  success = gdk_colormap_alloc_color(gdk_colormap_get_system(), 
				     color_s,
  				     FALSE, TRUE); 

  buffer  = gnome_canvas_rich_text_get_buffer(GNOME_CANVAS_RICH_TEXT(item_content_s));
  txt_tag = gtk_text_buffer_create_tag(buffer, NULL, 
				       "foreground-gdk", color_s,
				       "font",       gcompris_skin_font_board_medium,
				       NULL);
  gtk_text_buffer_get_end_iter(buffer, &iter_end);
  gtk_text_buffer_get_start_iter(buffer, &iter_start);
  gtk_text_buffer_apply_tag(buffer, txt_tag, &iter_start, &iter_end);

  g_free(color_string);

  /* 
   * Set the text
   */
  color_string = g_strdup_printf("#%x", gcompris_skin_get_color("menu/text") >> 8);
  gdk_color_parse(color_string, color);
  success = gdk_colormap_alloc_color(gdk_colormap_get_system(), 
				     color,
  				     FALSE, TRUE); 

  buffer  = gnome_canvas_rich_text_get_buffer(GNOME_CANVAS_RICH_TEXT(item_content));
  txt_tag = gtk_text_buffer_create_tag(buffer, NULL, 
				       "foreground-gdk", color,
				       "font",        gcompris_skin_font_board_medium,
				       NULL);
  gtk_text_buffer_get_end_iter(buffer, &iter_end);
  gtk_text_buffer_get_start_iter(buffer, &iter_start);
  gtk_text_buffer_apply_tag(buffer, txt_tag, &iter_start, &iter_end);

}

/** \brief create the area in which we display the board title and description
 *
 */
static void create_info_area(GnomeCanvasGroup *parent, MenuItems *menuitems)
{
  gint x = (double) info_x + info_w/2.0;
  gint y = info_y;

  if(parent    == NULL)
    return;

  menuitems->boardname_item_s = \
    gnome_canvas_item_new (parent,
			   gnome_canvas_text_get_type (),
			   "text", " ",
			   "font", gcompris_skin_font_board_big,
			   "x", (double) x + 1.0,
			   "y", (double) y + 1.0,
			   "anchor", GTK_ANCHOR_NORTH,
			   "fill_color_rgba",  gcompris_skin_color_shadow,
			   NULL);

  menuitems->boardname_item = \
    gnome_canvas_item_new (parent,
			   gnome_canvas_text_get_type (),
			   "text", " ",
			   "font", gcompris_skin_font_board_big,
			   "x", (double) x,
			   "y", (double) y,
			   "anchor", GTK_ANCHOR_NORTH,
			   "fill_color_rgba",  gcompris_skin_get_color("menu/text"),
			   NULL);

  menuitems->description_item_s = \
    GNOME_CANVAS_RICH_TEXT(gnome_canvas_item_new (parent,
						  gnome_canvas_rich_text_get_type (),
						  "x", (double) x + 1.0,
						  "y", (double) y + 28 + 1.0,
						  "width",  info_w,
						  "height", info_h - 28,
						  "anchor", GTK_ANCHOR_NORTH,
						  "justification", GTK_JUSTIFY_CENTER,
						  "grow_height", FALSE,
						  "cursor_visible", FALSE,
						  "cursor_blink", FALSE,
						  "editable", FALSE,
						  NULL));
  menuitems->description_item = \
    GNOME_CANVAS_RICH_TEXT(gnome_canvas_item_new (parent,
						  gnome_canvas_rich_text_get_type (),
						  "x", (double) x,
						  "y", (double) y + 28,
						  "width",  info_w,
						  "height", info_h - 28,
						  "anchor", GTK_ANCHOR_NORTH,
						  "justification", GTK_JUSTIFY_CENTER,
						  "grow_height", FALSE,
						  "cursor_visible", FALSE,
						  "cursor_blink", FALSE,
						  "editable", FALSE,
						  NULL));

  menuitems->author_item_s = \
    gnome_canvas_item_new (parent,
			   gnome_canvas_text_get_type (),
			   "text", " ",
			   "font", gcompris_skin_font_board_tiny,
			   "x", (double) x + 1.0,
			   "y", (double) y + 90 + 1.0,
  			   "anchor", GTK_ANCHOR_NORTH,
  			   "fill_color_rgba", gcompris_skin_color_shadow,
  			   "justification", GTK_JUSTIFY_CENTER,
			   NULL);

  menuitems->author_item = \
    gnome_canvas_item_new (parent,
			   gnome_canvas_text_get_type (),
			   "text", " ",
			   "font", gcompris_skin_font_board_tiny,
			   "x", (double) x,
			   "y", (double) y + 90,
  			   "anchor", GTK_ANCHOR_NORTH,
  			   "fill_color_rgba", gcompris_skin_get_color("menu/text"),
  			   "justification", GTK_JUSTIFY_CENTER,
			   NULL);

}

static void
free_stuff (GtkObject *obj, gpointer data)
{
  g_free (data);
}


static gdouble get_ratio(GdkPixbuf *pixmap, gdouble size)
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

static void create_top(GnomeCanvasGroup *parent, gchar *path)
{
  gchar **splitted_section;
  gint i = 1;
  GdkPixbuf *pixmap = NULL;
  gdouble ratio;
  GcomprisBoard *board;
  gchar *path1, *path2;

  GnomeCanvasItem *item;

  GcomprisProperties	*properties = gcompris_get_properties();

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

      if (current_top_x == 0.0){
	current_top_x = top_x;
	current_top_y = top_y + top_h/2.0;
      } else {
	pixmap = gcompris_load_skin_pixmap("button_forward.png");
	ratio = get_ratio(pixmap, top_arrow_size);
	
	gnome_canvas_item_new (parent,
			       gnome_canvas_pixbuf_get_type (),
			       "pixbuf", pixmap,
			       "x", current_top_x,
			       "y", current_top_y,
			       "width", (gdouble)  gdk_pixbuf_get_width(pixmap)*ratio,
			       "height", (gdouble) gdk_pixbuf_get_height(pixmap)*ratio,
			       "width-set", TRUE,
			       "height-set", TRUE,
			       "anchor", GTK_ANCHOR_WEST,
			       NULL);

	gdk_pixbuf_unref(pixmap);

	current_top_x += top_arrow_size + top_int_x;
	
      }
      
      board = gcompris_get_board_from_section(path1);

      pixmap = gcompris_load_pixmap(board->icon_name);

      ratio = get_ratio( pixmap, icon_size_top);
     
      item = gnome_canvas_item_new (parent,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", pixmap,
				    "x", current_top_x,
				    "y", current_top_y,
				    "width", (gdouble)  gdk_pixbuf_get_width(pixmap)*ratio,
				    "height", (gdouble) gdk_pixbuf_get_height(pixmap)*ratio,
				    "width-set", TRUE,
				    "height-set", TRUE,
				    "anchor", GTK_ANCHOR_WEST,
				    NULL);

      gdk_pixbuf_unref(pixmap);
      
      current_top_x += top_int_x + icon_size_top;
    
      g_object_set_data (G_OBJECT (item), "board", board);
      
      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) item_event,
			 menuitems);
      
      gtk_signal_connect(GTK_OBJECT(item), "event",
			 (GtkSignalFunc) gcompris_item_event_focus,
			 NULL);
 
      

      i++;
    }

  g_free(path1);
  
}

static void              display_welcome (void)
{
  GnomeCanvasItem *logo;
  GdkPixbuf *pixmap;

  if (actualSectionItem)
    {
      g_error("actualSectionItem exists in display_section !");
    }


  actualSectionItem = GNOME_CANVAS_GROUP(
					 gnome_canvas_item_new ( boardRootItem,
								 gnome_canvas_group_get_type (),
								 "x", (double) 0,
								 "y", (double) 0,
								 NULL));
  
  pixmap = gcompris_load_skin_pixmap("gcompris-about.png");


  logo = gnome_canvas_item_new (actualSectionItem,
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", pixmap,
				"x", (gdouble) display_x + display_w/2.0,
				"y", (gdouble) display_y + display_h/2.0,
				"anchor", GTK_ANCHOR_CENTER,
				NULL);

  gdk_pixbuf_unref(pixmap);
 
  if(G_IS_OBJECT(menuitems->boardname_item))
    gnome_canvas_item_set (menuitems->boardname_item,
			   "text", "GCompris V" VERSION,
			   NULL);
  
  if(G_IS_OBJECT(menuitems->description_item)
     && G_IS_OBJECT(menuitems->description_item_s))
    set_content(menuitems->description_item,
		menuitems->description_item_s,
		_("GCompris is a collection of educational games that provides different activities for children aged 2 and up."));

  if(G_IS_OBJECT(menuitems->author_item))
    gnome_canvas_item_set (menuitems->author_item,
			   "text", "",
			   NULL);
  if(G_IS_OBJECT(menuitems->boardname_item_s))
    gnome_canvas_item_set (menuitems->boardname_item_s,
			       "text", "GCompris V" VERSION,
			   NULL);
  
  if(G_IS_OBJECT(menuitems->author_item_s))
    gnome_canvas_item_set (menuitems->author_item_s,
			   "text", "",
			   NULL);
  
}

static void
menu_config_start(GcomprisBoard *agcomprisBoard,
	     GcomprisProfile *aProfile){
  if(gcomprisBoard!=NULL)
    {
      menu_pause(TRUE);
      gcompris_config_start();
    }
}

/* ======================= */
/* = config_stop        = */
/* ======================= */
static void 
menu_config_stop()
{
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
