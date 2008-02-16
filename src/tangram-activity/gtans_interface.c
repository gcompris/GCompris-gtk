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

void create_mainwindow (GooCanvasItem *rootitem)
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
  goo_canvas_widget_new ( rootitem,
			wdrawareagrande,
			X_BASE_BIGAREA,
			Y_BASE_BIGAREA,
			WIDTH_BIGAREA,
			WIDTH_BIGAREA,
			NULL
			);

  gtk_widget_show (wdrawareagrande);

  wdrawareapetite = gtk_drawing_area_new ();
  gtk_widget_set_name (wdrawareapetite, "wdrawareapetite");

  gtk_widget_set_events (wdrawareapetite, GDK_EXPOSURE_MASK | GDK_STRUCTURE_MASK);

  /* Gcompris */
  goo_canvas_widget_new ( rootitem,
			  wdrawareapetite,
			  X_BASE_SMALLAREA,
			  Y_BASE_SMALLAREA,
			  WIDTH_SMALLAREA,
			  WIDTH_SMALLAREA,
			  NULL
			  );

  gtk_widget_show (wdrawareapetite);

  g_signal_connect (GTK_OBJECT (wdrawareagrande), "expose_event",
                      GTK_SIGNAL_FUNC (on_wdrawareagrande_expose_event),
                      NULL);
  g_signal_connect (GTK_OBJECT (wdrawareagrande), "configure_event",
                      GTK_SIGNAL_FUNC (on_wdrawareagrande_configure_event),
                      NULL);
  g_signal_connect (GTK_OBJECT (wdrawareagrande), "button_press_event",
                      GTK_SIGNAL_FUNC (on_wdrawareagrande_button_press_event),
                      NULL);
  g_signal_connect (GTK_OBJECT (wdrawareagrande), "button_release_event",
                      GTK_SIGNAL_FUNC (on_wdrawareagrande_button_release_event),
                      NULL);
  g_signal_connect (GTK_OBJECT (wdrawareagrande), "motion_notify_event",
                      GTK_SIGNAL_FUNC (on_wdrawareagrande_motion_notify_event),
                      NULL);
  g_signal_connect (GTK_OBJECT (wdrawareapetite), "configure_event",
                      GTK_SIGNAL_FUNC (on_wdrawareapetite_configure_event),
                      NULL);
  g_signal_connect (GTK_OBJECT (wdrawareapetite), "expose_event",
                      GTK_SIGNAL_FUNC (on_wdrawareapetite_expose_event),
                      NULL);


  /* Gcompris */
  /* add here buttons */

  GdkPixbuf   *pixmap_l = NULL;
  GdkPixbuf   *pixmap_r = NULL;
  GdkPixbuf   *pixmap_show = NULL;
  GdkPixbuf   *pixmap_outline = NULL;
  GdkPixbuf   *pixmap_symetry = NULL;
  GooCanvasItem *previous_figure, *next_figure;
  GooCanvasItem *show_figure, *outline_figure, *symetry;

  pixmap_l = gc_skin_pixmap_load("button_backward.png");

  pixmap_r = gc_skin_pixmap_load("button_forward.png");

  previous_figure = goo_canvas_image_new (rootitem,
					  pixmap_l,
					  X_BASE_SMALLAREA,
					  Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 20,
					   NULL);

  next_figure = goo_canvas_image_new (rootitem,
				      pixmap_r,
				      X_BASE_SMALLAREA + WIDTH_SMALLAREA -
				      gdk_pixbuf_get_width (pixmap_r),
				      Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 20,
				       NULL);


  g_signal_connect(previous_figure, "button_press_event",
		     (GtkSignalFunc) on_arrow_clicked,
		     (gpointer) FALSE);

  gc_item_focus_init(previous_figure, NULL);

  g_signal_connect(next_figure, "button_press_event",
		     (GtkSignalFunc) on_arrow_clicked,
		     (gpointer) TRUE);

  gc_item_focus_init(next_figure, NULL);

  pixmap_show = gc_pixmap_load("tangram/gtans_show.png");

  pixmap_outline = gc_pixmap_load("tangram/gtans_outline.png");

  show_figure = goo_canvas_image_new (rootitem,
				      pixmap_show,
				      X_BASE_SMALLAREA,
				      Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 80,
				      NULL);

  outline_figure = goo_canvas_image_new (rootitem,
					 pixmap_outline,
					 X_BASE_SMALLAREA,
					 Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 130,
					 NULL);

  g_signal_connect(outline_figure, "button_press_event",
		     (GtkSignalFunc) on_outline_clicked,
		     NULL);

  g_signal_connect(show_figure, "button_press_event",
		     (GtkSignalFunc) on_show_clicked,
		     NULL);

  gc_item_focus_init(outline_figure, NULL);

  gc_item_focus_init(show_figure, NULL);

  pixmap_symetry = gc_pixmap_load("tangram/tool-flip.png");


  symetry = goo_canvas_image_new (rootitem,
				  pixmap_symetry,
				  X_BASE_SMALLAREA + WIDTH_SMALLAREA - 50,
				  Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 180,
				  NULL);

  g_signal_connect(symetry, "button_press_event",
		     (GtkSignalFunc) on_symetry_clicked,
		     NULL);

  gc_item_focus_init(symetry, NULL);


  /* rotation buttons */
  GdkPixbuf   *right_rot = NULL;
  GdkPixbuf   *left_rot = NULL;
  GdkPixbuf   *right_rot_big = NULL;
  GdkPixbuf   *left_rot_big = NULL;
  GooCanvasItem *l_rot_s, *r_rot_s, *l_rot_b,  *r_rot_b;

  right_rot       = gc_pixmap_load("tangram/gtans_rotate.png");
  left_rot        = gc_pixmap_load("tangram/gtans_rotate-left.png");

  r_rot_s = goo_canvas_image_new (rootitem,
				  right_rot,
				  X_BASE_SMALLAREA + WIDTH_SMALLAREA,
				  Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 60,
				   NULL);


  l_rot_s = goo_canvas_image_new (rootitem,
				  left_rot,
				  X_BASE_SMALLAREA + WIDTH_SMALLAREA - 100,
				  Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 60,
				  NULL);


  gdk_pixbuf_unref(right_rot);
  gdk_pixbuf_unref(left_rot);

  right_rot_big   = gc_pixmap_load("tangram/gtans_2x-rotate.png");
  left_rot_big    = gc_pixmap_load("tangram/gtans_2x-rotate-left.png");

  r_rot_b = goo_canvas_image_new (rootitem,
				  right_rot_big,
				  X_BASE_SMALLAREA + WIDTH_SMALLAREA,
				  Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 120,
				   NULL);


  l_rot_b = goo_canvas_image_new (rootitem,
				  left_rot_big,
				  X_BASE_SMALLAREA + WIDTH_SMALLAREA - 100,
				  Y_BASE_SMALLAREA + WIDTH_SMALLAREA + 120,
				   NULL);

  gdk_pixbuf_unref(right_rot_big);
  gdk_pixbuf_unref(left_rot_big);


  g_signal_connect(r_rot_s, "button_press_event",
		     (GtkSignalFunc) on_rotation_clicked,
		     (gpointer) 0);

  gc_item_focus_init(r_rot_s, NULL);

  g_signal_connect(l_rot_s, "button_press_event",
		     (GtkSignalFunc) on_rotation_clicked,
		     (gpointer) 1);

  gc_item_focus_init(l_rot_s, NULL);

  g_signal_connect(r_rot_b, "button_press_event",
		     (GtkSignalFunc) on_rotation_clicked,
		     (gpointer) 2);

  gc_item_focus_init(r_rot_b, NULL);

  g_signal_connect(l_rot_b, "button_press_event",
		     (GtkSignalFunc) on_rotation_clicked,
		     (gpointer) 3);

  gc_item_focus_init(l_rot_b, NULL);
}


