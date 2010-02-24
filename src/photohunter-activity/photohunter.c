/* gcompris - photohunter.c
 *
 * Copyright (C) 2009 Marc Le Douarain
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


#include <string.h>
#include "gcompris/gcompris.h"

static GcomprisBoard *gcomprisBoard = NULL;
static GooCanvasItem *boardRootItem = NULL;
static gboolean board_paused = TRUE;

static void		 start_board (GcomprisBoard *agcomprisBoard);
static void		 pause_board (gboolean pause);
static void		 end_board (void);
static void		 set_level(guint level);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);
static gboolean 	 increment_sublevel(void);

//#define GAME_DEBUG
#define FRAME_OFFSET 2
typedef struct
  {
    int PhotoWidth;
    int PhotoHeight;
    int SpaceX;
    int SpaceY;
  } StructGame;

#define POINT_SIZE 4
typedef guchar Pixel[POINT_SIZE];

GooCanvasItem *ItemPhoto[2] = {NULL, NULL };
GooCanvasItem *ItemFrame[2] = {NULL, NULL };

GArray *gDiffCoorArray = NULL; // coords list of the differences in the photos
GArray *gDiffFoundArray = NULL; // coords already discovered
StructGame Game;
int LoadNextLevelAfterEndOfBonusDisplay;

static void CleanLevelDatas( void );
static void StartLevel( );
static gint MouseClick(GooCanvasItem *item, GooCanvasItem *target,
		       GdkEvent *event, gpointer data);


/* Description of this plugin */
static BoardPlugin menu_bp =
  {
    NULL,
    NULL,
    "Photo hunter",
    "Find differences between two photos",
    "Marc Le Douarain (http://membres.lycos.fr/mavati)",
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

/* =====================================================================
 * IMPORTANT, REQUIRED TO "SEE" THE PLUGIN BOARD !
 * =====================================================================*/
GET_BPLUGIN_INFO(photohunter)

/* =====================================================================
 * (ALSO CALLED AFTER END OF BONUS DISPLAY WHEN A LEVEL IS COMPLETED)
 * =====================================================================*/
static void pause_board (gboolean pause)
{
  if(gcomprisBoard==NULL)
    return;

  if ( LoadNextLevelAfterEndOfBonusDisplay==TRUE && pause==FALSE )
    {
      LoadNextLevelAfterEndOfBonusDisplay = FALSE;
      if(increment_sublevel())
	StartLevel( );
    }

  board_paused = pause;
}

/* =====================================================================
 *
 * =====================================================================*/
static void start_board (GcomprisBoard *agcomprisBoard)
{
  if(agcomprisBoard!=NULL)
    {
      gcomprisBoard = agcomprisBoard;

      gc_set_default_background(goo_canvas_get_root_item(gcomprisBoard->canvas));

      gcomprisBoard->level = 1;
      gcomprisBoard->sublevel = 0;
      gcomprisBoard->number_of_sublevel = 0;

      /* Calculate the maxlevel based on the available data file for this board */
      gcomprisBoard->maxlevel=1;

      gchar *filename;
      while( (filename = gc_file_find_absolute("%s/board%d_0a.png",
					       gcomprisBoard->boarddir,
					       gcomprisBoard->maxlevel,
					       NULL)) )
	{
	  gcomprisBoard->maxlevel++;
	  g_free(filename);
	}
      g_free(filename);

      gcomprisBoard->maxlevel--;

      /* In this board, the sublevels are dynamicaly discovered based on data files */
      gcomprisBoard->number_of_sublevel = G_MAXINT;

      gc_bar_set(GC_BAR_LEVEL);
      gc_bar_location(-1, -1, 0.5);

      g_signal_connect(goo_canvas_get_root_item(gcomprisBoard->canvas),
		       "button_press_event",
		       (GtkSignalFunc) MouseClick, NULL);

      boardRootItem = NULL;
      gDiffCoorArray = g_array_new( FALSE, FALSE, sizeof(GooCanvasBounds) );
      gDiffFoundArray =  g_array_new( FALSE, FALSE, sizeof(GooCanvasBounds) );
      LoadNextLevelAfterEndOfBonusDisplay = FALSE;
      StartLevel( );
      pause_board(FALSE);
    }
}

/* =====================================================================
 *
 * =====================================================================*/
static void end_board ()
{
  if(gcomprisBoard!=NULL)
    {
      pause_board(TRUE);
      gc_score_end();
    }
  CleanLevelDatas( );

  g_signal_handlers_disconnect_by_func(goo_canvas_get_root_item(gcomprisBoard->canvas),
				       (GtkSignalFunc) MouseClick, NULL);

  gcomprisBoard = NULL;
  if(boardRootItem!=NULL)
    goo_canvas_item_remove( boardRootItem );
  if ( gDiffCoorArray!=NULL )
    g_array_free( gDiffCoorArray, TRUE );
  if ( gDiffFoundArray!=NULL )
    g_array_free( gDiffFoundArray, TRUE );

}

/* =====================================================================
 *
 * =====================================================================*/
static void set_level(guint level)
{
  if(gcomprisBoard!=NULL)
    {
      gcomprisBoard->level = level;
      gcomprisBoard->sublevel = 0;
      if ( gcomprisBoard->level>gcomprisBoard->maxlevel )
	gcomprisBoard->level = 1;
      StartLevel();
    }
}

/* =====================================================================
 *
 * =====================================================================*/
static gboolean is_our_board (GcomprisBoard *gcomprisBoard)
{
  if (gcomprisBoard)
    {
      if(g_strcasecmp(gcomprisBoard->type, "photohunter")==0)
	{
	  /* Set the plugin entry */
	  gcomprisBoard->plugin=&menu_bp;
	  return TRUE;
	}
    }
  return FALSE;
}

static gboolean increment_sublevel()
{
  gcomprisBoard->sublevel++;

  if(gcomprisBoard->sublevel>gcomprisBoard->number_of_sublevel) {
    /* Try the next level */
    gcomprisBoard->level++;
    gcomprisBoard->sublevel=0;

    if(gcomprisBoard->level>gcomprisBoard->maxlevel) { // the current board is finished : restart
      gcomprisBoard->level = 1;
      gcomprisBoard->sublevel=0;
    }

  }

  return TRUE;
}

static void CleanLevelDatas( void )
{
  int ScanPhoto;
  if ( gDiffCoorArray!=NULL )
    {
      if ( gDiffCoorArray->len>0 )
	g_array_remove_range( gDiffCoorArray, 0, gDiffCoorArray->len );
    }
  if ( gDiffFoundArray!=NULL )
    {
      if ( gDiffFoundArray->len>0 )
	g_array_remove_range( gDiffFoundArray, 0, gDiffFoundArray->len );
    }
  for( ScanPhoto=0; ScanPhoto<2; ScanPhoto++ )
    {
      if ( ItemPhoto[ ScanPhoto ]!=NULL )
	{
	  ItemPhoto[ ScanPhoto ] = NULL;
	}
      if ( ItemFrame[ ScanPhoto ]!=NULL )
	{
	  ItemFrame[ ScanPhoto ] = NULL;
	}
    }
}

static gboolean
diff_pixel (Pixel p1, Pixel p2)
{
  int i;
  for (i=0; i<POINT_SIZE; i++)
    if ( p1[i] != p2[i] )
      {
	return TRUE;
      }
  return FALSE;
}

static void
set_pixel (GdkPixbuf *pixbuf,
	   int x, int y,
	   Pixel point)
{
  int width, height, rowstride, n_channels;
  guchar *pixels, *p;
  int i;
  n_channels = gdk_pixbuf_get_n_channels (pixbuf);
  g_assert (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB);
  g_assert (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8);
  g_assert (n_channels <= 4);
  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);
  g_assert (x >= 0 && x < width);
  g_assert (y >= 0 && y < height);
  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixels = gdk_pixbuf_get_pixels (pixbuf);
  p = pixels + y * rowstride + x * n_channels;
  for( i=0; i<n_channels; i++)
    p[i] = point[i];
}

