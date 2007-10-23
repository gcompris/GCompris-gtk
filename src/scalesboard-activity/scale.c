/* gcompris - scale.c
 *
 * Copyright (C) 2006 Miguel de Izarra
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
static GooCanvasItem *group_g = NULL, *group_d= NULL;
static GooCanvasItem *bras = NULL;
static GooCanvasItem *answer_item = NULL;
GString *answer_string = NULL;

static void scale_destroy_all_items(void);
static void scale_next_level(void);

#define ITEM_X_MIN 0
#define ITEM_X_MAX 500
#define ITEM_Y_MIN 250
#define ITEM_Y_MAX 400
#define ITEM_W 45
#define ITEM_H 70

#define PLATE_X1 -40.0
#define PLATE_X2 246.0
#define PLATE_Y 33.0
#define PLATE_Y_DELTA 15.0
#define PLATE_W 190.0

#define BRAS_X 55.0
#define BRAS_Y -6.0

#define PLATE_SIZE 4 // number of place in the plate

typedef struct {
    GooCanvasItem * item;
    double x, y;
    int plate;        // 0 not place, 1 in plate_g, -1 in plate_d
    int plate_index;  // position in the plate
    int weight;
} ScaleItem;

static GList *item_list = NULL;
static int objet_weight = 0;

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

static int scale_drag_event(GooCanvasItem *w, GdkEvent *event, ScaleItem *item);

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
    process_ok,
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

static void start_board (GcomprisBoard *agcomprisBoard)
{
    if(agcomprisBoard!=NULL)
    {
        gcomprisBoard=agcomprisBoard;
        gcomprisBoard->level=1;
        gcomprisBoard->sublevel=1;
        gcomprisBoard->number_of_sublevel=5; /* Go to next level after this number of 'play' */
        gcomprisBoard->maxlevel = 4;
        gc_bar_set(GC_BAR_LEVEL|GC_BAR_OK|GC_BAR_CONFIG);

        gamewon = FALSE;
        pause_board(FALSE);

        GHashTable *config = gc_db_get_board_conf();
        gchar *drag_mode_str = g_hash_table_lookup( config, "drag_mode");

        if (drag_mode_str && (strcmp (drag_mode_str, "NULL") != 0))
            drag_mode = g_ascii_strtod(drag_mode_str, NULL);
        else
            drag_mode = 0;

	gc_set_background(goo_canvas_get_root_item(gcomprisBoard->canvas),
			  "scale/tabepice.jpg");

        gc_drag_start(goo_canvas_get_root_item(gcomprisBoard->canvas),
                (gc_Drag_Func)scale_drag_event, drag_mode);

	gc_score_start(SCORESTYLE_NOTE,
		gcomprisBoard->width - 220,
		450 ,
		gcomprisBoard->number_of_sublevel);
        scale_next_level();

    }
}

static void end_board ()
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
static void set_level (guint level)
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
static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str)
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

        tmpstr = g_strdup_printf(_("Weight = %s"), answer_string->str);
        g_object_set(answer_item,
                "text", tmpstr,
                NULL);
        g_free(tmpstr);
    }

    return TRUE;
}

// plate = 1 plate g
// plate = -1 plate d
// plate = 0 plate g - plate d
int get_weight_plate(int plate)
{
    GList *list;
    ScaleItem *item;
    int result=0;

    for(list = item_list; list; list=list->next)
    {
        item = list->data;
        if(item->plate == plate || plate==0)
        {
            result+= item->weight * item->plate;
        }
    }
    if(plate==-1)
        result = -result;
    return result;
}

static double last_delta=0;
void scale_anim_plate(void)
{
    double delta_y, x;
    double angle;
    int diff;

    diff = get_weight_plate(0);
    delta_y = CLAMP(PLATE_Y_DELTA / 10.0 * diff,
            -PLATE_Y_DELTA, PLATE_Y_DELTA);
    if(get_weight_plate(1)==0)
        delta_y = -PLATE_Y_DELTA;
    if(last_delta != delta_y)
    {
    last_delta = delta_y;

    angle = tan(delta_y / 138) * 180 / M_PI;

    g_object_get (group_g, "x", &x, NULL);
    goo_canvas_item_translate(group_g, x, delta_y);

    g_object_get (group_d, "x", &x, NULL);
    goo_canvas_item_translate(group_d, x, -delta_y);

    gc_item_rotate_with_center(bras, -angle, 138, 84);
    }
    if(diff ==0 && (gcomprisBoard->level == 2 || gcomprisBoard->level == 4))
    {
        GdkPixbuf *button_pixmap;
        double x_offset = 40, y_offset = 150;

        button_pixmap = gc_skin_pixmap_load("button_large2.png");
	int w = gdk_pixbuf_get_width(button_pixmap);
	int h = gdk_pixbuf_get_height(button_pixmap);
        goo_canvas_image_new (boardRootItem,
			      button_pixmap,
			      x_offset + w/2,
			      y_offset + h/2,
			      NULL);
        answer_item = goo_canvas_text_new(boardRootItem,
					  "",
					  x_offset + gdk_pixbuf_get_width(button_pixmap)/2,
					  y_offset + gdk_pixbuf_get_height(button_pixmap)/2,
					  -1,
					  GTK_ANCHOR_CENTER,
					  "font", gc_skin_font_board_title_bold,
					  "fill-color", "black",
					  NULL);
        gdk_pixbuf_unref(button_pixmap);

        answer_string = g_string_new(NULL);
        key_press(0,NULL,NULL);
    }
}

