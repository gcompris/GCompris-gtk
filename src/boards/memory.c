/* gcompris - memory.c
 *
 * Time-stamp: <2005/07/01 23:45:39 yves>
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

// FIXME: Cleanup of MemoryItem created struct is not done 

#include <errno.h>

#include "gcompris/gcompris.h"


#define SOUNDLISTFILE PACKAGE
#define MAX_MEMORY_WIDTH  7
#define MAX_MEMORY_HEIGHT 6

static GcomprisBoard *gcomprisBoard = NULL;

static GnomeCanvasGroup *boardRootItem = NULL;

static gint win_id = 0;

typedef enum
{
  MODE_IMAGE		= 0,
  MODE_TEXT		= 1
} Mode;
static Mode currentMode = MODE_IMAGE;

typedef enum
{
  ON_FRONT		= 0,
  ON_BACK		= 1,
  HIDDEN		= 2
} CardStatus;

static gchar *alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

typedef struct {
  char *image;
  char  text[2];
  guint status;
  GnomeCanvasItem *rootItem;
  GnomeCanvasItem *backcardItem;
  GnomeCanvasItem *framecardItem;
  GnomeCanvasItem *frontcardItem;
} MemoryItem;

static MemoryItem *firstCard = NULL;
static MemoryItem *secondCard = NULL;

/* Define the page area where memory cards can be displayed */
#define BASE_X1 50
#define BASE_Y1 50
#define BASE_X2 750
#define BASE_Y2 500

gint current_x;
gint current_y;
gint numberOfLine;
gint numberOfColumn;
gint remainingCards;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void set_level (guint level);

static GnomeCanvasItem *create_item(GnomeCanvasGroup *parent);
static void memory_destroy_all_items(void);
static void memory_next_level(void);
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, MemoryItem *memoryItem);

static void player_win();

static void display_card(MemoryItem *memoryItem, CardStatus cardStatus);

// Number of images for x and y by level
static guint levelDescription[] =
{
  0,0,
  3,2,
  4,2,
  4,3,
  4,4,
  4,4,
  5,4,
  6,4,
  6,5,
  MAX_MEMORY_WIDTH,MAX_MEMORY_HEIGHT
};

static MemoryItem *memoryArray[MAX_MEMORY_WIDTH][MAX_MEMORY_HEIGHT];

// List of images to use in the memory
static gchar *imageList[] =
{
  "gcompris/misc/apple.png",
  "gcompris/misc/bicycle.png",
  "gcompris/misc/bottle.png",
  "gcompris/misc/carot.png",
  "gcompris/misc/car.png",
  "gcompris/misc/castle.png",
  "gcompris/misc/cerise.png",
  "gcompris/misc/cocotier.png",
  "gcompris/misc/crown.png",
  "gcompris/misc/egg.png",
  "gcompris/misc/eggpot.png",
  "gcompris/misc/fishingboat.png",
  "gcompris/misc/flower.png",
  "gcompris/misc/flowerpot.png",
  "gcompris/misc/football.png",
  "gcompris/misc/fusee.png",
  "gcompris/misc/glass.png",
  "gcompris/misc/house.png",
  "gcompris/misc/lamp.png",
  "gcompris/misc/lighthouse.png",
  "gcompris/misc/light.png",
  "gcompris/misc/minivan.png",
  "gcompris/misc/peer.png",
  "gcompris/misc/pencil.png",
  "gcompris/misc/plane.png",
  "gcompris/misc/postcard.png",
  "gcompris/misc/postpoint.png",
  "gcompris/misc/rape.png",
  "gcompris/misc/raquette.png",
  "gcompris/misc/sailingboat.png",
  "gcompris/misc/sapin.png",
  "gcompris/misc/sofa.png",
  "gcompris/misc/star.png",
  "gcompris/misc/strawberry.png",
  "gcompris/misc/tree.png",
  "gcompris/misc/truck.png",
  "gcompris/misc/tuxplane.png",
  "gcompris/misc/tux.png",
  "gcompris/misc/windflag0.png",
  "gcompris/misc/windflag4.png",
  "gcompris/misc/windflag5.png",
};
#define NUMBER_OF_IMAGES 41

