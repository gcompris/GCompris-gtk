/* gcompris - maze.c
 *
 * Copyright (C) 2002 Bastiaan Verhoef
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
#include <assert.h>

#include "gcompris/gcompris.h"

#define SOUNDLISTFILE PACKAGE
/*-------------------*/
#define NORTH 1
#define WEST 2
#define SOUTH 4
#define EAST 8
#define SET 16
#define BAD 32
#define DOOR 64
#define MAX_WIDTH 37
#define MAX_HEIGHT 20
#define BASE_X1 50
#define BASE_Y1 50
#define BASE_X2 800
#define BASE_Y2 500

#define GOOD_COLOR	0x10F01040
#define BAD_COLOR	0xFF101040
#define LINE_COLOR	0xFFFFFFFF

/* array of the board */
static int Maze[MAX_WIDTH][MAX_HEIGHT];
/* historyarray */
static int pathhistory[MAX_WIDTH*MAX_HEIGHT][2];
static int p_index=0;

static int begin;
static int end; 
static int width=10;
static int height=20;
static int cellsize=20;
static int buffer=4;
static int board_border_x=20;
static int board_border_y=3;
static int thickness=2;
static int speed=120;
static int don=0;
/*-----------------------*/

GcomprisBoard *gcomprisBoard = NULL;
gboolean board_paused = TRUE;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static gboolean won=FALSE;
static gboolean computer_solving=FALSE;

static void repeat(void);
static void game_won(gboolean computersolve);

/* ================================================================ */
static GnomeCanvasGroup *boardRootItem = NULL;
static GnomeCanvasGroup *mazegroup = NULL;
static GnomeCanvasItem *tuxitem = NULL;

static GnomeCanvasItem *maze_create_item(GnomeCanvasGroup *parent);
static void maze_destroy_all_items(void);
static void maze_next_level(void);
static void set_level (guint level);
gint key_press(guint keyval);
/*--------------------*/
static void draw_a_rect(GnomeCanvasGroup *group, int x1, int y1, int x2, int y2, char *color);
static void draw_a_line(GnomeCanvasGroup *group, int x1, int y1, int x2, int y2, char *color);
static GnomeCanvasItem *draw_image(GnomeCanvasGroup *group, int x,int y, GdkPixbuf *pixmap);
static void move_image(GnomeCanvasGroup *group, int x,int y, GnomeCanvasItem *item);
static void draw_rect(GnomeCanvasGroup *group, int x,int y,char *color);
static void draw_combined_rect(GnomeCanvasGroup *group, int x1, int y1, int x2,int y2, char *color);
static void initMaze(void);
static int check(int x,int y);
static int* isPossible(int x, int y);
static void generateMaze(int x, int y);
static void removeSet(void);
static void draw_maze(void);
static void setlevelproperties(void);
static void solveMaze(void);
static gboolean movePos(int x1, int y1, int x2,int y2,gboolean computersolve);
static int* checkPos(int x, int y);
static gint solvM(GtkWidget *widget, gpointer data);
/*----------------------*/

/* Description of this plugin */
BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    N_("maze"),
    N_("Click on the right color"),
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
    NULL,
    set_level,//set_level,
    NULL,
    repeat
  };

/* =====================================================================
 *
 * =====================================================================*/
BoardPlugin *get_bplugin_info(void) {
  return &menu_bp;
}

/* =====================================================================
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 * =====================================================================*/
static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

//  if(gamewon == TRUE && pause == FALSE) /* the game is won */
//      game_won(FALSE);

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

  if(agcomprisBoard!=NULL) {
      gcomprisBoard=agcomprisBoard;
      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "gcompris/gcompris-bg.jpg");
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=9;
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL|GCOMPRIS_BAR_REPEAT);

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
      gcompris_score_end();
      maze_destroy_all_items();
  }
  gcomprisBoard = NULL;
}

/* =====================================================================
 *
 * =====================================================================*/
