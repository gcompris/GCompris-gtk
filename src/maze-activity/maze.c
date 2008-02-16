/* gcompris - maze.c
 *
 * Copyright (C) 2002 Bastiaan Verhoef
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

#include <librsvg/rsvg.h>
#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE
/*-------------------*/
#define NORTH 1
#define WEST 2
#define SOUTH 4
#define EAST 8
#define SET 16
#define BAD 32
#define WON 64
#define MAX_BREEDTE 37
#define MAX_HOOGTE 20
#define BASE_X1 50
#define BASE_Y1 50
#define BASE_X2 800
#define BASE_Y2 500

#define LINE_COLOR "white"

/* array of the board */
static int Maze[MAX_BREEDTE][MAX_HOOGTE];
static int position[MAX_BREEDTE*MAX_HOOGTE][2];

static int ind=0;
static int begin;
static int end;
static int breedte=10;
static int hoogte=20;
static int cellsize=20;
static int buffer=4;
static int board_border_x=20;
static int board_border_y=3;
static int thickness=2;
static gboolean run_fast=FALSE;

static gboolean modeIs2D=TRUE;
static gboolean modeRelative=FALSE;
static gboolean modeIsInvisible=FALSE;
/*-----------------------*/

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static int gamewon;

static void process_ok(void);
static void game_won(void);
static void repeat(void);

/* ================================================================ */
static GooCanvasItem *boardRootItem = NULL;
static GooCanvasItem *mazegroup     = NULL;
static GooCanvasItem *tuxgroup      = NULL;
static GooCanvasItem *wallgroup     = NULL;

static GooCanvasItem *warning_item   = NULL;
static GooCanvasItem *tuxitem        = NULL;

static GooCanvasItem *maze_create_item(GooCanvasItem *parent);
static void maze_destroy_all_items(void);
static void maze_next_level(void);
static void set_level (guint level);
static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str);
/*--------------------*/
static void draw_a_rect(GooCanvasItem *group, int x1, int y1, int x2, int y2, char *color);
static void draw_a_line(GooCanvasItem *group, int x1, int y1, int x2, int y2, guint32 color);
static GooCanvasItem *draw_image(GooCanvasItem *group, int x,int y, GdkPixbuf *pixmap);
static void move_image(GooCanvasItem *group, int x,int y, GooCanvasItem *item);
static void draw_rect(GooCanvasItem *group, int x,int y,char *color);
static void draw_combined_rect(GooCanvasItem *group, int x1, int y1, int x2,int y2, char *color);
static void initMaze(void);
static int check(int x,int y);
static int* isPossible(int x, int y);
static void generateMaze(int x, int y);
static void removeSet(void);
static void draw_background(GooCanvasItem *rootItem);
static void setlevelproperties(void);
static gboolean tux_event (GooCanvasItem *item,
			   GooCanvasItem *target,
			   GdkEventButton *event,
			   gpointer data);
static void update_tux(gint direction);

/*---------- 3D stuff ------------*/
static GooCanvasItem *threedgroup = NULL;
static gint viewing_direction=EAST;
static gboolean threeDactive=FALSE;
static gboolean mapActive=FALSE;
typedef float eyepos_t;

// x,y e ]-1 ... 1[
// z e ]-1 ... 1 ... oo[
static eyepos_t eye_pos_x=0, eye_pos_y=0, eye_pos_z=0;

static void threeDdisplay();
static void twoDdisplay();
static void draw3D();
static gint key_press_3D(guint keyval, gchar *commit_str, gchar *preedit_str);
static gint key_press_2D_relative(guint keyval, gchar *commit_str, gchar *preedit_str);
/*----------------------*/

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("Maze"),
    N_("Find your way out of the maze"),
    "Bastiaan Verhoef <b.f.verhoef@student.utwente.nl>",
    NULL,
    NULL,
    NULL,
    NULL,
    start_board,
    pause_board,
    end_board,
    is_our_board,
    key_press,
    process_ok,
    set_level,//set_level,
    NULL,
    repeat,
    NULL,
    NULL
  };

/* =====================================================================
 *
 * =====================================================================*/
GET_BPLUGIN_INFO(maze)

/* =====================================================================
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 * =====================================================================*/
static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  if(gamewon == TRUE && pause == FALSE) /* the game is won */
    game_won();

  board_paused = pause;
}

static void set_level (guint level) {
  if(gcomprisBoard!=NULL) {
    gcomprisBoard->level=level;
    maze_next_level();
  }
}

/* =====================================================================
 *
 * =====================================================================*/
static void start_board (GcomprisBoard *agcomprisBoard) {

  GdkPixbuf *pixmap = NULL;

  if(agcomprisBoard!=NULL) {
    gchar *img;

    gcomprisBoard=agcomprisBoard;

    /* disable im_context */
    gcomprisBoard->disable_im_context = TRUE;

    img = gc_skin_image_get("gcompris-bg.jpg");
    gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			    img);
    g_free(img);
    gcomprisBoard->level=1;
    gcomprisBoard->maxlevel=9;

    /* The mode defines if we run 2D or 3D */
    /* Default mode is 2D */
    modeRelative=FALSE;
    modeIsInvisible=FALSE;
    if(!gcomprisBoard->mode)
      modeIs2D=TRUE;
    else if(g_strncasecmp(gcomprisBoard->mode, "2DR", 3)==0) {
      /* 2D Relative */
      modeIs2D=TRUE;
      modeRelative=TRUE;
    } else if(g_strncasecmp(gcomprisBoard->mode, "2DI", 3)==0) {
      modeIs2D=TRUE;
      modeIsInvisible=TRUE;
    } else if(g_strncasecmp(gcomprisBoard->mode, "2D", 2)==0) {
      modeIs2D=TRUE;
    } else if(g_strncasecmp(gcomprisBoard->mode, "3D", 2)==0) {
      modeIs2D=FALSE;
    }

    if(!modeIs2D || modeIsInvisible) {
      pixmap = gc_skin_pixmap_load("maze-2d-bubble.png");
      if(pixmap) {
	gc_bar_set_repeat_icon(pixmap);
	gdk_pixbuf_unref(pixmap);
	gc_bar_set(GC_BAR_LEVEL|GC_BAR_REPEAT_ICON);
      } else {
	gc_bar_set(GC_BAR_LEVEL|GC_BAR_REPEAT);
      }
    } else {
      /* 2D Regular mode */
      gc_bar_set(GC_BAR_LEVEL);
    }

    gamewon = FALSE;

    maze_next_level();
    pause_board(FALSE);
  }
}

