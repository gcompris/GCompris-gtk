/* gcompris - algebra.c
 *
 * Time-stamp: <2001/12/02 22:28:51 bruno>
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

#include <ctype.h>

#include "gcompris/gcompris.h"

#define PLUSSIGNFILE '+'
#define MINUSSIGNFILE '-'
#define BYSIGNFILE '*'
#define DIVIDESIGNFILE ':'

/* Some constants for the numbers layout */
#define NUMBERSWIDTH       45
#define NUMBERSHEIGHT      64
#define NUMBERSSPACEHEIGHT NUMBERSHEIGHT

#define SOUNDLISTFILE PACKAGE

static GList *item_list = NULL;

static GcomprisBoard *gcomprisBoard = NULL;

static char *expected_result = NULL;

/* Default layout for the algebra operation */
static gboolean vertical_layout = FALSE;

static gboolean operation_done[10];

typedef struct {
  guint index;
  GnomeCanvasItem *item;
  GnomeCanvasItem *focus_item;
  GnomeCanvasItem *bad_item;
  GdkPixbuf *image;
  char *next;  /* Help : Should point to a ToBeFoundItem but don't know how to recurse on it */
  char *previous;
  char value;
  gboolean in_error;
} ToBeFoundItem;
static ToBeFoundItem *currentToBeFoundItem = NULL;

static GnomeCanvasGroup *boardRootItem = NULL;

static char currentOperation = PLUSSIGNFILE;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void set_level (guint level);
gint key_press(guint keyval);
static void process_ok(void);

static GnomeCanvasItem *algebra_create_item(GnomeCanvasGroup *parent);
static void algebra_destroy_item(GnomeCanvasItem *item);
static void algebra_destroy_all_items(void);
static void display_operand(GnomeCanvasGroup *parent, 
			    double x_align,
			    double y, 
			    char *operand_str,
			    gboolean masked);
static void get_random_number(guint *first_operand, guint *second_operand);
static void algebra_next_level(void);
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static void set_focus_item(ToBeFoundItem *toBeFoundItem, gboolean status);
static void init_operation();

/* Description of this plugin */
BoardPlugin menu_bp =
{
   NULL,
   NULL,
   N_("Learning Clock"),
   N_("Learn how to read the time"),
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
   process_ok,
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
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=9;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=10; /* Go to next level after this number of 'play' */
      gcompris_score_start(SCORESTYLE_NOTE, 
			   gcomprisBoard->width - 220, 
			   gcomprisBoard->height - 50, 
			   gcomprisBoard->number_of_sublevel);
      gcompris_bar_set(GCOMPRIS_BAR_LEVEL|GCOMPRIS_BAR_OK);

      /* The mode defines the operation */
      /* If added with the sign | then the vertical layout is selected */

      /* Default mode */
      if(!gcomprisBoard->mode)
	currentOperation=PLUSSIGNFILE;
      else if(g_strncasecmp(gcomprisBoard->mode, "+", 1)==0)
	currentOperation=PLUSSIGNFILE;
      else if(g_strncasecmp(gcomprisBoard->mode, "-", 1)==0)
	currentOperation=MINUSSIGNFILE;
      else if(g_strncasecmp(gcomprisBoard->mode, "*", 1)==0)
	currentOperation=BYSIGNFILE;
      else if(g_strncasecmp(gcomprisBoard->mode, "/", 1)==0)
	currentOperation=DIVIDESIGNFILE;
      
      if(g_strncasecmp(gcomprisBoard->mode+1, "|", 1)==0)
	 vertical_layout=TRUE;
      else
	vertical_layout=FALSE;

      init_operation();
      algebra_next_level();

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
      algebra_destroy_all_items();
    }
}

static void
set_level (guint level)
{


  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      gcomprisBoard->sublevel=1;
      algebra_next_level();
    }
}