gboolean is_our_board (GcomprisBoard *gcomprisBoard) {
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
	if (!computer_solving)
	{
		maze_destroy_all_items();
		gcompris_bar_set_level(gcomprisBoard);
		setlevelproperties();
		
		/* generate maze */
		initMaze();
		generateMaze((random()%width),(random()%height));
		removeSet();	
		
		/* Try the next level */
		maze_create_item(gnome_canvas_root(gcomprisBoard->canvas));
		draw_maze();
		
		/* make a new group for the items */
		begin=random()%height;
		end=random()%height;
		
		/* Draw the tux */
		pixmap = gcompris_load_pixmap("gcompris/misc/tux.png");
		if(pixmap)
		  {
			tuxitem = draw_image(mazegroup,0,begin,pixmap);
			gdk_pixbuf_unref(pixmap);
		  }
		
		/* Draw the target */
		pixmap = gcompris_load_pixmap("gcompris/misc/door.png");
		if(pixmap)
		  {
			draw_image(mazegroup,width-1,end,pixmap);
			gdk_pixbuf_unref(pixmap);
		  }
		pathhistory[p_index][0]=0;
		pathhistory[p_index][1]=begin;
		Maze[0][begin]=Maze[0][begin]+SET;	
		Maze[width-1][end]=Maze[width-1][end]+DOOR;
	  }
}
/* ======================================= */
static void setlevelproperties(){
	if (gcomprisBoard->level==1)
	{
		width=5;
		height=4;
		cellsize=70;
		buffer=8;
		
		board_border_x=(int) (BASE_X2-width*cellsize)/2;
		board_border_y=(int) (BASE_Y2-height*cellsize)/2;
	}
	else if (gcomprisBoard->level==2)
	{
		
		width=9;
		height=6;
		cellsize=70;
		buffer=7;
		board_border_x=(int) (BASE_X2-width*cellsize)/2;
		board_border_y=(int) (BASE_Y2-height*cellsize)/2;		
	}	
	else if (gcomprisBoard->level==3)
	{
		width=13;
		height=8;
		cellsize=60;
		buffer=6;
		board_border_x=(int) (BASE_X2-width*cellsize)/2;
		board_border_y=(int) (BASE_Y2-height*cellsize)/2;		
	}	
	else if (gcomprisBoard->level==4)
	{
		width=17;
		height=10;
		cellsize=45;
		buffer=5;
		board_border_x=(int) (BASE_X2-width*cellsize)/2;
		board_border_y=(int) (BASE_Y2-height*cellsize)/2;		
	}
	else if (gcomprisBoard->level==5)
	{
		width=21;
		height=12;
		cellsize=35;		
		buffer=4;
		board_border_x=(int) (BASE_X2-width*cellsize)/2;
		board_border_y=(int) (BASE_Y2-height*cellsize)/2;		
	}
	else if (gcomprisBoard->level==6)
	{
		width=25;
		height=14;
		cellsize=30;		
		board_border_x=(int) (BASE_X2-width*cellsize)/2;
		board_border_y=(int) (BASE_Y2-height*cellsize)/2;
	}	
	else if (gcomprisBoard->level==7)
	{
		width=29;
		height=16;
		cellsize=25;		
		board_border_x=(int) (BASE_X2-width*cellsize)/2;
		board_border_y=(int) (BASE_Y2-height*cellsize)/2;		
	}	
	else if (gcomprisBoard->level==8)
	{
		width=33;
		height=18;
		cellsize=23;		
		board_border_x=(int) (BASE_X2-width*cellsize)/2;
		board_border_y=(int) (BASE_Y2-height*cellsize)/2;		
	}	
	else if (gcomprisBoard->level==9)
	{
		width=37;
		height=20;
		cellsize=20;		
		board_border_x=(int) (BASE_X2-width*cellsize)/2;
		board_border_y=(int) (BASE_Y2-height*cellsize)/2;		
	}		
}
/* =====================================================================
 * Destroy all the items
 * =====================================================================*/
static void maze_destroy_all_items() {
	if (mazegroup!=NULL)
		gtk_object_destroy (GTK_OBJECT(mazegroup));
	if(boardRootItem!=NULL)
		gtk_object_destroy (GTK_OBJECT(boardRootItem));
	mazegroup = NULL;
	boardRootItem = NULL;
	

}

/* =====================================================================
 *
 * =====================================================================*/
static GnomeCanvasItem *maze_create_item(GnomeCanvasGroup *parent) {
	boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));
	mazegroup=GNOME_CANVAS_GROUP(gnome_canvas_item_new(boardRootItem,
				     gnome_canvas_group_get_type(),
				     "x",(double)width,
				     "y",(double)height,
				     NULL));

  return NULL;
}
/* =====================================================================
 *
 * =====================================================================*/
