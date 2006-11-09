/* gcompris - memory.c
 *
 * Time-stamp: <2006/08/21 23:33:28 bruno>
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
#include <string.h>

#include "gcompris/gcompris.h"


#define SOUNDLISTFILE PACKAGE
#define MAX_MEMORY_WIDTH  7
#define MAX_MEMORY_HEIGHT 6

//#define TEXT_FONT gc_skin_font_board_huge_bold
#define TEXT_FONT "Serif bold 28"

static gchar *op_fonts[10] =
  {
    "",
    "Serif bold 28",
    "Serif bold 24",
    "Serif bold 20",
    "Serif bold 20",
    "Serif bold 17",
    "Serif bold 13",
    "Serif bold 13",
    "Serif bold 13",
    "Serif bold 11",
  };

static GcomprisBoard *gcomprisBoard = NULL;

static GnomeCanvasGroup *boardRootItem = NULL;

static gint win_id = 0;

typedef enum
{
  MODE_NORMAL		= 0,
  MODE_TUX              = 1,
} Mode;
static Mode currentMode = MODE_NORMAL;

typedef enum
{
  UIMODE_NORMAL		= 0,
  UIMODE_SOUND          = 1,
} UiMode;
static UiMode currentUiMode = UIMODE_NORMAL;

typedef enum
{
  BOARDMODE_NORMAL	         = 0,
  BOARDMODE_SOUND                = 1,
  BOARDMODE_ADD                  = 2,
  BOARDMODE_MINUS                = 3,
  BOARDMODE_MULT                 = 4,
  BOARDMODE_DIV                  = 5,
  BOARDMODE_ADD_MINUS            = 6,
  BOARDMODE_MULT_DIV             = 7,
  BOARDMODE_ADD_MINUS_MULT_DIV   = 8,
} BoardMode;
static BoardMode currentBoardMode = BOARDMODE_NORMAL;

typedef enum
{
  ON_FRONT		= 0,
  ON_BACK		= 1,
  HIDDEN		= 2
} CardStatus;

static gchar *numbers;
static gchar *alphabet_lowercase;
static gchar *alphabet_uppercase;
static gchar *operators;
static gchar *op_add;
static gchar *op_minus;
static gchar *op_mult;
static gchar *op_div;

typedef struct {
  gchar *data;
  gint type;
  guint status;
  GnomeCanvasItem *rootItem;
  GnomeCanvasItem *backcardItem;
  GnomeCanvasItem *framecardItem;
  GnomeCanvasItem *frontcardItem;
  gboolean hidden;
  gchar *second_value;
} MemoryItem;

static MemoryItem *firstCard = NULL;
static MemoryItem *secondCard = NULL;

/* Define the page area where memory cards can be displayed for CARD MODE */
#define BASE_CARD_X1 50
#define BASE_CARD_Y1 50
#define BASE_CARD_X2 790
#define BASE_CARD_Y2 500
#define BASE_CARD_X1_TUX 200

/* Define the page area where memory cards can be displayed for SOUND MODE */
#define BASE_SOUND_X1 250
#define BASE_SOUND_Y1 30
#define BASE_SOUND_X2 600
#define BASE_SOUND_Y2 200
#define BASE_SOUND_X1_TUX BASE_SOUND_X1

/* The current page area where memory cards can be displayed */
gint base_x1;
gint base_y1;
gint base_x2;
gint base_y2;
gint base_x1_tux;

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

static void create_item(GnomeCanvasGroup *parent);
static void memory_destroy_all_items(void);
static void memory_next_level(void);
static gint item_event(GnomeCanvasItem *item, GdkEvent *event, MemoryItem *memoryItem);
static gint compare_card (gconstpointer a, gconstpointer b);

static void player_win();

static void display_card(MemoryItem *memoryItem, CardStatus cardStatus);

static void sound_callback(gchar *file);
static void start_callback(gchar *file);
static gboolean playing_sound = FALSE;

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

/* List of images to use in the memory */
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
#define NUMBER_OF_IMAGES G_N_ELEMENTS(imageList)

/* List of images to use in the memory */
static gchar *soundList[] =
{
   "sounds/LuneRouge/animaux/LRRain_in_garden_01_by_Lionel_Allorge_cut.ogg",
   "sounds/LuneRouge/animaux/LRBark_1_by_Lionel_Allorge_cut.ogg",
   "sounds/LuneRouge/animaux/LRBark_3_by_Lionel_Allorge_cut.ogg",
   "sounds/LuneRouge/animaux/LRFrogsInPondDuringStormByLionelAllorgeCut.ogg",
   "sounds/LuneRouge/engins/LRObject_falling_02_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/engins/LRTrain_slowing_down_01_by_Lionel_Allorge_cut.ogg",
   "sounds/LuneRouge/engins/LRStartAndStopCarEngine1ByLionelAllorgeCut.ogg",
   "sounds/LuneRouge/engins/LRObject_falling_01_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/humain/LRApplauses_1_by_Lionel_Allorge_cut.ogg",
   "sounds/LuneRouge/humain/LRHeart_beat_01_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/maison/LRDoor_Open_2_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/maison/LRRing_01_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/musique/LRBuddhist_gong_05_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/sf/LRWeird_4_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/sf/LRWeird_1_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/sf/LRWeird_2_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/sf/LRWeird_3_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/sf/LRWeird_5_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/sf/LRWeird_6_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/sf/LRET_phone_home_01_by_Lionel_Allorge_cut.ogg",
   "sounds/LuneRouge/usine/LRFactory_noise_02_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/usine/LRFactory_noise_03_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/usine/LRFactory_noise_04_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/usine/LRFactory_noise_05_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/usine/LRFactory_noise_06_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/usine/LRHits_01_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/usine/LRFireballs_01_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/usine/LRFactory_noise_01_by_Lionel_Allorge.ogg",
   "sounds/LuneRouge/LRLaPause_short.ogg",
   "sounds/memory/plick.ogg",
   "sounds/memory/tick.ogg",
   "sounds/memory/tri.ogg",
   "sounds/chronos/space/1.ogg",
   "sounds/chronos/space/2.ogg",
   "sounds/chronos/space/2.ogg",
   "sounds/melody/guitar/melody.ogg",
   "sounds/melody/guitar/son1.ogg",
   "sounds/melody/guitar/son2.ogg",
   "sounds/melody/guitar/son3.ogg",
   "sounds/melody/guitar/son4.ogg",
   "sounds/melody/tachos/son1.ogg",
   "sounds/melody/tachos/son2.ogg",
   "sounds/melody/tachos/son3.ogg",
   "sounds/melody/tachos/son4.ogg",
   "sounds/melody/tachos/melody.ogg"
};