gint key_press(guint keyval)
{
  guint c;
  gboolean stop = FALSE;

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
      process_ok();
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
      if(currentToBeFoundItem->previous!=NULL)
	{
	  set_focus_item(currentToBeFoundItem, FALSE);
	  currentToBeFoundItem = (ToBeFoundItem *)currentToBeFoundItem->previous;
	  set_focus_item(currentToBeFoundItem, TRUE);
	}
      return TRUE;
    case GDK_Left:
      if(currentToBeFoundItem->next!=NULL)
	{
	  set_focus_item(currentToBeFoundItem, FALSE);
	  currentToBeFoundItem = (ToBeFoundItem *)currentToBeFoundItem->next;
	  set_focus_item(currentToBeFoundItem, TRUE);
	}
      return TRUE;
    }

  c = tolower(keyval); 

  if(currentToBeFoundItem!=NULL &&
     keyval>=GDK_0 && keyval<=GDK_9)
    {
      currentToBeFoundItem->image = gcompris_load_number_pixmap(c);
      currentToBeFoundItem->value = c;
      gnome_canvas_item_set (currentToBeFoundItem->item,
			     "pixbuf", currentToBeFoundItem->image,
			     NULL);

      /* Not a failure (yet) */
      gnome_canvas_item_hide(currentToBeFoundItem->bad_item);
      currentToBeFoundItem->in_error = FALSE;

      set_focus_item(currentToBeFoundItem, FALSE);

      /* Move the focus to the next appropriate digit */
      while(!stop)
	{
	  if(vertical_layout)
	    {
	      if(currentToBeFoundItem->next!=NULL)
		currentToBeFoundItem = (ToBeFoundItem *)currentToBeFoundItem->next;
	      else
		stop = TRUE;
	    }
	  else
	    {
	      if(currentToBeFoundItem->previous!=NULL)
		currentToBeFoundItem = (ToBeFoundItem *)currentToBeFoundItem->previous;
	      else
		stop = TRUE;
	    }

	  if(currentToBeFoundItem->in_error==TRUE ||
	     currentToBeFoundItem->value=='?')
	    stop = TRUE;
	}

      set_focus_item(currentToBeFoundItem, TRUE);
    }
  return TRUE;
}



gboolean
is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "algebra")==0)
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
static void algebra_next_level() 
{

  gcompris_bar_set_level(gcomprisBoard);
  gcompris_score_set(gcomprisBoard->sublevel);

  algebra_destroy_all_items();

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  /* Try the next level */
  algebra_create_item(boardRootItem);

}


static void algebra_destroy_item(GnomeCanvasItem *item)
{
  item_list = g_list_remove (item_list, item);
  gtk_object_destroy (GTK_OBJECT(item));
}

/* Destroy all the items */
static void algebra_destroy_all_items()
{
  GnomeCanvasItem *item;
  gboolean stop = FALSE;

  while(g_list_length(item_list)>0) 
    {
      item = g_list_nth_data(item_list, 0);
      algebra_destroy_item(item);
    }

  if(currentToBeFoundItem!=NULL)
    {
      /* Move toBeFoundItem to the most next digit */
      while(!stop)
	{
	  if(currentToBeFoundItem->next!=NULL)
	    currentToBeFoundItem = (ToBeFoundItem *)currentToBeFoundItem->next;
	  else
	    stop = TRUE;
	}
      
      /* Now free toBeFoundItems */
      while(!stop)
	{
	  if(currentToBeFoundItem->previous!=NULL)
	    {
	      currentToBeFoundItem = (ToBeFoundItem *)currentToBeFoundItem->previous;
	      free(currentToBeFoundItem->next);
	    }
	  else
	    stop = TRUE;
	}
    }

  if(boardRootItem!=NULL)
    gtk_object_destroy (GTK_OBJECT(boardRootItem));
  
  boardRootItem=NULL;
}

