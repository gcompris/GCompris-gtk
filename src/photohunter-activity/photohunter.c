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
static void 	 set_level(guint level);
static gboolean		 is_our_board (GcomprisBoard *gcomprisBoard);

//#define GAME_DEBUG
#define NBR_OF_PHOTOS_PER_SESSION 9
#define FRAME_OFFSET 2
typedef struct StructGame
  {
    int PhotoWidth;
    int PhotoHeight;
    int SpaceX;
    int SpaceY;

    int NbrOfPhotosAvailable;
    int * PhotoNbrForThisLevel; // random photo to play for each level
  }StructGame;
typedef struct StructDiffCoor
  {
    int x1;
    int y1;
    int x2;
    int y2;
  }StructDiffCoor;

GooCanvasItem *ItemPhoto[2] = {NULL, NULL };
GooCanvasItem *ItemFrame[2] = {NULL, NULL };

GArray *gDiffCoorArray = NULL; // coords list of the differences in the photos
GArray *gDiffFoundArray = NULL; // coords already discovered
StructGame Game;
int LoadNextLevelAfterEndOfBonusDisplay;

static void CleanLevelDatas( void );
static gchar * ScanAndPickRandomFile( int  * pNbrOfFiles, int RandomSelection );
static void StartLevel( );
static gint MouseClick(GooCanvasItem *item, GooCanvasItem *target,
		       GdkEvent *event, gpointer data);
static void LoadCsvDiffFile( char * pFilename );


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
      gcomprisBoard->level++;
      if ( gcomprisBoard->level>gcomprisBoard->maxlevel )
	gcomprisBoard->level = 1;
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
      char * PhotoSelectedMark;
      int InitScan;
      int StorePhotoLevel;
      gcomprisBoard = agcomprisBoard;

      gc_set_default_background(goo_canvas_get_root_item(gcomprisBoard->canvas));

      // get nbr of photos available, and init as none found for now
      ScanAndPickRandomFile( &Game.NbrOfPhotosAvailable, -1 );
      if ( Game.NbrOfPhotosAvailable==0 )
	gc_dialog(_("Error: Absolutely no photo found in the data directory"),
		  gc_board_stop);

      gcomprisBoard->level = 1;
      // limit to "N" photos to play per game session (in case of more photos in the directory...)
      gcomprisBoard->maxlevel = (Game.NbrOfPhotosAvailable > NBR_OF_PHOTOS_PER_SESSION
				 ? NBR_OF_PHOTOS_PER_SESSION : Game.NbrOfPhotosAvailable);
      gcomprisBoard->sublevel = 0;
      gcomprisBoard->number_of_sublevel = 0;

      gc_bar_set(GC_BAR_LEVEL);

      Game.PhotoNbrForThisLevel = malloc( (gcomprisBoard->maxlevel+1) * sizeof(int) );
      PhotoSelectedMark = malloc( Game.NbrOfPhotosAvailable*sizeof(char) );
      if ( Game.PhotoNbrForThisLevel && PhotoSelectedMark )
	{
	  // precalc a list of photos to play for each level
	  for( InitScan=0; InitScan<Game.NbrOfPhotosAvailable ; InitScan++ )
	    PhotoSelectedMark[ InitScan ] = FALSE;
	  for( StorePhotoLevel=1; StorePhotoLevel<=gcomprisBoard->maxlevel;
	       StorePhotoLevel++ )
	    {
	      int RandVal;
	      // we never give the same photo...
	      do
		{
		  RandVal = (int)g_random_int_range( 0,
						     Game.NbrOfPhotosAvailable );
		}
	      while( PhotoSelectedMark[ RandVal ]!=FALSE );
	      PhotoSelectedMark[ RandVal ] = TRUE;
	      Game.PhotoNbrForThisLevel[ StorePhotoLevel ] = RandVal;
	    }
	  free( PhotoSelectedMark );

	  g_signal_connect(goo_canvas_get_root_item(gcomprisBoard->canvas),
			   "button_press_event",
			   (GtkSignalFunc) MouseClick, NULL);

	  boardRootItem =
	    goo_canvas_group_new (goo_canvas_get_root_item(gcomprisBoard->canvas), NULL);
	  gDiffCoorArray = g_array_new( FALSE, FALSE, sizeof(StructDiffCoor) );
	  gDiffFoundArray =  g_array_new( FALSE, FALSE, sizeof(StructDiffCoor) );
	  LoadNextLevelAfterEndOfBonusDisplay = FALSE;
	  StartLevel( );
	  pause_board(FALSE);
	}
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
  if ( Game.PhotoNbrForThisLevel!=NULL )
    free( Game.PhotoNbrForThisLevel );
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
	  goo_canvas_item_remove( ItemPhoto[ ScanPhoto ] );
	  ItemPhoto[ ScanPhoto ] = NULL;
	}
      if ( ItemFrame[ ScanPhoto ]!=NULL )
	{
	  goo_canvas_item_remove( ItemFrame[ ScanPhoto ] );
	  ItemFrame[ ScanPhoto ] = NULL;
	}
    }
}

