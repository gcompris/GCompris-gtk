/* gcompris - scale.c
 *
 * Copyright (C) 2006, 2008 Miguel de Izarra
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

#include "gcompris/gcompris.h"
#include <string.h> // for strcmp

#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void config_start(GcomprisBoard *agcomprisBoard,
			 GcomprisProfile *aProfile);
static void config_stop();
static void set_level (guint level);
static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str);
static void process_ok(void);
static int gamewon;
static void game_won(void);

static GooCanvasItem *boardRootItem = NULL;
static GooCanvasItem *group_g, *group_d, *group_m;
static GooCanvasItem *bras;
static GooCanvasItem *answer_item;
static GString *answer_string = NULL;

static void scale_destroy_all_items(void);
static void scale_next_level(void);

#define MODE_COUNT 0	// No conversions
#define MODE_WEIGHT 1	// kg <-> g conversions
#define DEFAULT_MODE MODE_COUNT
static gint board_mode = DEFAULT_MODE;
static gint bonus_item = GC_BONUS_SMILEY;

#define ITEM_X_MIN 125
#define ITEM_X_MAX 500
#define ITEM_Y_MIN 400
#define ITEM_Y_MAX 500
#define ITEM_W 45
#define ITEM_H 70

#define PLATE_Y 33.0
#define PLATE_Y_DELTA 15.0
#define PLATE_W 190.0

#define BRAS_X 55.0
#define BRAS_Y -6.0

#define PLATE_SIZE 4 // number of place in the plate

typedef struct
{
  GooCanvasItem *item;
  double x, y;
  int plate;        // 0 not place, 1 in plate_g, -1 in plate_d
  int plate_index;  // position in the plate
  int weight;
} ScaleItem;

static GList *item_list = NULL;
static int objet_weight = 0;
static gboolean ask_for_answer = FALSE;
static gdouble balance_left_y;
static gdouble balance_right_y;
static gdouble balance_left_x;
static gdouble balance_right_x;
static gdouble last_delta=0;
static gint drag_mode;
static const char *imageList[] = {
  "scale/chocolate_cake.png",
  "scale/pear.png",
  "scale/orange.png",
  "scale/suggar_box.png",
  "scale/flowerpot.png",
  "scale/glass.png"
};

static const int imageListCount = G_N_ELEMENTS(imageList);

static int scale_drag_event(GooCanvasItem *w,
			    GooCanvasItem *target,
			    GdkEvent *event,
			    ScaleItem *item);

/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Scale",
    "Balance the scales properly",
    "Miguel de Izarra <miguel2i@free.fr>",
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
    NULL,
    config_start,
    config_stop
  };

/*
 * Main entry point mandatory for each Gcompris's game
 * ---------------------------------------------------
 *
 */

GET_BPLUGIN_INFO(scale)

/*
 * in : boolean TRUE = PAUSE : FALSE = CONTINUE
 *
 */
static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  if(gamewon == TRUE && pause == FALSE) /* the game is won */
    {
      game_won();
    }

  board_paused = pause;
}

