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
#define WON 64
#define MAX_BREEDTE 37
#define MAX_HOOGTE 20
#define BASE_X1 50
#define BASE_Y1 50
#define BASE_X2 800
#define BASE_Y2 500


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
/*-----------------------*/

GcomprisBoard *gcomprisBoard = NULL;
gboolean board_paused = TRUE;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static int gamewon;

static void process_ok(void);
static void game_won(void);
static void repeat(void);

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
static void draw_background(void);
static void setlevelproperties(void);
/*----------------------*/

/* Description of this plugin */
BoardPlugin menu_bp =
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

  if(agcomprisBoard!=NULL) {
      gcomprisBoard=agcomprisBoard;
      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "gcompris/gcompris-bg.jpg");
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=9;
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL);

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

	maze_destroy_all_items();
	gcompris_bar_set_level(gcomprisBoard);
	setlevelproperties();

	gamewon = FALSE;
	initMaze();
	generateMaze((random()%breedte),(random()%hoogte));
	removeSet();	
	/* Try the next level */
	maze_create_item(gnome_canvas_root(gcomprisBoard->canvas));
	draw_background();
	/* make a new group for the items */
	begin=random()%hoogte;
	end=random()%hoogte;

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
	    draw_image(mazegroup,breedte-1,end,pixmap);
	    gdk_pixbuf_unref(pixmap);
	  }

	position[ind][0]=0;
	position[ind][1]=begin;
	Maze[0][begin]=Maze[0][begin]+SET;	
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
static void repeat (){
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
				     "x",(double)breedte,
				     "y",(double)hoogte,
				     NULL));

  return NULL;
}
/* =====================================================================
 *
 * =====================================================================*/
static void game_won() {
	gcompris_play_sound (SOUNDLISTFILE, "bonus");
  /* Try the next level */
	gcomprisBoard->level++;
	if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
		board_finished(BOARD_FINISHED_RANDOM);
		return;
	}
	maze_next_level();
}
/* =====================================================================
 *
 * =====================================================================*/
static void process_ok() {
}

static void
draw_a_rect(GnomeCanvasGroup *group,
	    int x1, int y1, int x2, int y2, char *color)
{
	gnome_canvas_item_new(group,gnome_canvas_rect_get_type(),
			      "x1",(double)x1,
			      "y1",(double)y1,
			      "x2",(double)x2,
			      "y2",(double)y2,
					"fill_color", color,
			      NULL);
}