static GooCanvasItem * LoadPhoto( char * file, int PhotoNbr )
{
  GooCanvasItem * item = NULL;
  GdkPixbuf * pixmap = gc_pixmap_load(file);
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
      gdk_pixbuf_unref(pixmap);
    }
  return item;
}

static void DrawCircle( int x1, int y1, int x2, int y2, char * color )
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

static void StartLevel( )
{
  {
    int ScanPhoto;
    char * str;
    int RandVal = Game.PhotoNbrForThisLevel[ gcomprisBoard->level ];

    CleanLevelDatas( );

    // get base filename datas to play
    gchar * RandomFileToLoad = ScanAndPickRandomFile( NULL, RandVal );

    for( ScanPhoto=0; ScanPhoto<2; ScanPhoto++ )
      {
	str = g_strdup_printf("%s/%s%c.jpg", gcomprisBoard->boarddir, RandomFileToLoad, ScanPhoto==0?'a':'b' );
	ItemPhoto[ ScanPhoto ] = LoadPhoto( str, ScanPhoto );
	g_free(str);
	ItemFrame[ ScanPhoto ] = DrawPhotoFrame( ScanPhoto, "black" );
      }
    str = gc_file_find_absolute("%s/%s.csv", gcomprisBoard->boarddir, RandomFileToLoad);
    LoadCsvDiffFile( str );
    g_free(str);
    g_free(RandomFileToLoad);

#ifdef GAME_DEBUG
    int scanposi;
    for( scanposi=0; scanposi<gDiffCoorArray->len; scanposi++)
      {
	StructDiffCoor * pDiff = &g_array_index (gDiffCoorArray, StructDiffCoor, scanposi);
	DrawCircle( Game.SpaceX+pDiff->x1, Game.SpaceY+pDiff->y1, Game.SpaceX+pDiff->x2, Game.SpaceY+pDiff->y2, "green" );
	DrawCircle( (2*Game.SpaceX+Game.PhotoWidth)+pDiff->x1, Game.SpaceY+pDiff->y1, (2*Game.SpaceX+Game.PhotoWidth)+pDiff->x2, Game.SpaceY+pDiff->y2, "green" );
      }
#endif

    gcomprisBoard->sublevel = 0;
    gcomprisBoard->number_of_sublevel = gDiffCoorArray->len;
    gc_score_start(SCORESTYLE_NOTE,
		   BOARDWIDTH - 195,
		   BOARDHEIGHT - 30,
		   gcomprisBoard->number_of_sublevel);
    gc_score_set(gcomprisBoard->sublevel);
    gc_bar_set_level(gcomprisBoard);

    if (gcomprisBoard->level == 1)
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
	  StructDiffCoor * pDiff = &g_array_index (gDiffCoorArray, StructDiffCoor, ScanPosi);
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
      StructDiffCoor * pClickedDiffFound = &g_array_index (gDiffCoorArray, StructDiffCoor, DiffFound);
      // not already found ?
      if ( gDiffFoundArray->len>0 )
	{
	  int ScanAlreadyFound;
	  for( ScanAlreadyFound=0; ScanAlreadyFound<gDiffFoundArray->len; ScanAlreadyFound++)
	    {
	      StructDiffCoor * pScanDiffFound = &g_array_index (gDiffFoundArray, StructDiffCoor, ScanAlreadyFound);
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
	  gcomprisBoard->sublevel++;
	  gc_score_set(gcomprisBoard->sublevel);
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


// Two call methods, firstly to know how many files are available (with RandomSelection to -1),
// secondly to return the random file selected.
gchar * ScanAndPickRandomFile( int  * pNbrOfFiles, int RandomSelection )
{
  int NbrOfFilesFound = 0;
  char SelectionFound = FALSE;
  gchar * FileChoosen = NULL;
  char * str = gc_file_find_absolute("%s", gcomprisBoard->boarddir );

  GDir * FilesDir = g_dir_open( str, 0, NULL );
  if ( FilesDir )
    {
      const gchar * File;
      do
	{
	  File = g_dir_read_name( FilesDir );
	  if ( File!=NULL )
	    {
	      if ( g_str_has_suffix(File, ".csv") )
		{
		  if ( RandomSelection==NbrOfFilesFound )
		    {
		      FileChoosen = g_strdup( File );
		      FileChoosen[ strlen(FileChoosen)-4 ] = '\0';
		      SelectionFound = TRUE;
		    }
		  NbrOfFilesFound++;
		}
	    }
	}
      while( File!=NULL && !SelectionFound );
      g_dir_close( FilesDir );
    }
  g_free( str );
  if ( pNbrOfFiles!=NULL )
    *pNbrOfFiles = NbrOfFilesFound;
  return FileChoosen;
}
// return nbr of fields found
int SplitCommasFieldsInPointersArray( char * LineDatas, char * PtrFieldsDatasFound[], int NbrMaxFields )
{
  int ScanField;
  for( ScanField=0; ScanField<NbrMaxFields; ScanField++ )
    PtrFieldsDatasFound[ ScanField ] = NULL;
  ScanField = 0;
  PtrFieldsDatasFound[ ScanField++ ] = LineDatas;
  do
    {
      do
	{
	  // comma ?
	  if ( *LineDatas==',' && *(LineDatas+1)!='\0' )
	    {
	      // test if not an empty field...
	      if ( *(LineDatas+1)!=',' )
		{
		  PtrFieldsDatasFound[ ScanField ] = LineDatas+1;
		  *LineDatas = '\0';
		}
	      ScanField++;
	    }
	  LineDatas++;
	}
      while( ScanField<NbrMaxFields-1 && *LineDatas!='\0' );
    }
  while( ScanField<NbrMaxFields-1 && *LineDatas!='\0' );
  return ScanField;
}
void ConvertCsvLine( char * FileLineDatas )
{
  char * PtrArraysCsv[20];
  int NbrInfos = SplitCommasFieldsInPointersArray( FileLineDatas, PtrArraysCsv, 20 );
  if ( NbrInfos>=4 )
    {
      StructDiffCoor Diff;
      Diff.x1 = atoi( PtrArraysCsv[ 0 ] );
      Diff.y1 = atoi( PtrArraysCsv[ 1 ] );
      Diff.x2 = atoi( PtrArraysCsv[ 2 ] );
      Diff.y2 = atoi( PtrArraysCsv[ 3 ] );
      g_array_append_val( gDiffCoorArray, Diff );
    }
}
void LoadCsvDiffFile( char * pFilename )
{
  char LineBuff[ 50 ];
  FILE * pFileDiffDesc = fopen( pFilename, "rt" );
  if ( pFileDiffDesc )
    {
      while( !feof( pFileDiffDesc ) )
	{
	  if ( fgets( LineBuff, 50, pFileDiffDesc )!=NULL )
	    {
	      if ( strlen( LineBuff )>=7 )
		ConvertCsvLine( LineBuff );
	    }
	}
      fclose( pFileDiffDesc );
    }
  else
    {
      gc_dialog(_("Error: Abnormally failed to load a data file"), gc_board_stop);
    }
}