static void game_won(gboolean computersolve) {
	gcompris_play_sound (SOUNDLISTFILE, "bonus");
  /* Try the next level */
	if (!computersolve)
		gcomprisBoard->level++;
	else computer_solving=FALSE;
	if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
		board_finished(BOARD_FINISHED_RANDOM);
		return;
	}
	maze_next_level();
}
/* =====================================================================
 *
 * =====================================================================*/
static void repeat() {
	if (!computer_solving)
	{
		won=FALSE;
		computer_solving=TRUE;
		don = gtk_timeout_add (speed, (GtkFunction) solvM, NULL);
	}
}

/* =====================================================================
 * Draw a rectangle
 * =====================================================================*/
static void draw_a_rect(GnomeCanvasGroup *group, int x1, int y1, int x2, int y2, char *color)
{
	if (group!=NULL)
		gnome_canvas_item_show(gnome_canvas_item_new(group,gnome_canvas_rect_get_type(),
					  "x1",(double)x1,
					  "y1",(double)y1,
					  "x2",(double)x2,
					  "y2",(double)y2,
						"fill_color", color,
					  NULL));
}

/* =====================================================================
 * Draw a line
 * =====================================================================*/
static void draw_a_line(GnomeCanvasGroup *group, int x1, int y1, int x2, int y2, char *color)
{
	GnomeCanvasPoints *points;
	if (group!=NULL)
	{	
			points = gnome_canvas_points_new (2);
		
			points->coords[0] = x1;
			points->coords[1] = y1;
			points->coords[2] = x2;
			points->coords[3] = y2;
			gnome_canvas_item_new(group,
						  gnome_canvas_line_get_type(),
						  "points", points,
						  "fill_color", color,
						  "width_units", (double)thickness,
						  NULL);
		
			gnome_canvas_points_free(points);
	}
}

/* =====================================================================
 * Draw a rectangle on the right position
 * =====================================================================*/
static void draw_rect(GnomeCanvasGroup *group, int x,int y,char *color)
{
	int x1,y1;
	if (group!=NULL)
	{
		y1=cellsize*(y)-height + board_border_y;
		x1=cellsize*(x)-width + board_border_x;
		draw_a_rect(group,x1+buffer,y1+buffer ,x1+cellsize-buffer ,y1+cellsize-buffer ,color);
	}
}

/* =====================================================================
 * Combines rectangles
 * =====================================================================*/
static void draw_combined_rect(GnomeCanvasGroup *group, int x1,int y1,int x2,int y2,char *color)
{
	int xx1,yy1,xx2,yy2;
	if (group!=NULL)
	{
		yy1=cellsize*(y1)-height + board_border_y;
		xx1=cellsize*(x1)-width + board_border_x;
		yy2=cellsize*(y2)-height + board_border_y;
		xx2=cellsize*(x2)-width + board_border_x;
		if (y1==y2 && x1<x2)
		{
			draw_a_rect(group,xx1+cellsize-buffer,yy1+buffer,xx2+buffer,yy2+cellsize-buffer,color);
		}
		else if (y1==y2 && x1>x2)
		{
			draw_a_rect(group,xx2+cellsize-buffer,yy2+buffer,xx1+buffer,yy1+cellsize-buffer,color);		
		}
		else if (x1==x2 && y1<y2)
		{
			draw_a_rect(group,xx1+buffer,yy1+cellsize-buffer,xx2+cellsize-buffer,yy2+buffer,color);
		}
		else if (x1==x2 && y1>y2)
		{
			draw_a_rect(group,xx2+buffer,yy2+cellsize-buffer,xx1+cellsize-buffer,yy1+buffer,color);
		}
	}

}

/* =====================================================================
 * Draw a image
 * =====================================================================*/
static GnomeCanvasItem *draw_image(GnomeCanvasGroup *group, int x,int y, GdkPixbuf *pixmap)
{
	GnomeCanvasItem *item = NULL;
	int x1,y1;
	if (group!=NULL)
	{
		y1=cellsize*(y)-height + board_border_y;
		x1=cellsize*(x)-width + board_border_x;
	
		item = gnome_canvas_item_new (group,
						  gnome_canvas_pixbuf_get_type (),
						  "pixbuf", pixmap, 
						  "x",	(double)x1+buffer,
						  "y",	(double)y1+buffer,
						  "width",	(double)cellsize-buffer*2,
						  "height",(double)cellsize-buffer*2,
						  "width_set", TRUE, 
						  "height_set", TRUE,
						  NULL);
	
		return(item);
	}
	else return NULL;
}