static void
start_board (GcomprisBoard *agcomprisBoard)
{
  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard=agcomprisBoard;
      gcomprisBoard->level=1;
      gcomprisBoard->sublevel=1;

      gc_bar_set(GC_BAR_LEVEL|GC_BAR_CONFIG);
      gc_bar_location(-1, -1, 0.7);

      if (strcmp(gcomprisBoard->mode, "count") == 0)
        board_mode = MODE_COUNT;
      else if (strcmp(gcomprisBoard->mode, "weight") == 0)
        board_mode = MODE_WEIGHT;
      else
        board_mode = DEFAULT_MODE;

      gcomprisBoard->maxlevel = (board_mode == MODE_COUNT) ? 4 : 5;
      gcomprisBoard->number_of_sublevel = (board_mode == MODE_COUNT) ? 5 : 3;
      bonus_item = (board_mode == MODE_COUNT) ? GC_BONUS_SMILEY : GC_BONUS_TUX;

      gamewon = FALSE;
      pause_board(FALSE);

      GHashTable *config = gc_db_get_board_conf();
      gchar *drag_mode_str = g_hash_table_lookup( config, "drag_mode");

      if (drag_mode_str && (strcmp (drag_mode_str, "NULL") != 0))
	drag_mode = g_ascii_strtod(drag_mode_str, NULL);
      else
	drag_mode = GC_DRAG_MODE_DEFAULT;

      gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			"scale/tabepice.jpg");

      gc_drag_start(goo_canvas_get_root_item(gcomprisBoard->canvas),
		    (GcDragFunc)scale_drag_event, drag_mode);

      gc_score_start(SCORESTYLE_NOTE,
		     BOARDWIDTH - 220,
		     450 ,
		     gcomprisBoard->number_of_sublevel);
      scale_next_level();

    }
}

static void
end_board ()
{
  if(gcomprisBoard!=NULL)
    {
      gc_drag_stop(goo_canvas_get_root_item(gcomprisBoard->canvas));
      pause_board(TRUE);
      scale_destroy_all_items();
      gc_score_end();
    }
  gcomprisBoard = NULL;
}

/* ======================================= */
static void
set_level (guint level)
{

  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level=level;
      gcomprisBoard->sublevel=1;
      scale_next_level();
    }
}
/* ======================================= */
gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    if(g_strcasecmp(gcomprisBoard->type, "scale")==0)
      {
	/* Set the plugin entry */
	gcomprisBoard->plugin=&menu_bp;

	return TRUE;
      }
  return FALSE;
}

/* ======================================= */
static gint
key_press(guint keyval, gchar *commit_str, gchar *preedit_str)
{
  if(!gcomprisBoard)
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
    case GDK_Left:
      break;
    case GDK_BackSpace:
      if(answer_string)
	g_string_truncate(answer_string, answer_string->len -1);
      break;
    }

  if(answer_string)
    {
      gchar *tmpstr;
      gchar c = commit_str ? commit_str[0] : 0;

      /* Limit the user entry to 5 digits */
      if(c>='0' && c<='9' && answer_string->len < 5)
	answer_string = g_string_append_c(answer_string, c);

      if (board_mode == MODE_WEIGHT)
        tmpstr = g_strdup_printf(_("Weight in g = %s"), answer_string->str);
      else
        tmpstr = g_strdup_printf(_("Weight = %s"), answer_string->str);
      g_object_set(answer_item,
		   "text", tmpstr,
		   NULL);
      g_free(tmpstr);
    }

  return TRUE;
}

// plate = 1 plate g (left)
// plate = -1 plate d (right)
// plate = 0 plate g - plate d
int
get_weight_plate(int plate)
{
  GList *list;
  ScaleItem *item;
  int result=0;

  for(list = item_list; list; list=list->next)
    {
      item = list->data;
      if(item->plate == plate || plate==0)
        {
	  result += item->weight * item->plate;
        }
    }
  if(plate==-1)
    result = -result;
  return result;
}

