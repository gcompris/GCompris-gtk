/* gcompris - algebra.c
 *
 * Time-stamp: <2006/08/21 23:29:57 bruno>
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
#include <string.h>

#include "gcompris/gcompris.h"

#define TIMER_X 400
#define TIMER_Y 140

#define PLUSSIGNFILE   '+'
#define MINUSSIGNFILE  '-'
#define BYSIGNFILE     'x'
#define DIVIDESIGNFILE ':'

static gchar *operators;
static gchar *op_add;
static gchar *op_minus;
static gchar *op_mult;
static gchar *op_div;

/* Some constants for the numbers layout */
#define NUMBERSWIDTH       45

#define SOUNDLISTFILE PACKAGE

static gboolean	 board_paused = TRUE;
static int	 leavenow;
static int	 gamewon;
static void	 game_won(void);

static int	 maxtime;

static GList *item_list = NULL;

static GcomprisBoard *gcomprisBoard = NULL;

static char *expected_result = NULL;

static gboolean operation_done[11];

typedef struct _ToBeFoundItem ToBeFoundItem;

struct _ToBeFoundItem{
  guint index;
  GnomeCanvasItem *item;
  GnomeCanvasItem *focus_item;
  GnomeCanvasItem *bad_item;
  ToBeFoundItem *next;
  ToBeFoundItem *previous;
  char value;
  gboolean in_error;
};
static ToBeFoundItem *currentToBeFoundItem = NULL;

static GnomeCanvasGroup *boardRootItem = NULL;

static char currentOperation[2];
static gchar *currentOperationText;

static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void		 end_board (void);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static void		 set_level (guint level);
static gint		 key_press(guint keyval, gchar *commit_str, gchar *preedit_str);
static void		 process_ok(void);

static GnomeCanvasItem	*algebra_create_item(GnomeCanvasGroup *parent);
static void		 algebra_destroy_item(GnomeCanvasItem *item);
static void		 algebra_destroy_all_items(void);
static void		 display_operand(GnomeCanvasGroup *parent,
					 double x_align,
					 double y,
					 char *operand_str,
					 gboolean masked);
static void		 get_random_number(guint *first_operand, guint *second_operand);
static void		 algebra_next_level(void);
static gint		 item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data);
static void		 set_focus_item(ToBeFoundItem *toBeFoundItem, gboolean status);
static void		 init_operation(void);

/* Description of this plugin */
static BoardPlugin menu_bp =
{
   NULL,
   NULL,
   "Practice the math operation",
   "Answer some algebra questions",
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
   NULL,
   NULL,
   NULL
};

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(algebra)

/*
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 *
 */
static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  /* Make the timer follow our pause status */
  gc_timer_pause(pause);

  if(gamewon == TRUE && pause == FALSE) /* the game is won */
      algebra_next_level();

  if(leavenow == TRUE && pause == FALSE)
    gc_bonus_end_display(GC_BOARD_FINISHED_RANDOM);

  board_paused = pause;
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

      gc_set_background(gnome_canvas_root(gcomprisBoard->canvas), "opt/scenery2_background.png");

      /* set initial values for this level */
      gcomprisBoard->level=1;
      gcomprisBoard->maxlevel=9;
      gcomprisBoard->sublevel=1;
      gcomprisBoard->number_of_sublevel=10; /* Go to next level after this number of 'play' */
      gc_score_start(SCORESTYLE_NOTE,
			   gcomprisBoard->width - 220,
			   gcomprisBoard->height - 50,
			   gcomprisBoard->number_of_sublevel);
      gc_bar_set(GC_BAR_LEVEL|GC_BAR_OK);

      /* TRANSLATORS: Put here the mathematical operators '+-x/' for  your language. */
      operators=_("+-×÷");
      g_assert(g_utf8_validate(operators,-1,NULL)); // require by all utf8-functions
      g_warning("Using operators %s", operators);

      op_add = g_malloc0(2*sizeof(gunichar));
      g_utf8_strncpy(op_add, g_utf8_offset_to_pointer (operators,0),1);

      op_minus = g_malloc0(2*sizeof(gunichar));
      g_utf8_strncpy(op_minus, g_utf8_offset_to_pointer (operators,1),1);

      op_mult = g_malloc0(2*sizeof(gunichar));
      g_utf8_strncpy(op_mult, g_utf8_offset_to_pointer (operators,2),1);

      op_div = g_malloc0(2*sizeof(gunichar));
      g_utf8_strncpy(op_div, g_utf8_offset_to_pointer (operators,3),1);

      /* The mode defines the operation */

      /* Default mode */
      if(!gcomprisBoard->mode)
	{
	  currentOperation[0]=PLUSSIGNFILE;
	  currentOperationText = op_add;
	}
      else if(g_strncasecmp(gcomprisBoard->mode, "+", 1)==0)
	{
	  currentOperation[0]=PLUSSIGNFILE;
	  currentOperationText = op_add;
	}
      else if(g_strncasecmp(gcomprisBoard->mode, "-", 1)==0)
	{
	  currentOperation[0]=MINUSSIGNFILE;
	  currentOperationText = op_minus;
	}
      else if(g_strncasecmp(gcomprisBoard->mode, "*", 1)==0)
	{
	  currentOperation[0]=BYSIGNFILE;
	  currentOperationText = op_mult;
	}
      else if(g_strncasecmp(gcomprisBoard->mode, "/", 1)==0)
	{
	  currentOperation[0]=DIVIDESIGNFILE;
	  currentOperationText = op_div;
	}
      currentOperation[1]='\0';

      init_operation();
      algebra_next_level();

      gamewon = FALSE;
      leavenow = FALSE;
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
      algebra_destroy_all_items();
    }
  gcomprisBoard = NULL;

  g_free(op_add);
  g_free(op_minus);
  g_free(op_mult);
  g_free(op_div);
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