static void
draw_a_line(GnomeCanvasGroup *group,
	    int x1, int y1, int x2, int y2, char *color)
{
	GnomeCanvasPoints *points;

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

static void draw_rect(GnomeCanvasGroup *group, int x,int y,char *color)
{
	int x1,y1;
	y1=cellsize*(y)-hoogte + board_border_y;
	x1=cellsize*(x)-breedte + board_border_x;
	draw_a_rect(group,x1+buffer,y1+buffer ,x1+cellsize-buffer ,y1+cellsize-buffer ,color);
}

/*
 * Same as draw rect but for an image
 * Returns the created item
 */
static GnomeCanvasItem *draw_image(GnomeCanvasGroup *group, int x,int y, GdkPixbuf *pixmap)
{
	GnomeCanvasItem *item = NULL;
	int x1,y1;

	y1=cellsize*(y)-hoogte + board_border_y;
	x1=cellsize*(x)-breedte + board_border_x;

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

/*
 * Same as draw rect but for an image
 */
static void move_image(GnomeCanvasGroup *group, int x,int y, GnomeCanvasItem *item)
{
	int x1,y1;
	y1=cellsize*(y)-hoogte + board_border_y;
	x1=cellsize*(x)-breedte + board_border_x;

	gnome_canvas_item_set (item,
			       "x",	(double)x1+buffer,
			       "y",	(double)y1+buffer,
			       NULL);
	gnome_canvas_item_raise_to_top(item);
}

static void draw_combined_rect(GnomeCanvasGroup *group, int x1,int y1,int x2,int y2,char *color)
{
	int xx1,yy1,xx2,yy2;
	yy1=cellsize*(y1)-hoogte + board_border_y;
	xx1=cellsize*(x1)-breedte + board_border_x;
	yy2=cellsize*(y2)-hoogte + board_border_y;
	xx2=cellsize*(x2)-breedte + board_border_x;
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

static void initMaze(void)
{
	int x,y;
	for (x=0; x<breedte;x++)
	{
	    for (y=0; y <hoogte; y++)
	    {
			Maze[x][y]=15;
	    }
	}
}

static int check(int x,int y)
{
	if ((Maze[x][y]-SET)>=0)
	    return 1;
	else return 0;
}
    
static int* isPossible(int x, int y)
{
	int wall=Maze[x][y];
	static int pos[5];
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
	if(x==(breedte-1))
	{
	    wall=wall-EAST;
	}
	if (y==(hoogte-1))
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

static void generateMaze(int x, int y)
{
	int *po;
	Maze[x][y]= Maze[x][y] + SET;
	po = isPossible(x,y);
	while (*po>0)
	{
	    int nr = *po;
	    int ran, in;
	    in=(random()%nr)+1;
		//printf("random: %d en %d mogelijkheden\n", in, *po);
	    ran=*(po + in);
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
		if (Maze[x][y]>=16)
		    Maze[x][y]=Maze[x][y]-SET;
	    }
	}
}


/* draw the background of the playing board */
static void
draw_background(void)
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
static void movePos(int x1, int y1, int x2,int y2)
{
	int richting,ret,wall,i,bo=1;
	richting=0;
	ret=1;
	wall=Maze[x1][y1];
	if (x1==x2 && y1>y2)
	{	
		richting=NORTH;
	}
	if (x1==x2 && y1<y2)
	{	
		richting=SOUTH;
	}
	if (x1>x2 && y1==y2)
	{
		richting=WEST;
	}
	if (x1<x2 && y1==y2)
	{
		richting=EAST;
	}
	if((wall-SET)>=0)
	{
		wall=wall-SET;
	}
	if((wall-EAST)>=0)
	{
		wall=wall-EAST;
		if (richting==EAST)
			ret=0;
	}
	if((wall-SOUTH)>=0)
	{
		wall=wall-SOUTH;
		if (richting==SOUTH)
			ret=0;
	}
	if((wall-WEST)>=0)
	{
		wall=wall-WEST;
		if (richting==WEST)
			ret=0;
	}
	if((wall-NORTH)>=0)
	{
		wall=wall-NORTH;
		if (richting==NORTH)
			ret=0;
	}
	if (ret)
	{
		if (Maze[x2][y2]-SET>=0)
		{
			for (i=(ind); i>=0 && bo; i--)
			{

				if(position[i][0]==x2 && position[i][1]==y2)
				{	
					bo=0;
					move_image(mazegroup,x2,y2,tuxitem);
					//					draw_rect(mazegroup,x2,y2,"blue");
				}
				else
				{
					Maze[position[i][0]][position[i][1]]=Maze[position[i][0]][position[i][1]]-SET;
					draw_rect(mazegroup,position[i][0],position[i][1],"red");
					draw_combined_rect(mazegroup,position[i-1][0],position[i-1][1],position[i][0],position[i][1],"red");
					ind--;
				}
				
				
			}
		}
		else
		{
			ind++;
			position[ind][0]=x2;
			position[ind][1]=y2;
			Maze[x2][y2]=Maze[x2][y2]+SET;
			if (position[ind][0]==(breedte-1) && position[ind][1]==(end))
				game_won();
			else
			{
				move_image(mazegroup,x2,y2,tuxitem);
				draw_combined_rect(mazegroup,x1,y1,x2,y2,"green");
				draw_rect(mazegroup,x1,y1,"green");
			}
		}
	}
	
}



gint key_press(guint keyval){

	if ( keyval== GDK_Left)
	{
		movePos(position[ind][0],position[ind][1],position[ind][0]-1,position[ind][1]);
		return TRUE;
	}
	else if (keyval==GDK_Right)
	{
		movePos(position[ind][0],position[ind][1],position[ind][0]+1,position[ind][1]);
		return TRUE;		
	}
	else if (keyval==GDK_Up)
	{
		movePos(position[ind][0],position[ind][1],position[ind][0],position[ind][1]-1);
		return TRUE;		
	}
	else if (keyval==GDK_Down)
	{
		movePos(position[ind][0],position[ind][1],position[ind][0],position[ind][1]+1);
		return TRUE;		
	}
  return FALSE;
}