/*
 * Same as draw rect but for an image
 */
static void move_image(GnomeCanvasGroup *group, int x,int y, GnomeCanvasItem *item)
{
	int x1,y1;
	if (group!=NULL)
	{
		y1=cellsize*(y)-height + board_border_y;
		x1=cellsize*(x)-width + board_border_x;
	
		gnome_canvas_item_set (item,
					   "x",	(double)x1+buffer,
					   "y",	(double)y1+buffer,
					   NULL);
		gnome_canvas_item_raise_to_top(item);
	}
}

/* =====================================================================
 * Init of the mazeboard.
 * set initvalue to 15= NORTH+SOUTH+WEST+EAST
 * So each cell in the bord have four walls
 * =====================================================================*/
static void initMaze(void)
{
	int x,y;
	for (x=0; x<width;x++)
	{
	    for (y=0; y <height; y++)
	    {
			Maze[x][y]=15;
	    }
	}
}

/* =====================================================================
 * Draw the maze
 * =====================================================================*/
static void draw_maze(void)
{
	int x,y,x1,y1;
	int wall;
	/*draw the lines*/
	for (x1=0; x1< width; x1++)
	{
	    for (y1=0; y1 < height; y1++)
	    {
			wall=Maze[x1][y1];;
			y=cellsize*(y1)+board_border_y;
			x=cellsize*(x1)+board_border_x;
			if (x1==0)
				draw_a_line(boardRootItem,x, y, x, y+cellsize, "black");

			if (y1==0)
				draw_a_line(boardRootItem,x, y, x+cellsize, y, "black");
			if ((wall-EAST>=0))
			{
				draw_a_line(boardRootItem,x+cellsize, y, x+cellsize, y+cellsize, "black");			
				wall=wall-EAST;
			}

			if ((wall-SOUTH)>=0)
			{
				draw_a_line(boardRootItem,x, y+cellsize, x+cellsize, y+cellsize, "black");
				wall=wall-SOUTH;
			}

	    }
	}
}

static int check(int x,int y)
{
	if ((Maze[x][y]-DOOR)>=0)
	{
		return 2;
	}
	if ((Maze[x][y]-BAD)>=0)
		return 1;
	if ((Maze[x][y]-SET)>=0)
	    return 1;
	else return 0;
}
/* =====================================================================
 * checks what are the possibilities to move
 * =====================================================================*/    
static int* isPossible(int x, int y)
{
	int wall=Maze[x][y];
	int pos[5];
	wall=wall-SET;
	pos[0]=0;
	if(x==0)
	{
	    wall=wall-WEST;
	}
	if (y==0)
	{
	    wall=wall-NORTH;
	}
	if(x==(width-1))
	{
	    wall=wall-EAST;
	}
	if (y==(height-1))
	{
	    wall=wall-SOUTH;
	}
	if (wall-EAST>=0)
	{
	    if(check(x+1,y)==0)
	    {
			pos[0]=pos[0]+1;
			pos[(pos[0])]=EAST;
	    }
	    wall=wall-EAST;
	}
	if (wall-SOUTH>=0)
	{
	    if (check(x,y+1)==0)
	    {
			pos[0]=pos[0]+1;
			pos[(pos[0])]=SOUTH;
	    }
	    wall=wall-SOUTH;
	}
	if (wall-WEST>=0)
	{
	    if (check(x-1,y)==0)
	    {
			pos[0]=pos[0]+1;
			pos[(pos[0])]=WEST;
	    }
	    wall=wall-WEST;
	}
	if (wall-NORTH>=0)
	{
	    if (check(x,y-1)==0)
	    {
			pos[0]=pos[0]+1;
			pos[(pos[0])]=NORTH;
	    }
	    wall=wall-NORTH;
	}
	return &pos[0];
}

/* =====================================================================
 * Generate a maze
 * =====================================================================*/