/* Description of this plugin */
static BoardPlugin menu_bp =
{
   NULL,
   NULL,
   N_("Memory"),
   N_("Find the matching pair"),
   "Bruno Coudoin <bruno.coudoin@free.fr>",
   NULL,
   NULL,
   NULL,
   NULL,
   start_board,
   pause_board,
   end_board,
   is_our_board,
   NULL,
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

GET_BPLUGIN_INFO(memory)

/*
 * in : boolean TRUE = PAUSE : FALSE = UNPAUSE
 *
 */
static void pause_board (gboolean pause)
{

  if(gcomprisBoard==NULL)
    return;

  if(pause)
    {
    }
  else
    {
    }
}

/*
 */
static void start_board (GcomprisBoard *agcomprisBoard)
{

  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;

      gcompris_set_background(gnome_canvas_root(gcomprisBoard->canvas), "images/scenery_background.png");

      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel = 9;
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL);

      /* Default mode */
      if(!gcomprisBoard->mode)
	currentMode=MODE_IMAGE;
      else if(g_strcasecmp(gcomprisBoard->mode, "image")==0)
	currentMode=MODE_IMAGE;

      memory_next_level();
    }
}

static void
end_board ()
{

  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      memory_destroy_all_items();
    }
  gcomprisBoard = NULL;
}

static void
set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      memory_next_level();
    }
}

static gboolean
is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "memory")==0)
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
static void memory_next_level() 
{
  gcompris_bar_set_level(gcomprisBoard);

  memory_destroy_all_items();

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  numberOfColumn = levelDescription[gcomprisBoard->level*2];
  numberOfLine   = levelDescription[gcomprisBoard->level*2+1];
  remainingCards = numberOfColumn * numberOfLine;

  gcomprisBoard->number_of_sublevel=1;
  gcomprisBoard->sublevel=0;

  create_item(boardRootItem);
}


/* Destroy all the items */
static void memory_destroy_all_items()
{
  gint x, y;

  firstCard = NULL;
  secondCard = NULL;

  if(boardRootItem!=NULL)
      gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem=NULL;

  if (win_id) {
    gtk_timeout_remove (win_id);
  }
  win_id = 0;

  // Clear the memoryArray
  for(x=0; x<MAX_MEMORY_WIDTH; x++)
    for(y=0; y<MAX_MEMORY_HEIGHT; y++)
      {
	g_free(memoryArray[x][y]);
	memoryArray[x][y] = NULL;	
      }
}

/*
 * Takes care to return a random pair of images (one by one)
 * the image is loaded in memoryItem->image
 *
 */
static void get_image(MemoryItem *memoryItem, guint x, guint y)
{
  guint i;
  guint rx, ry;

  if(memoryArray[x][y])
    {
      // Get the pair's image
      memoryItem->image = memoryArray[x][y]->image;
      strcpy((char *)&memoryItem->text, (char *)&memoryArray[x][y]->text);
      memoryArray[x][y] = memoryItem;
      return;
    }

  memoryArray[x][y] = memoryItem;


  switch(gcomprisBoard->level) {

  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
    /* Image mode */
    i = rand()%NUMBER_OF_IMAGES;
    memoryItem->image   = imageList[i];
    memoryItem->text[0] = '\0';
    break;

  case 5:
    /* Limited Text mode Numbers only */
    memoryItem->image    = NULL;
    memoryItem->text[0]  = alphabet[rand()%(strlen(alphabet)-52)];
    memoryItem->text[1]  = '\0';
    break;

  case 6:
    /* Limited Text mode Numbers + Capitals */
    memoryItem->image    = NULL;
    memoryItem->text[0]  = alphabet[rand()%(strlen(alphabet)-26)];
    memoryItem->text[1]  = '\0';
    break;

  default:
    /* Text mode ALL */
    memoryItem->image    = NULL;
    memoryItem->text[0]  = alphabet[rand()%strlen(alphabet)];
    memoryItem->text[1]  = '\0';
  }

  // Randomly set the pair
  rx = (int)(numberOfColumn*((double)rand()/RAND_MAX));
  ry = (int)(numberOfLine*((double)rand()/RAND_MAX));
	
  while(memoryArray[rx][ry])
    {
      rx++;
      // Wrap
      if(rx>=numberOfColumn)
	{
	  rx=0;
	  ry++;
	  if(ry>=numberOfLine)
	    ry=0;
	}
    }
  // Makes the pair point to this memoryItem for now
  memoryArray[rx][ry] = memoryItem;
}

