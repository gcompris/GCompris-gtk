/*
 * NE PAS ÉDITER CE FICHIER - il est généré par Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gnome.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  gtk_object_set_data_full (GTK_OBJECT (component), name, \
    gtk_widget_ref (widget), (GtkDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  gtk_object_set_data (GTK_OBJECT (component), name, widget)

static GnomeUIInfo file1_menu_uiinfo[] =
{
  GNOMEUIINFO_MENU_EXIT_ITEM (on_quit1_activate, NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo help1_menu_uiinfo[] =
{
  GNOMEUIINFO_HELP ("gcompris-edit"),
  GNOMEUIINFO_MENU_ABOUT_ITEM (on_about1_activate, NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo menubar1_uiinfo[] =
{
  GNOMEUIINFO_MENU_FILE_TREE (file1_menu_uiinfo),
  GNOMEUIINFO_MENU_HELP_TREE (help1_menu_uiinfo),
  GNOMEUIINFO_END
};

GtkWidget*
create_gcompris_edit (void)
{
  GtkWidget *gcompris_edit;
  GdkPixbuf *gcompris_edit_icon_pixbuf;
  GtkWidget *bonobodock1;
  GtkWidget *toolbar1;
  GtkWidget *hpaned1;
  GtkWidget *scrolledwindow1;
  GtkWidget *treeview;
  GtkWidget *notebook4;
  GtkWidget *scrolledwindow14;
  GtkWidget *viewport1;
  GtkWidget *table2;
  GtkWidget *label16;
  GtkWidget *label17;
  GtkWidget *label18;
  GtkWidget *label19;
  GtkWidget *label20;
  GtkWidget *label21;
  GtkWidget *label22;
  GtkWidget *label23;
  GtkWidget *label24;
  GtkWidget *label25;
  GtkWidget *label26;
  GtkWidget *iconentry;
  GtkWidget *label27;
  GtkWidget *combo1;
  GtkWidget *entry_type;
  GtkWidget *entry_name;
  GtkWidget *entry_title;
  GtkWidget *scrolledwindow15;
  GtkWidget *textview_description;
  GtkObject *spinbutton_difficulty_adj;
  GtkWidget *spinbutton_difficulty;
  GtkWidget *entry_author;
  GtkWidget *entry_directory;
  GtkWidget *scrolledwindow16;
  GtkWidget *textview_prerequisite;
  GtkWidget *scrolledwindow17;
  GtkWidget *textview_goal;
  GtkWidget *scrolledwindow18;
  GtkWidget *textview_manual;
  GtkWidget *scrolledwindow19;
  GtkWidget *textview_credit;
  GtkWidget *label15;

  gcompris_edit = gnome_app_new ("Gcompris Editor", _("Gcompris Editor"));
  gtk_widget_set_name (gcompris_edit, "gcompris_edit");
  gtk_window_set_default_size (GTK_WINDOW (gcompris_edit), 640, 480);
  gcompris_edit_icon_pixbuf = create_pixbuf (GNOME_ICONDIR"/gcompris-edit.png");
  if (gcompris_edit_icon_pixbuf)
    {
      gtk_window_set_icon (GTK_WINDOW (gcompris_edit), gcompris_edit_icon_pixbuf);
      gdk_pixbuf_unref (gcompris_edit_icon_pixbuf);
    }

  bonobodock1 = GNOME_APP (gcompris_edit)->dock;
  gtk_widget_set_name (bonobodock1, "bonobodock1");
  gtk_widget_show (bonobodock1);

  gnome_app_create_menus (GNOME_APP (gcompris_edit), menubar1_uiinfo);

  gtk_widget_set_name (menubar1_uiinfo[0].widget, "file1");

  gtk_widget_set_name (file1_menu_uiinfo[0].widget, "quit1");

  gtk_widget_set_name (menubar1_uiinfo[1].widget, "edit1");

  gtk_widget_set_name (menubar1_uiinfo[2].widget, "help1");

  gtk_widget_set_name (help1_menu_uiinfo[1].widget, "about1");

  toolbar1 = gtk_toolbar_new ();
  gtk_widget_set_name (toolbar1, "toolbar1");
  gtk_widget_show (toolbar1);
  gnome_app_add_toolbar (GNOME_APP (gcompris_edit), GTK_TOOLBAR (toolbar1), "toolbar1",
                                BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
                                BONOBO_DOCK_TOP, 1, 0, 0);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar1), GTK_TOOLBAR_BOTH);

  hpaned1 = gtk_hpaned_new ();
  gtk_widget_set_name (hpaned1, "hpaned1");
  gtk_widget_show (hpaned1);
  gnome_app_set_contents (GNOME_APP (gcompris_edit), hpaned1);
  gtk_paned_set_position (GTK_PANED (hpaned1), 150);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow1, "scrolledwindow1");
  gtk_widget_show (scrolledwindow1);
  gtk_paned_pack1 (GTK_PANED (hpaned1), scrolledwindow1, FALSE, TRUE);

  treeview = gtk_tree_view_new ();
  gtk_widget_set_name (treeview, "treeview");
  gtk_widget_show (treeview);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), treeview);

  notebook4 = gtk_notebook_new ();
  gtk_widget_set_name (notebook4, "notebook4");
  gtk_widget_show (notebook4);
  gtk_paned_pack2 (GTK_PANED (hpaned1), notebook4, TRUE, TRUE);

  scrolledwindow14 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow14, "scrolledwindow14");
  gtk_widget_show (scrolledwindow14);
  gtk_container_add (GTK_CONTAINER (notebook4), scrolledwindow14);

  viewport1 = gtk_viewport_new (NULL, NULL);
  gtk_widget_set_name (viewport1, "viewport1");
  gtk_widget_show (viewport1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow14), viewport1);

  table2 = gtk_table_new (12, 2, FALSE);
  gtk_widget_set_name (table2, "table2");
  gtk_widget_show (table2);
  gtk_container_add (GTK_CONTAINER (viewport1), table2);

  label16 = gtk_label_new (_("Icon"));
  gtk_widget_set_name (label16, "label16");
  gtk_widget_show (label16);
  gtk_table_attach (GTK_TABLE (table2), label16, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label16), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label16), 0, 0.5);

  label17 = gtk_label_new (_("Name"));
  gtk_widget_set_name (label17, "label17");
  gtk_widget_show (label17);
  gtk_table_attach (GTK_TABLE (table2), label17, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label17), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label17), 0, 0.5);

  label18 = gtk_label_new (_("Description"));
  gtk_widget_set_name (label18, "label18");
  gtk_widget_show (label18);
  gtk_table_attach (GTK_TABLE (table2), label18, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label18), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label18), 0, 0.5);

  label19 = gtk_label_new (_("Type"));
  gtk_widget_set_name (label19, "label19");
  gtk_widget_show (label19);
  gtk_table_attach (GTK_TABLE (table2), label19, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label19), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label19), 0, 0.5);

  label20 = gtk_label_new (_("Difficulty"));
  gtk_widget_set_name (label20, "label20");
  gtk_widget_show (label20);
  gtk_table_attach (GTK_TABLE (table2), label20, 0, 1, 5, 6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label20), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label20), 0, 0.5);

  label21 = gtk_label_new (_("Author"));
  gtk_widget_set_name (label21, "label21");
  gtk_widget_show (label21);
  gtk_table_attach (GTK_TABLE (table2), label21, 0, 1, 6, 7,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label21), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label21), 0, 0.5);

  label22 = gtk_label_new (_("Directory"));
  gtk_widget_set_name (label22, "label22");
  gtk_widget_show (label22);
  gtk_table_attach (GTK_TABLE (table2), label22, 0, 1, 7, 8,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label22), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label22), 0, 0.5);

  label23 = gtk_label_new (_("Prerequisite"));
  gtk_widget_set_name (label23, "label23");
  gtk_widget_show (label23);
  gtk_table_attach (GTK_TABLE (table2), label23, 0, 1, 8, 9,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label23), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label23), 0, 0.5);

  label24 = gtk_label_new (_("Goal"));
  gtk_widget_set_name (label24, "label24");
  gtk_widget_show (label24);
  gtk_table_attach (GTK_TABLE (table2), label24, 0, 1, 9, 10,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label24), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label24), 0, 0.5);

  label25 = gtk_label_new (_("Manual"));
  gtk_widget_set_name (label25, "label25");
  gtk_widget_show (label25);
  gtk_table_attach (GTK_TABLE (table2), label25, 0, 1, 10, 11,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label25), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label25), 0, 0.5);

  label26 = gtk_label_new (_("Credit"));
  gtk_widget_set_name (label26, "label26");
  gtk_widget_show (label26);
  gtk_table_attach (GTK_TABLE (table2), label26, 0, 1, 11, 12,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label26), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label26), 0, 0.5);

  iconentry = gnome_icon_entry_new (NULL, NULL);
  gtk_widget_set_name (iconentry, "iconentry");
  gtk_widget_show (iconentry);
  gtk_table_attach (GTK_TABLE (table2), iconentry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  label27 = gtk_label_new (_("Title"));
  gtk_widget_set_name (label27, "label27");
  gtk_widget_show (label27);
  gtk_table_attach (GTK_TABLE (table2), label27, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label27), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label27), 0, 0.5);

  combo1 = gtk_combo_new ();
  gtk_object_set_data (GTK_OBJECT (GTK_COMBO (combo1)->popwin),
                       "GladeParentKey", combo1);
  gtk_widget_set_name (combo1, "combo1");
  gtk_widget_show (combo1);
  gtk_table_attach (GTK_TABLE (table2), combo1, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  entry_type = GTK_COMBO (combo1)->entry;
  gtk_widget_set_name (entry_type, "entry_type");
  gtk_widget_show (entry_type);
  gtk_entry_set_editable (GTK_ENTRY (entry_type), FALSE);

  entry_name = gtk_entry_new ();
  gtk_widget_set_name (entry_name, "entry_name");
  gtk_widget_show (entry_name);
  gtk_table_attach (GTK_TABLE (table2), entry_name, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 2);
  gtk_entry_set_editable (GTK_ENTRY (entry_name), FALSE);

  entry_title = gtk_entry_new ();
  gtk_widget_set_name (entry_title, "entry_title");
  gtk_widget_show (entry_title);
  gtk_table_attach (GTK_TABLE (table2), entry_title, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 1);
  gtk_entry_set_editable (GTK_ENTRY (entry_title), FALSE);

  scrolledwindow15 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow15, "scrolledwindow15");
  gtk_widget_show (scrolledwindow15);
  gtk_table_attach (GTK_TABLE (table2), scrolledwindow15, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 1);

  textview_description = gtk_text_view_new ();
  gtk_widget_set_name (textview_description, "textview_description");
  gtk_widget_show (textview_description);
  gtk_container_add (GTK_CONTAINER (scrolledwindow15), textview_description);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview_description), FALSE);

  spinbutton_difficulty_adj = gtk_adjustment_new (1, 1, 3, 1, 1, 1);
  spinbutton_difficulty = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_difficulty_adj), 1, 0);
  gtk_widget_set_name (spinbutton_difficulty, "spinbutton_difficulty");
  gtk_widget_show (spinbutton_difficulty);
  gtk_table_attach (GTK_TABLE (table2), spinbutton_difficulty, 1, 2, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 2);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbutton_difficulty), TRUE);

  entry_author = gtk_entry_new ();
  gtk_widget_set_name (entry_author, "entry_author");
  gtk_widget_show (entry_author);
  gtk_table_attach (GTK_TABLE (table2), entry_author, 1, 2, 6, 7,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 2);
  gtk_entry_set_editable (GTK_ENTRY (entry_author), FALSE);

  entry_directory = gtk_entry_new ();
  gtk_widget_set_name (entry_directory, "entry_directory");
  gtk_widget_show (entry_directory);
  gtk_table_attach (GTK_TABLE (table2), entry_directory, 1, 2, 7, 8,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 2);
  gtk_entry_set_editable (GTK_ENTRY (entry_directory), FALSE);

  scrolledwindow16 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow16, "scrolledwindow16");
  gtk_widget_show (scrolledwindow16);
  gtk_table_attach (GTK_TABLE (table2), scrolledwindow16, 1, 2, 8, 9,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 2);

  textview_prerequisite = gtk_text_view_new ();
  gtk_widget_set_name (textview_prerequisite, "textview_prerequisite");
  gtk_widget_show (textview_prerequisite);
  gtk_container_add (GTK_CONTAINER (scrolledwindow16), textview_prerequisite);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview_prerequisite), FALSE);

  scrolledwindow17 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow17, "scrolledwindow17");
  gtk_widget_show (scrolledwindow17);
  gtk_table_attach (GTK_TABLE (table2), scrolledwindow17, 1, 2, 9, 10,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 2);

  textview_goal = gtk_text_view_new ();
  gtk_widget_set_name (textview_goal, "textview_goal");
  gtk_widget_show (textview_goal);
  gtk_container_add (GTK_CONTAINER (scrolledwindow17), textview_goal);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview_goal), FALSE);

  scrolledwindow18 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow18, "scrolledwindow18");
  gtk_widget_show (scrolledwindow18);
  gtk_table_attach (GTK_TABLE (table2), scrolledwindow18, 1, 2, 10, 11,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 2);

  textview_manual = gtk_text_view_new ();
  gtk_widget_set_name (textview_manual, "textview_manual");
  gtk_widget_show (textview_manual);
  gtk_container_add (GTK_CONTAINER (scrolledwindow18), textview_manual);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview_manual), FALSE);

  scrolledwindow19 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow19, "scrolledwindow19");
  gtk_widget_show (scrolledwindow19);
  gtk_table_attach (GTK_TABLE (table2), scrolledwindow19, 1, 2, 11, 12,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 2);

  textview_credit = gtk_text_view_new ();
  gtk_widget_set_name (textview_credit, "textview_credit");
  gtk_widget_show (textview_credit);
  gtk_container_add (GTK_CONTAINER (scrolledwindow19), textview_credit);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview_credit), FALSE);

  label15 = gtk_label_new (_("Description"));
  gtk_widget_set_name (label15, "label15");
  gtk_widget_show (label15);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook4), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook4), 0), label15);
  gtk_label_set_justify (GTK_LABEL (label15), GTK_JUSTIFY_LEFT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (gcompris_edit, gcompris_edit, "gcompris_edit");
  GLADE_HOOKUP_OBJECT (gcompris_edit, bonobodock1, "bonobodock1");
  GLADE_HOOKUP_OBJECT (gcompris_edit, menubar1_uiinfo[0].widget, "file1");
  GLADE_HOOKUP_OBJECT (gcompris_edit, file1_menu_uiinfo[0].widget, "quit1");
  GLADE_HOOKUP_OBJECT (gcompris_edit, menubar1_uiinfo[1].widget, "edit1");
  GLADE_HOOKUP_OBJECT (gcompris_edit, menubar1_uiinfo[2].widget, "help1");
  GLADE_HOOKUP_OBJECT (gcompris_edit, help1_menu_uiinfo[1].widget, "about1");
  GLADE_HOOKUP_OBJECT (gcompris_edit, toolbar1, "toolbar1");
  GLADE_HOOKUP_OBJECT (gcompris_edit, hpaned1, "hpaned1");
  GLADE_HOOKUP_OBJECT (gcompris_edit, scrolledwindow1, "scrolledwindow1");
  GLADE_HOOKUP_OBJECT (gcompris_edit, treeview, "treeview");
  GLADE_HOOKUP_OBJECT (gcompris_edit, notebook4, "notebook4");
  GLADE_HOOKUP_OBJECT (gcompris_edit, scrolledwindow14, "scrolledwindow14");
  GLADE_HOOKUP_OBJECT (gcompris_edit, viewport1, "viewport1");
  GLADE_HOOKUP_OBJECT (gcompris_edit, table2, "table2");
  GLADE_HOOKUP_OBJECT (gcompris_edit, label16, "label16");
  GLADE_HOOKUP_OBJECT (gcompris_edit, label17, "label17");
  GLADE_HOOKUP_OBJECT (gcompris_edit, label18, "label18");
  GLADE_HOOKUP_OBJECT (gcompris_edit, label19, "label19");
  GLADE_HOOKUP_OBJECT (gcompris_edit, label20, "label20");
  GLADE_HOOKUP_OBJECT (gcompris_edit, label21, "label21");
  GLADE_HOOKUP_OBJECT (gcompris_edit, label22, "label22");
  GLADE_HOOKUP_OBJECT (gcompris_edit, label23, "label23");
  GLADE_HOOKUP_OBJECT (gcompris_edit, label24, "label24");
  GLADE_HOOKUP_OBJECT (gcompris_edit, label25, "label25");
  GLADE_HOOKUP_OBJECT (gcompris_edit, label26, "label26");
  GLADE_HOOKUP_OBJECT (gcompris_edit, iconentry, "iconentry");
  GLADE_HOOKUP_OBJECT (gcompris_edit, label27, "label27");
  GLADE_HOOKUP_OBJECT (gcompris_edit, combo1, "combo1");
  GLADE_HOOKUP_OBJECT (gcompris_edit, entry_type, "entry_type");
  GLADE_HOOKUP_OBJECT (gcompris_edit, entry_name, "entry_name");
  GLADE_HOOKUP_OBJECT (gcompris_edit, entry_title, "entry_title");
  GLADE_HOOKUP_OBJECT (gcompris_edit, scrolledwindow15, "scrolledwindow15");
  GLADE_HOOKUP_OBJECT (gcompris_edit, textview_description, "textview_description");
  GLADE_HOOKUP_OBJECT (gcompris_edit, spinbutton_difficulty, "spinbutton_difficulty");
  GLADE_HOOKUP_OBJECT (gcompris_edit, entry_author, "entry_author");
  GLADE_HOOKUP_OBJECT (gcompris_edit, entry_directory, "entry_directory");
  GLADE_HOOKUP_OBJECT (gcompris_edit, scrolledwindow16, "scrolledwindow16");
  GLADE_HOOKUP_OBJECT (gcompris_edit, textview_prerequisite, "textview_prerequisite");
  GLADE_HOOKUP_OBJECT (gcompris_edit, scrolledwindow17, "scrolledwindow17");
  GLADE_HOOKUP_OBJECT (gcompris_edit, textview_goal, "textview_goal");
  GLADE_HOOKUP_OBJECT (gcompris_edit, scrolledwindow18, "scrolledwindow18");
  GLADE_HOOKUP_OBJECT (gcompris_edit, textview_manual, "textview_manual");
  GLADE_HOOKUP_OBJECT (gcompris_edit, scrolledwindow19, "scrolledwindow19");
  GLADE_HOOKUP_OBJECT (gcompris_edit, textview_credit, "textview_credit");
  GLADE_HOOKUP_OBJECT (gcompris_edit, label15, "label15");

  return gcompris_edit;
}

GtkWidget*
create_about1 (void)
{
  const gchar *authors[] = {
    "Bruno Coudoin",
    NULL
  };
  const gchar *documenters[] = { NULL };
  /* TRANSLATORS: Replace this string with your names, one name per line. */
  gchar *translators = _("translator_credits");
  GtkWidget *about1;

  if (!strcmp (translators, "translator_credits"))
    translators = NULL;
  about1 = gnome_about_new ("Gcompris Editor", VERSION,
                        _("Released under GPL"),
                        "",
                        authors,
                        documenters,
                        translators,
                        NULL);
  gtk_widget_set_name (about1, "about1");

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (about1, about1, "about1");

  return about1;
}