static void generateMaze(int x, int y)
{
	int *possible;
	Maze[x][y]= Maze[x][y] + SET;
	possible = isPossible(x,y);
	while (*possible>0)
	{
	    int nr = *possible;
	    int ran, in;
	    in=(random()%nr)+1;
	    ran=*(possible + in);
	    if (nr>=1)
			switch (ran)
			{
			case EAST:
				Maze[x][y]=Maze[x][y]-EAST;
				Maze[x+1][y]=Maze[x+1][y]-WEST;
				generateMaze(x+1,y);
				break;
			case SOUTH:
				Maze[x][y]=Maze[x][y]-SOUTH;
				Maze[x][y+1]=Maze[x][y+1]-NORTH;
				generateMaze(x,y+1);
				break;
			case WEST:
				Maze[x][y]=Maze[x][y]-WEST;
				Maze[x-1][y]=Maze[x-1][y]-EAST;		
				generateMaze(x-1,y);
				break;
			case NORTH:
				Maze[x][y]=Maze[x][y]-NORTH;
				Maze[x][y-1]=Maze[x][y-1]-SOUTH;
				generateMaze(x,y-1);
				break;
			
			}
		possible=isPossible(x,y);
	}

}

static int* checkPos(int x, int y)
{
	int wall=Maze[x][y];
	gboolean found_door=FALSE;
	int pos[5];
	wall=wall-SET;
	pos[0]=0;
	if (wall-EAST>=0)
		wall=wall-EAST;
	else 
	{
		if(check(x+1,y)==0)
		{
			pos[0]=pos[0]+1;
			pos[(pos[0])]=EAST;
		}
		else if(check(x+1,y)==2)
		{
			pos[0]=1;
			pos[1]=EAST;
			found_door=TRUE;
		}		
	}
	if (wall-SOUTH>=0)
	    wall=wall-SOUTH;
	else
	{
	    if (check(x,y+1)==0)
	    {		
			pos[0]=pos[0]+1;
			pos[(pos[0])]=SOUTH;
		}
		else if(check(x,y+1)==2)
		{
			pos[0]=1;
			pos[1]=SOUTH;
			found_door=TRUE;
		}			
	}
	if (wall-WEST>=0)
	    wall=wall-WEST;
	else
	{
		if(check(x-1,y)==0)
		{
			pos[0]=pos[0]+1;
			pos[(pos[0])]=WEST;		
		}
		else if(check(x-1,y)==2)
		{
			pos[0]=1;
			pos[1]=WEST;
			found_door=TRUE;
		}			
	}
	if (wall-NORTH>=0)
	    wall=wall-NORTH;
	else
	{
	    if (check(x,y-1)==0)
		{
			pos[0]=pos[0]+1;
			pos[(pos[0])]=NORTH;
		}
		else if(check(x,y-1)==2)
		{
			pos[0]=1;
			pos[1]=NORTH;
			found_door=TRUE;
		}			
	}
	if (found_door)
		pos[0]=1;
	return &pos[0];
}

static gint solvM(GtkWidget *widget, gpointer data)
{
	solveMaze();
	if (!won && mazegroup!=NULL)
		don = gtk_timeout_add (speed, (GtkFunction) solvM, NULL);	
  return FALSE;
}

/* =====================================================================
 * computersolving algoritme
 * =====================================================================*/
static void solveMaze()
{
	int *possible;
	possible = checkPos(pathhistory[p_index][0],pathhistory[p_index][1]);
	if (*possible>0 && !won)
	{
	    int nr = *possible;
	    int ran, in;
	    in=(random()%nr)+1;
	    ran=*(possible+in);
	    if (nr>=1)
			switch (ran)
			{
			case EAST:

				movePos(pathhistory[p_index][0],pathhistory[p_index][1],pathhistory[p_index][0]+1,pathhistory[p_index][1],TRUE);
				break;
			case SOUTH:
				movePos(pathhistory[p_index][0],pathhistory[p_index][1],pathhistory[p_index][0],pathhistory[p_index][1]+1,TRUE);
				break;
			case WEST:
				movePos(pathhistory[p_index][0],pathhistory[p_index][1],pathhistory[p_index][0]-1,pathhistory[p_index][1],TRUE);
				break;
			case NORTH:
				movePos(pathhistory[p_index][0],pathhistory[p_index][1],pathhistory[p_index][0],pathhistory[p_index][1]-1,TRUE);
				break;
			}
		possible=checkPos(pathhistory[p_index][0],pathhistory[p_index][1]);
	}
	else 
		movePos(pathhistory[p_index][0],pathhistory[p_index][1],pathhistory[p_index-1][0],pathhistory[p_index-1][1],TRUE);
}

static void removeSet(void)
{
	int x,y;
	for (x=0; x< width;x++)
	{
	    for (y=0; y < height; y++)
	    {
		if (Maze[x][y]>=16)
		    Maze[x][y]=Maze[x][y]-SET;
	    }
	}
}

