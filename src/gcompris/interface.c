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

#include <gnome.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

static GnomeUIInfo file1_menu_uiinfo[] =
{
  GNOMEUIINFO_MENU_SAVE_ITEM (on_save1_activate, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_EXIT_ITEM (on_exit1_activate, NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo edit1_menu_uiinfo[] =
{
  GNOMEUIINFO_MENU_CUT_ITEM (on_cut1_activate, NULL),
  GNOMEUIINFO_MENU_COPY_ITEM (on_copy1_activate, NULL),
  GNOMEUIINFO_MENU_PASTE_ITEM (on_paste1_activate, NULL),
  GNOMEUIINFO_MENU_CLEAR_ITEM (on_clear1_activate, NULL),
  GNOMEUIINFO_SEPARATOR,
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
  GNOMEUIINFO_MENU_EDIT_TREE (edit1_menu_uiinfo),
  GNOMEUIINFO_MENU_HELP_TREE (help1_menu_uiinfo),
  GNOMEUIINFO_END
};

GtkWidget*
create_gcompris_edit (void)
{
  GtkWidget *gcompris_edit;
  GtkWidget *dock1;
  GtkWidget *toolbar1;
  GtkWidget *tmp_toolbar_icon;
  GtkWidget *button1;
  GtkWidget *button2;
  GtkWidget *button4;
  GtkWidget *button5;
  GtkWidget *button3;
  GtkWidget *hpaned1;
  GtkWidget *scrolledwindow1;
  GtkWidget *ctree1;
  GtkWidget *label1;
  GtkWidget *label2;
  GtkWidget *label3;
  GtkWidget *notebook1;
  GtkWidget *scrolledwindow_description;
  GtkWidget *viewport1;
  GtkWidget *table1;
  GtkWidget *label6;
  GtkWidget *label11;
  GtkWidget *label10;
  GtkWidget *label9;
  GtkWidget *label8;
  GtkWidget *combo1;
  GtkWidget *combo_entryType;
  GtkWidget *label17;
  GtkWidget *label18;
  GtkWidget *label19;
  GtkObject *spinbuttonDifficulty_adj;
  GtkWidget *spinbuttonDifficulty;
  GtkWidget *entryAuthor;
  GtkWidget *entryBoardDir;
  GtkWidget *hbox1;
  GtkWidget *pixmapBoardIcon;
  GtkWidget *buttonIconList;
  GtkWidget *label20;
  GtkWidget *label12;
  GtkWidget *label7;
  GtkWidget *entryName;
  GtkWidget *label21;
  GtkWidget *entryTitle;
  GtkWidget *scrolledwindow3;
  GtkWidget *textPrerequisite;
  GtkWidget *scrolledwindow4;
  GtkWidget *textGoal;
  GtkWidget *scrolledwindow5;
  GtkWidget *textManual;
  GtkWidget *scrolledwindow6;
  GtkWidget *textCredit;
  GtkWidget *scrolledwindow2;
  GtkWidget *textDescription;
  GtkWidget *label4;
  GtkWidget *empty_notebook_page;
  GtkWidget *label5;
  GtkWidget *appbar1;
  GtkTooltips *tooltips;

  tooltips = gtk_tooltips_new ();

  gcompris_edit = gnome_app_new ("Edit", _("Edit"));
  gtk_widget_set_name (gcompris_edit, "gcompris_edit");
  gtk_object_set_data (GTK_OBJECT (gcompris_edit), "gcompris_edit", gcompris_edit);
  gtk_window_set_default_size (GTK_WINDOW (gcompris_edit), 610, 450);

  dock1 = GNOME_APP (gcompris_edit)->dock;
  gtk_widget_set_name (dock1, "dock1");
  gtk_widget_ref (dock1);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "dock1", dock1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (dock1);

  gnome_app_create_menus (GNOME_APP (gcompris_edit), menubar1_uiinfo);

  gtk_widget_set_name (menubar1_uiinfo[0].widget, "file1");
  gtk_widget_ref (menubar1_uiinfo[0].widget);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "file1",
                            menubar1_uiinfo[0].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_set_name (file1_menu_uiinfo[0].widget, "save1");
  gtk_widget_ref (file1_menu_uiinfo[0].widget);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "save1",
                            file1_menu_uiinfo[0].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_set_name (file1_menu_uiinfo[1].widget, "separator1");
  gtk_widget_ref (file1_menu_uiinfo[1].widget);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "separator1",
                            file1_menu_uiinfo[1].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_set_name (file1_menu_uiinfo[2].widget, "exit1");
  gtk_widget_ref (file1_menu_uiinfo[2].widget);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "exit1",
                            file1_menu_uiinfo[2].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_set_name (menubar1_uiinfo[1].widget, "edit1");
  gtk_widget_ref (menubar1_uiinfo[1].widget);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "edit1",
                            menubar1_uiinfo[1].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_set_name (edit1_menu_uiinfo[0].widget, "cut1");
  gtk_widget_ref (edit1_menu_uiinfo[0].widget);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "cut1",
                            edit1_menu_uiinfo[0].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_set_name (edit1_menu_uiinfo[1].widget, "copy1");
  gtk_widget_ref (edit1_menu_uiinfo[1].widget);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "copy1",
                            edit1_menu_uiinfo[1].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_set_name (edit1_menu_uiinfo[2].widget, "paste1");
  gtk_widget_ref (edit1_menu_uiinfo[2].widget);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "paste1",
                            edit1_menu_uiinfo[2].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_set_name (edit1_menu_uiinfo[3].widget, "clear1");
  gtk_widget_ref (edit1_menu_uiinfo[3].widget);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "clear1",
                            edit1_menu_uiinfo[3].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_set_name (edit1_menu_uiinfo[4].widget, "separator2");
  gtk_widget_ref (edit1_menu_uiinfo[4].widget);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "separator2",
                            edit1_menu_uiinfo[4].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_set_name (menubar1_uiinfo[2].widget, "help1");
  gtk_widget_ref (menubar1_uiinfo[2].widget);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "help1",
                            menubar1_uiinfo[2].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_set_name (help1_menu_uiinfo[1].widget, "about1");
  gtk_widget_ref (help1_menu_uiinfo[1].widget);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "about1",
                            help1_menu_uiinfo[1].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  toolbar1 = gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_BOTH);
  gtk_widget_set_name (toolbar1, "toolbar1");
  gtk_widget_ref (toolbar1);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "toolbar1", toolbar1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (toolbar1);
  gnome_app_add_toolbar (GNOME_APP (gcompris_edit), GTK_TOOLBAR (toolbar1), "toolbar1",
                                GNOME_DOCK_ITEM_BEH_EXCLUSIVE,
                                GNOME_DOCK_TOP, 1, 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (toolbar1), 1);
  gtk_toolbar_set_space_size (GTK_TOOLBAR (toolbar1), 16);
  gtk_toolbar_set_space_style (GTK_TOOLBAR (toolbar1), GTK_TOOLBAR_SPACE_LINE);
  gtk_toolbar_set_button_relief (GTK_TOOLBAR (toolbar1), GTK_RELIEF_NONE);

  tmp_toolbar_icon = gnome_stock_pixmap_widget (gcompris_edit, GNOME_STOCK_PIXMAP_ADD);
  button1 = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar1),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                _("Ajouter"),
                                _("Nouveau fichier"), NULL,
                                tmp_toolbar_icon, NULL, NULL);
  gtk_widget_set_name (button1, "button1");
  gtk_widget_ref (button1);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "button1", button1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button1);

  tmp_toolbar_icon = gnome_stock_pixmap_widget (gcompris_edit, GNOME_STOCK_PIXMAP_REMOVE);
  button2 = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar1),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                _("Supprimer"),
                                _("Ouvrir un fichier"), NULL,
                                tmp_toolbar_icon, NULL, NULL);
  gtk_widget_set_name (button2, "button2");
  gtk_widget_ref (button2);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "button2", button2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button2);

  tmp_toolbar_icon = gnome_stock_pixmap_widget (gcompris_edit, GNOME_STOCK_PIXMAP_UP);
  button4 = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar1),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                _("Monter"),
                                NULL, NULL,
                                tmp_toolbar_icon, NULL, NULL);
  gtk_widget_set_name (button4, "button4");
  gtk_widget_ref (button4);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "button4", button4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button4);

  tmp_toolbar_icon = gnome_stock_pixmap_widget (gcompris_edit, GNOME_STOCK_PIXMAP_DOWN);
  button5 = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar1),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                _("Descendre"),
                                NULL, NULL,
                                tmp_toolbar_icon, NULL, NULL);
  gtk_widget_set_name (button5, "button5");
  gtk_widget_ref (button5);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "button5", button5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button5);

  tmp_toolbar_icon = gnome_stock_pixmap_widget (gcompris_edit, GNOME_STOCK_PIXMAP_SAVE);
  button3 = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar1),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                _("Enregistrer"),
                                _("Enregistrer le fichier"), NULL,
                                tmp_toolbar_icon, NULL, NULL);
  gtk_widget_set_name (button3, "button3");
  gtk_widget_ref (button3);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "button3", button3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button3);

  hpaned1 = gtk_hpaned_new ();
  gtk_widget_set_name (hpaned1, "hpaned1");
  gtk_widget_ref (hpaned1);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "hpaned1", hpaned1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hpaned1);
  gnome_app_set_contents (GNOME_APP (gcompris_edit), hpaned1);
  gtk_paned_set_position (GTK_PANED (hpaned1), 257);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow1, "scrolledwindow1");
  gtk_widget_ref (scrolledwindow1);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "scrolledwindow1", scrolledwindow1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow1);
  gtk_paned_pack1 (GTK_PANED (hpaned1), scrolledwindow1, FALSE, TRUE);
  gtk_widget_set_usize (scrolledwindow1, 168, -2);

  ctree1 = gtk_ctree_new (3, 0);
  gtk_widget_set_name (ctree1, "ctree1");
  gtk_widget_ref (ctree1);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "ctree1", ctree1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (ctree1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), ctree1);
  gtk_widget_set_usize (ctree1, 150, -2);
  gtk_tooltips_set_tip (tooltips, ctree1, _("List of activities"), NULL);
  gtk_clist_set_column_width (GTK_CLIST (ctree1), 0, 80);
  gtk_clist_set_column_width (GTK_CLIST (ctree1), 1, 80);
  gtk_clist_set_column_width (GTK_CLIST (ctree1), 2, 80);
  gtk_clist_column_titles_show (GTK_CLIST (ctree1));

  label1 = gtk_label_new (_("Name"));
  gtk_widget_set_name (label1, "label1");
  gtk_widget_ref (label1);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "label1", label1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label1);
  gtk_clist_set_column_widget (GTK_CLIST (ctree1), 0, label1);
  gtk_widget_set_usize (label1, 104, -2);

  label2 = gtk_label_new (_("Title"));
  gtk_widget_set_name (label2, "label2");
  gtk_widget_ref (label2);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "label2", label2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label2);
  gtk_clist_set_column_widget (GTK_CLIST (ctree1), 1, label2);
  gtk_widget_set_usize (label2, 93, -2);

  label3 = gtk_label_new (_("Difficulty"));
  gtk_widget_set_name (label3, "label3");
  gtk_widget_ref (label3);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "label3", label3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label3);
  gtk_clist_set_column_widget (GTK_CLIST (ctree1), 2, label3);
  gtk_widget_set_usize (label3, 9, -2);

  notebook1 = gtk_notebook_new ();
  gtk_widget_set_name (notebook1, "notebook1");
  gtk_widget_ref (notebook1);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "notebook1", notebook1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (notebook1);
  gtk_paned_pack2 (GTK_PANED (hpaned1), notebook1, TRUE, TRUE);

  scrolledwindow_description = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow_description, "scrolledwindow_description");
  gtk_widget_ref (scrolledwindow_description);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "scrolledwindow_description", scrolledwindow_description,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow_description);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow_description);

  viewport1 = gtk_viewport_new (NULL, NULL);
  gtk_widget_set_name (viewport1, "viewport1");
  gtk_widget_ref (viewport1);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "viewport1", viewport1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (viewport1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow_description), viewport1);

  table1 = gtk_table_new (12, 2, FALSE);
  gtk_widget_set_name (table1, "table1");
  gtk_widget_ref (table1);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "table1", table1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table1);
  gtk_container_add (GTK_CONTAINER (viewport1), table1);

  label6 = gtk_label_new (_("Icon"));
  gtk_widget_set_name (label6, "label6");
  gtk_widget_ref (label6);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "label6", label6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label6);
  gtk_table_attach (GTK_TABLE (table1), label6, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);

  label11 = gtk_label_new (_("Board dir"));
  gtk_widget_set_name (label11, "label11");
  gtk_widget_ref (label11);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "label11", label11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label11);
  gtk_table_attach (GTK_TABLE (table1), label11, 0, 1, 7, 8,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label11), 0, 0.5);

  label10 = gtk_label_new (_("Author"));
  gtk_widget_set_name (label10, "label10");
  gtk_widget_ref (label10);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "label10", label10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label10);
  gtk_table_attach (GTK_TABLE (table1), label10, 0, 1, 6, 7,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label10), 0, 0.5);

  label9 = gtk_label_new (_("Difficulty"));
  gtk_widget_set_name (label9, "label9");
  gtk_widget_ref (label9);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "label9", label9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label9);
  gtk_table_attach (GTK_TABLE (table1), label9, 0, 1, 5, 6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label9), 0, 0.5);

  label8 = gtk_label_new (_("Type"));
  gtk_widget_set_name (label8, "label8");
  gtk_widget_ref (label8);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "label8", label8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label8);
  gtk_table_attach (GTK_TABLE (table1), label8, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label8), 0, 0.5);

  combo1 = gtk_combo_new ();
  gtk_widget_set_name (combo1, "combo1");
  gtk_widget_ref (combo1);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "combo1", combo1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (combo1);
  gtk_table_attach (GTK_TABLE (table1), combo1, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  combo_entryType = GTK_COMBO (combo1)->entry;
  gtk_widget_set_name (combo_entryType, "combo_entryType");
  gtk_widget_ref (combo_entryType);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "combo_entryType", combo_entryType,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (combo_entryType);
  gtk_entry_set_editable (GTK_ENTRY (combo_entryType), FALSE);

  label17 = gtk_label_new (_("Prerequisite"));
  gtk_widget_set_name (label17, "label17");
  gtk_widget_ref (label17);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "label17", label17,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label17);
  gtk_table_attach (GTK_TABLE (table1), label17, 0, 1, 8, 9,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label17), 0, 0.5);

  label18 = gtk_label_new (_("Goal"));
  gtk_widget_set_name (label18, "label18");
  gtk_widget_ref (label18);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "label18", label18,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label18);
  gtk_table_attach (GTK_TABLE (table1), label18, 0, 1, 9, 10,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label18), 0, 0.5);

  label19 = gtk_label_new (_("Manual"));
  gtk_widget_set_name (label19, "label19");
  gtk_widget_ref (label19);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "label19", label19,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label19);
  gtk_table_attach (GTK_TABLE (table1), label19, 0, 1, 10, 11,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label19), 0, 0.5);

  spinbuttonDifficulty_adj = gtk_adjustment_new (1, 1, 3, 1, 10, 10);
  spinbuttonDifficulty = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonDifficulty_adj), 1, 0);
  gtk_widget_set_name (spinbuttonDifficulty, "spinbuttonDifficulty");
  gtk_widget_ref (spinbuttonDifficulty);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "spinbuttonDifficulty", spinbuttonDifficulty,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (spinbuttonDifficulty);
  gtk_table_attach (GTK_TABLE (table1), spinbuttonDifficulty, 1, 2, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinbuttonDifficulty), TRUE);

  entryAuthor = gtk_entry_new ();
  gtk_widget_set_name (entryAuthor, "entryAuthor");
  gtk_widget_ref (entryAuthor);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "entryAuthor", entryAuthor,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entryAuthor);
  gtk_table_attach (GTK_TABLE (table1), entryAuthor, 1, 2, 6, 7,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_editable (GTK_ENTRY (entryAuthor), FALSE);

  entryBoardDir = gtk_entry_new ();
  gtk_widget_set_name (entryBoardDir, "entryBoardDir");
  gtk_widget_ref (entryBoardDir);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "entryBoardDir", entryBoardDir,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entryBoardDir);
  gtk_table_attach (GTK_TABLE (table1), entryBoardDir, 1, 2, 7, 8,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_editable (GTK_ENTRY (entryBoardDir), FALSE);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (hbox1, "hbox1");
  gtk_widget_ref (hbox1);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "hbox1", hbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox1);
  gtk_table_attach (GTK_TABLE (table1), hbox1, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  pixmapBoardIcon = gtk_type_new (gnome_pixmap_get_type ());
  gtk_widget_set_name (pixmapBoardIcon, "pixmapBoardIcon");
  gtk_widget_ref (pixmapBoardIcon);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "pixmapBoardIcon", pixmapBoardIcon,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pixmapBoardIcon);
  gtk_box_pack_start (GTK_BOX (hbox1), pixmapBoardIcon, TRUE, TRUE, 0);

  buttonIconList = gtk_button_new_with_label (_("..."));
  gtk_widget_set_name (buttonIconList, "buttonIconList");
  gtk_widget_ref (buttonIconList);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "buttonIconList", buttonIconList,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (buttonIconList);
  gtk_box_pack_start (GTK_BOX (hbox1), buttonIconList, FALSE, FALSE, 0);
  GTK_WIDGET_UNSET_FLAGS (buttonIconList, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, buttonIconList, _("Enregistrer le fichier"), NULL);

  label20 = gtk_label_new (_("Credit"));
  gtk_widget_set_name (label20, "label20");
  gtk_widget_ref (label20);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "label20", label20,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label20);
  gtk_table_attach (GTK_TABLE (table1), label20, 0, 1, 11, 12,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label20), 0, 0.5);

  label12 = gtk_label_new (_("Description"));
  gtk_widget_set_name (label12, "label12");
  gtk_widget_ref (label12);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "label12", label12,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label12);
  gtk_table_attach (GTK_TABLE (table1), label12, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label12), 0, 0.5);

  label7 = gtk_label_new (_("Name"));
  gtk_widget_set_name (label7, "label7");
  gtk_widget_ref (label7);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "label7", label7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label7);
  gtk_table_attach (GTK_TABLE (table1), label7, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label7), 0, 0.5);

  entryName = gtk_entry_new ();
  gtk_widget_set_name (entryName, "entryName");
  gtk_widget_ref (entryName);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "entryName", entryName,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entryName);
  gtk_table_attach (GTK_TABLE (table1), entryName, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_editable (GTK_ENTRY (entryName), FALSE);

  label21 = gtk_label_new (_("Title"));
  gtk_widget_set_name (label21, "label21");
  gtk_widget_ref (label21);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "label21", label21,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label21);
  gtk_table_attach (GTK_TABLE (table1), label21, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label21), 0, 0.5);

  entryTitle = gtk_entry_new ();
  gtk_widget_set_name (entryTitle, "entryTitle");
  gtk_widget_ref (entryTitle);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "entryTitle", entryTitle,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entryTitle);
  gtk_table_attach (GTK_TABLE (table1), entryTitle, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_editable (GTK_ENTRY (entryTitle), FALSE);

  scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow3, "scrolledwindow3");
  gtk_widget_ref (scrolledwindow3);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "scrolledwindow3", scrolledwindow3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow3);
  gtk_table_attach (GTK_TABLE (table1), scrolledwindow3, 1, 2, 8, 9,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow3), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  textPrerequisite = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (textPrerequisite, "textPrerequisite");
  gtk_widget_ref (textPrerequisite);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "textPrerequisite", textPrerequisite,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (textPrerequisite);
  gtk_container_add (GTK_CONTAINER (scrolledwindow3), textPrerequisite);
  gtk_text_set_editable (GTK_TEXT (textPrerequisite), TRUE);

  scrolledwindow4 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow4, "scrolledwindow4");
  gtk_widget_ref (scrolledwindow4);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "scrolledwindow4", scrolledwindow4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow4);
  gtk_table_attach (GTK_TABLE (table1), scrolledwindow4, 1, 2, 9, 10,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow4), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  textGoal = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (textGoal, "textGoal");
  gtk_widget_ref (textGoal);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "textGoal", textGoal,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (textGoal);
  gtk_container_add (GTK_CONTAINER (scrolledwindow4), textGoal);
  gtk_text_set_editable (GTK_TEXT (textGoal), TRUE);

  scrolledwindow5 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow5, "scrolledwindow5");
  gtk_widget_ref (scrolledwindow5);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "scrolledwindow5", scrolledwindow5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow5);
  gtk_table_attach (GTK_TABLE (table1), scrolledwindow5, 1, 2, 10, 11,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow5), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  textManual = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (textManual, "textManual");
  gtk_widget_ref (textManual);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "textManual", textManual,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (textManual);
  gtk_container_add (GTK_CONTAINER (scrolledwindow5), textManual);
  gtk_text_set_editable (GTK_TEXT (textManual), TRUE);

  scrolledwindow6 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow6, "scrolledwindow6");
  gtk_widget_ref (scrolledwindow6);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "scrolledwindow6", scrolledwindow6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow6);
  gtk_table_attach (GTK_TABLE (table1), scrolledwindow6, 1, 2, 11, 12,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow6), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  textCredit = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (textCredit, "textCredit");
  gtk_widget_ref (textCredit);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "textCredit", textCredit,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (textCredit);
  gtk_container_add (GTK_CONTAINER (scrolledwindow6), textCredit);
  gtk_text_set_editable (GTK_TEXT (textCredit), TRUE);

  scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow2, "scrolledwindow2");
  gtk_widget_ref (scrolledwindow2);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "scrolledwindow2", scrolledwindow2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow2);
  gtk_table_attach (GTK_TABLE (table1), scrolledwindow2, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  textDescription = gtk_text_new (NULL, NULL);
  gtk_widget_set_name (textDescription, "textDescription");
  gtk_widget_ref (textDescription);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "textDescription", textDescription,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (textDescription);
  gtk_container_add (GTK_CONTAINER (scrolledwindow2), textDescription);

  label4 = gtk_label_new (_("Description"));
  gtk_widget_set_name (label4, "label4");
  gtk_widget_ref (label4);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "label4", label4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label4);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label4);

  empty_notebook_page = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (empty_notebook_page);
  gtk_container_add (GTK_CONTAINER (notebook1), empty_notebook_page);

  label5 = gtk_label_new (_("Content"));
  gtk_widget_set_name (label5, "label5");
  gtk_widget_ref (label5);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "label5", label5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label5);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label5);

  appbar1 = gnome_appbar_new (TRUE, TRUE, GNOME_PREFERENCES_NEVER);
  gtk_widget_set_name (appbar1, "appbar1");
  gtk_widget_ref (appbar1);
  gtk_object_set_data_full (GTK_OBJECT (gcompris_edit), "appbar1", appbar1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (appbar1);
  gnome_app_set_statusbar (GNOME_APP (gcompris_edit), appbar1);

  gnome_app_install_menu_hints (GNOME_APP (gcompris_edit), menubar1_uiinfo);
  gtk_signal_connect (GTK_OBJECT (ctree1), "tree_select_row",
                      GTK_SIGNAL_FUNC (on_ctree1_tree_select_row),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (buttonIconList), "clicked",
                      GTK_SIGNAL_FUNC (on_buttonIconList_clicked),
                      NULL);

  gtk_widget_grab_focus (ctree1);
  gtk_widget_grab_default (ctree1);
  gtk_object_set_data (GTK_OBJECT (gcompris_edit), "tooltips", tooltips);

  return gcompris_edit;
}

GtkWidget*
create_window_iconlist (void)
{
  GtkWidget *window_iconlist;
  GtkWidget *iconselection;

  window_iconlist = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (window_iconlist, "window_iconlist");
  gtk_object_set_data (GTK_OBJECT (window_iconlist), "window_iconlist", window_iconlist);
  gtk_window_set_title (GTK_WINDOW (window_iconlist), _("Icon List"));

  iconselection = gnome_icon_selection_new ();
  gtk_widget_set_name (iconselection, "iconselection");
  gtk_widget_ref (iconselection);
  gtk_object_set_data_full (GTK_OBJECT (window_iconlist), "iconselection", iconselection,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (iconselection);
  gtk_container_add (GTK_CONTAINER (window_iconlist), iconselection);

  return window_iconlist;
}