static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str)
{
  guint c;
  gboolean stop = FALSE;

  if(!boardRootItem)
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
    case GDK_Right:
      if(currentToBeFoundItem->previous!=NULL)
	{
	  set_focus_item(currentToBeFoundItem, FALSE);
	  currentToBeFoundItem = (ToBeFoundItem *)currentToBeFoundItem->previous;
	  set_focus_item(currentToBeFoundItem, TRUE);
	}
      return TRUE;
    case GDK_Delete:
    case GDK_BackSpace:
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
      char number[2];
      number[1] = '\0';
      number[0] = (char)c;

      currentToBeFoundItem->value = c;
      gnome_canvas_item_set (currentToBeFoundItem->item,
			     "text", (char *)&number,
			     NULL);

      /* Not a failure (yet) */
      gnome_canvas_item_hide(currentToBeFoundItem->bad_item);
      currentToBeFoundItem->in_error = FALSE;

      set_focus_item(currentToBeFoundItem, FALSE);

      /* Move the focus to the next appropriate digit */
      while(!stop)
	{
	  if(currentToBeFoundItem->previous!=NULL)
	    currentToBeFoundItem = (ToBeFoundItem *)currentToBeFoundItem->previous;
	  else
	    stop = TRUE;

	  if(currentToBeFoundItem->in_error==TRUE ||
	     currentToBeFoundItem->value=='?')
	    stop = TRUE;
	}

      set_focus_item(currentToBeFoundItem, TRUE);
    }
  return TRUE;
}



static gboolean
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

static void timer_end()
{
  gamewon = FALSE;
  leavenow = TRUE;
  algebra_destroy_all_items();
  gc_bonus_display(gamewon, GC_BONUS_SMILEY);
}