/* =====================================================================
 * move from position (x1,y1) to (x2,y2) if possible
 * =====================================================================*/
static gboolean movePos(int x1, int y1, int x2,int y2, gboolean computersolve)
{
	int direction,wall,i;
	gboolean pos_to_move,found_cycle;
	found_cycle=FALSE;
	direction=0;
	pos_to_move=TRUE;
	wall=Maze[x1][y1];
	/* looking for what is the direction */
	if (x1==x2 && y1>y2)
	{	
		direction=NORTH;
	}
	if (x1==x2 && y1<y2)
	{	
		direction=SOUTH;
	}
	if (x1>x2 && y1==y2)
	{
		direction=WEST;
	}
	if (x1<x2 && y1==y2)
	{
		direction=EAST;
	}
	if((wall-SET)>=0)
	{
		wall=wall-SET;
	}
	
	/* check if move to that direction is possible */
	if((wall-EAST)>=0)
	{
		wall=wall-EAST;
		if (direction==EAST)
			pos_to_move=FALSE;
	}
	if((wall-SOUTH)>=0)
	{
		wall=wall-SOUTH;
		if (direction==SOUTH)
			pos_to_move=FALSE;
	}
	if((wall-WEST)>=0)
	{
		wall=wall-WEST;
		if (direction==WEST)
			pos_to_move=FALSE;
	}
	if((wall-NORTH)>=0)
	{
		wall=wall-NORTH;
		if (direction==NORTH)
			pos_to_move=FALSE;
	}
	/* move position */
	if (pos_to_move)
	{
		if (Maze[x2][y2]-DOOR>=0)
		{
			if (computersolve)
			{
				won=TRUE;
				gtk_timeout_remove(don);
				game_won(computersolve);
			}
			else
				game_won(FALSE);
		}
		else if (Maze[x2][y2]-SET>=0)
		{
			for (i=(p_index); i>=0 && !found_cycle; i--)
			{

				if(pathhistory[i][0]==x2 && pathhistory[i][1]==y2)
				{	
					found_cycle=TRUE;
					move_image(mazegroup,x2,y2,tuxitem);
				}
				else
				{
					Maze[pathhistory[i][0]][pathhistory[i][1]]=Maze[pathhistory[i][0]][pathhistory[i][1]]-SET;
					draw_rect(mazegroup,pathhistory[i][0],pathhistory[i][1],"red");
					Maze[pathhistory[i][0]][pathhistory[i][1]]=Maze[pathhistory[i][0]][pathhistory[i][1]]+BAD;
					draw_combined_rect(mazegroup,pathhistory[i-1][0],pathhistory[i-1][1],pathhistory[i][0],pathhistory[i][1],"red");
					p_index--;
				}
				
				
			}
		}
		else
		{
			p_index++;
			pathhistory[p_index][0]=x2;
			pathhistory[p_index][1]=y2;
			Maze[x2][y2]=Maze[x2][y2]+SET;
			move_image(mazegroup,x2,y2,tuxitem);
			draw_combined_rect(mazegroup,x1,y1,x2,y2,"green");
			draw_rect(mazegroup,x1,y1,"green");
		}
		return TRUE;
	}
	else
		return FALSE;
	
}


gint key_press(guint keyval){

	if ( keyval== GDK_Left)
	{
		if (!computer_solving)
			movePos(pathhistory[p_index][0],pathhistory[p_index][1],pathhistory[p_index][0]-1,pathhistory[p_index][1],FALSE);
		return TRUE;
	}
	else if (keyval==GDK_Right)
	{
		if (!computer_solving)
			movePos(pathhistory[p_index][0],pathhistory[p_index][1],pathhistory[p_index][0]+1,pathhistory[p_index][1],FALSE);
		return TRUE;		
	}
	else if (keyval==GDK_Up)
	{
		if (!computer_solving)
			movePos(pathhistory[p_index][0],pathhistory[p_index][1],pathhistory[p_index][0],pathhistory[p_index][1]-1,FALSE);
		return TRUE;		
	}
	else if (keyval==GDK_Down)
	{
		if (!computer_solving)
			movePos(pathhistory[p_index][0],pathhistory[p_index][1],pathhistory[p_index][0],pathhistory[p_index][1]+1,FALSE);
		return TRUE;		
	}
  return FALSE;
}