/* =====================================================================
 *
 * =====================================================================*/
static void end_board () {

  if(gcomprisBoard!=NULL){
    pause_board(TRUE);
    maze_destroy_all_items();
  }
  gcomprisBoard = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
static gboolean is_our_board (GcomprisBoard *gcomprisBoard) {
  if (gcomprisBoard) {
    if(g_strcasecmp(gcomprisBoard->type, "maze")==0) {
      /* Set the plugin entry */
      gcomprisBoard->plugin=&menu_bp;
      return TRUE;
    }
  }
  return FALSE;
}

/* =====================================================================
 * set initial values for the next level
 * =====================================================================*/
static void maze_next_level() {
  GdkPixbuf *pixmap = NULL;

  maze_destroy_all_items();
  gc_bar_set_level(gcomprisBoard);
  setlevelproperties();

  mapActive = FALSE;

  gamewon = FALSE;
  initMaze();
  generateMaze((g_random_int()%breedte),(g_random_int()%hoogte));
  removeSet();
  /* Try the next level */
  maze_create_item(goo_canvas_get_root_item(gcomprisBoard->canvas));
  draw_background(wallgroup);

  if(modeIsInvisible) {
    g_object_set (wallgroup, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
  }

  /* make a new group for the items */
  begin=g_random_int()%hoogte;
  end=g_random_int()%hoogte;

  /* Draw the tux */
  GError *error = NULL;
  RsvgHandle *svg_handle = NULL;
  gchar *filename = gc_file_find_absolute("maze/tux_top_south.svg");
  svg_handle = rsvg_handle_new_from_file (filename, &error);
  tuxitem = goo_svg_item_new (tuxgroup, svg_handle,
			      NULL);
  g_free(filename);
  g_object_unref (svg_handle);

  goo_canvas_item_translate(tuxgroup,
			    cellsize*(0)-breedte + board_border_x,
			    cellsize*(begin)-hoogte + board_border_y);

  g_signal_connect(tuxitem,
		   "button_press_event",
		   (GtkSignalFunc) tux_event, NULL);

  /* Draw the target */
  pixmap = gc_pixmap_load("maze/door.png");
  if(pixmap)
    {
      draw_image(mazegroup,breedte-1,end,pixmap);
      gdk_pixbuf_unref(pixmap);
    }

  position[ind][0]=0;
  position[ind][1]=begin;
  Maze[0][begin]=Maze[0][begin]+SET;
  viewing_direction=EAST;
  threeDactive=FALSE;

  if (gcomprisBoard->level==1) run_fast=FALSE;
  if (gcomprisBoard->level==4) run_fast=TRUE;

  update_tux(viewing_direction);

  if(!modeIs2D)
    threeDdisplay();

}
/* ======================================= */
static void setlevelproperties(){
  if (gcomprisBoard->level==1)
    {
      breedte=5;
      hoogte=4;
      cellsize=70;
      buffer=8;

      board_border_x=(int) (BASE_X2-breedte*cellsize)/2;
      board_border_y=(int) (BASE_Y2-hoogte*cellsize)/2;
    }
  else if (gcomprisBoard->level==2)
    {

      breedte=9;
      hoogte=6;
      cellsize=70;
      buffer=7;
      board_border_x=(int) (BASE_X2-breedte*cellsize)/2;
      board_border_y=(int) (BASE_Y2-hoogte*cellsize)/2;
    }
  else if (gcomprisBoard->level==3)
    {
      breedte=13;
      hoogte=8;
      cellsize=60;
      buffer=6;
      board_border_x=(int) (BASE_X2-breedte*cellsize)/2;
      board_border_y=(int) (BASE_Y2-hoogte*cellsize)/2;
    }
  else if (gcomprisBoard->level==4)
    {
      breedte=17;
      hoogte=10;
      cellsize=45;
      buffer=5;
      board_border_x=(int) (BASE_X2-breedte*cellsize)/2;
      board_border_y=(int) (BASE_Y2-hoogte*cellsize)/2;
    }
  else if (gcomprisBoard->level==5)
    {
      breedte=21;
      hoogte=12;
      cellsize=35;
      buffer=4;
      board_border_x=(int) (BASE_X2-breedte*cellsize)/2;
      board_border_y=(int) (BASE_Y2-hoogte*cellsize)/2;
    }
  else if (gcomprisBoard->level==6)
    {
      breedte=25;
      hoogte=14;
      cellsize=30;
      board_border_x=(int) (BASE_X2-breedte*cellsize)/2;
      board_border_y=(int) (BASE_Y2-hoogte*cellsize)/2;
    }
  else if (gcomprisBoard->level==7)
    {
      breedte=29;
      hoogte=16;
      cellsize=25;
      board_border_x=(int) (BASE_X2-breedte*cellsize)/2;
      board_border_y=(int) (BASE_Y2-hoogte*cellsize)/2;
    }
  else if (gcomprisBoard->level==8)
    {
      breedte=33;
      hoogte=18;
      cellsize=23;
      board_border_x=(int) (BASE_X2-breedte*cellsize)/2;
      board_border_y=(int) (BASE_Y2-hoogte*cellsize)/2;
    }
  else if (gcomprisBoard->level==9)
    {
      breedte=37;
      hoogte=20;
      cellsize=20;
      board_border_x=(int) (BASE_X2-breedte*cellsize)/2;
      board_border_y=(int) (BASE_Y2-hoogte*cellsize)/2;
    }
}

/*
 * Repeat let the user get a help map in 3D mode
 *
 */
static void repeat () {
  GdkPixbuf *pixmap = NULL;

  if(modeIsInvisible) {
    if(mapActive) {
      g_object_set (wallgroup, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      /* Hide the warning */
      g_object_set (warning_item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
      mapActive = FALSE;
    } else {
      g_object_set (wallgroup, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
      /* Display a warning that you can't move there */
      g_object_set (warning_item, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
      mapActive = TRUE;
    }
  }

  if(modeIs2D)
    return;

  if(threeDactive) {

    pixmap = gc_skin_pixmap_load("maze-3d-bubble.png");
    if(pixmap) {
      gc_bar_set_repeat_icon(pixmap);
      gdk_pixbuf_unref(pixmap);
    }
    twoDdisplay();
    /* Display a warning that you can't move there */
    g_object_set (warning_item, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);

  } else {

    pixmap = gc_skin_pixmap_load("maze-2d-bubble.png");
    if(pixmap) {
      gc_bar_set_repeat_icon(pixmap);
      gdk_pixbuf_unref(pixmap);
    }
    g_object_set (warning_item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
    threeDdisplay();
  }

}

/* =====================================================================
 * Destroy all the items
 * =====================================================================*/
static void maze_destroy_all_items() {
  if(boardRootItem!=NULL)
    goo_canvas_item_remove(boardRootItem);

  if (threedgroup!=NULL)
    goo_canvas_item_remove(threedgroup);

  mazegroup = NULL;
  tuxgroup = NULL;
  wallgroup = NULL;
  boardRootItem = NULL;
  threedgroup=NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
static GooCanvasItem *
maze_create_item(GooCanvasItem *parent)
{
  gchar *message;

  boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
					NULL);

  mazegroup = goo_canvas_group_new(boardRootItem,
				   NULL);

  goo_canvas_item_translate(mazegroup,
			    breedte,
			    hoogte);

  tuxgroup = goo_canvas_group_new(mazegroup,
				   NULL);

  wallgroup = goo_canvas_group_new(boardRootItem,
				   NULL);

  if(modeIsInvisible) {
    message = _("Look at your position, then switch back to invisible mode to continue your moves");
  } else {
    message = _("Look at your position, then switch back to 3D mode to continue your moves");
  }

  warning_item = goo_canvas_text_new (boardRootItem,
				      message,
				      (double) BOARDWIDTH/2,
				      (double) BOARDHEIGHT-20,
				      -1,
				      GTK_ANCHOR_CENTER,
				      "font", gc_skin_font_board_small,
				      "fill_color_rgba", gc_skin_color_content,
				      NULL);
  g_object_set (warning_item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);

  return NULL;
}
/* =====================================================================
 *
 * =====================================================================*/
static void game_won() {
  twoDdisplay();
  /* Try the next level */
  gcomprisBoard->level++;
  if(gcomprisBoard->level > gcomprisBoard->maxlevel) { // the current board is finished : bail out
    gc_bonus_end_display(GC_BOARD_FINISHED_RANDOM);
    return;
  }
  gc_sound_play_ogg ("sounds/bonus.wav", NULL);

  maze_next_level();
}
/* =====================================================================
 *
 * =====================================================================*/
static void process_ok() {
}

static void
draw_a_rect(GooCanvasItem *group,
	    int x1, int y1, int x2, int y2,
	    char *color)
{
  goo_canvas_rect_new(group,
		      x1,
		      y1,
		      x2 - x1,
		      y2 - y1,
		      "fill-color", color,
		      "stroke-color", color,
		      "line-width", 1.0,
		      NULL);
}

static void
draw_a_line(GooCanvasItem *group,
	    int x1, int y1, int x2, int y2, guint32 color)
{
  goo_canvas_polyline_new(group, FALSE, 2,
			  (double)x1, (double)y1,
			  (double)x2, (double)y2,
			  "fill_color_rgba", color,
			  "line-width", (double)thickness,
			  NULL);

}

static void
draw_rect(GooCanvasItem *group, int x,int y,char *color)
{
  int x1,y1;
  y1=cellsize*(y)-hoogte + board_border_y;
  x1=cellsize*(x)-breedte + board_border_x;
  draw_a_rect(group,
	      x1+buffer,y1+buffer ,x1+cellsize-buffer,
	      y1+cellsize-buffer,
	      color);
}

/*
 * Same as draw rect but for an image
 * Returns the created item
 */
static GooCanvasItem *
draw_image(GooCanvasItem *group,
	   int x,int y, GdkPixbuf *pixmap)
{
  GooCanvasItem *item = NULL;
  int x1,y1;
  GdkPixbuf *pixmap2;

  y1=cellsize*(y)-hoogte + board_border_y;
  x1=cellsize*(x)-breedte + board_border_x;

  pixmap2 = gdk_pixbuf_scale_simple (pixmap,
				     cellsize-buffer*2,
				     cellsize-buffer*2,
				     GDK_INTERP_BILINEAR);
  item = goo_canvas_image_new (group,
			       pixmap2,
			       x1+buffer,
			       y1+buffer,
			       NULL);
  gdk_pixbuf_unref(pixmap2);

  return(item);
}

/*
 * Same as draw rect but for an image
 */
static void move_image(GooCanvasItem *group,
		       int x, int y, GooCanvasItem *item)
{
  int x1,y1;
  y1=cellsize*(y)-hoogte + board_border_y;
  x1=cellsize*(x)-breedte + board_border_x;

  goo_canvas_item_set_transform(item, NULL);
  goo_canvas_item_translate(item,
  			    x1, y1);

  goo_canvas_item_raise(item, NULL);
}

static void draw_combined_rect(GooCanvasItem *group,
			       int x1,int y1,int x2,int y2,
			       char *color)
{
  int xx1,yy1,xx2,yy2;

  yy1=cellsize*(y1)-hoogte + board_border_y;
  xx1=cellsize*(x1)-breedte + board_border_x;
  yy2=cellsize*(y2)-hoogte + board_border_y;
  xx2=cellsize*(x2)-breedte + board_border_x;

  if (y1==y2 && x1<x2)
    {
      draw_a_rect(group,
		  xx1+cellsize-buffer,
		  yy1+buffer,
		  xx2+buffer,
		  yy2+cellsize-buffer,
		  color);
    }
  else if (y1==y2 && x1>x2)
    {
      draw_a_rect(group,
		  xx2+cellsize-buffer,
		  yy2+buffer,
		  xx1+buffer,
		  yy1+cellsize-buffer,
		  color);
    }
  else if (x1==x2 && y1<y2)
    {
      draw_a_rect(group,
		  xx1+buffer,
		  yy1+cellsize-buffer,
		  xx2+cellsize-buffer,
		  yy2+buffer,
		  color);
    }
  else if (x1==x2 && y1>y2)
    {
      draw_a_rect(group,
		  xx2+buffer,
		  yy2+cellsize-buffer,
		  xx1+cellsize-buffer,
		  yy1+buffer,
		  color);
    }

}

static void initMaze(void)
{
  int x, y;
  for (x=0; x<breedte; x++)
    {
      for (y=0; y <hoogte; y++)
	{
	  Maze[x][y]=15;
	}
    }
}

static int check(int x,int y)
{
  if (Maze[x][y]&SET)
    return 1;
  else return 0;
}

static int* isPossible(int x, int y)
{
  int wall=Maze[x][y];
  static int pos[5];
  wall&=~SET;
  pos[0]=0;
  if(x==0)
    {
      wall&=~WEST;
    }
  if (y==0)
    {
      wall&=~NORTH;
    }
  if(x==(breedte-1))
    {
      wall&=~EAST;
    }
  if (y==(hoogte-1))
    {
      wall&=~SOUTH;
    }
  if (wall&EAST)
    {
      if(check(x+1,y)==0)
	{
	  pos[0]=pos[0]+1;
	  pos[(pos[0])]=EAST;
	}
    }
  if (wall&SOUTH)
    {
      if (check(x,y+1)==0)
	{
	  pos[0]=pos[0]+1;
	  pos[(pos[0])]=SOUTH;
	}
    }
  if (wall&WEST)
    {
      if (check(x-1,y)==0)
	{
	  pos[0]=pos[0]+1;
	  pos[(pos[0])]=WEST;
	}
    }
  if (wall&NORTH)
    {
      if (check(x,y-1)==0)
	{
	  pos[0]=pos[0]+1;
	  pos[(pos[0])]=NORTH;
	}
    }
  return &pos[0];
}

static void generateMaze(int x, int y)
{
  int *po;
  Maze[x][y]= Maze[x][y] + SET;
  po = isPossible(x,y);
  while (*po>0)
    {
      int nr = *po;
      int ran, in;
      in=(g_random_int()%nr)+1;
      //printf("random: %d en %d mogelijkheden\n", in, *po);
      ran=*(po + in);
      if (nr>=1)
	switch (ran)
	  {
	  case EAST:
	    Maze[x][y]&=~EAST;
	    Maze[x+1][y]&=~WEST;
	    generateMaze(x+1,y);
	    break;
	  case SOUTH:
	    Maze[x][y]&=~SOUTH;
	    Maze[x][y+1]&=~NORTH;
	    generateMaze(x,y+1);
	    break;
	  case WEST:
	    Maze[x][y]&=~WEST;
	    Maze[x-1][y]&=~EAST;
	    generateMaze(x-1,y);
	    break;
	  case NORTH:
	    Maze[x][y]&=~NORTH;
	    Maze[x][y-1]&=~SOUTH;
	    generateMaze(x,y-1);
	    break;

	  }
      po=isPossible(x,y);
    }

}

static void removeSet(void)
{
  int x,y;
  for (x=0; x< breedte;x++)
    {
      for (y=0; y < hoogte; y++)
	{
	  Maze[x][y]&=~SET;
	}
    }
}


/* draw the background of the playing board */
static void
draw_background(GooCanvasItem *rootItem)
{
  int x,y,x1,y1;
  int wall;
  /*draw the lines*/
  for (x1=0; x1< breedte; x1++)
    {
      for (y1=0; y1 < hoogte; y1++)
	{
	  wall=Maze[x1][y1];;
	  y=cellsize*(y1)+board_border_y;
	  x=cellsize*(x1)+board_border_x;
	  if (x1==0)
	    draw_a_line(rootItem,
			x, y, x, y+cellsize,
			gc_skin_get_color("maze/wall color"));

	  if (y1==0)
	    draw_a_line(rootItem,
			x, y, x+cellsize, y,
			gc_skin_get_color("maze/wall color"));

	  if (wall&EAST)
	    draw_a_line(rootItem,
			x+cellsize, y, x+cellsize, y+cellsize,
			gc_skin_get_color("maze/wall color"));

	  if (wall&SOUTH)
	    draw_a_line(rootItem,x,
			y+cellsize, x+cellsize, y+cellsize,
			gc_skin_get_color("maze/wall color"));

	}
    }
}

static void movePos(int x1, int y1, int x2,int y2, int richting)
{
  int ret,wall,i,bo=1;
  ret=1;
  wall=Maze[x1][y1];

  if (wall&richting)
    {
      gc_sound_play_ogg ("sounds/brick.wav", NULL);
      ret=0;
    }
  if (ret)
    {
      gc_sound_play_ogg ("sounds/prompt.wav", NULL);
      if (Maze[x2][y2]&SET)
	{
	  for (i=(ind); i>=0 && bo; i--)
	    {

	      if(position[i][0]==x2 && position[i][1]==y2)
		{
		  bo=0;
		  move_image(mazegroup,x2,y2,tuxgroup);
		  //					draw_rect(mazegroup,x2,y2,"blue");
		}
	      else
		{
		  Maze[position[i][0]][position[i][1]]&=~SET;
		  draw_rect(mazegroup, position[i][0], position[i][1], "red");
		  draw_combined_rect(mazegroup,
				     position[i-1][0],position[i-1][1],
				     position[i][0],position[i][1],
				     "red");
		  ind--;
		}


	    }
	}
      else
	{
	  ind++;
	  position[ind][0]=x2;
	  position[ind][1]=y2;
	  Maze[x2][y2]|=SET;
	  if (position[ind][0]==(breedte-1) && position[ind][1]==(end))
	    {
	      gamewon = TRUE;
	      twoDdisplay();
	      gc_bonus_display(gamewon, GC_BONUS_LION);
	    }
	  else
	    {
	      draw_combined_rect(mazegroup, x1, y1, x2, y2, "green");
	      draw_rect(mazegroup,x1,y1,"green");
	      move_image(mazegroup,x2,y2,tuxgroup);
	    }
	}
    }
}

/* return available directions, do not count direction we are coming from
   returns 0 if no or more than one direction is possible */
static guint available_direction(guint last_step)
{	guint number=0,result=0;
 if (last_step!=WEST && !(Maze[position[ind][0]][position[ind][1]]&EAST))
   {  number++; result|=EAST; }
 if (last_step!=EAST && !(Maze[position[ind][0]][position[ind][1]]&WEST))
   {  number++; result|=WEST; }
 if (last_step!=NORTH && !(Maze[position[ind][0]][position[ind][1]]&SOUTH))
   {  number++; result|=SOUTH; }
 if (last_step!=SOUTH && !(Maze[position[ind][0]][position[ind][1]]&NORTH))
   {  number++; result|=NORTH; }
 if (number>1) return 0;
 return result;
}

static void one_step(guint richting)
{
  update_tux(richting);

  switch (richting)
    {
    case WEST: movePos(position[ind][0],position[ind][1],position[ind][0]-1,position[ind][1],richting);
      return;
    case EAST: movePos(position[ind][0],position[ind][1],position[ind][0]+1,position[ind][1],richting);
      return;
    case NORTH: movePos(position[ind][0],position[ind][1],position[ind][0],position[ind][1]-1,richting);
      return;
    case SOUTH: movePos(position[ind][0],position[ind][1],position[ind][0],position[ind][1]+1,richting);
      return;
    }
}

static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str)
{
  guint richting=0,level=gcomprisBoard->level;

  if(board_paused)
    return FALSE;

  if (threeDactive) return key_press_3D(keyval, commit_str, preedit_str);

  if (modeRelative) return key_press_2D_relative(keyval, commit_str, preedit_str);

  switch (keyval)
    {
    case GDK_Left:
      /* When In 3D Mode, can't move tux in the 2D mode */
      if(!modeIs2D || mapActive)
	return TRUE;

      richting=WEST;
      break;
    case GDK_Right:
      /* When In 3D Mode, can't move tux in the 2D mode */
      if(!modeIs2D || mapActive)
	return TRUE;

      richting=EAST;
      break;
    case GDK_Up:
      /* When In 3D Mode, can't move tux in the 2D mode */
      if(!modeIs2D || mapActive)
	return TRUE;

      richting=NORTH;
      break;
    case GDK_Down:
      /* When In 3D Mode, can't move tux in the 2D mode */
      if(!modeIs2D || mapActive)
	return TRUE;

      richting=SOUTH;
      break;
    case GDK_3:
    case GDK_space:
      if(modeIsInvisible) {
	gc_sound_play_ogg ("sounds/flip.wav", NULL);
	if(mapActive) {
	  g_object_set (wallgroup, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
	  /* Hide the warning */
	  g_object_set (warning_item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
	  mapActive = FALSE;
	} else {
	  g_object_set (wallgroup, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
	  /* Display a warning that you can't move there */
	  g_object_set (warning_item, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
	  mapActive = TRUE;
	}
      }

      /* switch to 3D only if allowed in the mode */
      if(!modeIs2D)
	threeDdisplay();
      return TRUE;
    default: return FALSE;
    }
  if (Maze[position[ind][0]][position[ind][1]]&richting)
    {
      gc_sound_play_ogg ("sounds/brick.wav", NULL);
      return TRUE;
    }
  one_step(richting);
  viewing_direction=richting;

  /* run until we come to a fork, (make sure to stop on next level!) */
  while (run_fast && (richting=available_direction(richting))
	 && gcomprisBoard->level==level)
    {
      one_step(richting);
      viewing_direction=richting;
    }
  return TRUE;
}

static gboolean
tux_event (GooCanvasItem *item,
	   GooCanvasItem *target,
	   GdkEventButton *event,
	   gpointer data)
{
  printf("tux_event\n");
  run_fast=!run_fast;
  return FALSE;
}

/*---------- 3D stuff below --------------*/

/* bit magic: rotation = bit rotation */
#define TURN_LEFT(d) ((((d)<<1)|((d)>>3))&15)
#define TURN_RIGHT(d) ((((d)>>1)|((d)<<3))&15)
#define U_TURN(d) ((((d)>>2)|((d)<<2))&15)

static gint key_press_2D_relative(guint keyval, gchar *commit_str, gchar *preedit_str)
{
  guint richting=0,level=gcomprisBoard->level;

  printf("key_press_2D_relative\n");
  switch (keyval)
    {
    case GDK_Left: viewing_direction=TURN_LEFT(viewing_direction);
      gc_sound_play_ogg ("sounds/grow.wav", NULL);
      update_tux(viewing_direction);
      return TRUE;
      break;
    case GDK_Right: viewing_direction=TURN_RIGHT(viewing_direction);
      gc_sound_play_ogg ("sounds/grow.wav", NULL);
      update_tux(viewing_direction);
      return TRUE;
      break;
    case GDK_Up: one_step(viewing_direction);
      break;
    case GDK_Down:
      gc_sound_play_ogg ("sounds/grow.wav", NULL);
      viewing_direction=TURN_RIGHT(viewing_direction);
      viewing_direction=TURN_RIGHT(viewing_direction);
      update_tux(viewing_direction);
      break;
    default: return FALSE;
    }

  richting=viewing_direction;

  /* run until we come to a fork, (make sure to stop on next level!) */
  while (run_fast && (richting=available_direction(richting))
	 && gcomprisBoard->level==level)
    {
      one_step(richting);
      viewing_direction=richting;
    }

  return TRUE;
}

static gint key_press_3D(guint keyval, gchar *commit_str, gchar *preedit_str)
{
  switch (keyval)
    {
    case GDK_Left: viewing_direction=TURN_LEFT(viewing_direction);
      gc_sound_play_ogg ("sounds/grow.wav", NULL);
      break;
    case GDK_Right: viewing_direction=TURN_RIGHT(viewing_direction);
      gc_sound_play_ogg ("sounds/grow.wav", NULL);
      break;
    case GDK_Up: one_step(viewing_direction);
      break;
    case GDK_Down:
      viewing_direction=TURN_RIGHT(viewing_direction);
      viewing_direction=TURN_RIGHT(viewing_direction);
      gc_sound_play_ogg ("sounds/grow.wav", NULL);
      break;
    case GDK_2:
    case GDK_space:
      /* Display a warning that you can't move there */
      g_object_set (warning_item, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
      twoDdisplay();
      return TRUE;
    case GDK_E: case GDK_e: eye_pos_y+=0.1; if (eye_pos_y>0.9) eye_pos_y=0.9; break;
    case GDK_X: case GDK_x: eye_pos_y-=0.1; if (eye_pos_y<-0.9) eye_pos_y=-0.9; break;
    case GDK_D: case GDK_d: eye_pos_x+=0.1; if (eye_pos_x>0.9) eye_pos_x=0.9; break;
    case GDK_S: case GDK_s: eye_pos_x-=0.1; if (eye_pos_x<-0.9) eye_pos_x=-0.9; break;
    case GDK_Y: case GDK_y: case GDK_Z: case GDK_z: eye_pos_z+=0.1; break;
    case GDK_R: case GDK_r: eye_pos_z-=0.1; if (eye_pos_z<-0.9) eye_pos_z=-0.9; break;
    default: return FALSE;
    }
  update_tux(viewing_direction);
  draw3D();
  return TRUE;
}

struct Trapez
{  int x_left,x_right,y_left_top,y_left_bottom,y_right_top,y_right_bottom;
};

static GooCanvasItem *
draw_Trapez(GooCanvasItem *group,
	    struct Trapez t,const char *c1, const char *c2)
{
  GooCanvasPoints *pts=goo_canvas_points_new(4);
  GooCanvasItem *res=NULL;
  pts->coords[0]=t.x_left;
  pts->coords[1]=t.y_left_top;
  pts->coords[2]=t.x_right;
  pts->coords[3]=t.y_right_top;
  pts->coords[4]=t.x_right;
  pts->coords[5]=t.y_right_bottom;
  pts->coords[6]=t.x_left;
  pts->coords[7]=t.y_left_bottom;
  res=goo_canvas_polyline_new(group, FALSE, 0,
			      "points", (GooCanvasPoints*)pts,
			      "fill-color", c1,
			      "stroke-color", c2,
			      "line-width", 1.0,
			      NULL);
  return res;
}

struct vector
{  int x,y;
};

static struct vector
vector_ctor(int x, int y)
{
  struct vector r;
  r.x=x; r.y=y;
  return r;
};

#if 0
static void print_vector(FILE *f, struct vector v)
{  fprintf(f,"(%d;%d)",v.x,v.y); }

static void print_Trapez(FILE *f, struct Trapez t)
{  fprintf(f,"(%d; %d..%d)..(%d; %d..%d)",
	   t.x_left,t.y_left_top,t.y_left_bottom,
	   t.x_right,t.y_right_top,t.y_right_bottom);
}
#endif

static gboolean
is_wall2(struct vector viewpos, int viewdir)
{
  if (viewpos.x<0 || viewpos.y<0 || viewpos.x>=breedte || viewpos.y>=hoogte)
    return TRUE;
  return Maze[viewpos.x][viewpos.y]&viewdir;
}

/*
  (facing north)
  dx.dy:*    *    *    *    *    *
  **-2.1*-1.1**0.1**1.1**2.1**
  *    *    *    *    *    *
  -1.1  0.1  1.1  2.1
  *    *    *    *
  **-1.0**0.0**1.0**
  *    *    *    *
  0.0\/1.0
  * /\ *
  ********
  */

/* rotate a vector by specified amount */
/* this corresponds to multiplying with ( cos a, -sin a )
   ( sin a, cos a  )  */

static struct vector
vector_turn(struct vector v,int angle) /* 1=90deg, 2=180deg */
{
  switch (angle&3)
    {  case 0: return v;
    case 1: return vector_ctor(-v.y,v.x);
    case 2: return vector_ctor(-v.x,-v.y);
    case 3: return vector_ctor(v.y,-v.x);
    }
  return v; // quiet
}

static struct vector
vector_add(struct vector v, struct vector w)
{
  return vector_ctor(v.x+w.x, v.y+w.y);
}

/* returns result in 90° steps ( 1=90° ...) */
static gint
angle(gint a, gint b)
{
  if (a==b) return 0;
  if (a==TURN_LEFT(b)) return 1;
  if (a==U_TURN(b)) return 2;
  return 3;
}

static struct vector
invert_y(struct vector v)
{
  return vector_ctor(v.x, -v.y);
}

/* we have to invert the y component of our view beam because the
   screen and the labyrinth are left handed systems (unlike
   classical vector algebra) */

static gboolean
is_visible(struct vector viewpos, int viewdir,
	   struct vector distance, gboolean left_side, gboolean *is_exit)
{
  struct vector where=vector_add(viewpos,invert_y(vector_turn(distance,angle(viewdir,NORTH))));
  gint direction=left_side ? TURN_LEFT(viewdir) : viewdir;

 if (is_wall2(where,direction))
   return TRUE;
 if ((where.x==breedte-2 && direction==EAST && where.y==end)
     || (where.x==breedte-1
	 && (where.y==end
	     || (direction==NORTH && where.y==end+1)
	     || (direction==SOUTH && where.y==end-1))))
   {
     *is_exit=TRUE;
     return TRUE;
   }
 return FALSE;
}

/* screen coordinate of edge (applied ray interception theorems) */
static int
transform(int s0, int w, int lx, int ly, eyepos_t ex, eyepos_t ez)
{
  return s0
    + w*ex
    + (w*(1+ez)*(2*lx-(1+ex)))/(2*ly+1+ez);
}

/* inverse of transform (lx(sx)) */
/* note: it is possible to use integer arithmetic by returning dividend and divisor separately ... */
static float inverse_transform(int s0, int w, int sx, int ly, eyepos_t ex, eyepos_t ez)
{
  return ((sx-s0-w*ex)*(2*ly+1+ez)
	  + w*(1+ex)*(1+ez)
	  ) / (float)(2*w*(1+ez));
}

struct screenparam
{
  struct vector pos, size; /* middle position, radius */
  struct vector screendist; /* position of (x,y,z)(1,1,-1) onscreen */
  /* which is the eye-screen distance in screen coordinates */
};

/* these calculate the inverted function of wall_coords */

/* leftmost wall which ('s right edge) is visible right of xmin */
/* take left edge, floor (round down) */
static int
dx_left(struct screenparam sp, int xmin, int dy, gboolean left_side)
{
  if (left_side)
    {  if (!dy) return xmin>sp.pos.x-sp.screendist.x /* ?1:0 */ ;
      if (xmin<(sp.pos.x+sp.screendist.x*eye_pos_x))
	return dx_left(sp,xmin,dy,FALSE)+1;
      else return dx_left(sp,xmin,dy-1,FALSE)+1;
    }
  return (int)(floorf(inverse_transform(sp.pos.x, sp.screendist.x, xmin,
					dy, eye_pos_x, eye_pos_z)));
}

/* rightmost wall which ('s left edge) is visible */
/* take right edge, ceil (round up) */
static int
dx_right(struct screenparam sp, int xmax, int dy, gboolean left_side)
{
  if (left_side)
    {  if (!dy) return xmax>sp.pos.x+sp.screendist.x /* ?1:0 */ ;
      if (xmax<(sp.pos.x+sp.screendist.x*eye_pos_x))
	return dx_right(sp,xmax,dy-1,FALSE);
      else return dx_right(sp,xmax,dy,FALSE);
    }
  return (int)(ceilf(inverse_transform(sp.pos.x, sp.screendist.x, xmax,
				       dy, eye_pos_x, eye_pos_z))) - 1;
}

static struct Trapez
wall_coords(struct screenparam sp, struct vector distance, gboolean left_side)
{
  struct Trapez r;

  // leftmost/rightmost wall (special handling)
  if (left_side && !distance.y)
    {  if (distance.x<=0)
	{  r.x_left=sp.pos.x-sp.size.x;
	  r.y_left_top=sp.pos.y-sp.size.y;
	  r.y_left_bottom=sp.pos.y+sp.size.y;
	  r.x_right=sp.pos.x-sp.screendist.x;
	  r.y_right_top=sp.pos.y-sp.screendist.y;
	  r.y_right_bottom=sp.pos.y+sp.screendist.y;
	}
      else
	{  r.x_right=sp.pos.x+sp.size.x;
	  r.y_right_top=sp.pos.y-sp.size.y;
	  r.y_right_bottom=sp.pos.y+sp.size.y;
	  r.x_left=sp.pos.x+sp.screendist.x;
	  r.y_left_top=sp.pos.y-sp.screendist.y;
	  r.y_left_bottom=sp.pos.y+sp.screendist.y;
	}
      goto test;
    }

  r.x_left=transform(sp.pos.x, sp.screendist.x, distance.x, distance.y, eye_pos_x, eye_pos_z);
  // the y sign is inverted (screen coords)
  r.y_left_top=transform(sp.pos.y, sp.screendist.y, 0, distance.y, eye_pos_y, eye_pos_z);
  r.y_left_bottom=transform(sp.pos.y, sp.screendist.y, 1, distance.y, eye_pos_y, eye_pos_z);
  if (left_side)
    {  r.x_right=transform(sp.pos.x, sp.screendist.x, distance.x, distance.y-1, eye_pos_x, eye_pos_z);
      r.y_right_top=transform(sp.pos.y, sp.screendist.y, 0, distance.y-1, eye_pos_y, eye_pos_z);
      r.y_right_bottom=transform(sp.pos.y, sp.screendist.y, 1, distance.y-1, eye_pos_y, eye_pos_z);
      if (distance.x<=0)
	{  // swap coords since more distant edge becomes left
	  int h;
	  h=r.x_left; r.x_left=r.x_right; r.x_right=h;
	  h=r.y_left_top; r.y_left_top=r.y_right_top; r.y_right_top=h;
	  h=r.y_left_bottom; r.y_left_bottom=r.y_right_bottom; r.y_right_bottom=h;
	}
    }
  else // front wall
    {  r.x_right=transform(sp.pos.x, sp.screendist.x, distance.x+1, distance.y, eye_pos_x, eye_pos_z);
      r.y_right_top=r.y_left_top;
      r.y_right_bottom=r.y_left_bottom;
    }
 test:
  g_assert(r.x_left<=r.x_right);
  g_assert(r.y_left_top<=r.y_left_bottom);
  g_assert(r.y_right_top<=r.y_right_bottom);
  return r;
}

static struct Trapez
Trapez_hide(const struct Trapez t, int xmin, int xmax)
{
  struct Trapez r=t;
  if (xmax<xmin) return t;

  if (xmin>t.x_left)
    {  r.x_left=xmin;
      r.y_left_top=t.y_left_top + (xmin-t.x_left)*(t.y_right_top-t.y_left_top)
	/(t.x_right-t.x_left);
      r.y_left_bottom=t.y_left_bottom + (xmin-t.x_left)*(t.y_right_bottom-t.y_left_bottom)
	/(t.x_right-t.x_left);
    }
  if (xmax<t.x_right)
    {  r.x_right=xmax;
      r.y_right_top=t.y_right_top - (t.x_right-xmax)*(t.y_right_top-t.y_left_top)
	/(t.x_right-t.x_left);
      r.y_right_bottom=t.y_right_bottom - (t.x_right-xmax)*(t.y_right_bottom-t.y_left_bottom)
	/(t.x_right-t.x_left);
    }
  g_assert(r.x_left<=r.x_right);
  g_assert(xmin<=r.x_left);
  g_assert(r.x_right<=xmax);
  g_assert(r.y_left_top<=r.y_left_bottom);
  g_assert(r.y_right_top<=r.y_right_bottom);
  return r;
}

static const char *
color(int dir)
{
  if (dir==EAST) return "white";
  if (dir==WEST) return "grey";
  return "light grey";
}

static void
gcDisplay(struct vector position, int viewdir,
	  struct screenparam sp, int xmin, int xmax, int dy, gboolean left_wall)
{
  int dxl=dx_left(sp,xmin,dy,left_wall),
    dxr=dx_right(sp,xmax,dy,left_wall),
    i=0;
  gboolean is_exit=FALSE;

  if (dxl<=0) // seek from the middle left for a wall
    {  if (dxr<i) i=dxr;
      while (i>=dxl && !is_visible(position,viewdir,vector_ctor(i,dy),left_wall,&is_exit))
	--i;
    }
  if (i>=dxl) // wall found
    {  // draw it
      struct Trapez t=Trapez_hide(wall_coords(sp,vector_ctor(i,dy),left_wall),xmin,xmax);
      draw_Trapez(threedgroup,t,is_exit?"green":color(left_wall?TURN_LEFT(viewdir):viewdir),"black");
      // draw left of it
      if (t.x_left-1>=xmin) gcDisplay(position,viewdir,sp,xmin,t.x_left-1,dy,left_wall);
      // right of it ...
      xmin=t.x_right+1;
    }

  i=1;
  is_exit=FALSE;
  if (dxr>=1) // seek from the middle right for a wall
    {  if (dxl>i) i=dxl;
      while (i<=dxr && !is_visible(position,viewdir,vector_ctor(i,dy),left_wall,&is_exit))
	++i;
    }
  if (i<=dxr) // wall found
    {
      struct Trapez t=Trapez_hide(wall_coords(sp,vector_ctor(i,dy),left_wall),xmin,xmax);
      draw_Trapez(threedgroup,t,is_exit?"green":color(left_wall?TURN_RIGHT(viewdir):viewdir),"black");
      // draw right of it
      if (t.x_right+1<xmax)
	gcDisplay(position,viewdir,sp,t.x_right+1,xmax,dy,left_wall);
      // draw right of it
      if (t.x_right+1<xmax)
	gcDisplay(position,viewdir,sp,t.x_right+1,xmax,dy,left_wall);
      // left of it ...
      xmax=t.x_left-1;
    }

  if (xmin<=xmax) // draw in the middle (no wall there)
    gcDisplay(position,viewdir,sp,xmin,xmax,dy+!left_wall,!left_wall);
}

static void
Display3(struct vector position, int viewdir,
	 struct screenparam sp)
{
  gcDisplay(position, viewdir, sp, sp.pos.x-sp.size.x, sp.pos.x+sp.size.x,
	    0, TRUE);
}

static struct screenparam
screenparam_ctor(int px,int py,int sx,int sy,int sdx,int sdy)
{
  struct screenparam r;
  r.pos=vector_ctor(px,py);
  r.size=vector_ctor(sx,sy);
  r.screendist=vector_ctor(sdx,sdy);
  return r;
}

static void
draw3D()
{
#define MAINX 400
#define MAINY 240
#define MAINSX 400
#define MAINSY 240

  if (threedgroup!=NULL)
    {
      goo_canvas_item_remove(threedgroup);
      threedgroup = NULL;
    }
  if (!threeDactive) return;
  threedgroup = goo_canvas_group_new(goo_canvas_get_root_item(gcomprisBoard->canvas),
				     NULL);
  Display3(vector_ctor(position[ind][0],position[ind][1]),viewing_direction,
	   screenparam_ctor(MAINX,MAINY,MAINSX,MAINSY,0.95*MAINSX,0.95*MAINSY));
}

static void
twoDdisplay()
{
  char *fileskin;
  gc_sound_play_ogg ("sounds/flip.wav", NULL);
  fileskin = gc_skin_image_get("gcompris-bg.jpg");
  gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas), fileskin);
  g_free(fileskin);

  if (threedgroup)
    g_object_set (threedgroup, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL);
  g_object_set (boardRootItem, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL);
  threeDactive=FALSE;
}

static void
threeDdisplay()
{
  gc_sound_play_ogg ("sounds/flip.wav", NULL);
  gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
		    "maze/maze-bg.jpg");
  g_object_set (boardRootItem, "visibility",
		GOO_CANVAS_ITEM_INVISIBLE, NULL);
  threeDactive=TRUE;
  draw3D();
}

static void
update_tux(gint direction)
{
  gint rotation = 0;
  GooCanvasBounds bounds;
  gdouble scale;

  /* Our svg image of tux is faced south */
  switch(direction)
    {
    case EAST:
      rotation = -90;
      break;
    case WEST:
      rotation = 90;
      break;
    case NORTH:
      rotation = 180;
      break;
    case SOUTH:
      rotation = 0;
      break;
    }

  goo_canvas_item_set_transform(tuxitem, NULL);

  goo_canvas_item_get_bounds(tuxitem, &bounds);

  scale = (gdouble) cellsize / (bounds.x2 - bounds.x1);
  goo_canvas_item_scale(tuxitem, scale, scale);

  goo_canvas_item_rotate( tuxitem, rotation,
			  (bounds.x2-bounds.x1)/2,
			  (bounds.y2-bounds.y1)/2);
}