void
scale_anim_plate(void)
{
  double delta_y;
  double angle;
  double scale;
  int diff;

  // in MODE_WEIGHT the granularity is gramm, so we use a different factor
  scale = (board_mode == MODE_WEIGHT) ? 2000.0 : 10.0;
  diff = get_weight_plate(0);
  delta_y = CLAMP(PLATE_Y_DELTA / scale * diff,
		  -PLATE_Y_DELTA, PLATE_Y_DELTA);

  if(get_weight_plate(1) == 0)
    delta_y = -PLATE_Y_DELTA;

  if(last_delta != delta_y)
    {
      goo_canvas_item_translate(group_g, 0, -last_delta);
      goo_canvas_item_translate(group_d, 0, last_delta);

      last_delta = delta_y;

      angle = tan(delta_y / 138) * 180 / M_PI;

      goo_canvas_item_translate(group_g, 0, delta_y);
      goo_canvas_item_translate(group_d, 0, -delta_y);

      gc_item_rotate_with_center(bras, -angle, 138, 84);
    }

  if(diff == 0 && ask_for_answer)
    {

      double x_offset = BOARDWIDTH/2;
      double y_offset = BOARDHEIGHT*0.7;

      GooCanvasItem *item = goo_canvas_svg_new (boardRootItem,
						gc_skin_rsvg_get(),
						"svg-id", "#BUTTON_TEXT",
						NULL);
      SET_ITEM_LOCATION_CENTER(item,
			       x_offset / 2,
			       y_offset);
      goo_canvas_item_scale(item, 2, 1);

      answer_item = goo_canvas_text_new(boardRootItem,
					"",
					x_offset,
					y_offset,
					-1,
					GTK_ANCHOR_CENTER,
					"font", gc_skin_font_board_title_bold,
					"fill-color", "white",
					NULL);

      answer_string = g_string_new(NULL);
      key_press(0, NULL, NULL);
    }
  else if(diff == 0)
    process_ok();
}

// if plate = 1 , move to plate g (left)
// if plate = -1, move to plate d (right)
// if plate = 0 , move to the item list
void
scale_item_move_to(ScaleItem *item, int plate)
{
  ScaleItem *scale;
  GList *list;
  gboolean found;
  int index;

  if(plate != 0)
    {
      if(item->plate)
	item->plate_index = -1;
      else
	gc_sound_play_ogg ("sounds/eraser1.wav", NULL);

      // find the first free place in the plate
      for(index=0; index < 2 * PLATE_SIZE; index++)
        {
	  found = FALSE;
	  for(list = item_list; list; list = list->next)
            {
	      scale = list->data;
	      if(scale->plate_index == index && scale->plate == plate)
		found=TRUE;
            }

	  if(!found)
            {   // move to the plate
	      gdouble x = (plate == 1 ? balance_left_x : balance_right_x);
	      gdouble y = (plate == 1
			   ? balance_left_y + last_delta
			   : balance_right_y - last_delta);

	      item->plate = plate;
	      item->plate_index = index;

	      /* Reparent */
	      g_object_ref(item->item);
	      goo_canvas_item_remove(item->item);
	      goo_canvas_item_add_child ((plate == 1 ? group_g : group_d),
					 item->item, -1);
	      g_object_unref(item->item);

	      gc_item_absolute_move(item->item,
				    x + (index % PLATE_SIZE) * ITEM_W,
				    y + PLATE_Y - ITEM_H + 5 - (index >= PLATE_SIZE ? ITEM_H : 0));
	      break;
            }
        }
      if(found)   // can't find place
	plate=0;
    }

  if(plate==0)
    {   // move the item to the list
      if(item->plate)
	gc_sound_play_ogg ("sounds/eraser1.wav", NULL);
      item->plate = 0;
      /* Reparent */
      g_object_ref(item->item);
      goo_canvas_item_remove(item->item);
      goo_canvas_item_add_child (group_m,
				 item->item, -1);
      g_object_unref(item->item);

      gc_item_absolute_move(item->item,
			    item->x,
			    item->y);
      goo_canvas_item_raise(item->item, NULL);
    }

  scale_anim_plate();
  if (!gamewon)
    gc_item_focus_init(item->item, NULL);
}

static gboolean
scale_item_event (GooCanvasItem *item,
		  GooCanvasItem *target,
		  GdkEventButton *event,
		  ScaleItem *scale)
{
  if(answer_string)
    return FALSE;

  if(event->type == GDK_BUTTON_PRESS && event->button==3)
    scale_item_move_to(scale, 0);

  return FALSE;
}