static void display_operand(GnomeCanvasGroup *parent, 
			    double x_align,
			    double y, 
			    char *operand_str,
			    gboolean masked)
{
  GdkPixbuf *algebra_pixmap = NULL;
  GnomeCanvasItem *item, *focus_item = NULL, *bad_item = NULL;
  int i;
  ToBeFoundItem *toBeFoundItem=NULL;
  ToBeFoundItem *previousToBeFoundItem=NULL;;

  for(i=strlen(operand_str)-1; i>=0; i--)
    {
      if(masked) 
	{
	  algebra_pixmap = gcompris_load_number_pixmap('?');
	} 
      else 
	{
	  algebra_pixmap = gcompris_load_number_pixmap(operand_str[i]);
	}

      item = gnome_canvas_item_new (parent,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", algebra_pixmap, 
				    "x", x_align-((strlen(operand_str)-i)*NUMBERSWIDTH)
				    +(NUMBERSWIDTH-gdk_pixbuf_get_width(algebra_pixmap))/2-
				    gdk_pixbuf_get_width(algebra_pixmap)/2,
				    "y", y - gdk_pixbuf_get_height(algebra_pixmap)/2,
				    "width", (double) gdk_pixbuf_get_width(algebra_pixmap),
				    "height", (double) gdk_pixbuf_get_height(algebra_pixmap),
				    NULL);
      gdk_pixbuf_unref(algebra_pixmap);
      item_list = g_list_append (item_list, item);

      if(masked) 
	{
	  focus_item = gnome_canvas_item_new (parent,
					      gnome_canvas_rect_get_type (),
					      "x1",  x_align-((strlen(operand_str)-i)*NUMBERSWIDTH)
					      +(NUMBERSWIDTH-gdk_pixbuf_get_width(algebra_pixmap))/2 + 5
					      - (gdk_pixbuf_get_width(algebra_pixmap))/2,
					      "y1", (double)y + (gdk_pixbuf_get_height(algebra_pixmap))/2,
					      "x2", (double)x_align-((strlen(operand_str)-i)*NUMBERSWIDTH)
					      +(NUMBERSWIDTH-gdk_pixbuf_get_width(algebra_pixmap))/2 + 
					      gdk_pixbuf_get_width(algebra_pixmap) - 5
					      - (gdk_pixbuf_get_width(algebra_pixmap))/2,
					      "y2", (double)y + (gdk_pixbuf_get_height(algebra_pixmap))/2 + 5,
					      "fill_color", "black",
					      "outline_color", "blue",
					      "width_units", (double)2,
					      NULL);

	  item_list = g_list_append (item_list, focus_item);

	  algebra_pixmap = gcompris_load_pixmap("gcompris/buttons/bad.png");
	  bad_item = gnome_canvas_item_new (parent,
					    gnome_canvas_pixbuf_get_type (),
					    "pixbuf", algebra_pixmap, 
					    "x", x_align-((strlen(operand_str)-i)*NUMBERSWIDTH)
					    +(NUMBERSWIDTH-gdk_pixbuf_get_width(algebra_pixmap))/2 
					    -gdk_pixbuf_get_width(algebra_pixmap)/2,
					    "y", y -gdk_pixbuf_get_height(algebra_pixmap)/2,
					    "width", (double) gdk_pixbuf_get_width(algebra_pixmap),
					    "height", (double) gdk_pixbuf_get_height(algebra_pixmap),
					    NULL);
	  gdk_pixbuf_unref(algebra_pixmap);
	  gnome_canvas_item_hide(bad_item);

	  item_list = g_list_append (item_list, bad_item);

	}

      if(masked) 
	{
	  toBeFoundItem = malloc(sizeof(ToBeFoundItem));
	  toBeFoundItem->bad_item=bad_item;
	  toBeFoundItem->index=i;
	  toBeFoundItem->value='?';
	  toBeFoundItem->image=algebra_pixmap;
	  toBeFoundItem->item=item;
	  toBeFoundItem->focus_item=focus_item;
	  toBeFoundItem->previous=(char *)previousToBeFoundItem;
	  toBeFoundItem->next=NULL;

	  /* I Create a double linked list with the toBeFoundItem in order to navigate through them */
	  if(previousToBeFoundItem!=NULL)
	    previousToBeFoundItem->next=(char *)toBeFoundItem;

	  previousToBeFoundItem=toBeFoundItem;

	  /* Init the focussed item on the right digit depending on vertical_layout */
	  if(i==(vertical_layout?strlen(operand_str)-1:0))
	    {
	      currentToBeFoundItem=toBeFoundItem;
	      set_focus_item(toBeFoundItem, TRUE);
	    }
	  else
	    {
	      set_focus_item(toBeFoundItem, FALSE);
	    }
	  gtk_signal_connect(GTK_OBJECT(item), "event",
			     (GtkSignalFunc) item_event,
			     toBeFoundItem);
	}
    }
}

