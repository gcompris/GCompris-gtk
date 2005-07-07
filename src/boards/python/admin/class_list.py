#  gcompris - class_list.py
# 
# Copyright (C) 2005 Bruno Coudoin and Yves Combe
# 
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
# 
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# 

import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gtk
import gtk.gdk
import gobject
from gettext import gettext as _

# Database
from pysqlite2 import dbapi2 as sqlite

import class_edit

# Class Management
(
  COLUMN_CLASSID,
  COLUMN_NAME,
  COLUMN_TEACHER,
  COLUMN_CLASS_EDITABLE
) = range(4)

class Class_list:
  """GCompris Class List Table"""


  # area is the drawing area for the list
  def __init__(self, canvas, db_connect, db_cursor, area, hgap, vgap):

      self.rootitem = canvas
      self.cur = db_cursor
      self.con = db_connect
      
      # ---------------
      # Class Management
      # ---------------

      # Grab the class data
      self.cur.execute('select * from class')
      self.class_data = self.cur.fetchall()

      # Create the table
      sw = gtk.ScrolledWindow()
      sw.set_shadow_type(gtk.SHADOW_ETCHED_IN)
      sw.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)

      # create tree model
      model = self.__create_model_class()

      # create tree view
      treeview_class = gtk.TreeView(model)
      treeview_class.set_rules_hint(True)
      treeview_class.set_search_column(COLUMN_NAME)

      sw.add(treeview_class)

      # Some constants for the layout
      but_height = hgap * 2
      but_width  = hgap * 6
            
      self.rootitem.add(
        gnome.canvas.CanvasWidget,
        widget=sw,
        x=area[0] + hgap,
        y=area[1],
        width=area[2]-area[0]-hgap*2-but_width,
        height=area[3]-area[1],
        anchor=gtk.ANCHOR_NW,
        size_pixels=False)
      sw.show()
      treeview_class.show()

      # add columns to the tree view
      self.__add_columns_class(treeview_class)

      # Add buttons

      button_add = gtk.Button(_("Add class"))
      button_add.connect("clicked", self.on_add_class_clicked, model)
      self.rootitem.add(
        gnome.canvas.CanvasWidget,
        widget=button_add,
        x=area[2]-but_width,
        y=area[1],
        width=100,
        height=but_height,
        anchor=gtk.ANCHOR_NW,
        size_pixels=False)
      button_add.show()
      
      button_rem = gtk.Button(_("Remove class"))
      button_rem.connect("clicked", self.on_remove_class_clicked, treeview_class)
      self.rootitem.add(
        gnome.canvas.CanvasWidget,
        widget=button_rem,
        x=area[2]-but_width,
        y=area[1] + but_height + vgap,
        width=100,
        height=but_height,
        anchor=gtk.ANCHOR_NW,
        size_pixels=False)
      button_rem.show()

      button_rem = gtk.Button(_("Edit class"))
      button_rem.connect("clicked", self.on_edit_class_clicked, treeview_class)
      self.rootitem.add(
        gnome.canvas.CanvasWidget,
        widget=button_rem,
        x=area[2]-but_width,
        y=area[1] + but_height*2 + vgap*2,
        width=100,
        height=but_height,
        anchor=gtk.ANCHOR_NW,
        size_pixels=False)
      button_rem.show()

  # -------------------
  # Class Management
  # -------------------

  def __create_model_class(self):
    model = gtk.ListStore(
      gobject.TYPE_INT,
      gobject.TYPE_STRING,
      gobject.TYPE_STRING,
      gobject.TYPE_BOOLEAN)

    for item in self.class_data:
      iter = model.append()
      model.set(iter,
                 COLUMN_CLASSID,   item[COLUMN_CLASSID],
                 COLUMN_NAME,      item[COLUMN_NAME],
                 COLUMN_TEACHER,   item[COLUMN_TEACHER],
                 COLUMN_CLASS_EDITABLE,  True)
    return model


  def __add_columns_class(self, treeview):
    
    model = treeview.get_model()

    # columns for name
    renderer = gtk.CellRendererText()
    renderer.connect("edited", self.on_cell_class_edited, model)
    renderer.set_data("column", COLUMN_NAME)
    column = gtk.TreeViewColumn(_('Name'), renderer,
                                text=COLUMN_NAME,
                                editable=COLUMN_CLASS_EDITABLE)
    column.set_sort_column_id(COLUMN_NAME)
    treeview.append_column(column)

    # columns for first name
    renderer = gtk.CellRendererText()
    renderer.connect("edited", self.on_cell_class_edited, model)
    renderer.set_data("column", COLUMN_TEACHER)
    column = gtk.TreeViewColumn(_('Teacher'), renderer,
                                text=COLUMN_TEACHER,
                                editable=COLUMN_CLASS_EDITABLE)
    column.set_sort_column_id(COLUMN_TEACHER)
    treeview.append_column(column)


  # Return the next class id
  def get_next_class_id(self):
    self.cur.execute('select max(class_id) from class')
    class_id = self.cur.fetchone()[0]
    if(class_id == None):
      class_id=0
    else:
      class_id += 1
      
    return class_id


  # Add class in the model
  def add_class_in_model(self, model, aclass):
    iter = model.append()
    model.set (iter,
               COLUMN_CLASSID,    aclass[COLUMN_CLASSID],
               COLUMN_NAME,       aclass[COLUMN_NAME],
               COLUMN_TEACHER,    aclass[COLUMN_TEACHER],
               COLUMN_CLASS_EDITABLE,  True
               )

    
  #
  def on_add_class_clicked(self, button, model):
    class_id = self.get_next_class_id()

    new_class = [class_id, "?", "?", "?", "?", 0]
    self.add_class_in_model(model, new_class)


  def on_remove_class_clicked(self, button, treeview):

    selection = treeview.get_selection()
    model, iter = selection.get_selected()

    if iter:
      path = model.get_path(iter)[0]
      class_id = model.get_value(iter, COLUMN_CLASSID)
      model.remove(iter)
      # Remove it from the base
      print "Deleting class_id=" + str(class_id)
      self.cur.execute('delete from class where class_id=?', (class_id,))
      self.con.commit()


  def on_cell_class_edited(self, cell, path_string, new_text, model):

    iter = model.get_iter_from_string(path_string)
    path = model.get_path(iter)[0]
    column = cell.get_data("column")

    class_id = model.get_value(iter, COLUMN_CLASSID)
    
    if column == COLUMN_NAME:
      model.set(iter, column, new_text)

    elif column == COLUMN_TEACHER:
      model.set(iter, column, new_text)

    class_data = (class_id,
                  model.get_value(iter, COLUMN_NAME),
                  model.get_value(iter, COLUMN_TEACHER))
    # Save the changes in the base
    self.cur.execute('insert or replace into class (class_id, name, teacher) values (?, ?, ?)', class_data)
    self.con.commit()


  def on_edit_class_clicked(self, button, treeview):

    selection = treeview.get_selection()
    model, iter = selection.get_selected()

    if iter:
      path = model.get_path(iter)[0]
      class_id   = model.get_value(iter, COLUMN_CLASSID)
      class_name = model.get_value(iter, COLUMN_NAME)
      class_edit.ClassEdit(self.con, self.cur, class_id, class_name)

    else:
      # Tell the user to select a class first
      dialog = gtk.MessageDialog(None,
                                 gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                                 gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                                 _("You must first select a class in the list"))
      dialog.run()
      dialog.destroy()