static int
scale_drag_event(GooCanvasItem *w,
		 GooCanvasItem *target,
		 GdkEvent *event,
		 ScaleItem *scale)
{
  double x, y;
  double item_x, item_y;

  if(answer_string)   // disable, waiting a answer
    return FALSE;

  switch(event->type)
    {
    case GDK_BUTTON_PRESS:
      gc_drag_offset_save(event);
      goo_canvas_item_raise(goo_canvas_item_get_parent(scale->item),
			    NULL);
      goo_canvas_item_raise(scale->item, NULL);
      break;

    case GDK_MOTION_NOTIFY:
      gc_drag_item_move(event, NULL);
      break;

    case GDK_BUTTON_RELEASE:
      {
	int plate;

	item_x = event->button.x;
	item_y = event->button.y;

	goo_canvas_convert_from_item_space(goo_canvas_item_get_canvas(w),
					   scale->item, &item_x, &item_y);

	x = item_x;
	y = item_y;

	goo_canvas_convert_to_item_space(goo_canvas_item_get_canvas(w),
					 group_g, &x, &y);

	if( -ITEM_W < x
	    && x < PLATE_W + ITEM_W
	    && abs(y - PLATE_Y) < ITEM_H)
	  plate = 1;
	else
	  {
	    x = item_x;
	    y = item_y;
	    goo_canvas_convert_to_item_space(goo_canvas_item_get_canvas(w),
					     group_d, &x, &y);

	    if( -ITEM_W < x
		&& x < PLATE_W + ITEM_W
		&& abs(y - PLATE_Y) < ITEM_H)
	      plate = -1;
	    else
	      plate=0;
	  }
	scale_item_move_to(scale, plate);
      }
      break;
    default:
      break;
    }

  return FALSE;
}

// show_weight:
// 0 -> no unit,
// 1 -> display g, 1000 -> display kg,
// 10000 -> random (weighted 1:2)
static ScaleItem *
scale_list_add_weight(GooCanvasItem *group,
		      gint weight, int show_weight)
{
  ScaleItem *new_item;
  GdkPixbuf *pixmap;
  gchar *weight_text;
  double x, y;
  GList *last;

  last = g_list_last(item_list);
  if(last)
    {
      new_item = last->data;
      x = new_item->x + ITEM_W;
      y = new_item->y;
      if(x > ITEM_X_MAX)
        {
	  x = ITEM_X_MIN;
	  y += ITEM_H;
	  if(y > ITEM_Y_MAX)
	    g_warning("No more place for new item");
        }
    }
  else
    {
      x = ITEM_X_MIN;
      y = ITEM_Y_MIN;
    }

  new_item = g_new0(ScaleItem, 1);
  new_item->x = x;
  new_item->y = y;
  new_item->weight = weight;

  /* If selected, display multiples of 500g as 'g' or 'kg' randomly */
  if (show_weight > 9999) {
    if ((weight % 500) == 0)
  	show_weight = g_random_int_range(1,3000);
    else
	show_weight = 1;
  }

  if (show_weight < 1000) {
    	weight_text = g_strdup_printf("%d%s", weight, show_weight ? "\n  g" : "");
  } else {
  	int thousand = weight / 1000;
  	int hundred = (weight % 1000) / 100;
    	weight_text = g_strdup_printf("%c %c\n  kg", '0' + thousand, '0' + hundred);
  }
  pixmap = gc_pixmap_load("scale/masse.png");

  new_item->item = goo_canvas_group_new(group, NULL);
  goo_canvas_image_new(new_item->item,
		       pixmap,
		       0, 0,
		       NULL);
  goo_canvas_text_new(new_item->item,
		      weight_text,
		      18,
		      35,
		      -1,
		      GTK_ANCHOR_CENTER,
		      "font", "sans 10",
		      "fill_color_rgba", 0x000000FFL,
		      NULL);

  goo_canvas_item_translate(new_item->item,
			    new_item->x,
			    new_item->y);
  gdk_pixbuf_unref(pixmap);
  g_free(weight_text);

  gc_item_focus_init(new_item->item, NULL);
  g_signal_connect(new_item->item, "button_press_event",
		   (GtkSignalFunc)gc_drag_event, new_item);
  g_signal_connect(new_item->item, "button_release_event",
		   (GtkSignalFunc)gc_drag_event, new_item);
  g_signal_connect(new_item->item, "button_press_event",
  		   (GtkSignalFunc) scale_item_event, new_item);

  item_list = g_list_append(item_list, new_item);
  return new_item;
}