static void
get_pixel (GdkPixbuf *pixbuf,
	   int x, int y,
	   Pixel point)
{
  int width, height, rowstride, n_channels;
  guchar *pixels, *p;
  int i;
  n_channels = gdk_pixbuf_get_n_channels (pixbuf);
  g_assert (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB);
  g_assert (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8);
  g_assert (n_channels <= 4);
  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);
  if (x < 0 || x >= width ||
      y < 0 || y >= height)
    {
      for( i=0; i<POINT_SIZE; i++)
	point[i] = 0;
      return;
    }
  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixels = gdk_pixbuf_get_pixels (pixbuf);
  p = pixels + y * rowstride + x * n_channels;
  for( i=0; i<n_channels; i++)
    point[i] = p[i];
  for( i=n_channels; i<POINT_SIZE; i++)
    point[i] = 0;
}

/* From Wikipedia
   http://en.wikipedia.org/wiki/Flood_fill

   Flood-fill (node, target-color, replacement-color):
   1. If the color of node is not equal to target-color, return.
   2. Set the color of node to replacement-color.
   3. Perform Flood-fill (one step to the west of node, target-color, replacement-color).
   Perform Flood-fill (one step to the east of node, target-color, replacement-color).
   Perform Flood-fill (one step to the north of node, target-color, replacement-color).
   Perform Flood-fill (one step to the south of node, target-color, replacement-color).
   4. Return.
*/
static void flood_check(GdkPixbuf *pixbuf1, GdkPixbuf *pixbuf2,
			int x, int y,
			GooCanvasBounds *bounds)
{
  Pixel p1, p2;
  get_pixel(pixbuf1, x, y, p1);
  get_pixel(pixbuf2, x, y, p2);
  if ( ! diff_pixel(p1, p2) )
    return;

  set_pixel(pixbuf1, x, y, p2);

  bounds->x1 = MIN(bounds->x1, x);
  bounds->y1 = MIN(bounds->y1, y);
  bounds->x2 = MAX(bounds->x2, x);
  bounds->y2 = MAX(bounds->y2, y);

  flood_check(pixbuf1, pixbuf2, x + 1, y, bounds);
  flood_check(pixbuf1, pixbuf2, x - 1, y, bounds);
  flood_check(pixbuf1, pixbuf2, x, y - 1, bounds);
  flood_check(pixbuf1, pixbuf2, x, y + 1, bounds);

  flood_check(pixbuf1, pixbuf2, x + 1, y + 1, bounds);
  flood_check(pixbuf1, pixbuf2, x + 1, y - 1, bounds);
  flood_check(pixbuf1, pixbuf2, x - 1, y - 1, bounds);
  flood_check(pixbuf1, pixbuf2, x - 1, y + 1, bounds);

  return;
}