/* set initial values for the next level */
static void algebra_next_level()
{

  gc_bar_set_level(gcomprisBoard);
  gc_score_set(gcomprisBoard->sublevel);

  algebra_destroy_all_items();

  boardRootItem = GNOME_CANVAS_GROUP(
				     gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
							    gnome_canvas_group_get_type (),
							    "x", (double) 0,
							    "y", (double) 0,
							    NULL));

  maxtime = 20;
  gc_timer_display(TIMER_X, TIMER_Y,
			 GCOMPRIS_TIMER_BALLOON, maxtime, timer_end);

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
  ToBeFoundItem *next;

  gc_timer_end();

  while(g_list_length(item_list)>0)
    {
      item = g_list_nth_data(item_list, 0);
      algebra_destroy_item(item);
    }

  while(currentToBeFoundItem)
  {
      next = currentToBeFoundItem -> next;
      g_free(currentToBeFoundItem);
      currentToBeFoundItem = next;
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
  GnomeCanvasItem *item, *focus_item = NULL, *bad_item = NULL;
  int i;
  ToBeFoundItem *toBeFoundItem=NULL;
  ToBeFoundItem *previousToBeFoundItem=NULL;;

  for(i=strlen(operand_str)-1; i>=0; i--)
    {
      char operand[2] = "?";

      if(!masked)
	operand[0] = operand_str[i];

      item = gnome_canvas_item_new (parent,
				    gnome_canvas_text_get_type (),
				    "text", &operand,
				    "font", gc_skin_font_board_huge_bold,
				    "anchor", GTK_ANCHOR_CENTER,
				    "x", x_align-((strlen(operand_str)-i)*NUMBERSWIDTH),
				    "y", y,
				    "fill_color_rgba", 0x2c2cFFFF,
				    NULL);
      item_list = g_list_append (item_list, item);

      if(masked)
	{
	  focus_item = gnome_canvas_item_new (parent,
					      gnome_canvas_text_get_type (),
					      "text", "_",
					      "font", gc_skin_font_board_huge_bold,
					      "anchor", GTK_ANCHOR_CENTER,
					      "x", x_align-((strlen(operand_str)-i)*NUMBERSWIDTH),
					      "y", y,
					      "fill_color_rgba", 0x00ae00FF,
					      NULL);

	  item_list = g_list_append (item_list, focus_item);

	  bad_item = gnome_canvas_item_new (parent,
					    gnome_canvas_text_get_type (),
					    "text", "/",
					    "font", gc_skin_font_board_huge_bold,
					    "anchor", GTK_ANCHOR_CENTER,
					    "x", x_align-((strlen(operand_str)-i)*NUMBERSWIDTH),
					    "y", y,
					    "fill_color_rgba", 0xFF0000FF,
					    NULL);
	  gnome_canvas_item_hide(bad_item);

	  item_list = g_list_append (item_list, bad_item);

	}

      if(masked)
	{
	  toBeFoundItem = malloc(sizeof(ToBeFoundItem));
	  toBeFoundItem->bad_item=bad_item;
	  toBeFoundItem->index=i;
	  toBeFoundItem->value='?';
	  toBeFoundItem->item=item;
	  toBeFoundItem->focus_item=focus_item;
	  toBeFoundItem->previous=previousToBeFoundItem;
	  toBeFoundItem->next=NULL;

	  /* I Create a double linked list with the toBeFoundItem in order to navigate through them */
	  if(previousToBeFoundItem!=NULL)
	    previousToBeFoundItem->next=toBeFoundItem;

	  previousToBeFoundItem=toBeFoundItem;

	  /* Init the focussed item on the right digit */
	  if(i==0)
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
  GnomeCanvasItem *item;
  guint first_operand = 0;
  guint second_operand = 0;
  char *first_operand_str = NULL;
  char *second_operand_str = NULL;
  char *audioOperand = NULL;
  guint longuest;
  double x_align;
  double x, y;
  double y_firstline = 0;



  /* Some globals for this board layout */
  y_firstline = gcomprisBoard->height/5;


  get_random_number(&first_operand, &second_operand);
  first_operand_str = g_strdup_printf("%d", first_operand);

  second_operand_str = g_strdup_printf("%d", second_operand);

  /* Calc the longuest value */
  longuest = MAX(strlen(first_operand_str), strlen(second_operand_str));

  x_align = (gcomprisBoard->width - (longuest*3*NUMBERSWIDTH))/2 + NUMBERSWIDTH*(strlen(first_operand_str)) - 200;

  /* First operand */
  display_operand(parent, x_align, y_firstline, first_operand_str, FALSE);

  /* Second operand */
  display_operand(parent, x_align + NUMBERSWIDTH*(strlen(second_operand_str)+1),
		  y_firstline, second_operand_str, FALSE);

  /* Display the operator */
  x=(double) x_align;
  y=(double) y_firstline;
  item = gnome_canvas_item_new (parent,
				gnome_canvas_text_get_type (),
				"text", currentOperationText,
				"font", gc_skin_font_board_huge_bold,
				"x", x,
				"y", y,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", 0xFF3333FF,
				NULL);
  item_list = g_list_append (item_list, item);

  /* Now the equal sign*/
  item = gnome_canvas_item_new (parent,
				gnome_canvas_text_get_type (),
				"text", "=",
				"font", gc_skin_font_board_huge_bold,
				"x", x_align + NUMBERSWIDTH*(strlen(second_operand_str)+1),
				"y", y,
				"anchor", GTK_ANCHOR_CENTER,
				"fill_color_rgba", 0xFF3333FF,
				NULL);
  item_list = g_list_append (item_list, item);

  /* Display the empty area */
  g_free(expected_result);
  switch(currentOperation[0])
    {
    case PLUSSIGNFILE:
      expected_result = g_strdup_printf("%d", first_operand+second_operand);
      audioOperand = "sounds/$LOCALE/misc/plus.ogg";
      break;
    case MINUSSIGNFILE:
      expected_result = g_strdup_printf("%d", first_operand-second_operand);
      audioOperand = "sounds/$LOCALE/misc/minus.ogg";
      break;
    case BYSIGNFILE:
      expected_result = g_strdup_printf("%d", first_operand*second_operand);
      audioOperand = "sounds/$LOCALE/misc/by.ogg";
      break;
    case DIVIDESIGNFILE:
      expected_result = g_strdup_printf("%d", first_operand/second_operand);
      audioOperand = "sounds/$LOCALE/misc/outof.ogg";
      break;
    default:
      g_error("Bad Operation");
    }
  display_operand(parent, x_align + NUMBERSWIDTH*(strlen(second_operand_str)+
						  strlen(expected_result)+2),
		  y_firstline, expected_result, TRUE);


  {
    gchar *str1 = NULL;
    gchar *str2 = NULL;

    if(strlen(first_operand_str)==1)
      str1 = gc_sound_alphabet(first_operand_str);
    else
      str1 = g_strdup_printf("%s.ogg", first_operand_str);

    if(strlen(second_operand_str)==1)
      str2 = gc_sound_alphabet(second_operand_str);
    else
      str2 = g_strdup_printf("%s.ogg", second_operand_str);

    g_free(first_operand_str);
    g_free(second_operand_str);

    first_operand_str = g_strdup_printf("sounds/$LOCALE/alphabet/%s", str1);
    second_operand_str = g_strdup_printf("sounds/$LOCALE/alphabet/%s", str2);

    gc_sound_play_ogg(first_operand_str, audioOperand , second_operand_str, "sounds/$LOCALE/misc/equal.ogg", NULL);

    g_free(str1);
    g_free(str2);

    g_free(first_operand_str);
    g_free(second_operand_str);
  }
  return (item);
}

static void set_focus_item(ToBeFoundItem *toBeFoundItem, gboolean status)
{
  if(status)
    {
      gnome_canvas_item_show (toBeFoundItem->focus_item);
    }
  else
    {
      gnome_canvas_item_hide (toBeFoundItem->focus_item);
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
	  /* remember the appropriate digit to focus next */
	  hasfail=currentToBeFoundItem;
	}
      currentToBeFoundItem=(ToBeFoundItem *)currentToBeFoundItem->next;
    }
  currentToBeFoundItem=toBeFoundItem;

  if(hasfail==NULL)
    {
      game_won();
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

  if(board_paused)
    return FALSE;

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

      gc_sound_play_ogg ("sounds/gobble.ogg", NULL);
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
  gint i = (1+g_random_int()%10);

  // Get the next free slot
  while(operation_done[i]==TRUE && j>=0)
    {
      j--;
      i++;
      if(i>10)
	i=1;
    }
  operation_done[i]=TRUE;
  return i;
}

static void init_operation()
{
  int i;

  for(i=1; i<=10; i++)
    operation_done[i] = FALSE;
}

/* Returns a random int based on the current level */
static void get_random_number(guint *first_operand, guint *second_operand)
{
  guint min, max;

  switch(currentOperation[0])
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
      *second_operand = (min+g_random_int()%(max-min+1));
      *first_operand  = *second_operand*(min+g_random_int()%max);
      break;
    default:
      g_error("Bad Operation");
    }
}

/* ==================================== */
static void game_won()
{
  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->sublevel=1;
    gcomprisBoard->level++;
    if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
      gc_bonus_end_display(GC_BOARD_FINISHED_RANDOM);
      return;
    }

    gamewon = TRUE;
    algebra_destroy_all_items();
    gc_bonus_display(gamewon, GC_BONUS_SMILEY);
    init_operation();
    return;
  }
  algebra_next_level();
}