static GnomeCanvasItem *create_item(GnomeCanvasGroup *parent)
{
  MemoryItem *memoryItem;
  gint x, y;
  gint height, width;
  gint height2, width2;
  GdkPixbuf *pixmap = NULL;
  double xratio = 0;
  double yratio = 0;
  double card_shadow_w, card_shadow_h;

  // Calc width and height of one card
  width  = (BASE_X2-BASE_X1)/numberOfColumn;
  height = (BASE_Y2-BASE_Y1)/numberOfLine;

  /* Remove a little bit of space for the card shadow */
  height2 = height * 0.9;
  width2  = width  * 0.9;


  for(x=0; x<numberOfColumn; x++)
    {
      for(y=0; y<numberOfLine; y++)
	{

	  memoryItem = g_malloc(sizeof(MemoryItem));
	  
	  memoryItem->rootItem = \
	    gnome_canvas_item_new (parent,
				   gnome_canvas_group_get_type (),
				   "x", (double) BASE_X1 + x*width,
				   "y", (double) BASE_Y1 + y*height,
				   NULL);
	  
	  pixmap = gcompris_load_pixmap("gcompris/misc/backcard.png");
	  memoryItem->backcardItem = \
	    gnome_canvas_item_new (GNOME_CANVAS_GROUP(memoryItem->rootItem),
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf", pixmap, 
				   "x", (double) 0,
				   "y", (double) 0,
				   "width", (double) width*0.9,
				   "height", (double) height*0.9,
				   "width_set", TRUE, 
				   "height_set", TRUE,
				   NULL);
	  gdk_pixbuf_unref(pixmap);

	  pixmap = gcompris_load_pixmap("gcompris/misc/emptycard.png");
	  memoryItem->framecardItem = \
	    gnome_canvas_item_new (GNOME_CANVAS_GROUP(memoryItem->rootItem),
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf", pixmap, 
				   "x", (double) 0,
				   "y", (double) 0,
				   "width", (double) width*0.9,
				   "height", (double) height*0.9,
				   "width_set", TRUE, 
				   "height_set", TRUE,
				   NULL);
	  gnome_canvas_item_hide(memoryItem->framecardItem);
	  gdk_pixbuf_unref(pixmap);
  

	  // Display the image itself while taking care of its size and maximize the ratio
	  get_image(memoryItem, x, y);

	  if(memoryItem->image) {
	    pixmap = gcompris_load_pixmap(memoryItem->image);

	    yratio=(height2*0.8)/(float)gdk_pixbuf_get_height(pixmap);
	    xratio=(width2*0.8)/(float)gdk_pixbuf_get_width(pixmap);
	    yratio=xratio=MIN(xratio, yratio);
	    card_shadow_w = width*0.07;
	    card_shadow_h = height*0.07;

	    memoryItem->frontcardItem =	\
	      gnome_canvas_item_new (GNOME_CANVAS_GROUP(memoryItem->rootItem),
				     gnome_canvas_pixbuf_get_type (),
				     "pixbuf", pixmap, 
				     "x", (double) ((width*0.9)-
						    gdk_pixbuf_get_width(pixmap)*xratio*0.8)/2 -
				     card_shadow_w,
				     "y", (double) ((height*0.9)-
						    gdk_pixbuf_get_height(pixmap)*yratio*0.8)/2 -
				     card_shadow_h,
				     "width", (double) gdk_pixbuf_get_width(pixmap)*xratio*0.8,
				     "height", (double) gdk_pixbuf_get_height(pixmap)*yratio*0.8,
				     "width_set", TRUE, 
				     "height_set", TRUE,
				     NULL);
	    gdk_pixbuf_unref(pixmap);
  
	  } else {
	    /* It's a letter */
	    memoryItem->frontcardItem =	 \
	      gnome_canvas_item_new (GNOME_CANVAS_GROUP(memoryItem->rootItem),
				     gnome_canvas_text_get_type (),
				     "text", &memoryItem->text,
				     "font", gcompris_skin_font_board_huge_bold,
				     "x", (double) (width*0.8)/2,
				     "y", (double) (height*0.8)/2,
				     "anchor", GTK_ANCHOR_CENTER,
				     "fill_color_rgba", 0x99CDFFFF,
				     NULL);

	  }

	  gnome_canvas_item_hide(memoryItem->frontcardItem);
	  gtk_signal_connect(GTK_OBJECT(memoryItem->rootItem), "event",
			     (GtkSignalFunc) item_event,
			     memoryItem);

	}
    }

  return (NULL);
}