static GnomeCanvasItem *algebra_create_item(GnomeCanvasGroup *parent)
{
  GdkPixbuf *algebra_pixmap = NULL;
  GnomeCanvasItem *item;
  guint first_operand, second_operand;
  char *first_operand_str = NULL;
  char *second_operand_str = NULL;
  guint longuest;
  double x_align;
  double x, y;
  double y_firstline = 0;



  /* Some globals for this board layout */
  y_firstline = gcomprisBoard->height/3;


  get_random_number(&first_operand, &second_operand);
  first_operand_str = g_strdup_printf("%d", first_operand);

  second_operand_str = g_strdup_printf("%d", second_operand);

  /* Calc the longuest value */
  longuest = MAX(strlen(first_operand_str), strlen(second_operand_str));

  if(vertical_layout)
    /* Vertical layout : Warning x_align is the right assigned value for display_operand */
    x_align = gcomprisBoard->width - (gcomprisBoard->width - (longuest*NUMBERSWIDTH))/2;
  else
    x_align = (gcomprisBoard->width - (longuest*3*NUMBERSWIDTH))/2 + NUMBERSWIDTH*(strlen(first_operand_str));

  /* First operand */
  display_operand(parent, x_align, y_firstline, first_operand_str, FALSE);

  /* Second operand */
  if(vertical_layout)
      display_operand(parent, x_align, y_firstline + NUMBERSSPACEHEIGHT, second_operand_str, FALSE);
  else
      display_operand(parent, x_align + NUMBERSWIDTH*(strlen(second_operand_str)+1),
		      y_firstline, second_operand_str, FALSE);

  /* Display the operator */
  algebra_pixmap = gcompris_load_operation_pixmap(currentOperation);
  if(vertical_layout)
    {
      x=(double) gcomprisBoard->width - x_align - NUMBERSWIDTH - 20;
      y=(double) y_firstline + NUMBERSSPACEHEIGHT;
    }
  else
    {
      x=(double) x_align;
      y=(double) y_firstline;
    }
  item = gnome_canvas_item_new (parent,
				gnome_canvas_pixbuf_get_type (),
				"pixbuf", algebra_pixmap, 
				"x", x -  gdk_pixbuf_get_width(algebra_pixmap)/2,
				"y", y -  gdk_pixbuf_get_height(algebra_pixmap)/2,
				"width", (double)  gdk_pixbuf_get_width(algebra_pixmap),
				"height", (double) gdk_pixbuf_get_height(algebra_pixmap),
				NULL);
  item_list = g_list_append (item_list, item);

  /* Now the separator line or the equal sign*/
  if(vertical_layout)
    {
      item = gnome_canvas_item_new (parent,
				    gnome_canvas_rect_get_type (),
				    "x1", (double) gcomprisBoard->width - x_align - NUMBERSWIDTH - 20,
				    "y1", (double) y_firstline + NUMBERSSPACEHEIGHT + NUMBERSSPACEHEIGHT/2,
				    "x2", (double) x_align+(NUMBERSWIDTH-gdk_pixbuf_get_width(algebra_pixmap))/2,
				    "y2", (double)  y_firstline + NUMBERSSPACEHEIGHT + NUMBERSSPACEHEIGHT/2 + 5,
				    "fill_color", "blue",
				    "outline_color", "green",
				    "width_units", (double)1,
				    NULL);
    }
  else
    {
      algebra_pixmap = gcompris_load_operation_pixmap('=');
      item = gnome_canvas_item_new (parent,
				    gnome_canvas_pixbuf_get_type (),
				    "pixbuf", algebra_pixmap, 
				    "x", (double) x_align + NUMBERSWIDTH*(strlen(second_operand_str)+1)
				    - gdk_pixbuf_get_width(algebra_pixmap)/2,
				    "y", (double) y_firstline - gdk_pixbuf_get_height(algebra_pixmap)/2,
				    "width", (double)  gdk_pixbuf_get_width(algebra_pixmap),
				    "height", (double) gdk_pixbuf_get_height(algebra_pixmap),
				    NULL);
    }
  item_list = g_list_append (item_list, item);

  /* Display the empty area */
  g_free(expected_result);
  switch(currentOperation)
    {
    case PLUSSIGNFILE:
      expected_result = g_strdup_printf("%d", first_operand+second_operand);
      break;
    case MINUSSIGNFILE:
      expected_result = g_strdup_printf("%d", first_operand-second_operand);
      break;
    case BYSIGNFILE:
      expected_result = g_strdup_printf("%d", first_operand*second_operand);
      break;
    case DIVIDESIGNFILE:
      expected_result = g_strdup_printf("%d", first_operand/second_operand);
      break;
    default:
      g_error("Bad Operation");
    }
  if(vertical_layout)
    display_operand(parent, x_align, y_firstline + NUMBERSSPACEHEIGHT*2 + 10, expected_result, TRUE);
  else
    display_operand(parent, x_align + NUMBERSWIDTH*(strlen(second_operand_str)+
						    strlen(expected_result)+2),
		    y_firstline, expected_result, TRUE);

  g_free(first_operand_str);
  g_free(second_operand_str);

  return (item);
}

static void set_focus_item(ToBeFoundItem *toBeFoundItem, gboolean status)
{
  if(status)
    {
      gnome_canvas_item_set (toBeFoundItem->focus_item,
			     "fill_color", "blue",
			     "outline_color", "green",
			     NULL);
    }
  else
    {
      gnome_canvas_item_set (toBeFoundItem->focus_item,
			     "fill_color", NULL,
			     "outline_color", NULL,
			     NULL);
    }
}