// show_weight:
// 0 -> no display,
// 1 -> display g,
// 1000 -> display kg, 10000 -> random (weighted 1:2)
static ScaleItem *
scale_list_add_object(GooCanvasItem *group,
		      GdkPixbuf *pixmap,
		      int weight, int plate,
		      int show_weight)
{
  GooCanvasItem *item;
  ScaleItem * new_item;

  item = goo_canvas_image_new(group,
			      pixmap,
			      PLATE_SIZE * ITEM_W * .5
			      - gdk_pixbuf_get_width(pixmap)/2,
			      PLATE_Y + 5 - gdk_pixbuf_get_height(pixmap),
			      NULL);
  goo_canvas_item_lower(item, NULL);

  if(show_weight)
    {   // display the object weight
      double x,y;
      gchar * text;

      x = PLATE_SIZE * ITEM_W * .5;
      y = PLATE_Y - 20.0;
      if (board_mode == MODE_WEIGHT) {
      	if (show_weight > 9999) {
          if ((weight % 500) == 0)
      	    show_weight = g_random_int_range(1,3000);
	  else
	    show_weight = 1;
        }

      	if (show_weight < 1000)
      		text = g_strdup_printf("%d g", objet_weight);
      	else
      		text = g_strdup_printf("%.1f kg", objet_weight / 1000.0);
      } else {
      	text = g_strdup_printf("%d", objet_weight);
      }

      goo_canvas_text_new(group,
			  text,
			  x + 1.0,
			  y + 1.0,
			  -1,
			  GTK_ANCHOR_CENTER,
			  "font", gc_skin_font_board_title_bold,
			  "fill_color_rgba", gc_skin_color_shadow,
			  NULL);
      goo_canvas_text_new(group,
			  text,
			  x,
			  y,
			  -1,
			  GTK_ANCHOR_CENTER,
			  "font", gc_skin_font_board_title_bold,
			  "fill_color_rgba", gc_skin_color_content,
			  NULL);
      g_free(text);
    }

  new_item = g_new0(ScaleItem, 1);
  new_item->weight = weight;
  new_item->plate = plate;
  new_item->plate_index = -1;
  new_item->item = item;

  item_list = g_list_append(item_list, new_item);
  return new_item;
}

// test if adding elements in table can produce total
static gboolean
test_addition(int total, int *table, int len)
{
  int i;

  if(total == 0)
    return TRUE;

  for(i=0; i<len; i++)
    {
      if(table[i] <= total && table[i] != 0)
        {
	  gboolean result;
	  int cur;
	  cur = table[i];
	  table[i] = 0;
	  result = test_addition(total-cur, table, len);
	  table[i] = cur;
	  if(result)
	    return TRUE;
        }
    }
  return FALSE;
}