static GooCanvasItem * LoadPhoto( GdkPixbuf *pixmap, int PhotoNbr )
{
  GooCanvasItem * item = NULL;

  if(pixmap)
    {
      int PosiX,PosiY;
      if ( PhotoNbr==0 )
	{
	  Game.PhotoWidth = gdk_pixbuf_get_width( pixmap );
	  Game.PhotoHeight = gdk_pixbuf_get_height( pixmap );
	  Game.SpaceX = (BOARDWIDTH-2*Game.PhotoWidth)/3;
	  Game.SpaceY = (BOARDHEIGHT-Game.PhotoHeight)/2;
	  PosiX = Game.SpaceX;
	  PosiY = Game.SpaceY;
	}
      else
	{
	  PosiX = 2*Game.SpaceX+Game.PhotoWidth;
	  PosiY = Game.SpaceY;
	}
      item = goo_canvas_image_new (boardRootItem,
				   pixmap,
				   PosiX,
				   PosiY,
				   NULL);
    }
  return item;
}

static void
DrawCircle( int x1, int y1, int x2, int y2, char *color )
{
  int wid2=(x2-x1)/2;
  int hei2=(y2-y1)/2;
  goo_canvas_ellipse_new (boardRootItem,
			  x1+wid2,
			  y1+hei2,
			  wid2,
			  hei2,
			  "stroke-color", color,
			  "line-width", (double)1,
			  NULL);
}
static GooCanvasItem * DrawPhotoFrame( int PhotoNbr, char * color )
{
  int x1, y1;
  GooCanvasItem * item = NULL;
  if ( PhotoNbr==0 )
    {
      x1 = Game.SpaceX-FRAME_OFFSET;
      y1 = Game.SpaceY-FRAME_OFFSET;
    }
  else
    {
      x1 = 2*Game.SpaceX+Game.PhotoWidth-FRAME_OFFSET;
      y1 = Game.SpaceY-FRAME_OFFSET;
    }
  item = goo_canvas_rect_new( boardRootItem,
			      x1, y1,
			      Game.PhotoWidth+FRAME_OFFSET +2, Game.PhotoHeight+FRAME_OFFSET +2,
			      /*"fill_color"*/"stroke_color", color,
			      NULL );
  return item;
}

