#include <gtk/gtk.h>


void
on_mainwindow_destroy                  (GtkObject       *object,
                                        gpointer         user_data);

void
on_mquit_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mtanscolor_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mtanstexture_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mhighcolor_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mhightexture_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mbgcolor_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mbgtexture_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_msilcolor_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_msilbgcolor_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mfigfile_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_msaveconf_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mabout_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mhelp_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_wdrawareagrande_expose_event        (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

gboolean
on_wdrawareagrande_configure_event     (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

gboolean
on_wdrawareagrande_button_press_event  (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_wdrawareagrande_button_release_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_wdrawareagrande_motion_notify_event (GtkWidget       *widget,
                                        GdkEventMotion  *event,
                                        gpointer         user_data);

gboolean
on_wdrawareapetite_configure_event     (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

gboolean
on_wdrawareapetite_expose_event        (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

void
on_bunsel_clicked                      (GtkButton       *button,
                                        gpointer         user_data);

void
on_bhlptan_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_bhlpout_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_fileselectwindow_delete_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_bfsok_clicked                       (GtkButton       *button,
                                        gpointer         user_data);

void
on_bfscancel_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_colorselectwindow_delete_event      (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_bcsok_clicked                       (GtkButton       *button,
                                        gpointer         user_data);

void
on_bcscancel_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_babclose_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_bhlclose_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_aboutwindow_destroy                 (GtkObject       *object,
                                        gpointer         user_data);

void
on_helpwindow_destroy                  (GtkObject       *object,
                                        gpointer         user_data);

void
on_mhlpoutcolor_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mhlptancolor_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_badjust_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_bfignr_changed                      (GtkEditable     *editable,
                                        gpointer         user_data);


void
on_maccuracy1_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_maccuracy2_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_maccuracy3_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_msilbgcol2_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_msizinc_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_msizdec_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mclrstat_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mclrall_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mrotcont_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mrotstp_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