static void
scale_prepare_level()
{
  GdkPixbuf *pixmap;
  int show_weight = 0;
  const int default_list_weight[10] = { 1, 2, 2, 5, 5, 10, 10};
  int list_weight[10]= {0};
  int i, tmp[5];

  group_m = goo_canvas_group_new(boardRootItem, NULL);

  switch(gcomprisBoard->level)
    {
    case 1:
    case 2:
      objet_weight = g_random_int_range(5,20);
      for(i=0; i<10; i++)
	list_weight[i] = default_list_weight[i];
      show_weight = gcomprisBoard->level == 1;
      break;
    case 3:
    case 4:
      while(1)
	{
	  for(i=0; i< 5; i++)
	    do
	      tmp[i] = default_list_weight[g_random_int_range(0,10)];
	    while(tmp[i]==0);

	  objet_weight=0;
	  for(i=0; i<5; i++)
	    objet_weight += g_random_int_range(-1,2) * tmp[i];
	  objet_weight = abs(objet_weight);

	  if(!test_addition(objet_weight, tmp, 5))
	    break;
	}
      for(i=0; i<5; i++)
	list_weight[i] = tmp[i];
      show_weight = gcomprisBoard->level == 3;
      break;
    }

  ask_for_answer = !show_weight;
  for(i=0; list_weight[i] ; i++)
    scale_list_add_weight(group_m, list_weight[i], 0);

  pixmap = gc_pixmap_load(imageList[g_random_int_range(0,imageListCount)]);
  scale_list_add_object(group_d, pixmap, objet_weight, -1, show_weight);
  gdk_pixbuf_unref(pixmap);
}

// Defines a set of available weight-items and possible values for the "unknown" object
struct weight_set {
	const int list_weight[10];
	int range_min;
	int range_max;
};

static struct weight_set weight1 = {
    	.list_weight = { 100,200,400,500,800,1000 },
	.range_min = 500,
	.range_max = 2000,
};

static struct weight_set weight2 = {
    	.list_weight = { 100,100,200,500,1000,1500,2000,2500,3000 },
	.range_min = 200,
	.range_max = 9000,
};
static struct weight_set weight3 = {
   	.list_weight = { 100,200,300,600,900,1200,1500 },
	.range_min = 500,
	.range_max = 3000,
};
static struct weight_set weight4 = {
   	.list_weight = { 100,100,200,200,200,500,500,1000 },
	.range_min = 300,
	.range_max = 2500,
};

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static void scale_prepare_level_weight()
{
    GdkPixbuf *pixmap;
    int show_weight_objet = 1;
    int show_weight_mass = 1;

    // A level may use the following weight-sets
    struct weight_set *set1[] = { &weight1, &weight2, &weight3, &weight4 };
    struct weight_set *set2[] = { &weight1, &weight2, &weight4 };
    struct weight_set *set3[] = { &weight1, &weight2, &weight3, &weight4 };
    struct weight_set *set4[] = { &weight1, &weight2, &weight4 };
    struct weight_set *set5[] = { &weight1, &weight4 };
    struct weight_set *set = &weight1;
    const int *list_weight;
    int i, tmp[6] = { 0 };

    group_m = goo_canvas_group_new(boardRootItem, NULL);
    switch(gcomprisBoard->level)
    {
        case 1:
	    set = set1[g_random_int_range(0,ARRAY_SIZE(set1))];
            objet_weight = g_random_int_range(set->range_min / 100, set->range_max / 100) * 100;
            list_weight = set->list_weight;
            show_weight_objet = 1;
            show_weight_mass = 1;
            break;
	case 2:
	    set = set2[g_random_int_range(0,ARRAY_SIZE(set2))];
            objet_weight = g_random_int_range(set->range_min / 100, set->range_max / 100) * 100;
            list_weight = set->list_weight;
            show_weight_objet = 10000;
            show_weight_mass = 10000;
            break;
        case 3:
	    set = set3[g_random_int_range(0,ARRAY_SIZE(set3))];
            objet_weight = g_random_int_range(set->range_min / 100, set->range_max / 100) * 100;
            list_weight = set->list_weight;
            show_weight_objet = 0;
            show_weight_mass = 1;
            break;
        case 4:
	    set = set4[g_random_int_range(0,ARRAY_SIZE(set4))];
            objet_weight = g_random_int_range(set->range_min / 100, set->range_max / 100) * 100;
            list_weight = set->list_weight;
            show_weight_objet = 0;
            show_weight_mass = 10000;
            break;
	case 5:
	    set = set5[g_random_int_range(0,ARRAY_SIZE(set5))];
            objet_weight = g_random_int_range(set->range_min / 100, set->range_max / 100) * 100;
            list_weight = set->list_weight;
            while(1)
            {
                for(i=0; i< 5; i++)
                    do
                        tmp[i] = list_weight[g_random_int_range(0,10)];
                    while(tmp[i]==0);

                objet_weight=0;
                for(i=0; i<5; i++)
                    objet_weight += g_random_int_range(-1,2) * tmp[i];
                objet_weight = abs(objet_weight);

                if(!test_addition(objet_weight, tmp, 5))
                    break;
            }
            list_weight = tmp;
            show_weight_objet = 0;
            show_weight_mass = 10000;
            break;
    }

    ask_for_answer = !show_weight_objet;

    for(i=0; list_weight[i] ; i++)
        scale_list_add_weight(group_m, list_weight[i], show_weight_mass);

    pixmap = gc_pixmap_load(imageList[g_random_int_range(0,imageListCount)]);
    scale_list_add_object(group_d, pixmap, objet_weight,-1, show_weight_objet);
    gdk_pixbuf_unref(pixmap);
}

