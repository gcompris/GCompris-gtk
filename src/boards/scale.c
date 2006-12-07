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
#define SOUNDLISTFILE PACKAGE

static GcomprisBoard *gcomprisBoard = NULL;
static gboolean board_paused = TRUE;

static void start_board (GcomprisBoard *agcomprisBoard);
static void pause_board (gboolean pause);
static void end_board (void);
static gboolean is_our_board (GcomprisBoard *gcomprisBoard);
static void set_level (guint level);
static gint key_press(guint keyval, gchar *commit_str, gchar *preedit_str);
static void process_ok(void);
static int gamewon;
static void game_won(void);

static GnomeCanvasGroup *boardRootItem = NULL;
static GnomeCanvasGroup *group_g = NULL, *group_d= NULL;
static GnomeCanvasItem *bras = NULL;
static GnomeCanvasItem *answer_item = NULL;
GString *answer_string = NULL;

static void scale_destroy_all_items(void);
static void scale_next_level(void);

#define ITEM_X_MIN 0
#define ITEM_X_MAX 500
#define ITEM_Y_MIN 250
#define ITEM_Y_MAX 400
#define ITEM_W 45
#define ITEM_H 70

#define PLATEAU_X1 -40.0
#define PLATEAU_X2 246.0
#define PLATEAU_Y 33.0
#define PLATEAU_Y_DELTA 15.0

#define BRAS_X 55.0
#define BRAS_Y -6.0

#define PLATEAU_SIZE 4 // number of place in the plateau

typedef struct {
    GnomeCanvasItem * item;
    double x, y;
    int plateau_index;  // position in the plateau or -1 if not in the plateau
    int weight;
} ScaleItem;

static GList *item_list = NULL;
static int total_weight = 0;

static const char *imageList[] = {
    "gcompris/food/chocolate_cake.png",
    "gcompris/food/pear.png",
    "gcompris/food/orange.png",
    "gcompris/food/suggar_box.png",
    "gcompris/misc/flowerpot.png",
    "gcompris/misc/glass.png"
};

static const int imageListCount = G_N_ELEMENTS(imageList);

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
    NULL,
    NULL
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
        gcomprisBoard->number_of_sublevel=3; /* Go to next level after this number of 'play' */
        gcomprisBoard->maxlevel = 3;
        gc_bar_set(GC_BAR_LEVEL|GC_BAR_OK);
        
        scale_next_level();

        gamewon = FALSE;
        pause_board(FALSE);
    }
}

static void end_board ()
{
    if(gcomprisBoard!=NULL)
    {
        pause_board(TRUE);
        scale_destroy_all_items();
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
        gnome_canvas_item_set(answer_item,
                "text", tmpstr,
                NULL);
        g_free(tmpstr);
    }

    return TRUE;
}

int get_total_weight(void)
{
    GList *list;
    ScaleItem *item;
    int result=0;

    for(list = item_list; list; list=list->next)
    {
        item = list->data;
        if(item->plateau_index != -1)
        {
            result+= item->weight;
        }
    }
    return result;
}

void scale_anim_plateau(void)
{
    double affine[6];
    double delta_y, x;
    double angle;
    int diff;

    diff = get_total_weight() - total_weight;
    delta_y = CLAMP(PLATEAU_Y_DELTA / 10.0 * diff,
            -PLATEAU_Y_DELTA, PLATEAU_Y_DELTA);
    if(get_total_weight()==0)
        delta_y = -PLATEAU_Y_DELTA;
    angle = tan(delta_y / 138) * 180 / M_PI;
    
    gtk_object_get (GTK_OBJECT (group_g), "x", &x, NULL);
    art_affine_translate(affine, x, delta_y);
    gnome_canvas_item_affine_absolute(GNOME_CANVAS_ITEM(group_g), affine);
    
    gtk_object_get (GTK_OBJECT (group_d), "x", &x, NULL);
    art_affine_translate(affine, x, -delta_y);
    gnome_canvas_item_affine_absolute(GNOME_CANVAS_ITEM(group_d), affine);

    gc_item_rotate_with_center(bras, -angle, 138, 84);

    if(diff ==0 && gcomprisBoard->level == 2)
    {
        GdkPixbuf *button_pixmap;
        double x_offset = 40, y_offset = 150;

        button_pixmap = gc_skin_pixmap_load("button_large2.png");
        gnome_canvas_item_new (boardRootItem,
                gnome_canvas_pixbuf_get_type (),
                "pixbuf",  button_pixmap,
                "x", x_offset,
                "y", y_offset,
                NULL);
        answer_item = gnome_canvas_item_new(boardRootItem,
                gnome_canvas_text_get_type(),
                "font", gc_skin_font_board_title_bold,
                "x", x_offset + gdk_pixbuf_get_width(button_pixmap)/2,
                "y", y_offset + gdk_pixbuf_get_height(button_pixmap)/2,
                "anchor", GTK_ANCHOR_CENTER,
                "fill_color", "white",
                NULL);
        gdk_pixbuf_unref(button_pixmap);

        answer_string = g_string_new(NULL);
        key_press(0,NULL,NULL);
    }
}