gchar *get_next_datafile()
{
  gchar *filename;
  while( ((filename = gc_file_find_absolute("%s/board%d_%da.png",
					    gcomprisBoard->boarddir,
					    gcomprisBoard->level,
					    gcomprisBoard->sublevel,
					    NULL)) == NULL)
	 && ((gcomprisBoard->level != 1) || (gcomprisBoard->sublevel!=0)))
    {
      /* Try the next level */
      gcomprisBoard->sublevel = gcomprisBoard->number_of_sublevel;
      if(!increment_sublevel())
	{
	  g_free(filename);
	  return NULL;
	}
    }

  if ( !filename)
    return NULL;

  filename[strlen(filename)-5] = '\0';
  return filename;
}

void
search_diffs(GdkPixbuf *pixbuf1, GdkPixbuf *pixbuf2)
{
  int x, y;
  int width, height;
  width = gdk_pixbuf_get_width (pixbuf1);
  height = gdk_pixbuf_get_height (pixbuf2);

  for ( x=0; x<width; x++)
    {
      for ( y=0; y<height; y++)
	{
	  Pixel p1, p2;
	  get_pixel(pixbuf1, x, y, p1);
	  get_pixel(pixbuf2, x, y, p2);
	  if ( diff_pixel(p1, p2) )
	    {
	      GooCanvasBounds bounds;
	      bounds.x1 = G_MAXINT;
	      bounds.y1 = G_MAXINT;
	      bounds.x2 = 0;
	      bounds.y2 = 0;
	      flood_check(pixbuf1, pixbuf2, x, y, &bounds);

	      /* Hack, we don't push too small areas */
	      if ( (bounds.x2 - bounds.x1) *
		   (bounds.y2 - bounds.y1) > 10 )
		g_array_append_val( gDiffCoorArray, bounds );
	    }
	}
    }
}

static void StartLevel( )
{
  {
    int ScanPhoto;
    char * str;

    if(boardRootItem!=NULL)
      goo_canvas_item_remove( boardRootItem );

    CleanLevelDatas( );

    boardRootItem =
      goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas), NULL);

    // get base filename datas to play
    gchar * RandomFileToLoad = get_next_datafile();

    if (!RandomFileToLoad)
      {
	gc_dialog(_("Error: Absolutely no photo found in the data directory"),
		  gc_board_stop);
	return;
      }

    GdkPixbuf *pixmap[2];
    for( ScanPhoto=0; ScanPhoto<2; ScanPhoto++ )
      {
	str = g_strdup_printf("%s%c.png", RandomFileToLoad,
			      ScanPhoto==0?'a':'b' );
	pixmap[ScanPhoto] = gc_pixmap_load(str);
	ItemPhoto[ ScanPhoto ] = LoadPhoto( pixmap[ScanPhoto], ScanPhoto );
	g_free(str);
	ItemFrame[ ScanPhoto ] = DrawPhotoFrame( ScanPhoto, "black" );
      }


    search_diffs(pixmap[0], pixmap[1]);

    for( ScanPhoto=0; ScanPhoto<2; ScanPhoto++ )
      gdk_pixbuf_unref(pixmap[ScanPhoto]);

    g_free(RandomFileToLoad);

#ifdef GAME_DEBUG
    int scanposi;
    for( scanposi=0; scanposi<gDiffCoorArray->len; scanposi++)
      {
	GooCanvasBounds *pDiff = \
	  &g_array_index (gDiffCoorArray, GooCanvasBounds, scanposi);
	printf("diff %d (%lf %lf) (%lf %lf)\n", scanposi,
	       pDiff->x1, pDiff->y1,
	       pDiff->x2, pDiff->y2);
	DrawCircle( Game.SpaceX+pDiff->x1, Game.SpaceY+pDiff->y1,
		    Game.SpaceX+pDiff->x2, Game.SpaceY+pDiff->y2,
		    "green" );
	DrawCircle( (2*Game.SpaceX+Game.PhotoWidth)+pDiff->x1,
		    Game.SpaceY+pDiff->y1,
		    (2*Game.SpaceX+Game.PhotoWidth)+pDiff->x2,
		    Game.SpaceY+pDiff->y2,
		    "green" );
      }