static void
scale_next_level()
{
  GdkPixbuf *pixmap, *pixmap2;
  GooCanvasItem *item, *balance;
  gdouble balance_x;

  gc_bar_set_level(gcomprisBoard);

  scale_destroy_all_items();
  gamewon = FALSE;

  gc_score_set(gcomprisBoard->sublevel);

  // create the scale
  pixmap = gc_pixmap_load("scale/balance.png");
  balance_x = (BOARDWIDTH - gdk_pixbuf_get_width(pixmap))/2;
  balance_left_y = balance_right_y = \
    (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap))/2;

  boardRootItem = \
    goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
			  NULL);

  pixmap2 = gc_pixmap_load("scale/bras.png");
  bras = goo_canvas_image_new(boardRootItem,
			      pixmap2,
			      BOARDWIDTH/2 - gdk_pixbuf_get_width(pixmap2)/2,
			      balance_left_y - 10,
			      NULL);
  gdk_pixbuf_unref(pixmap2);

  balance = goo_canvas_image_new(boardRootItem,
				 pixmap,
				 balance_x,
				 balance_left_y,
				 NULL);
  gdk_pixbuf_unref(pixmap);

  /* create left plate */
  group_g = goo_canvas_group_new (boardRootItem,
				  NULL);
  balance_left_x = balance_x - 40;
  goo_canvas_item_translate(group_g,
			    balance_left_x,
			    balance_left_y);


  pixmap = gc_pixmap_load("scale/plateau.png");
  item = goo_canvas_image_new(group_g,
			      pixmap,
			      0,
			      PLATE_Y,
			      NULL);
  gdk_pixbuf_unref(pixmap);

  /* create right plate */
  group_d = goo_canvas_group_new(boardRootItem, NULL);
  balance_right_x = BOARDWIDTH/2 + 40;
  goo_canvas_item_translate(group_d,
			    balance_right_x,
			    balance_right_y);

  pixmap = gc_pixmap_load("scale/plateau.png");
  pixmap2 = gdk_pixbuf_flip(pixmap, TRUE);
  item = goo_canvas_image_new(group_d,
			      pixmap2,
			      0,
			      PLATE_Y,
			      NULL);
  gdk_pixbuf_unref(pixmap);
  gdk_pixbuf_unref(pixmap2);

  /* display some hint */
  if(gcomprisBoard->level > 2 || board_mode == MODE_WEIGHT)
    goo_canvas_text_new(boardRootItem,
			_("Take care, you can drop masses on both sides of the scale."),
			BOARDWIDTH/2,
			ITEM_Y_MIN - 20,
			-1,
			GTK_ANCHOR_CENTER,
			"font", gc_skin_font_board_medium,
			"fill-color", "darkblue",
			NULL);

  if (board_mode == MODE_COUNT)
    scale_prepare_level();
  else
    scale_prepare_level_weight();

  last_delta=0;
  scale_anim_plate();
}