static void process_ok()
{
  ToBeFoundItem *toBeFoundItem;
  ToBeFoundItem *hasfail=NULL;

  set_focus_item(currentToBeFoundItem, FALSE);
  /* Go to the rightmost digit */
  while(currentToBeFoundItem->previous!=NULL)
    {
      currentToBeFoundItem=(ToBeFoundItem *)currentToBeFoundItem->previous;
    }
  
  toBeFoundItem=currentToBeFoundItem;
  /* Check the numbers one by one and redline errors */
  while(currentToBeFoundItem!=NULL)
    {
      if(currentToBeFoundItem->value!=expected_result[currentToBeFoundItem->index])
	{
	  gnome_canvas_item_show(currentToBeFoundItem->bad_item);
	  currentToBeFoundItem->in_error = TRUE;
	  /* Depending on vertical_layout, remember the appropriate digit to focus next */
	  if(vertical_layout && hasfail==NULL)
	    hasfail=currentToBeFoundItem;
	  else
	    hasfail=currentToBeFoundItem;
	}
      currentToBeFoundItem=(ToBeFoundItem *)currentToBeFoundItem->next;
    }
  currentToBeFoundItem=toBeFoundItem;
  
  if(hasfail==NULL)
    {
      gcomprisBoard->sublevel++;

      if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel) {
	/* Try the next level */
	gcomprisBoard->level++;
	if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
	  board_finished(BOARD_FINISHED_RANDOM);
	  return;
	}

	gcomprisBoard->sublevel=1;
	init_operation();
	gcompris_play_sound (SOUNDLISTFILE, "bonus");
      } else {
	gcompris_play_sound (SOUNDLISTFILE, "gobble");
      }

      algebra_next_level();
    }
  else
    {
      set_focus_item(hasfail, TRUE);
      currentToBeFoundItem=hasfail;
    }
}

/* Callback for the 'toBeFoundItem' */
static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  ToBeFoundItem *toBeFoundItem;

  toBeFoundItem=(ToBeFoundItem *)data;

  switch (event->type) 
    {
    case GDK_ENTER_NOTIFY:
      break;
    case GDK_LEAVE_NOTIFY:

      break;
    case GDK_BUTTON_PRESS:
      /* Remove focus from previous selection */
      set_focus_item(currentToBeFoundItem, FALSE);

      /* Set focus on current selection */
      set_focus_item(toBeFoundItem, TRUE);

      currentToBeFoundItem=toBeFoundItem;

      gcompris_play_sound (SOUNDLISTFILE, "gobble");
      break;
      
    default:
      break;
    }
  return FALSE;
}

/*
 * Return the next non already chosed operand
 *
 */
static gint get_operand()
{
  gint j = 10;
  gint i = (1+rand()%10);
  
  // Get the next free slot
  while(operation_done[i]==TRUE && j>0)
    {
      j--;
      i++;
      if(i>9)
	i=0;
    }
  operation_done[i]=TRUE;

  return i;
}

static void init_operation()
{
  int i;

  for(i=0; i<10; i++)
    operation_done[i] = FALSE;
}

/* Returns a random int based on the current level */
static void get_random_number(guint *first_operand, guint *second_operand)
{
  guint min, max;

  switch(currentOperation)
    {
    case PLUSSIGNFILE:
      *first_operand = get_operand();
      *second_operand  = gcomprisBoard->level;
      break;
    case BYSIGNFILE:
      *first_operand  = gcomprisBoard->level;
      *second_operand = get_operand();
      break;
    case MINUSSIGNFILE:
      *first_operand  = gcomprisBoard->level+9;
      *second_operand = get_operand();
      break;
    case DIVIDESIGNFILE:
      switch(gcomprisBoard->level)
	{
	case 1 :
	  min = 1;
	  max = 2;
	  break;
	case 2 :
	  min = 2;
	  max = 3;
	  break;
	case 3 :
	  min = 4;
	  max = 5;
	  break;
	case 4 :
	  min = 6;
	  max = 7;
	  break;
	case 5 :
	  min = 8;
	  max = 9;
	  break;
	case 6 :
	  min = 1;
	  max = 10;
	  break;
	default :
	  min = 1;
	  max = 10;
	}
      *second_operand = (min+rand()%(max-min+1));
      *first_operand  = *second_operand*(min+rand()%max);
      break;
    default:
      g_error("Bad Operation");
    }
}




/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