#endif

    gc_score_start(SCORESTYLE_NOTE,
		   BOARDWIDTH - 195,
		   BOARDHEIGHT - 30,
		   gDiffCoorArray->len);
    gc_score_set(0);
    gc_bar_set_level(gcomprisBoard);

    if (gcomprisBoard->level == 1 && gcomprisBoard->sublevel == 0)
      {
	GooCanvasItem *item =
	  goo_canvas_svg_new (boardRootItem,
			      gc_skin_rsvg_get(),
			      "svg-id", "#BAR_BG",
			      NULL);
	SET_ITEM_LOCATION_CENTER(item,
				 BOARDWIDTH/2,
				 BOARDHEIGHT - 100);

	goo_canvas_text_new (boardRootItem,
			     _("Click on the differences between the two images."),
			     BOARDWIDTH/2,
			     BOARDHEIGHT - 100,
			     -1,
			     GTK_ANCHOR_CENTER,
			     "font", gc_skin_font_board_small,
			     "fill_color_rgba", gc_skin_color_text_button,
			     "alignment", PANGO_ALIGN_CENTER,
			     NULL);
      }
  }
}

static int TestIfClickedOnDiff( int ClickX, int ClickY )
{
  int NumDiff = -1;
  int ScanPosi,ScanPhoto;
  for( ScanPhoto=0; ScanPhoto<2; ScanPhoto++ )
    {
      for( ScanPosi=0; ScanPosi<gDiffCoorArray->len; ScanPosi++)
	{
	  int OffsetX = (ScanPhoto==0)?Game.SpaceX:(Game.SpaceX*2+Game.PhotoWidth);
	  int OffsetY = Game.SpaceY;
	  GooCanvasBounds * pDiff = &g_array_index (gDiffCoorArray, GooCanvasBounds, ScanPosi);
	  if ( OffsetX+pDiff->x1 <= ClickX && ClickX<= OffsetX+pDiff->x2
	       && OffsetY+pDiff->y1 <= ClickY && ClickY<= OffsetY+pDiff->y2 )
	    {
	      NumDiff = ScanPosi;
	    }
	}
    }
  return NumDiff;
}
static void TestClick( int ClickX, int ClickY )
{
  int DiffFound = TestIfClickedOnDiff( ClickX, ClickY );
  // a diff found ?
  if ( DiffFound!=-1 )
    {
      GooCanvasBounds * pClickedDiffFound = &g_array_index (gDiffCoorArray, GooCanvasBounds, DiffFound);
      // not already found ?
      if ( gDiffFoundArray->len>0 )
	{
	  int ScanAlreadyFound;
	  for( ScanAlreadyFound=0; ScanAlreadyFound<gDiffFoundArray->len; ScanAlreadyFound++)
	    {
	      GooCanvasBounds * pScanDiffFound = &g_array_index (gDiffFoundArray, GooCanvasBounds, ScanAlreadyFound);
	      if ( pScanDiffFound->x1==pClickedDiffFound->x1 && pScanDiffFound->y1==pClickedDiffFound->y1
		   && pScanDiffFound->x2==pClickedDiffFound->x2 && pScanDiffFound->y2==pClickedDiffFound->y2 )
		{
		  DiffFound = -1;
		}
	    }
	}
      if ( DiffFound!=-1 )
	{
	  g_array_append_val( gDiffFoundArray, *pClickedDiffFound );
	  // draw the found difference on the photos
	  DrawCircle( Game.SpaceX+pClickedDiffFound->x1, Game.SpaceY+pClickedDiffFound->y1, Game.SpaceX+pClickedDiffFound->x2, Game.SpaceY+pClickedDiffFound->y2, "yellow" );
	  DrawCircle( (2*Game.SpaceX+Game.PhotoWidth)+pClickedDiffFound->x1, Game.SpaceY+pClickedDiffFound->y1, (2*Game.SpaceX+Game.PhotoWidth)+pClickedDiffFound->x2, Game.SpaceY+pClickedDiffFound->y2, "yellow" );
	  gc_score_set(gDiffFoundArray->len);
	  // end ???
	  if ( gDiffFoundArray->len==gDiffCoorArray->len )
	    {
	      gc_bonus_display( GC_BOARD_WIN, GC_BONUS_SMILEY );
	      LoadNextLevelAfterEndOfBonusDisplay = TRUE;
	    }
	}
    }
}
static gint
MouseClick(GooCanvasItem *item, GooCanvasItem *target,
	   GdkEvent *event, gpointer data)
{
  double x, y;

  if (!gcomprisBoard || board_paused)
    return FALSE;

  x = event->button.x;
  y = event->button.y;
  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      goo_canvas_convert_from_item_space(goo_canvas_item_get_canvas(item),
					 item, &x, &y);
      TestClick( (int)x, (int)y );
      break;
    default:
      break;
    }
  return FALSE;
}