static void scale_destroy_all_items()
{
  GList *list;

  if(item_list)
    {
      for(list = item_list; list ; list = list->next)
	g_free(list->data);
      g_list_free(item_list);
      item_list = NULL;
    }
  if(answer_string)
    {
      g_string_free(answer_string, TRUE);
      answer_string = NULL;
    }

  if(boardRootItem)
    goo_canvas_item_remove(boardRootItem);
  boardRootItem = NULL;

}

static void game_won()
{
  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel > gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->sublevel=1;
    gcomprisBoard->level++;
    if(gcomprisBoard->level > gcomprisBoard->maxlevel)
      gcomprisBoard->level = gcomprisBoard->maxlevel;

    gc_sound_play_ogg ("sounds/bonus.wav", NULL);
  }
  scale_next_level();
}

/* ==================================== */
static void process_ok()
{
  gboolean good_answer = TRUE;

  if(board_paused)
    return;

  if(answer_string)
    {
      gint answer_weight ;
      answer_weight = g_strtod(answer_string->str, NULL);
      good_answer = answer_weight == objet_weight;
    }
  if(get_weight_plate(0)== 0 && good_answer)
    {
      gamewon = TRUE;
      scale_destroy_all_items();
      gc_bonus_display(gamewon, bonus_item);
    }
  else
    gc_bonus_display(gamewon, bonus_item);
}

/* ************************************* */
/* *            Configuration          * */
/* ************************************* */


/* ======================= */
/* = config_start        = */
/* ======================= */

static GcomprisProfile *profile_conf;
static GcomprisBoard   *board_conf;

static void save_table (gpointer key,
			gpointer value,
			gpointer user_data)
{
  gc_db_set_board_conf ( profile_conf,
			 board_conf,
			 (gchar *) key,
			 (gchar *) value);
}

static void conf_ok(GHashTable *table)
{
  if (!table){
    if (gcomprisBoard)
      pause_board(FALSE);
    return;
  }

  g_hash_table_foreach(table, (GHFunc) save_table, NULL);

  if (gcomprisBoard){
    GHashTable *config;

    if (profile_conf)
      config = gc_db_get_board_conf();
    else
      config = table;

    gchar *drag_mode_str = g_hash_table_lookup( config, "drag_mode");

    if (drag_mode_str && (g_strcasecmp (drag_mode_str, "NULL") != 0))
      drag_mode = (gint ) g_ascii_strtod(drag_mode_str, NULL);
    else
      drag_mode = 0;

    if (profile_conf)
      g_hash_table_destroy(config);

    gc_drag_change_mode(drag_mode);

    scale_next_level();

    pause_board(FALSE);
  }

  board_conf = NULL;
  profile_conf = NULL;

}

static void
config_start(GcomprisBoard *agcomprisBoard,
	     GcomprisProfile *aProfile)
{
  board_conf = agcomprisBoard;
  profile_conf = aProfile;

  if (gcomprisBoard)
    pause_board(TRUE);

  gchar * label = g_strdup_printf(_("<b>%s</b> configuration\n for profile <b>%s</b>"),
				  agcomprisBoard->name,
				  aProfile? aProfile->name : "");

  GcomprisBoardConf *bconf;
  bconf = gc_board_config_window_display( label,
					  (GcomprisConfCallback )conf_ok);

  g_free(label);

  /* init the combo to previously saved value */
  GHashTable *config = gc_db_get_conf( profile_conf, board_conf);

  gchar *drag_mode_str = g_hash_table_lookup( config, "drag_mode");
  gint drag_previous;

  if (drag_mode_str && (strcmp (drag_mode_str, "NULL") != 0))
    drag_previous = (gint ) g_ascii_strtod(drag_mode_str, NULL);
  else
    drag_previous = 0;

  gc_board_config_combo_drag(bconf, drag_mode);

}


/* ======================= */
/* = config_stop        = */
/* ======================= */
static void config_stop()
{
}