int scale_item_event(GnomeCanvasItem *w, GdkEvent *event, ScaleItem *item)
{
    ScaleItem *scale;
    GList *list;
    gboolean found;
    int index;

    if(event->type != GDK_BUTTON_PRESS)
        return FALSE;
    if(event->button.button != 1)
        return FALSE;
    if(answer_string)   // disable, waiting a answer
        return FALSE;

    if(item->plateau_index == -1)
    {
        // find the first free place
        for(index=0; index < PLATEAU_SIZE; index ++)
        {
            found = FALSE;
            for(list = item_list; list; list = list->next)
            {
                scale = list->data;
                if(scale->plateau_index == index)
                    found=TRUE;
            }
            if(!found)
            {   // move to the plateau
                item->plateau_index=index;
                gnome_canvas_item_reparent(item->item, group_g);
                gnome_canvas_item_set(item->item,
                        "x", (double)index * ITEM_W,
                        "y", (double)PLATEAU_Y-ITEM_H + 5,
                        NULL);
                scale_anim_plateau();
                break;
            }
        }
    }
    else
    {   // move the item to the list
        item->plateau_index = -1;
        gnome_canvas_item_reparent(item->item, boardRootItem);
        gnome_canvas_item_set(item->item,
                "x", item->x,
                "y", item->y, NULL);
        scale_anim_plateau();
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
    new_item->plateau_index = -1;
    new_item->weight = weight;
    
    filename = g_strdup_printf("scales/masse%d.png", weight);
    pixmap = gc_pixmap_load(filename);
    new_item->item = gnome_canvas_item_new(boardRootItem,
            gnome_canvas_pixbuf_get_type(),
            "pixbuf", pixmap,
            "x", new_item->x,
            "y", new_item->y, NULL);
    g_free(filename);
    gdk_pixbuf_unref(pixmap);
    
    g_signal_connect(new_item->item, "event", (GtkSignalFunc)gc_item_focus_event, NULL);
    g_signal_connect(new_item->item, "event", (GtkSignalFunc)scale_item_event, new_item);
    
    item_list = g_list_append(item_list, new_item);
    return new_item;
}

static void scale_next_level()
{
    GdkPixbuf *pixmap, *pixmap2;
    GnomeCanvasItem *item, *balance;
    double balance_x, balance_y;

    item = gc_set_background(gnome_canvas_root(gcomprisBoard->canvas), 
        "scales/tabepice.jpg");
    gc_bar_set_level(gcomprisBoard);

    scale_destroy_all_items();
    gamewon = FALSE;

    // create the balance
    pixmap = gc_pixmap_load("scales/balance.png");
    balance_x = (BOARDWIDTH - gdk_pixbuf_get_width(pixmap))/2;
    balance_y = (BOARDHEIGHT - gdk_pixbuf_get_height(pixmap))/2;

    boardRootItem = GNOME_CANVAS_GROUP(gnome_canvas_item_new (gnome_canvas_root(gcomprisBoard->canvas),
                gnome_canvas_group_get_type (),
                "x", balance_x,
                "y", balance_y,
                NULL));

    balance = item = gnome_canvas_item_new(boardRootItem, 
            gnome_canvas_pixbuf_get_type(),
            "pixbuf", pixmap,
            "x", (double)0,
            "y", (double)0,
            NULL);
    gdk_pixbuf_unref(pixmap);
    
    // create plateau gauche
    group_g = GNOME_CANVAS_GROUP(gnome_canvas_item_new(boardRootItem,
                gnome_canvas_group_get_type(),
                "x", PLATEAU_X1,
                "y", 0.0,
                NULL));
    pixmap = gc_pixmap_load("scales/plateau.png");
    item = gnome_canvas_item_new(group_g,
            gnome_canvas_pixbuf_get_type(),
            "pixbuf", pixmap,
            "x", 0.0, "y", PLATEAU_Y, NULL);
    gdk_pixbuf_unref(pixmap);
    
    // create plateau droit
    group_d = GNOME_CANVAS_GROUP(gnome_canvas_item_new(boardRootItem,
                gnome_canvas_group_get_type(),
                "x", PLATEAU_X2,
                "y", 0.0,
                NULL));
    pixmap = gc_pixmap_load("scales/plateau.png");
    pixmap2 = gdk_pixbuf_flip(pixmap, TRUE);
    item = gnome_canvas_item_new(group_d,
            gnome_canvas_pixbuf_get_type(),
            "pixbuf", pixmap2,
            "x", 0.0, "y", PLATEAU_Y, NULL);
    gdk_pixbuf_unref(pixmap);
    gdk_pixbuf_unref(pixmap2);

    pixmap = gc_pixmap_load(imageList[g_random_int_range(0,imageListCount)]);
    item = gnome_canvas_item_new(group_d,
            gnome_canvas_pixbuf_get_type(),
            "pixbuf", pixmap,
            "x", ((double)PLATEAU_SIZE * ITEM_W - gdk_pixbuf_get_width(pixmap))/2.0,
            "y", PLATEAU_Y + 5 - gdk_pixbuf_get_height(pixmap), NULL);
    gnome_canvas_item_lower_to_bottom(item);
    gdk_pixbuf_unref(pixmap);

    pixmap = gc_pixmap_load("scales/bras.png");
    bras = item = gnome_canvas_item_new(boardRootItem,
            gnome_canvas_pixbuf_get_type(),
            "pixbuf", pixmap,
            "x", BRAS_X,
            "y", BRAS_Y,
            NULL);
    gdk_pixbuf_unref(pixmap);
    gnome_canvas_item_raise_to_top(balance);

    // calc the total weight
    total_weight = g_random_int_range(5,20);
    scale_list_add_weight(1);
    scale_list_add_weight(2);
    scale_list_add_weight(2);
    scale_list_add_weight(5);
    scale_list_add_weight(5);
    scale_list_add_weight(10);
    scale_list_add_weight(10);
    scale_list_add_weight(10);

    if(gcomprisBoard->level == 1)
    {   // display the object weight
        double x,y;
        gchar * text;
        
        x = PLATEAU_SIZE * ITEM_W * .5;
        y = PLATEAU_Y - 10.0;
        text = g_strdup_printf("%d", total_weight);
        gnome_canvas_item_new(group_d,
                gnome_canvas_text_get_type(),
                "text", text,
                "font", gc_skin_font_board_medium,
                "x", x + 1.0,
                "y", y + 1.0,
                "anchor", GTK_ANCHOR_CENTER,
                "fill_color_rgba", gc_skin_color_shadow,
                NULL);
        gnome_canvas_item_new(group_d,
                gnome_canvas_text_get_type(),
                "text", text,
                "font", gc_skin_font_board_medium,
                "x", x,
                "y", y,
                "anchor", GTK_ANCHOR_CENTER,
                "fill_color_rgba", gc_skin_color_content,
                NULL);
        g_free(text);
    }
    scale_anim_plateau();
}

static void scale_destroy_all_items()
{
    GList *list;

    if(boardRootItem)
        gtk_object_destroy (GTK_OBJECT(boardRootItem));
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
            gc_bonus_end_display(BOARD_FINISHED_RANDOM);
            return;
        }
        gc_sound_play_ogg ("sounds/bonus.ogg", NULL);
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
        good_answer = answer_weight == total_weight;
    }
    if(total_weight == get_total_weight() && good_answer)
    {
        gamewon = TRUE;
        scale_destroy_all_items();
        gc_bonus_display(gamewon, BONUS_SMILEY);
    }
    else
        gc_bonus_display(gamewon, BONUS_SMILEY);
}