// if plate = 1 , move to plate g
// if plate = -1, move to plate d
// if plate = 0 , move to the item list
void scale_item_move_to(ScaleItem *item, int plate)
{
    ScaleItem *scale;
    GList *list;
    gboolean found;
    int index;

    if(plate!=0)
    {
        if(item->plate)
	  item->plate_index = -1;
	else
	  gc_sound_play_ogg ("sounds/eraser1.wav", NULL);

        // find the first free place in the plate
        for(index=0; index < PLATE_SIZE; index ++)
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
                item->plate = plate;
                item->plate_index=index;

		/* Reparent */
		g_object_ref(item->item);
		goo_canvas_item_add_child (goo_canvas_item_get_parent(plate == 1 ? group_g : group_d),
					   item->item, -1);
		g_object_unref(item->item);

                g_object_set(item->item,
                        "x", (double)index * ITEM_W,
                        "y", (double)PLATE_Y-ITEM_H + 5,
                        NULL);
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
		g_object_ref(item->item);
		goo_canvas_item_add_child (goo_canvas_item_get_parent(boardRootItem),
					   item->item, -1);
		g_object_unref(item->item);

        g_object_set(item->item,
                "x", item->x,
                "y", item->y, NULL);
    }

    scale_anim_plate();
}

static int scale_item_event(GooCanvasItem *w, GdkEvent *event, ScaleItem *scale)
{
    if(answer_string)
        return FALSE;

    if(event->type == GDK_BUTTON_PRESS && event->button.button==3)
                scale_item_move_to(scale, 0);
    return FALSE;
}

static int scale_drag_event(GooCanvasItem *w, GdkEvent *event, ScaleItem *scale)
{
    int plate=0;
    double x,y;

    if(answer_string)   // disable, waiting a answer
        return FALSE;

    switch(event->type)
    {
        case GDK_BUTTON_PRESS:
            gc_drag_offset_save(event);
            g_object_get(G_OBJECT(scale->item), "x", &x, "y", &y, NULL);
            //goo_canvas_item_i2w(scale->item, &x,&y);
	    g_object_ref(scale->item);
	    goo_canvas_item_add_child (goo_canvas_item_get_parent(boardRootItem),
				       scale->item, -1);
	    g_object_unref(scale->item);
            //goo_canvas_convert_to_item_space(scale->item, &x, &y);
            g_object_set(scale->item, "x", x, "y", y, NULL);
            break;
        case GDK_MOTION_NOTIFY:
            gc_drag_item_move(event);
            break;
        case GDK_BUTTON_RELEASE:
            x=event->button.x;
            y=event->button.y;
            //goo_canvas_convert_to_item_space(group_g, &x, &y);
            if(-ITEM_W < x && x < PLATE_W + ITEM_W && abs(y-PLATE_Y) < ITEM_H)
                plate = 1;
            else
            {
                x=event->button.x;
                y=event->button.y;
                //goo_canvas_convert_to_item_space(group_d, &x, &y);
                if(-ITEM_W < x && x < PLATE_W + ITEM_W && abs(y-PLATE_Y) < ITEM_H)
                    plate = -1;
                else
                    plate=0;
            }
            scale_item_move_to(scale, plate);
            break;
        default:
            break;
    }

    return FALSE;
}

static ScaleItem * scale_list_add_weight(gint weight)
{
    ScaleItem *new_item;
    GdkPixbuf *pixmap;
    gchar *filename;
    double x, y;
    GList *last;

    last=g_list_last(item_list);
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

    filename = g_strdup_printf("scale/masse%d.png", weight);
    pixmap = gc_pixmap_load(filename);
    int w = gdk_pixbuf_get_width(pixmap);
    int h = gdk_pixbuf_get_height(pixmap);
    new_item->item = goo_canvas_image_new(boardRootItem,
					  pixmap,
					  new_item->x + w/2,
					  new_item->y + h/2,
					  NULL);
    g_free(filename);
    gdk_pixbuf_unref(pixmap);

    g_signal_connect(new_item->item, "enter_notify_event", (GtkSignalFunc)gc_item_focus_event, NULL);
    g_signal_connect(new_item->item, "enter_notify_event", (GtkSignalFunc)gc_drag_event, new_item);
    g_signal_connect(new_item->item, "enter_notify_event", (GtkSignalFunc) scale_item_event, new_item);

    item_list = g_list_append(item_list, new_item);
    return new_item;
}

