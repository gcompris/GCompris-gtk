#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "gcompris.h"
#include "gcompris-edit.h"

void
on_save1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_cut1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_copy1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_paste1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_clear1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_ctree1_tree_select_row              (GtkCTree        *ctree,
                                        GList           *node,
                                        gint             column,
                                        gpointer         user_data)
{
  GcomprisBoard *gcomprisBoard = gtk_ctree_node_get_row_data(ctree, (GtkCTreeNode *)node);

  printf("selected %s\n", gcomprisBoard->name);
  gcompris_edit_display_description(gcomprisBoard);
}


void
on_buttonIconList_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
  printf("icon list clicked\n");

  gcompris_edit_display_iconlist();
}

