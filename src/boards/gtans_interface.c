#include "gcompris/gcompris.h"

#include "gtans_callbacks.h"
#include "gtans_interface.h"
#include "gtans_support.h"

#define X_BASE_BIGAREA 300
#define Y_BASE_BIGAREA 50
#define WIDTH_BIGAREA 400

#define X_BASE_SMALLAREA 50
#define Y_BASE_SMALLAREA 50
#define WIDTH_SMALLAREA 200

static int deb = 0;

void create_mainwindow (GnomeCanvasGroup *rootitem)
{
  GtkWidget *mainwindow;
  GtkWidget *vbox1;
  GtkWidget *menubar1;
  GtkWidget *mgame;
  GtkWidget *mgame_menu;
  GtkAccelGroup *mgame_menu_accels;
  GtkWidget *mquit;
  GtkWidget *mprefs;
  GtkWidget *mprefs_menu;
  GtkAccelGroup *mprefs_menu_accels;
  GtkWidget *mtans;
  GtkWidget *mtans_menu;
  GtkAccelGroup *mtans_menu_accels;
  GtkWidget *mtanscolor;
  GtkWidget *mtanstexture;
  GtkWidget *mhitan;
  GtkWidget *mhitan_menu;
  GtkAccelGroup *mhitan_menu_accels;
  GtkWidget *mhighcolor;
  GtkWidget *mhightexture;
  GtkWidget *mbg;
  GtkWidget *mbg_menu;
  GtkAccelGroup *mbg_menu_accels;
  GtkWidget *mbgcolor;
  GtkWidget *mbgtexture;
  GtkWidget *mhlpoutcolor;
  GtkWidget *s_parateur1;
  GtkWidget *msilcolor;
  GtkWidget *msilbgcolor;
  GtkWidget *msilbgcol2;
  GtkWidget *mhlptancolor;
  GtkWidget *s_parateur2;
  GtkWidget *imsize;
  GtkWidget *imsize_menu;
  GtkAccelGroup *imsize_menu_accels;
  GtkWidget *msizinc;
  GtkWidget *msizdec;
  GtkWidget *maccuracy;
  GtkWidget *maccuracy_menu;
  GtkAccelGroup *maccuracy_menu_accels;
  GSList *accgrp_group = NULL;
  GtkWidget *maccuracy1;
  GtkWidget *maccuracy2;
  GtkWidget *maccuracy3;
  GtkWidget *rotstp;
  GtkWidget *rotstp_menu;
  GtkAccelGroup *rotstp_menu_accels;
  GSList *rotgrp_group = NULL;
  GtkWidget *mrotcont;
  GtkWidget *mrotstp;
  GtkWidget *separator1;
  GtkWidget *mfigfile;
  GtkWidget *s_parateur3;
  GtkWidget *msaveconf;
  GtkWidget *mmisc;
  GtkWidget *mmisc_menu;
  GtkAccelGroup *mmisc_menu_accels;
  GtkWidget *mabout;
  GtkWidget *mhelp;
  GtkWidget *sep17;
  GtkWidget *mclrstat;
  GtkWidget *mclrall;
  GtkWidget *table1;
  GtkWidget *aspectframe1;
  GtkWidget *wdrawareagrande;
  GtkWidget *aspectframe2;
  GtkWidget *wdrawareapetite;
  GtkWidget *vbox2;
  GtkWidget *hbox1;
  GtkWidget *label2;
  GtkObject *bfignr_adj;
  GtkWidget *bfignr;
  GtkWidget *hseparator2;
  GtkWidget *bunsel;
  GtkWidget *hseparator1;
  GtkWidget *bhlptan;
  GtkWidget *bhlpout;
  GtkWidget *wstatusbar;

  /* GCompris : suppression of all menus */
  g_assert(rootitem != NULL);

  wdrawareagrande = gtk_drawing_area_new ();
  gtk_widget_set_name (wdrawareagrande, "wdrawareagrande");
  //gtk_widget_set_size_request     (wdrawareagrande,
  //                               WIDTH_BIGAREA,
  //			   WIDTH_BIGAREA
  //			   );

  //gtk_container_add (GTK_CONTAINER (aspectframe1), wdrawareagrande);

  g_warning("DEBUG %d",deb++);

  gtk_widget_set_events (wdrawareagrande, GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_STRUCTURE_MASK);

  g_warning("DEBUG %d",deb++);

  /* Gcompris */
  gnome_canvas_item_new      ( rootitem,
			       gnome_canvas_widget_get_type(),
			       "widget", wdrawareagrande,
			       "x", (double) X_BASE_BIGAREA,
			       "y", (double) Y_BASE_BIGAREA,
			       "width", (double) WIDTH_BIGAREA,
			       "height", (double) WIDTH_BIGAREA,
			       NULL
			       );

  gtk_widget_show (wdrawareagrande);

  g_warning("DEBUG %d",deb++);



  g_warning("DEBUG %d",deb++);

  wdrawareapetite = gtk_drawing_area_new ();
  gtk_widget_set_name (wdrawareapetite, "wdrawareapetite");

  gtk_widget_set_events (wdrawareapetite, GDK_EXPOSURE_MASK | GDK_STRUCTURE_MASK);

  g_warning("DEBUG %d",deb++);

  /* Gcompris */
  gnome_canvas_item_new      ( rootitem,
			       gnome_canvas_widget_get_type(),
			       "widget", wdrawareapetite,
			       "x", (double) X_BASE_SMALLAREA,
			       "y", (double) Y_BASE_SMALLAREA,
			       "width", (double) WIDTH_SMALLAREA,
			       "height", (double) WIDTH_SMALLAREA,
			       NULL
			       );

  gtk_widget_show (wdrawareapetite);

  g_warning("DEBUG %d",deb++);

  gtk_signal_connect (GTK_OBJECT (wdrawareagrande), "expose_event",
                      GTK_SIGNAL_FUNC (on_wdrawareagrande_expose_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (wdrawareagrande), "configure_event",
                      GTK_SIGNAL_FUNC (on_wdrawareagrande_configure_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (wdrawareagrande), "button_press_event",
                      GTK_SIGNAL_FUNC (on_wdrawareagrande_button_press_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (wdrawareagrande), "button_release_event",
                      GTK_SIGNAL_FUNC (on_wdrawareagrande_button_release_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (wdrawareagrande), "motion_notify_event",
                      GTK_SIGNAL_FUNC (on_wdrawareagrande_motion_notify_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (wdrawareapetite), "configure_event",
                      GTK_SIGNAL_FUNC (on_wdrawareapetite_configure_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (wdrawareapetite), "expose_event",
                      GTK_SIGNAL_FUNC (on_wdrawareapetite_expose_event),
                      NULL);


  /* Gcompris */
  /* add here buttons */

  GdkPixbuf   *pixmap_l = NULL;
  GdkPixbuf   *pixmap_r = NULL;
  GdkPixbuf   *pixmap_show = NULL;
  GdkPixbuf   *pixmap_outline = NULL;
  GdkPixbuf   *pixmap_symetry = NULL;
  GnomeCanvasItem *previous_figure, *next_figure;
  GnomeCanvasItem *show_figure, *outline_figure, *symetry;

  pixmap_l = gcompris_load_skin_pixmap("button_backward.png");

  pixmap_r = gcompris_load_skin_pixmap("button_forward.png");

  previous_figure = gnome_canvas_item_new (rootitem,
					   gnome_canvas_pixbuf_get_type (),
					   "pixbuf", pixmap_l, 
					   "x", (double) X_BASE_SMALLAREA,
					   "y", (double) Y_BASE_SMALLAREA + WIDTH_SMALLAREA,
					   NULL);
      
  next_figure = gnome_canvas_item_new (rootitem,
				       gnome_canvas_pixbuf_get_type (),
				       "pixbuf", pixmap_r, 
				       "x", (double) X_BASE_SMALLAREA + WIDTH_SMALLAREA,
				       "y", (double) Y_BASE_SMALLAREA + WIDTH_SMALLAREA,
				       "anchor", GTK_ANCHOR_NE,
				       NULL);
      
  gtk_signal_connect(GTK_OBJECT(previous_figure), "event",
		     (GtkSignalFunc) on_arrow_clicked,
		     (gpointer) FALSE);
      
  gtk_signal_connect(GTK_OBJECT(previous_figure), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);
      
  gtk_signal_connect(GTK_OBJECT(next_figure), "event",
		     (GtkSignalFunc) on_arrow_clicked,
		     (gpointer) TRUE);
  
  gtk_signal_connect(GTK_OBJECT(next_figure), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);


  pixmap_show = gcompris_load_pixmap("gtans/gtans_show.png");

  pixmap_outline = gcompris_load_pixmap("gtans/gtans_outline.png");
  
  show_figure = gnome_canvas_item_new (rootitem,
				       gnome_canvas_pixbuf_get_type (),
				       "pixbuf", pixmap_show, 
				       "x", (double) X_BASE_SMALLAREA + WIDTH_SMALLAREA/2,
				       "y", (double) Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 50,
				       "anchor",  GTK_ANCHOR_N,
				       NULL);
      
  outline_figure = gnome_canvas_item_new (rootitem,
				       gnome_canvas_pixbuf_get_type (),
				       "pixbuf", pixmap_outline, 
				       "x", (double) X_BASE_SMALLAREA + WIDTH_SMALLAREA/2,
				       "y", (double) Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 100,
				       "anchor", GTK_ANCHOR_N,
				       NULL);
      
  gtk_signal_connect(GTK_OBJECT(outline_figure), "event",
		     (GtkSignalFunc) on_outline_clicked,
		     NULL);
      
  gtk_signal_connect(GTK_OBJECT(show_figure), "event",
		     (GtkSignalFunc) on_show_clicked,
		     NULL);
      
  gtk_signal_connect(GTK_OBJECT(outline_figure), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);
      
  gtk_signal_connect(GTK_OBJECT(show_figure), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);

  pixmap_symetry = gcompris_load_skin_pixmap("draw/tool-flip.png");

      
  symetry = gnome_canvas_item_new (rootitem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf", pixmap_symetry, 
				   "x", (double) X_BASE_SMALLAREA + WIDTH_SMALLAREA/2,
				   "y", (double) Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 150,
				   "anchor", GTK_ANCHOR_N,
				   NULL);
      
  gtk_signal_connect(GTK_OBJECT(symetry), "event",
		     (GtkSignalFunc) on_symetry_clicked,
		     NULL);
      
  gtk_signal_connect(GTK_OBJECT(symetry), "event",
		     (GtkSignalFunc) gcompris_item_event_focus,
		     NULL);
  

}