static ScaleItem * scale_list_add_object(GdkPixbuf *pixmap, int weight, int plate, gboolean show_weight)
{
    GooCanvasItem *item;
    ScaleItem * new_item;

    item = goo_canvas_image_new(group_d,
				pixmap,
				PLATE_SIZE * ITEM_W - gdk_pixbuf_get_width(pixmap)/2.0,
				PLATE_Y + 5 - gdk_pixbuf_get_height(pixmap),
				NULL);
    goo_canvas_item_lower(item, NULL);

    if(show_weight)
    {   // display the object weight
        double x,y;
        gchar * text;

        x = PLATE_SIZE * ITEM_W * .5;
        y = PLATE_Y - 20.0;
        text = g_strdup_printf("%d", objet_weight);
        goo_canvas_text_new(group_d,
			    text,
			    x + 1.0,
			    y + 1.0,
			    -1,
			    GTK_ANCHOR_CENTER,
			    "font", gc_skin_font_board_medium,
			    "fill_color_rgba", gc_skin_color_shadow,
			    NULL);
        goo_canvas_text_new(group_d,
			    text,
			    x,
			    y,
			    -1,
			    GTK_ANCHOR_CENTER,
			    "font", gc_skin_font_board_medium,
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
static gboolean test_addition(int total, int *table, int len)
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

static void scale_make_level()
{
    GdkPixbuf *pixmap;
    gboolean show_weight=FALSE;
    const int default_list_weight[10] = { 1,2,2,5,5,10,10};
    int list_weight[10]= {0};
    int i, tmp[5];

    switch(gcomprisBoard->level)
    {
        case 1:
        case 2:
            objet_weight = g_random_int_range(5,20);
            for(i=0;i<10; i++)
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
            for(i=0;i<5; i++)
                list_weight[i] = tmp[i];
            show_weight = gcomprisBoard->level == 3;
            break;
    }

    for(i=0; list_weight[i] ; i++)
        scale_list_add_weight(list_weight[i]);

    pixmap = gc_pixmap_load(imageList[g_random_int_range(0,imageListCount)]);
    scale_list_add_object(pixmap, objet_weight,-1, show_weight);
    gdk_pixbuf_unref(pixmap);
}


static void scale_next_level()
{
    GdkPixbuf *pixmap, *pixmap2;
    GooCanvasItem *item, *balance;
    double balance_x, balance_y;

    gc_bar_set_level(gcomprisBoard);

    scale_destroy_all_items();
    gamewon = FALSE;

	gc_score_set(gcomprisBoard->sublevel);
    // create the balance
    pixmap = gc_pixmap_load("scale/balance.png");
    balance_x = (BOARDWIDTH - gdk_pixbuf_get_width(pixmap))/2;
    balance_y = (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap))/2;

    boardRootItem = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
				   NULL);

    balance = item = goo_canvas_image_new(boardRootItem,
					  pixmap,
					  balance_x,
					  balance_y,
					  NULL);
    gdk_pixbuf_unref(pixmap);

    // create plate left
    group_g = goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas),
				    NULL);
    goo_canvas_item_translate(group_g, PLATE_X1, 0);


    pixmap = gc_pixmap_load("scale/plateau.png");
    item = goo_canvas_image_new(group_g,
				pixmap,
				0,
				PLATE_Y,
				NULL);
    gdk_pixbuf_unref(pixmap);

    // create plate right
    group_d = goo_canvas_group_new(boardRootItem, NULL);
    goo_canvas_item_translate(boardRootItem,
			      PLATE_X2,
			      0.0);

    pixmap = gc_pixmap_load("scale/plateau.png");
    pixmap2 = gdk_pixbuf_flip(pixmap, TRUE);
    item = goo_canvas_image_new(group_d,
				pixmap2,
				0,
				PLATE_Y,
				NULL);
    gdk_pixbuf_unref(pixmap);
    gdk_pixbuf_unref(pixmap2);

    pixmap = gc_pixmap_load("scale/bras.png");
    bras = item = goo_canvas_image_new(boardRootItem,
				       pixmap,
				       BRAS_X,
				       BRAS_Y,
				       NULL);
    gdk_pixbuf_unref(pixmap);
    goo_canvas_item_raise(balance, NULL);

    /* display some hint */
    if(gcomprisBoard->level > 2)
      goo_canvas_text_new(boardRootItem,
			  _("Take care, you can drop masses on both sides of the scale."),
			  200.0,
			  220.0,
			  -1,
			  GTK_ANCHOR_CENTER,
			  "font", gc_skin_font_board_medium,
			  "fill-color", "darkblue",
			  NULL);

    scale_make_level();
    last_delta=0;
    scale_anim_plate();
}

static void scale_destroy_all_items()
{
    GList *list;

    if(boardRootItem)
        goo_canvas_item_remove(boardRootItem);
    boardRootItem = NULL;

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
}

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
    if(get_weight_plate(0)==0 && good_answer)
    {
        gamewon = TRUE;
        scale_destroy_all_items();
        gc_bonus_display(gamewon, GC_BONUS_SMILEY);
    }
    else
        gc_bonus_display(gamewon, GC_BONUS_SMILEY);
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

    gc_drag_change_mode( drag_mode);

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

  gc_board_config_window_display( label,
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

  gc_board_config_combo_drag( drag_mode);

}


/* ======================= */
/* = config_stop        = */
/* ======================= */
static void config_stop()
{
}