static void player_win()
{
  gcompris_play_ogg ("bonus", NULL);
  /* Try the next level */
  gcomprisBoard->level++;
  if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
    board_finished(BOARD_FINISHED_RANDOM);
    return;
  }
  memory_next_level();
}

static void display_card(MemoryItem *memoryItem, CardStatus cardStatus)
{

  switch (cardStatus)
    {
    case ON_FRONT:
      gnome_canvas_item_hide(memoryItem->backcardItem);
      gnome_canvas_item_show(memoryItem->framecardItem);
      gnome_canvas_item_show(memoryItem->frontcardItem);
      break;
    case ON_BACK:
      gnome_canvas_item_show(memoryItem->backcardItem);
      gnome_canvas_item_hide(memoryItem->framecardItem);
      gnome_canvas_item_hide(memoryItem->frontcardItem);
      break;
    case HIDDEN:
      gnome_canvas_item_hide(memoryItem->backcardItem);
      gnome_canvas_item_hide(memoryItem->framecardItem);
      gnome_canvas_item_hide(memoryItem->frontcardItem);
      break;
    }

}

/*
 * Used to hide card after a timer
 *
 */
static gint hide_card (GtkWidget *widget, gpointer data)
{
  if(firstCard!=NULL)
    {
      display_card(firstCard, HIDDEN);
      firstCard  = NULL;
    }

  if(secondCard!=NULL)
    {
      display_card(secondCard, HIDDEN);
      secondCard  = NULL;
    }
  win_id = 0;

  remainingCards -= 2;
  if(remainingCards<=0)
    player_win();

  return (FALSE);
}

static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, MemoryItem *memoryItem)
{

  if(!gcomprisBoard)
    return FALSE;

   switch (event->type) 
     {
     case GDK_BUTTON_PRESS:
       switch(event->button.button) 
         {
         case 1:
	   if(win_id)
	     return FALSE;

	   if(secondCard)
	     {
	       display_card(firstCard, ON_BACK);
	       firstCard = NULL;
	       display_card(secondCard, ON_BACK);
	       secondCard = NULL;	       
	     }

	   display_card(memoryItem, ON_FRONT);
	   
	   if(!firstCard)
	     {
	       firstCard = memoryItem;
	     }
	   else
	     {
	       // Check he/she did not click on the same card twice
	       if(firstCard==memoryItem)
		 return FALSE;

	       secondCard = memoryItem;

	       // Check win
	       if(firstCard->image && secondCard->image) {
		 /* It's images in both cards */
		 if(strcmp(firstCard->image, secondCard->image)==0)
		   {
		     gcompris_play_ogg ("gobble", NULL);
		     win_id = gtk_timeout_add (1000,
					     (GtkFunction) hide_card, NULL);
		   }
	       } else if (!firstCard->image && !secondCard->image) {
		 /* It's text in both cards */
		  if(strcmp(&firstCard->text, &secondCard->text)==0)
		   {
		     gcompris_play_ogg ("gobble", NULL);
		     win_id = gtk_timeout_add (1000,
					     (GtkFunction) hide_card, NULL);
		   }
	       }
	     }
	   break;
	 default:
	   break;
	 }
     default:
       break;
     }
   return FALSE;
}


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
