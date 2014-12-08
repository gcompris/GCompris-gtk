#  gcompris - class_list.py
#
# Copyright (C) 2005, 2008 Bruno Coudoin and Yves Combe
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, see <http://www.gnu.org/licenses/>.
#

import goocanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gtk
import gtk.gdk
import gobject
from gcompris import gcompris_gettext as _

import constants

import class_edit
import user_list

# Class Management
(
  COLUMN_CLASSID,
  COLUMN_NAME,
  COLUMN_TEACHER,
) = range(3)


class Class_list:
  """GCompris Class List Table"""


  # area is the drawing area for the list
  def __init__(self, frame, db_connect, db_cursor):

      self.cur = db_cursor
      self.con = db_connect

      self.class_data = []

      # ---------------
      # Class Management
      # ---------------

      # create tree model
      model = self.__create_model_class()

      # Main box is vertical
      top_box = gtk.VBox(False, 8)
      top_box.show()
      frame.add(top_box)

      # First line
      group_hbox = gtk.HBox(False, 8)
      group_hbox.show()
      top_box.add(group_hbox)

      left_box = gtk.VBox(False, 8)
      left_box.show()
      group_hbox.add(left_box)

      right_box = gtk.VBox(False, 8)
      right_box.show()
      group_hbox.add(right_box)


      # Create the table
      sw = gtk.ScrolledWindow()
      sw.show()
      sw.set_shadow_type(gtk.SHADOW_ETCHED_IN)
      sw.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)

      # create tree view
      self.treeview_class = gtk.TreeView(model)
      self.treeview_class.show()
      self.treeview_class.set_rules_hint(True)
      self.treeview_class.set_search_column(COLUMN_NAME)

      sw.add(self.treeview_class)

      left_box.pack_start(sw, True, True, 0)

      # add columns to the tree view
      self.__add_columns_class(self.treeview_class)

      # Add buttons

      button = gtk.Button(stock='gtk-add')
      button.connect("clicked", self.on_add_class_clicked, self.treeview_class)
      right_box.pack_start(button, False, False, 0)
      button.show()

      self.button_edit = gtk.Button(stock='gtk-edit')
      self.button_edit.connect("clicked", self.on_edit_class_clicked, self.treeview_class)
      right_box.pack_start(self.button_edit, False, False, 0)
      self.button_edit.show()
      # Not editable until one class is selected
      self.button_edit.set_sensitive(False)

      self.button_remove = gtk.Button(stock='gtk-remove')
      self.button_remove.connect("clicked", self.on_remove_class_clicked, self.treeview_class)
      right_box.pack_start(self.button_remove, False, False, 0)
      self.button_remove.show()
      # Not removable until one class is selected
      self.button_remove.set_sensitive(False)

      # User list for the group
      user_hbox = gtk.HBox(False, 8)
      user_hbox.show()
      top_box.add(user_hbox)


      self.list_user = user_list.User_list(user_hbox,
                                           self.con, self.cur)

      # Missing callbacks
      selection = self.treeview_class.get_selection()
      selection.connect('changed', self.class_changed_cb, self.list_user)


  # -------------------
  # Class Management
  # -------------------

  def __create_model_class(self):
    model = gtk.ListStore(
      gobject.TYPE_INT,
      gobject.TYPE_STRING,
      gobject.TYPE_STRING,
      gobject.TYPE_BOOLEAN)

    # Grab the class data
    self.cur.execute('select * from class')
    self.class_data = self.cur.fetchall()

    for item in self.class_data:
      iter = model.append()
      model.set(iter,
                 COLUMN_CLASSID,   item[COLUMN_CLASSID],
                 COLUMN_NAME,      item[COLUMN_NAME],
                 COLUMN_TEACHER,   item[COLUMN_TEACHER])
    return model


  def __add_columns_class(self, treeview):
    model = treeview.get_model()

    # Total column length must be 400

    # columns for name
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_NAME)
    column = gtk.TreeViewColumn(_('Class'), renderer,
                                text=COLUMN_NAME)
    column.set_sort_column_id(COLUMN_NAME)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_CLASSNAME)
    treeview.append_column(column)

    # columns for teacher
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_TEACHER)
    column = gtk.TreeViewColumn(_('Teacher'), renderer,
                                text=COLUMN_TEACHER)
    column.set_sort_column_id(COLUMN_TEACHER)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_TEACHER)
    treeview.append_column(column)


  # Add class in the model
  def add_class_in_model(self, model, aclass):
    iter = model.append()
    model.set (iter,
               COLUMN_CLASSID,    aclass[COLUMN_CLASSID],
               COLUMN_NAME,       aclass[COLUMN_NAME],
               COLUMN_TEACHER,    aclass[COLUMN_TEACHER]
               )


  def on_remove_class_clicked(self, button, treeview):
    selection = treeview.get_selection()
    model, iter = selection.get_selected()

    if iter:
      path = model.get_path(iter)[0]
      class_id = model.get_value(iter, COLUMN_CLASSID)
      model.remove(iter)

      #
      # Remove it from the base (Triggers maintains other tables)
      #
      self.cur.execute('DELETE FROM class WHERE class_id=?', (class_id,))
      self.con.commit()

      self.list_user.reload(class_id)


  def on_edit_class_clicked(self, button, treeview):
    selection = treeview.get_selection()
    model, iter = selection.get_selected()

    if iter:
      path = model.get_path(iter)[0]
      class_id      = model.get_value(iter, COLUMN_CLASSID)
      class_name    = model.get_value(iter, COLUMN_NAME)
      teacher_name  = model.get_value(iter, COLUMN_TEACHER)
      class_edit.ClassEdit(self.con, self.cur,
                           class_id, class_name, teacher_name,
                           self)


  def on_add_class_clicked(self, button, treeview):
    model = treeview.get_model()
    class_id = constants.get_next_class_id(self.con, self.cur)

    class_edit.ClassEdit(self.con, self.cur,
                         class_id, "", "",
                         self)


  # The class is changed ...
  def class_changed_cb(self, selection, user_list):
    model, iter = selection.get_selected()

    if iter:
      path = model.get_path(iter)[0]
      class_id = model.get_value(iter, COLUMN_CLASSID)
      user_list.reload(class_id)

      # The Unaffected class is not editable.
      if class_id == 1:
        self.button_edit.set_sensitive(False)
        self.button_remove.set_sensitive(False)
      else:
        self.button_edit.set_sensitive(True)
        self.button_remove.set_sensitive(True)


  # Reload data (class data and users)
  def reload(self, class_id, class_name, class_teacher):
    # We need to find the row matching this class_id.
    # If not found, it's a new class to create
    model = self.treeview_class.get_model()
    iter = model.get_iter_first()

    updated = False
    # Loop over each class raw
    while(iter):
      path = model.get_path(iter)[0]
      tmp_class_id = model.get_value(iter, COLUMN_CLASSID)

      if(tmp_class_id == class_id):

        # Now update the class_name and class_teacher if provided
        if class_name:
          model.set(iter, COLUMN_NAME, class_name)

        if class_teacher:
          model.set(iter, COLUMN_TEACHER, class_teacher)

        updated = True
        # It's updated now
        break

      iter = model.iter_next(iter)



    # The job not done yet, it's a new class.
    if(not updated):
      new_class = [class_id, class_name, class_teacher]
      self.add_class_in_model(model, new_class)

    # Reload the selected class
    selection = self.treeview_class.get_selection()
    if(selection):
      model, iter = selection.get_selected()
      if iter:
        path = model.get_path(iter)[0]
        sel_class_id = model.get_value(iter, COLUMN_CLASSID)
        self.list_user.reload(sel_class_id)
