#include "gcompris/gcompris.h"

#include "gtans_callbacks.h"
#include "gtans_interface.h"
#include "gtans_support.h"

#define X_BASE_BIGAREA 340
#define Y_BASE_BIGAREA 50
#define WIDTH_BIGAREA 400

#define X_BASE_SMALLAREA 50
#define Y_BASE_SMALLAREA 50
#define WIDTH_SMALLAREA 200

void create_mainwindow (GnomeCanvasGroup *rootitem)
{
  GtkWidget *wdrawareagrande;
  GtkWidget *wdrawareapetite;


  /* GCompris : suppression of all menus */
  g_assert(rootitem != NULL);

  wdrawareagrande = gtk_drawing_area_new ();
  gtk_widget_set_name (wdrawareagrande, "wdrawareagrande");
  //gtk_widget_set_size_request     (wdrawareagrande,
  //                               WIDTH_BIGAREA,
  //			   WIDTH_BIGAREA
  //			   );

  //gtk_container_add (GTK_CONTAINER (aspectframe1), wdrawareagrande);

  gtk_widget_set_events (wdrawareagrande, GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_STRUCTURE_MASK);

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

  wdrawareapetite = gtk_drawing_area_new ();
  gtk_widget_set_name (wdrawareapetite, "wdrawareapetite");

  gtk_widget_set_events (wdrawareapetite, GDK_EXPOSURE_MASK | GDK_STRUCTURE_MASK);

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

  pixmap_l = gc_skin_pixmap_load("button_backward.png");

  pixmap_r = gc_skin_pixmap_load("button_forward.png");

  previous_figure = gnome_canvas_item_new (rootitem,
					   gnome_canvas_pixbuf_get_type (),
					   "pixbuf", pixmap_l,
					   "x", (double) X_BASE_SMALLAREA,
					   "y", (double) Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 10,
					   NULL);

  next_figure = gnome_canvas_item_new (rootitem,
				       gnome_canvas_pixbuf_get_type (),
				       "pixbuf", pixmap_r,
				       "x", (double) X_BASE_SMALLAREA + WIDTH_SMALLAREA,
				       "y", (double) Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 10,
				       "anchor", GTK_ANCHOR_NE,
				       NULL);


  gtk_signal_connect(GTK_OBJECT(previous_figure), "event",
		     (GtkSignalFunc) on_arrow_clicked,
		     (gpointer) FALSE);

  gtk_signal_connect(GTK_OBJECT(previous_figure), "event",
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);

  gtk_signal_connect(GTK_OBJECT(next_figure), "event",
		     (GtkSignalFunc) on_arrow_clicked,
		     (gpointer) TRUE);

  gtk_signal_connect(GTK_OBJECT(next_figure), "event",
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);


  pixmap_show = gc_pixmap_load("gtans/gtans_show.png");

  pixmap_outline = gc_pixmap_load("gtans/gtans_outline.png");

  show_figure = gnome_canvas_item_new (rootitem,
				       gnome_canvas_pixbuf_get_type (),
				       "pixbuf", pixmap_show,
				       "x", (double) X_BASE_SMALLAREA,
				       "y", (double) Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 80,
				       "anchor",  GTK_ANCHOR_W,
				       NULL);

  outline_figure = gnome_canvas_item_new (rootitem,
				       gnome_canvas_pixbuf_get_type (),
				       "pixbuf", pixmap_outline,
				       "x", (double) X_BASE_SMALLAREA,
				       "y", (double) Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 130,
				       "anchor", GTK_ANCHOR_W,
				       NULL);

  gtk_signal_connect(GTK_OBJECT(outline_figure), "event",
		     (GtkSignalFunc) on_outline_clicked,
		     NULL);

  gtk_signal_connect(GTK_OBJECT(show_figure), "event",
		     (GtkSignalFunc) on_show_clicked,
		     NULL);

  gtk_signal_connect(GTK_OBJECT(outline_figure), "event",
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);

  gtk_signal_connect(GTK_OBJECT(show_figure), "event",
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);

  pixmap_symetry = gc_skin_pixmap_load("draw/tool-flip.png");


  symetry = gnome_canvas_item_new (rootitem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf", pixmap_symetry,
				   "x", (double) X_BASE_SMALLAREA + WIDTH_SMALLAREA - 50,
				   "y", (double) Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 180,
				   "anchor", GTK_ANCHOR_N,
				   NULL);

  gtk_signal_connect(GTK_OBJECT(symetry), "event",
		     (GtkSignalFunc) on_symetry_clicked,
		     NULL);

  gtk_signal_connect(GTK_OBJECT(symetry), "event",
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);


  /* rotation buttons */
  GdkPixbuf   *right_rot = NULL;
  GdkPixbuf   *left_rot = NULL;
  GdkPixbuf   *right_rot_big = NULL;
  GdkPixbuf   *left_rot_big = NULL;
  GnomeCanvasItem *l_rot_s, *r_rot_s, *l_rot_b,  *r_rot_b;

  right_rot       = gc_pixmap_load("gtans/gtans_rotate.png");
  left_rot        = gc_pixmap_load("gtans/gtans_rotate-left.png");

  r_rot_s = gnome_canvas_item_new (rootitem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf", right_rot,
				   "x", (double) X_BASE_SMALLAREA + WIDTH_SMALLAREA,
				   "y", (double) Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 60,
				   "anchor", GTK_ANCHOR_NE,
				   NULL);


  l_rot_s = gnome_canvas_item_new (rootitem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf", left_rot,
				   "x", (double) X_BASE_SMALLAREA + WIDTH_SMALLAREA - 100,
				   "y", (double) Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 60,
				   "anchor", GTK_ANCHOR_NW,
				   NULL);


  gdk_pixbuf_unref(right_rot);
  gdk_pixbuf_unref(left_rot);

  right_rot_big   = gc_pixmap_load("gtans/gtans_2x-rotate.png");
  left_rot_big    = gc_pixmap_load("gtans/gtans_2x-rotate-left.png");

  r_rot_b = gnome_canvas_item_new (rootitem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf", right_rot_big,
				   "x", (double) X_BASE_SMALLAREA + WIDTH_SMALLAREA,
				   "y", (double) Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 120,
				   "anchor", GTK_ANCHOR_NE,
				   NULL);


  l_rot_b = gnome_canvas_item_new (rootitem,
				   gnome_canvas_pixbuf_get_type (),
				   "pixbuf", left_rot_big,
				   "x", (double) X_BASE_SMALLAREA + WIDTH_SMALLAREA - 100,
				   "y", (double) Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 120,
				   "anchor", GTK_ANCHOR_NW,
				   NULL);

  gdk_pixbuf_unref(right_rot_big);
  gdk_pixbuf_unref(left_rot_big);


  gtk_signal_connect(GTK_OBJECT(r_rot_s), "event",
		     (GtkSignalFunc) on_rotation_clicked,
		     (gpointer) 0);

  gtk_signal_connect(GTK_OBJECT(r_rot_s), "event",
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);

  gtk_signal_connect(GTK_OBJECT(l_rot_s), "event",
		     (GtkSignalFunc) on_rotation_clicked,
		     (gpointer) 1);

  gtk_signal_connect(GTK_OBJECT(l_rot_s), "event",
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);

  gtk_signal_connect(GTK_OBJECT(r_rot_b), "event",
		     (GtkSignalFunc) on_rotation_clicked,
		     (gpointer) 2);

  gtk_signal_connect(GTK_OBJECT(r_rot_b), "event",
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);

  gtk_signal_connect(GTK_OBJECT(l_rot_b), "event",
		     (GtkSignalFunc) on_rotation_clicked,
		     (gpointer) 3);

  gtk_signal_connect(GTK_OBJECT(l_rot_b), "event",
		     (GtkSignalFunc) gc_item_focus_event,
		     NULL);

}