#define NUMBER_OF_SOUNDS G_N_ELEMENTS(soundList)

static SoundPolicy sound_policy;

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
 * Against computer version
 * ------------------------
 *
 */

static gboolean to_tux = FALSE;
static GQueue *tux_memory;
static gint tux_memory_size;
static gint tux_memory_sizes[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
static gint tux_pairs = 0;
static gint player_pairs = 0;

static void add_card_in_tux_memory(MemoryItem *card);
static MemoryItem *find_card_in_tux_memory(MemoryItem *card);
static void remove_card_from_tux_memory(MemoryItem *card);
static gint tux_play();

typedef struct {
  MemoryItem *first;
  MemoryItem *second;
} WINNING;

static GList *winning_pairs = NULL;

static gint tux_id = 0;

/*
 *
 * Operation versions
 *
 */


/*  max number 1 / max number 2 */
/*  to allow level with calculus like 7*12 */

static guint add_levelDescription[10][2] =
{
  {0,0},
  {5,5},
  {5,7},
  {7,7},
  {7,10},
  {10,10},
  {10,15},
  {15,15},
  {25,25},
  {50,50},
};

static guint minus_levelDescription[10][2] =
{
  {0,0},
  {5,5},
  {5,7},
  {7,7},
  {7,10},
  {10,10},
  {10,15},
  {15,15},
  {25,25},
  {50,50},
};


static guint mult_levelDescription[10][2] =
{
  {0,0},
  {5,5},
  {5,7},
  {7,7},
  {7,10},
  {10,10},
  {12,12},
  {15,15},
  {20,20},
  {25,25},
};

static guint div_levelDescription[10][2] =
{
  {0,0},
  {5,5},
  {5,7},
  {7,7},
  {7,10},
  {10,10},
  {12,12},
  {15,15},
  {20,20},
  {25,25},
};


/*
 * random without repeted token
 * ------------------------
 *
 */

#define TYPE_IMAGE     1
#define TYPE_NUMBER    2
#define TYPE_UPPERCASE 4
#define TYPE_LOWERCASE 8
#define TYPE_SOUND     16
#define TYPE_ADD       32
#define TYPE_MINUS     64
#define TYPE_MULT      128
#define TYPE_DIV       256

static GList *passed_token = NULL;

static GnomeCanvasItem *tux;
static GnomeCanvasItem *tux_score;
static GnomeCanvasItem *player_score;
static GnomeCanvasItem *tux_score_s;
static GnomeCanvasItem *player_score_s;

/* set the type of the token returned in string in returned_type */
void get_random_token(int token_type, gint *returned_type, gchar **string, gchar **second_value)
{
  gchar *result = NULL;
  gchar *second = NULL;
  gboolean skip;

  gint max_token;
  gint j, i, k;
  gint type;

  typedef struct {
    gint bound;
    gint type;
  } DATUM ;

  GList *data= NULL;
  GList *list;

  max_token = 0;

  if (token_type & TYPE_IMAGE){
    max_token += NUMBER_OF_IMAGES;

    DATUM *dat = g_malloc0(sizeof(DATUM));
    dat->bound = max_token;
    dat->type =  TYPE_IMAGE;
    data = g_list_append(data, dat);
  }

  if (token_type & TYPE_NUMBER) {
    max_token += g_utf8_strlen (numbers, -1);
    DATUM *dat = g_malloc0(sizeof(DATUM));
    dat->bound = max_token;
    dat->type =  TYPE_NUMBER;
    data = g_list_append(data, dat);
  }

  if (token_type & TYPE_UPPERCASE){
    max_token += g_utf8_strlen (alphabet_uppercase, -1);
    DATUM *dat = g_malloc0(sizeof(DATUM));
    dat->bound = max_token;
    dat->type =   TYPE_UPPERCASE;
    data = g_list_append(data, dat);
  }

  if (token_type & TYPE_LOWERCASE){
    max_token += g_utf8_strlen (alphabet_lowercase, -1);;
    DATUM *dat = g_malloc0(sizeof(DATUM));
    dat->bound = max_token;
    dat->type =   TYPE_LOWERCASE;
    data = g_list_append(data, dat);
  }


  if (token_type & TYPE_SOUND){
    max_token += NUMBER_OF_SOUNDS;
    DATUM *dat = g_malloc0(sizeof(DATUM));
    dat->bound = max_token;
    dat->type =   TYPE_SOUND;
    data = g_list_append(data, dat);
  }

  if (token_type & TYPE_ADD){
    max_token += (add_levelDescription[gcomprisBoard->level][0]+1)*(add_levelDescription[gcomprisBoard->level][1]+1);
    DATUM *dat = g_malloc0(sizeof(DATUM));
    dat->bound = max_token;
    dat->type =   TYPE_ADD;
    data = g_list_append(data, dat);
  }

  if (token_type & TYPE_MINUS){
    max_token += (minus_levelDescription[gcomprisBoard->level][0]+1)*(minus_levelDescription[gcomprisBoard->level][1]+1);
    DATUM *dat = g_malloc0(sizeof(DATUM));
    dat->bound = max_token;
    dat->type =   TYPE_MINUS;
    data = g_list_append(data, dat);
  }

  if (token_type & TYPE_MULT){
   max_token += (mult_levelDescription[gcomprisBoard->level][0]+1)*(mult_levelDescription[gcomprisBoard->level][1]+1);
    DATUM *dat = g_malloc0(sizeof(DATUM));
    dat->bound = max_token;
    dat->type =   TYPE_MULT;
    data = g_list_append(data, dat);
  }

  if (token_type & TYPE_DIV){
    max_token += (div_levelDescription[gcomprisBoard->level][0]+1)*(div_levelDescription[gcomprisBoard->level][1]+1);
    DATUM *dat = g_malloc0(sizeof(DATUM));
    dat->bound = max_token;
    dat->type =   TYPE_DIV;
    data = g_list_append(data, dat);
  }



  g_assert(max_token >0);

  i = g_random_int()%max_token;

  for (list = data; list != NULL; list = list->next)
    if ( i < ((DATUM *)list->data)->bound)
      break;

  j=-1;

  do {
    skip = FALSE;
    g_free(result);
    result = NULL;
    g_free(second);
    j++;

    if ((i+j) == max_token) {
      list = data;
    }

    if ((i+j)%max_token == ((DATUM *)list->data)->bound)
      list = list->next;

    /* calculate index in right table */
    k = (i+j)%max_token - (list->prev ? ((DATUM *)list->prev->data)->bound : 0);


    type = ((DATUM *)list->data)->type;

    switch (type) {
    case TYPE_IMAGE:
      result= g_strdup(imageList[k]);
      break;
    case TYPE_NUMBER:
      result = g_malloc0(2*sizeof(gunichar));
      g_utf8_strncpy(result, g_utf8_offset_to_pointer (numbers,k),1);
      break;
    case TYPE_UPPERCASE:
      result = g_malloc0(2*sizeof(gunichar));
      g_utf8_strncpy(result, g_utf8_offset_to_pointer (alphabet_uppercase,k),1);
      break;
    case TYPE_LOWERCASE:
      result = g_malloc0(2*sizeof(gunichar));
      g_utf8_strncpy(result, g_utf8_offset_to_pointer (alphabet_lowercase,k),1);
      break;
    case TYPE_SOUND:
      result = g_strdup(soundList[k]);
      break;
    case TYPE_ADD:
      {
	int i, j;
	i = k %  add_levelDescription[gcomprisBoard->level][0];
	j = k /  add_levelDescription[gcomprisBoard->level][0];
	result = g_strdup_printf("%d%s%d",i,op_add,j);
	second = g_strdup_printf("%d",i+j);;
	break;
      }
    case TYPE_MINUS:
      {
	int i, j;
	i = k %  minus_levelDescription[gcomprisBoard->level][0];
	j = k /  minus_levelDescription[gcomprisBoard->level][0];
	result = g_strdup_printf("%d%s%d",i+j,op_minus,i);
	second = g_strdup_printf("%d",j);;
	break;
      }
    case TYPE_MULT:
      {
	int i, j;
	i = k %  mult_levelDescription[gcomprisBoard->level][0];
	j = k /  mult_levelDescription[gcomprisBoard->level][0];
	result = g_strdup_printf("%d%s%d",i,op_mult,j);
	second = g_strdup_printf("%d",i*j);;
	break;
      }
    case TYPE_DIV:
      {
	int i1, i2;
	i1 = k %  div_levelDescription[gcomprisBoard->level][0];
	if (i1==0) skip=TRUE;
	i2 = k /  div_levelDescription[gcomprisBoard->level][0];
	result = g_strdup_printf("%d%s%d",i1*i2,op_div,i1);
	second = g_strdup_printf("%d",i2);
	break;
      }
    default:
      /* should never append */
      g_error("never !");
      break;
    }

  } while (skip || ((j < max_token )
	   && (passed_token && result && g_list_find_custom(passed_token, result, (GCompareFunc)strcmp))));

  g_assert (j < max_token);

  passed_token = g_list_append( passed_token, result);

  *returned_type = type;

  *string = result;

  if (second_value)
    *second_value = second;

  for (list = data; list != NULL; list=list->next)
    g_free(list->data);

  g_list_free(data);

}




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

static gboolean Paused = FALSE;

static void pause_board (gboolean pause)
{

  if(gcomprisBoard==NULL)
    return;

  Paused = pause;

  if(pause){
    if (currentMode == MODE_TUX){
      if (tux_id){
	g_source_remove(tux_id);
	tux_id = 0;
      }
    }
  }
  else {
    if (remainingCards<=0)
      memory_next_level();
    else {
      if (currentMode == MODE_TUX){
	if (to_tux){
	  tux_id = g_timeout_add (2000,
				  (GSourceFunc) tux_play, NULL);
	}
      }
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

      gcomprisBoard->level = 1;
      gcomprisBoard->maxlevel = 9;
      gc_bar_set(GC_BAR_LEVEL);

      /* Default mode */

      if(!gcomprisBoard->mode){
	currentMode=MODE_NORMAL;
	currentUiMode=UIMODE_NORMAL;
	currentBoardMode=BOARDMODE_NORMAL;
      } else {
	if(g_strcasecmp(gcomprisBoard->mode, "tux")==0){
	  currentMode=MODE_TUX;
	  currentUiMode=UIMODE_NORMAL;
	  currentBoardMode=BOARDMODE_NORMAL;
	} else {
	  if(g_strcasecmp(gcomprisBoard->mode, "sound")==0){
	    currentMode=MODE_NORMAL;
	    currentUiMode=UIMODE_SOUND;
	    currentBoardMode=BOARDMODE_SOUND;
	  } else {
	    if(g_strcasecmp(gcomprisBoard->mode, "sound_tux")==0){
	      currentMode=MODE_TUX;
	      currentUiMode=UIMODE_SOUND;
	      currentBoardMode=BOARDMODE_SOUND;
	    } else {
	      if(g_strcasecmp(gcomprisBoard->mode, "add")==0){
		currentMode=MODE_NORMAL;
		currentUiMode=UIMODE_NORMAL;
		currentBoardMode=BOARDMODE_ADD;
	      } else {
		if(g_strcasecmp(gcomprisBoard->mode, "add_tux")==0){
		  currentMode=MODE_TUX;
		  currentUiMode=UIMODE_NORMAL;
		  currentBoardMode=BOARDMODE_ADD;
		} else {
		  if(g_strcasecmp(gcomprisBoard->mode, "minus")==0){
		    currentMode=MODE_NORMAL;
		    currentUiMode=UIMODE_NORMAL;
		    currentBoardMode=BOARDMODE_MINUS;
		  } else {
		    if(g_strcasecmp(gcomprisBoard->mode, "minus_tux")==0){
		      currentMode=MODE_TUX;
		      currentUiMode=UIMODE_NORMAL;
		      currentBoardMode=BOARDMODE_MINUS;
		    } else {
		      if(g_strcasecmp(gcomprisBoard->mode, "mult")==0){
			currentMode=MODE_NORMAL;
			currentUiMode=UIMODE_NORMAL;
			currentBoardMode=BOARDMODE_MULT;
		      } else {
			if(g_strcasecmp(gcomprisBoard->mode, "mult_tux")==0){
			  currentMode=MODE_TUX;
			  currentUiMode=UIMODE_NORMAL;
			  currentBoardMode=BOARDMODE_MULT;
			} else {
			  if(g_strcasecmp(gcomprisBoard->mode, "div")==0){
			    currentMode=MODE_NORMAL;
			    currentUiMode=UIMODE_NORMAL;
			    currentBoardMode=BOARDMODE_DIV;
			  } else {
			    if(g_strcasecmp(gcomprisBoard->mode, "div_tux")==0){
			      currentMode=MODE_TUX;
			      currentUiMode=UIMODE_NORMAL;
			      currentBoardMode=BOARDMODE_DIV;
			    } else {
			      if(g_strcasecmp(gcomprisBoard->mode, "add_minus")==0){
				currentMode=MODE_NORMAL;
				currentUiMode=UIMODE_NORMAL;
				currentBoardMode=BOARDMODE_ADD_MINUS;
			      } else {
				if(g_strcasecmp(gcomprisBoard->mode, "add_minus_tux")==0){
				  currentMode=MODE_TUX;
				  currentUiMode=UIMODE_NORMAL;
				  currentBoardMode=BOARDMODE_ADD_MINUS;
				} else {
				  if(g_strcasecmp(gcomprisBoard->mode, "mult_div")==0){
				    currentMode=MODE_NORMAL;
				    currentUiMode=UIMODE_NORMAL;
				    currentBoardMode=BOARDMODE_MULT_DIV;
				  } else {
				    if(g_strcasecmp(gcomprisBoard->mode, "mult_div_tux")==0){
				      currentMode=MODE_TUX;
				      currentUiMode=UIMODE_NORMAL;
				      currentBoardMode=BOARDMODE_MULT_DIV;
				    } else {
				      if(g_strcasecmp(gcomprisBoard->mode, "add_minus_mult_div")==0){
					currentMode=MODE_NORMAL;
					currentUiMode=UIMODE_NORMAL;
					currentBoardMode=BOARDMODE_ADD_MINUS_MULT_DIV;
				      } else {
					if(g_strcasecmp(gcomprisBoard->mode, "add_minus_mult_div_tux")==0){
					  currentMode=MODE_TUX;
					  currentUiMode=UIMODE_NORMAL;
					  currentBoardMode=BOARDMODE_ADD_MINUS_MULT_DIV;
					} else {
					  currentMode=MODE_NORMAL;
					  currentUiMode=UIMODE_NORMAL;
					  currentBoardMode=BOARDMODE_NORMAL;
					  g_warning("Fallback mode set to images");
					}
				      }
				    }
				  }
				}
			      }
			    }
			  }
			}
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}
      }


      if (currentUiMode == UIMODE_SOUND)
	{
	  GcomprisProperties	*properties = gc_prop_get();

	  gc_sound_pause();

	  /* initial state to restore */
	  sound_policy = gc_sound_policy_get();

	  gc_sound_policy_set(PLAY_AND_INTERRUPT);

	  gc_set_background(gnome_canvas_root(gcomprisBoard->canvas), "images/gcompris_band.png");
	  base_x1 = BASE_SOUND_X1;
	  base_y1 = BASE_SOUND_Y1;
	  base_x2 = BASE_SOUND_X2;
	  base_y2 = BASE_SOUND_Y2;
	  base_x1_tux = BASE_SOUND_X1_TUX;

	  if(!properties->fx) {
	    gc_dialog(_("Error: this activity cannot be played with the\nsound effects disabled.\nGo to the configuration dialog to\nenable the sound"), gc_board_stop);
	    return;
	  }

	}
      else
	{
	  gc_set_background(gnome_canvas_root(gcomprisBoard->canvas), "images/scenery_background.png");
	  base_x1 = BASE_CARD_X1;
	  base_y1 = BASE_CARD_Y1;
	  base_x2 = BASE_CARD_X2;
	  base_y2 = BASE_CARD_Y2;
	  base_x1_tux = BASE_CARD_X1_TUX;
	}


      /* TRANSLATORS: Put here the numbers in your language */
      numbers=_("0123456789");
      g_assert(g_utf8_validate(numbers,-1,NULL)); // require by all utf8-functions

      /* TRANSLATORS: Put here the alphabet lowercase in your language */
      alphabet_lowercase=_("abcdefghijklmnopqrstuvwxyz");
      g_assert(g_utf8_validate(alphabet_lowercase,-1,NULL)); // require by all utf8-functions

      g_warning("Using lowercase %s", alphabet_lowercase);

      /* TRANSLATORS: Put here the alphabet uppercase in your language */
      alphabet_uppercase=_("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
      g_assert(g_utf8_validate(alphabet_uppercase,-1,NULL)); // require by all utf8-functions
      g_warning("Using uppercase %s", alphabet_uppercase);

      /* TRANSLATORS: Put here the mathematical operators "+-×÷" for  your language. */
      operators=_("+-Ã—Ã·");
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


      g_warning("Using operators %s %s %s %s", op_add, op_minus,
		                               op_mult, op_div);

      if (currentMode == MODE_TUX){
	tux_memory_size = tux_memory_sizes[gcomprisBoard->level];
	tux_memory = g_queue_new ();
      }

      Paused = FALSE;

      to_tux = FALSE;
      if (currentUiMode == UIMODE_SOUND){
	playing_sound = TRUE;
	gc_sound_play_ogg_cb("sounds/LuneRouge/musique/LRBuddhist_gong_05_by_Lionel_Allorge.ogg",start_callback);
      } else
	playing_sound = FALSE;

      memory_next_level();
    }
}

static void
end_board ()
{
  if (currentUiMode == UIMODE_SOUND) {
    gc_sound_policy_set(sound_policy);
    gc_sound_resume();
  }

  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);

      memory_destroy_all_items();
      if (currentMode == MODE_TUX){
	g_queue_free(tux_memory);
	tux_memory = NULL;
      }
    }
  g_free(op_add);
  g_free(op_minus);
  g_free(op_mult);
  g_free(op_div);
  
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

static void update_scores()
{
  gchar *tux_score_str;
  gchar *player_score_str;

  tux_score_str = g_strdup_printf("%d", tux_pairs);
  player_score_str = g_strdup_printf("%d", player_pairs);

  gnome_canvas_item_set(tux_score,      "text", tux_score_str, NULL);
  gnome_canvas_item_set(player_score,   "text", player_score_str, NULL);
  gnome_canvas_item_set(tux_score_s,    "text", tux_score_str, NULL);
  gnome_canvas_item_set(player_score_s, "text", player_score_str, NULL);

  g_free(tux_score_str);
  g_free(player_score_str);
}

/* set initial values for the next level */
static void memory_next_level()
{
  gc_bar_set_level(gcomprisBoard);

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

  if (currentMode == MODE_TUX){
	tux_memory_size = tux_memory_sizes[gcomprisBoard->level];
	g_warning("tux_memory_size %d", tux_memory_size );
	tux_pairs = 0;
	player_pairs = 0;
	update_scores();
  }
}


/* Destroy all the items */
static void memory_destroy_all_items()
{
  gint x, y;

  firstCard = NULL;
  secondCard = NULL;

  /* Remove timers first */
  if (win_id) {
    g_source_remove (win_id);
  }
  win_id = 0;

  if (currentMode == MODE_TUX){
    if (tux_id) {
      g_source_remove (tux_id);
    }
    tux_id =0;
    to_tux = FALSE;
  }

  /* Now destroy all items */
  if(boardRootItem!=NULL)
      gtk_object_destroy (GTK_OBJECT(boardRootItem));

  boardRootItem=NULL;

  // Clear the memoryArray
  for(x=0; x<MAX_MEMORY_WIDTH; x++)
    for(y=0; y<MAX_MEMORY_HEIGHT; y++)
      {
	if (memoryArray[x][y])
	  g_free(memoryArray[x][y]->second_value);
	g_free(memoryArray[x][y]);
	memoryArray[x][y] = NULL;
      }

  GList *list;

  for (list = passed_token; list != NULL; list=list->next)
    g_free(list->data);

  g_list_free(passed_token);

  passed_token = NULL;

  if (currentMode == MODE_TUX){
    for (list = winning_pairs; list != NULL; list=list->next)
      g_free(list->data);

    g_list_free(winning_pairs);

    winning_pairs = NULL;
    while (g_queue_pop_head (tux_memory));
    //tux_memory = NULL;
  }

}

/*
 * Takes care to return a random pair of images (one by one)
 * the image is loaded in memoryItem->image
 *
 */
static void get_image(MemoryItem *memoryItem, guint x, guint y)
{
  guint rx, ry;

  memoryItem->hidden = FALSE;

  if(memoryArray[x][y])
    {
      // Get the pair's image
      if (memoryArray[x][y]->type & (TYPE_ADD|TYPE_MINUS|TYPE_MULT|TYPE_DIV)){
	memoryItem->data = memoryArray[x][y]->second_value;
	memoryItem->type =  memoryArray[x][y]->type;
	memoryArray[x][y] = memoryItem;
	// if created by g_malloc0, this is not usefull;
	//memoryItem->second_value = NULL;
      }
      else {
	memoryItem->data = memoryArray[x][y]->data;
	memoryItem->type =  memoryArray[x][y]->type;
	memoryArray[x][y] = memoryItem;
      }
      return;
    }


  memoryArray[x][y] = memoryItem;

  switch (currentBoardMode) {
  case BOARDMODE_SOUND:
    get_random_token ( TYPE_SOUND, &memoryItem->type,  &memoryItem->data, NULL);
    g_assert (memoryItem->type ==  TYPE_SOUND);
    break;
  case BOARDMODE_NORMAL:
    switch(gcomprisBoard->level) {

    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
      /* Image mode */
      get_random_token ( TYPE_IMAGE, &memoryItem->type,  &memoryItem->data, NULL);
      g_assert (memoryItem->type ==  TYPE_IMAGE);
      break;

    case 5:
      /* Limited Text mode Numbers only */
      get_random_token ( TYPE_NUMBER, &memoryItem->type,  &memoryItem->data, NULL);
      g_assert (memoryItem->type ==  TYPE_NUMBER);
      break;

    case 6:
      /* Limited Text mode Numbers + Capitals */
      get_random_token ( TYPE_NUMBER | TYPE_UPPERCASE, &memoryItem->type,  &memoryItem->data, NULL);
      g_assert((memoryItem->type == TYPE_NUMBER)||(memoryItem->type==TYPE_UPPERCASE));
      break;

    default:
      /* Text mode ALL */
      get_random_token ( TYPE_NUMBER | TYPE_UPPERCASE | TYPE_LOWERCASE, &memoryItem->type,  &memoryItem->data, NULL);
      g_assert (memoryItem->type & ( TYPE_NUMBER | TYPE_UPPERCASE | TYPE_LOWERCASE));
      break;
    }
    break;
  case BOARDMODE_ADD:
    get_random_token ( TYPE_ADD, &memoryItem->type,  &memoryItem->data, &memoryItem->second_value);
    g_assert (memoryItem->type == TYPE_ADD);
    break;
  case BOARDMODE_MINUS:
    get_random_token ( TYPE_MINUS, &memoryItem->type,  &memoryItem->data, &memoryItem->second_value);
    g_assert (memoryItem->type == TYPE_MINUS);
    break;
  case BOARDMODE_MULT:
    get_random_token ( TYPE_MULT, &memoryItem->type,  &memoryItem->data, &memoryItem->second_value);
    g_assert (memoryItem->type == TYPE_MULT);
    break;
  case BOARDMODE_DIV:
    get_random_token ( TYPE_DIV, &memoryItem->type,  &memoryItem->data, &memoryItem->second_value);
    g_assert (memoryItem->type == TYPE_DIV);
    break;
  case BOARDMODE_ADD_MINUS:
    get_random_token ( TYPE_ADD | TYPE_MINUS, &memoryItem->type,  &memoryItem->data, &memoryItem->second_value);
    g_assert (memoryItem->type & (TYPE_ADD | TYPE_MINUS));
    break;
  case BOARDMODE_MULT_DIV:
    get_random_token ( TYPE_MULT | TYPE_DIV, &memoryItem->type,  &memoryItem->data, &memoryItem->second_value);
    g_assert (memoryItem->type & (TYPE_MULT | TYPE_DIV));
    break;
  case BOARDMODE_ADD_MINUS_MULT_DIV:
    get_random_token ( TYPE_ADD | TYPE_MINUS |TYPE_MULT | TYPE_DIV , &memoryItem->type,  &memoryItem->data, &memoryItem->second_value);
    g_assert (memoryItem->type & (TYPE_ADD | TYPE_MINUS |TYPE_MULT | TYPE_DIV));
    break;

  default:
    g_error("Don't now in what mode run !");
    break;
  }

  g_warning("returned token %s for item x=%d y=%d", memoryItem->data, x, y);


  // Randomly set the pair
  rx = g_random_int_range( 0, numberOfColumn);
  ry = g_random_int_range(0, numberOfLine);

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

static void create_item(GnomeCanvasGroup *parent)
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
  width  = (base_x2-(currentMode == MODE_TUX ? base_x1_tux : base_x1))/numberOfColumn;
  height = (base_y2-base_y1)/numberOfLine;

  /* Remove a little bit of space for the card shadow */
  height2 = height * 0.95;
  width2  = width  * 0.95;


  if (currentUiMode == UIMODE_SOUND) {
    GdkPixbuf *pixmap =  gc_pixmap_load("images/transparent_square2.png");
    gnome_canvas_item_new (GNOME_CANVAS_GROUP(parent),
			   gnome_canvas_pixbuf_get_type (),
			   "pixbuf", pixmap,
			   "x", (double) (currentMode == MODE_TUX ? base_x1_tux : base_x1) - 20,
			   "y", (double) base_y1 - 15,
			   NULL);
    gdk_pixbuf_unref(pixmap);
  }

  if (currentMode == MODE_TUX){
    GdkPixbuf *pixmap_tux =  gc_pixmap_load("images/tux-teacher.png");

    tux = gnome_canvas_item_new (GNOME_CANVAS_GROUP(parent),
				 gnome_canvas_pixbuf_get_type (),
				 "pixbuf", pixmap_tux,
				 "x", (double) 50,
				 "y", (double) 20,
				 NULL);
    gdk_pixbuf_unref(pixmap_tux);
    
    tux_score_s = gnome_canvas_item_new (GNOME_CANVAS_GROUP(parent),
				       gnome_canvas_text_get_type (),
				       "font", gc_skin_font_board_huge_bold,
				       "x", (double) 100+1.0,
				       "y", (double) 200+1.0,
				       "anchor", GTK_ANCHOR_CENTER,
				       "fill_color_rgba", 0x101010FF,
				       NULL);

    player_score_s = gnome_canvas_item_new (GNOME_CANVAS_GROUP(parent),
					  gnome_canvas_text_get_type (),
					  "font", gc_skin_font_board_huge_bold,
					  "x", (double) 100+1.0,
					  "y", (double) BASE_CARD_Y2 - 20+1.0,
					  "anchor", GTK_ANCHOR_CENTER,
					  "fill_color_rgba", 0x101010FF,
					  NULL);

    tux_score = gnome_canvas_item_new (GNOME_CANVAS_GROUP(parent),
				       gnome_canvas_text_get_type (),
				       "font", gc_skin_font_board_huge_bold,
				       "x", (double) 100,
				       "y", (double) 200,
				       "anchor", GTK_ANCHOR_CENTER,
				       "fill_color_rgba", 0xFF0F0FFF,
				       NULL);

    player_score = gnome_canvas_item_new (GNOME_CANVAS_GROUP(parent),
					  gnome_canvas_text_get_type (),
					  "font", gc_skin_font_board_huge_bold,
					  "x", (double) 100,
					  "y", (double) BASE_CARD_Y2 - 20,
					  "anchor", GTK_ANCHOR_CENTER,
					  "fill_color_rgba", 0xFF0F0FFF,
					  NULL);
  }

  for(x=0; x<numberOfColumn; x++)
    {
      for(y=0; y<numberOfLine; y++)
	{

	  memoryItem = g_malloc0(sizeof(MemoryItem));

	  memoryItem->rootItem = \
	    gnome_canvas_item_new (parent,
				   gnome_canvas_group_get_type (),
				   "x", (double) (currentMode == MODE_TUX ? base_x1_tux : base_x1) + x*width,
				   "y", (double) base_y1 + y*height,
				   NULL);

	  if (currentUiMode == UIMODE_SOUND)
	    pixmap = gc_pixmap_load("gcompris/misc/Tux_mute.png");
	  else
	    pixmap = gc_pixmap_load("gcompris/misc/backcard.png");

	  memoryItem->backcardItem = \
	    gnome_canvas_item_new (GNOME_CANVAS_GROUP(memoryItem->rootItem),
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf", pixmap,
				   "x", (double) 0,
				   "y", (double) 0,
				   "width", (double) width2,
				   "height", (double) height2,
				   "width_set", TRUE,
				   "height_set", TRUE,
				   NULL);
	  gdk_pixbuf_unref(pixmap);

	  if (currentUiMode != UIMODE_SOUND){
	    pixmap = gc_pixmap_load("gcompris/misc/emptycard.png");
	    memoryItem->framecardItem = \
	      gnome_canvas_item_new (GNOME_CANVAS_GROUP(memoryItem->rootItem),
				     gnome_canvas_pixbuf_get_type (),
				     "pixbuf", pixmap,
				     "x", (double) 0,
				     "y", (double) 0,
				     "width", (double) width2,
				     "height", (double) height2,
				     "width_set", TRUE,
				     "height_set", TRUE,
				     NULL);
	    gnome_canvas_item_hide(memoryItem->framecardItem);
	    gdk_pixbuf_unref(pixmap);
	  }


	  // Display the image itself while taking care of its size and maximize the ratio
	  get_image(memoryItem, x, y);

	  if (currentUiMode == UIMODE_SOUND){
	    pixmap = gc_pixmap_load("gcompris/misc/Tux_play.png");
	    memoryItem->frontcardItem =	\
	      gnome_canvas_item_new (GNOME_CANVAS_GROUP(memoryItem->rootItem),
				     gnome_canvas_pixbuf_get_type (),
				     "pixbuf", pixmap,
				     "x", (double) 0,
				     "y", (double) 0,
				     "width", (double) width2,
				     "height", (double) height2,
				     "width_set", TRUE,
				     "height_set", TRUE,
				     NULL);
	    gdk_pixbuf_unref(pixmap);
	  }
	  else {
	    if(memoryItem->type == TYPE_IMAGE) {
	      pixmap = gc_pixmap_load(memoryItem->data);

	      yratio=(height2*0.8)/(float)gdk_pixbuf_get_height(pixmap);
	      xratio=(width2*0.8)/(float)gdk_pixbuf_get_width(pixmap);
	      yratio=xratio=MIN(xratio, yratio);
	      card_shadow_w = width*0.05;
	      card_shadow_h = height*0.05;

	      memoryItem->frontcardItem =	\
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(memoryItem->rootItem),
				       gnome_canvas_pixbuf_get_type (),
				       "pixbuf", pixmap,
				       "x", (double) ((width2)-
						      gdk_pixbuf_get_width(pixmap)*xratio*0.8)/2 -
				       card_shadow_w,
				       "y", (double) ((height2)-
						      gdk_pixbuf_get_height(pixmap)*yratio*0.8)/2 -
				       card_shadow_h,
				       "width", (double) gdk_pixbuf_get_width(pixmap)*xratio*0.8,
				       "height", (double) gdk_pixbuf_get_height(pixmap)*yratio*0.8,
				       "width_set", TRUE,
				       "height_set", TRUE,
				       NULL);
	      gdk_pixbuf_unref(pixmap);

	    } else {
	      gchar *font;
	      if (memoryItem->type & (TYPE_ADD|TYPE_MINUS|TYPE_MULT|TYPE_DIV))
		font = op_fonts[gcomprisBoard->level];
	      else
		font = TEXT_FONT;
	      /* It's a letter */
	      memoryItem->frontcardItem =	 \
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(memoryItem->rootItem),
				       gnome_canvas_text_get_type (),
				       "text", memoryItem->data,
				       "font", font,
				       "x", (double) (width2*0.9)/2,
				       "y", (double) (height2*0.9)/2,
				       "anchor", GTK_ANCHOR_CENTER,
				       "fill_color_rgba", 0x559ADDFF,
				       NULL);

	    }
	  }

	  gnome_canvas_item_hide(memoryItem->frontcardItem);
	  gtk_signal_connect(GTK_OBJECT(memoryItem->rootItem), "event",
			     (GtkSignalFunc) item_event,
			     memoryItem);

	}
    }

  //return (NULL);
}

static void player_win()
{
  gc_sound_play_ogg ("sounds/bonus.ogg", NULL);
  /* Try the next level */
  if (tux_pairs <= player_pairs)
    gcomprisBoard->level++;
  if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : bail out
    gc_bonus_end_display(BOARD_FINISHED_RANDOM);
    return;
  }
  gc_bonus_display((tux_pairs <= player_pairs), BONUS_RANDOM);

}

static void display_card(MemoryItem *memoryItem, CardStatus cardStatus)
{

  if (currentUiMode == UIMODE_SOUND){
    switch (cardStatus)
      {
      case ON_FRONT:
	g_assert(memoryItem->hidden == FALSE);
	gnome_canvas_item_hide(memoryItem->backcardItem);
	gnome_canvas_item_show(memoryItem->frontcardItem);
	playing_sound = TRUE;
	gc_sound_play_ogg_cb (memoryItem->data, sound_callback);
	break;
      case ON_BACK:
	gnome_canvas_item_show(memoryItem->backcardItem);
	gnome_canvas_item_hide(memoryItem->frontcardItem);
	break;
      case HIDDEN:
	gnome_canvas_item_hide(memoryItem->backcardItem);
	gnome_canvas_item_hide(memoryItem->frontcardItem);
	memoryItem->hidden = TRUE;
	break;
      }
  }
  else {
    switch (cardStatus)
      {
      case ON_FRONT:
	g_assert(memoryItem->hidden == FALSE);
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
	memoryItem->hidden = TRUE;
	break;
      }
  }

}

/*
 * Used to hide card after a timer
 *
 */
static gint hide_card (GtkWidget *widget, gpointer data)
{
  if (currentMode == MODE_TUX){
    GList *list = NULL;
    GList *to_remove = NULL;

    for (list =  winning_pairs; list != NULL; list=list->next)
      if ((((WINNING *) list->data)->first == firstCard) || (((WINNING *) list->data)->first == secondCard) || (((WINNING *) list->data)->second == firstCard) || (((WINNING *) list->data)->second == secondCard) ){
	to_remove = g_list_append( to_remove, list->data);
      }

    for (list =  to_remove; list != NULL; list=list->next){
      void *data = list->data;
      winning_pairs = g_list_remove (winning_pairs, list->data);
      g_free (data);
      g_warning("Again %d winning pairs in tux list! ", g_list_length(winning_pairs));
    }

    g_list_free(to_remove);

    if (to_tux)
      tux_pairs++;
    else
      player_pairs++;

    update_scores();
  }

  if(firstCard!=NULL)
    {
      display_card(firstCard, HIDDEN);
      if (currentMode == MODE_TUX)
	remove_card_from_tux_memory(firstCard);
      firstCard  = NULL;
    }

  if(secondCard!=NULL)
    {
      display_card(secondCard, HIDDEN);
      if (currentMode == MODE_TUX)
	remove_card_from_tux_memory(secondCard);
      secondCard  = NULL;
    }
  win_id = 0;

  remainingCards -= 2;
  if(remainingCards<=0){
    if (currentMode == MODE_TUX){
      if (tux_id){
	g_source_remove(tux_id);
	tux_id = 0;
	to_tux = FALSE;
      }
    }
    player_win();
  }

  return (FALSE);
}

static void check_win()
{

  gint timeout, timeout_tux;

  if (currentUiMode == UIMODE_SOUND){
    timeout = 200;
    timeout_tux = 500;
  }
  else {
    timeout = 1000;
    timeout_tux = 2000;
  }

  // Check win
  if (compare_card((gpointer) firstCard, (gpointer) secondCard) == 0) {
    gc_sound_play_ogg ("sounds/gobble.ogg", NULL);
    win_id = g_timeout_add (timeout,
			    (GSourceFunc) hide_card, NULL);
    return;
  }

  if (currentMode == MODE_TUX){
		 /* time to tux to play, after a delay */
    to_tux = TRUE;
    g_warning("Now tux will play !");
    tux_id = g_timeout_add (timeout_tux,
			    (GSourceFunc) tux_play, NULL);
    return;
  }

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

	   if (currentMode == MODE_TUX){
	     if (to_tux){
	       g_warning("He ! it's tux turn !");
	       return FALSE;
	     }
	   }

	   if (playing_sound){
	     g_warning("wait a minute, the sound is playing !");
	     //return FALSE;
	   }

	   if(win_id)
	     return FALSE;

	   if(secondCard)
	     {
	       display_card(firstCard, ON_BACK);
	       firstCard = NULL;
	       display_card(secondCard, ON_BACK);
	       secondCard = NULL;
	     }

	   if(!firstCard)
	     {
	       firstCard = memoryItem;
	       if (currentMode == MODE_TUX)
		 add_card_in_tux_memory(memoryItem);
	       display_card(memoryItem, ON_FRONT);
	       return TRUE;
	     }
	   else
	     {
	       // Check he/she did not click on the same card twice
	       if(firstCard==memoryItem)
		 return FALSE;

	       secondCard = memoryItem;
	       if (currentMode == MODE_TUX)
		 add_card_in_tux_memory(memoryItem);
	       display_card(memoryItem, ON_FRONT);
	       if (currentUiMode == UIMODE_SOUND)
		 // Check win is called from callback return
		 return TRUE;
	       else {
		 check_win();
		 return TRUE;
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

void add_card_in_tux_memory(MemoryItem *card)
{
  MemoryItem *first = NULL;

  g_warning("Adding card %s in tux memory ", card->data);

  /* check if card is already in memory */
  remove_card_from_tux_memory(card);

  first = find_card_in_tux_memory(card);

  if (first){
    g_warning("found %s and %s !", first->data, card->data);
    WINNING *win = g_malloc0(sizeof(WINNING));
    win->first = card;
    win->second = first;
    winning_pairs = g_list_append( winning_pairs, win);
    g_warning("Now %d winning pairs in tux list! ", g_list_length(winning_pairs));

    remove_card_from_tux_memory(first);
  }

  g_queue_push_head(tux_memory, card);

  if (g_queue_get_length (tux_memory)>tux_memory_size){
    g_queue_pop_tail(tux_memory);

  g_warning("Now tuxmemory size = %d", g_queue_get_length (tux_memory));

  }
}

static gint
compare_card (gconstpointer a,
	      gconstpointer b)
{
  MemoryItem *card1 = (MemoryItem *)a;
  MemoryItem *card2 = (MemoryItem *)b;

  if (card1->type & (TYPE_ADD|TYPE_MINUS|TYPE_MULT|TYPE_DIV)){
    if ((!card1->second_value) && ( card2->second_value)){
      return strcmp(card1->data,card2->second_value);
    }
    if ((!card2->second_value) && ( card1->second_value)){
      return strcmp(card2->data,card1->second_value);
    }
    return -1;
  }
  return ((card1->data == card2->data) ? 0 : -1);
}

MemoryItem *find_card_in_tux_memory(MemoryItem *card)
{
  GList *link;

  if ((link = g_queue_find_custom(tux_memory, card, compare_card)) != NULL)
    return link->data;
  else
    return NULL;
}

void remove_card_from_tux_memory(MemoryItem *card)
{
  g_queue_remove(tux_memory, card);
}

static gint tux_play(){
  int rx, ry;

  if (Paused){
    g_warning("Paused");
    return TRUE;
  }

  g_warning("Now tux playing !");

  if(secondCard)
    {
      display_card(firstCard, ON_BACK);
      firstCard = NULL;
      display_card(secondCard, ON_BACK);
      secondCard = NULL;
    }

  if (winning_pairs){
    g_warning("I will won !");
    if (!firstCard){
      g_warning("case 1");
      firstCard = ((WINNING *) winning_pairs->data)->first ;
      display_card(firstCard, ON_FRONT);
      if (currentUiMode == UIMODE_SOUND)
	return FALSE;
      else
	return TRUE;
    } else {
      g_warning("case 2");
      secondCard = ((WINNING *) winning_pairs->data)->second;
      display_card(secondCard, ON_FRONT);
      if (currentUiMode == UIMODE_SOUND)
	return FALSE;
      else {
	gc_sound_play_ogg ("sounds/gobble.ogg", NULL);
	win_id = g_timeout_add (1000,
				(GSourceFunc) hide_card, NULL);
	return TRUE;
      }
    }
  }

  // Randomly set the pair
  rx = g_random_int_range( 0, numberOfColumn);
  ry = g_random_int_range(0, numberOfLine);

  gboolean  stay_unknown = (remainingCards > (g_queue_get_length (tux_memory)
					      + (firstCard ? 1 : 0)));

  g_warning("remainingCards %d tux_memory %d -> stay_unknown %d ",
	    remainingCards,
	    g_queue_get_length (tux_memory),
	    stay_unknown );

  while((memoryArray[rx][ry]->hidden) || (memoryArray[rx][ry] == firstCard)
	|| (stay_unknown && g_queue_find(tux_memory,memoryArray[rx][ry])))
    {
      g_warning("Loop to find %d %d %s", rx, ry, memoryArray[rx][ry]->data);
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

  if (!firstCard){
    g_warning("case 3");
    firstCard = memoryArray[rx][ry];
    add_card_in_tux_memory(firstCard);
    display_card(firstCard, ON_FRONT);
    g_warning("Now tux replay !");
    if (currentUiMode == UIMODE_SOUND)
      return FALSE;
    else
      return TRUE;
  } else {
    g_warning("case 4");
    secondCard = memoryArray[rx][ry];
    add_card_in_tux_memory(secondCard);
    display_card(secondCard, ON_FRONT);
    if (currentUiMode == UIMODE_SOUND)
      return FALSE;
    else {
      if (compare_card(firstCard, secondCard)==0){
	gc_sound_play_ogg ("sounds/gobble.ogg", NULL);
	g_warning("Now tux win !");
	win_id = g_timeout_add (1000,
				(GSourceFunc) hide_card, NULL);
	return TRUE;
      } else{
	to_tux = FALSE;
	return FALSE;
      }
    }
  }
  return FALSE;
}

static void sound_callback(gchar *file)
{
  if (! gcomprisBoard)
    return;

  g_warning("sound_callback %s", file);

  playing_sound = FALSE;
  if (currentMode == MODE_TUX){
    if (to_tux) {
      if (firstCard && secondCard){
	if (compare_card(firstCard, secondCard)==0){
	  gc_sound_play_ogg ("sounds/gobble.ogg", NULL);
	  win_id = g_timeout_add (1000,
				  (GSourceFunc) hide_card, NULL);
	  tux_id = g_timeout_add (2000,
				  (GSourceFunc) tux_play, NULL);
	  return;
	} else {
	  to_tux = FALSE;
	  return;
	}
      } else {
	g_warning("Now tux will replay !");
	tux_id = g_timeout_add (2000,
			      (GSourceFunc) tux_play, NULL);
	return;
      }
    }
  }
  if (firstCard && secondCard)
    check_win();
}


static void start_callback(gchar *file){
  if (!gcomprisBoard)
    return;

  if (currentUiMode != UIMODE_SOUND)
    return;

  playing_sound = FALSE;
}
