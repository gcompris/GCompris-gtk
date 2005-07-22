#  gcompris - group_list.py
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

import group_user_list
import group_edit

import constants

# Group Management
(
  COLUMN_GROUPID,
  COLUMN_NAME,
  COLUMN_DESCRIPTION,
  COLUMN_GROUP_EDITABLE
) = range(4)

class Group_list:
  """GCompris Group List Table"""


  # area is the drawing area for the list
  def __init__(self, canvas, db_connect, db_cursor, area, hgap, vgap):

      self.rootitem = canvas
      self.cur = db_cursor
      self.con = db_connect

      # The class_id to work on
      self.current_class_id = 0
      self.class_list = []

      # The group_id selected
      self.current_group_id = 0
      
      # ---------------
      # Group Management
      # ---------------

      frame = gtk.Frame(_("Group"))
      frame.show()

      # create tree model
      self.group_model = self.__create_model_group()

      # Main box is vertical
      top_box = gtk.VBox(False, 8)
      top_box.show()
      frame.add(top_box)

      # First line label and combo
      label_box = gtk.HBox(False, 8)
      label_box.show()
      top_box.pack_start(label_box, False, False, 0)


      # Let the user select the class to work on
      #
      # Grab the class list and put it in a combo
      class_box = gtk.HBox(False, 8)
      class_box.show()
      label_box.pack_start(class_box, False, False, 0)
      
      class_label = gtk.Label(_('Select a class:'))
      class_label.show()
      label_box.pack_start(class_label, False, False, 0)
      
      self.cur.execute('select * from class where class_id>1')
      class_list = self.cur.fetchall()

      self.combo_class = gtk.combo_box_new_text()
      self.combo_class.show()
      for aclass in class_list:
        self.combo_class.append_text(aclass[1])
        # Save in a list the combo index => the class_id
        self.class_list.append(aclass[0])
        
      self.combo_class.set_active(self.current_class_id)
      label_box.pack_end(self.combo_class, True, True, 0)

      # Second line groups and button
      group_hbox = gtk.HBox(False, 8)
      group_hbox.show()
      top_box.add(group_hbox)

      grouplist_box = gtk.VBox(False, 8)
      grouplist_box.show()
      group_hbox.add(grouplist_box)

      vbox_button = gtk.VBox(False, 8)
      vbox_button.show()
      group_hbox.add(vbox_button)

      
      # Create the table
      sw = gtk.ScrolledWindow()
      sw.show()
      sw.set_shadow_type(gtk.SHADOW_ETCHED_IN)
      sw.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)

      # create tree view
      treeview_group = gtk.TreeView(self.group_model)
      treeview_group.show()
      treeview_group.set_rules_hint(True)
      treeview_group.set_search_column(COLUMN_NAME)

      sw.add(treeview_group)

      grouplist_box.pack_start(sw, True, True, 0)
      

      # add columns to the tree view
      self.__add_columns_group(treeview_group)

      # Add buttons
      button = gtk.Button(stock='gtk-add')
      button.connect("clicked", self.on_add_group_clicked, self.group_model)
      vbox_button.pack_start(button, False, False, 0)
      button.show()
      
      button = gtk.Button(stock='gtk-edit')
      button.connect("clicked", self.on_edit_group_clicked, treeview_group)
      vbox_button.pack_start(button, False, False, 0)
      button.show()

      button = gtk.Button(stock='gtk-remove')
      button.connect("clicked", self.on_remove_group_clicked, treeview_group)
      vbox_button.pack_start(button, False, False, 0)
      button.show()

      # User list for the group
      user_hbox = gtk.HBox(False, 8)
      user_hbox.show()
      top_box.add(user_hbox)

      self.group_user = group_user_list.Group_user_list(user_hbox,
                                                        self.con, self.cur,
                                                        self.current_group_id)

      # Missing callbacks
      self.combo_class.connect('changed', self.class_changed_cb)
      selection = treeview_group.get_selection()
      selection.connect('changed', self.group_changed_cb, self.group_user)

      # Pack it all
      self.rootitem.add(
        gnome.canvas.CanvasWidget,
        widget=frame,
        x=area[0] + hgap,
        y=area[1],
        width=area[2]-area[0] - hgap*2,
        height=area[3]-area[1],
        anchor=gtk.ANCHOR_NW,
        size_pixels=False)

      # Load lists
      self.class_changed_cb(self.combo_class)
      self.reload_group()

  # -------------------
  # Group Management
  # -------------------

  # Update the group list area
  def reload_group(self):

    # Remove all entries in the list
    self.group_model.clear()
    
    # Grab the group data
    self.cur.execute('select group_id, name, description from groups where class_id=?',
                     (self.current_class_id,))
    self.group_data = self.cur.fetchall()

    for agroup in self.group_data:
      self.add_group_in_model(self.group_model, agroup)


  # Create the model for the group list
  def __create_model_group(self):
    model = gtk.ListStore(
      gobject.TYPE_INT,
      gobject.TYPE_STRING,
      gobject.TYPE_STRING,
      gobject.TYPE_BOOLEAN)

    return model


  def __add_columns_group(self, treeview):
    
    model = treeview.get_model()

    # columns for name
    renderer = gtk.CellRendererText()
    renderer.connect("edited", self.on_cell_group_edited, model)
    renderer.set_data("column", COLUMN_NAME)
    column = gtk.TreeViewColumn(_('Group'), renderer,
                                text=COLUMN_NAME,
                                editable=COLUMN_GROUP_EDITABLE)
    column.set_sort_column_id(COLUMN_NAME)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_GROUPNAME)
    treeview.append_column(column)

    # columns for description
    renderer = gtk.CellRendererText()
    renderer.connect("edited", self.on_cell_group_edited, model)
    renderer.set_data("column", COLUMN_DESCRIPTION)
    column = gtk.TreeViewColumn(_('Description'), renderer,
                                text=COLUMN_DESCRIPTION,
                                editable=COLUMN_GROUP_EDITABLE)
    column.set_sort_column_id(COLUMN_DESCRIPTION)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_GROUPDESCRIPTION)
    treeview.append_column(column)


  # Return the next group id
  def get_next_group_id(self):
    self.cur.execute('select max(group_id) from groups')
    group_id = self.cur.fetchone()[0]
    if(group_id == None):
      group_id=0
    else:
      group_id += 1
      
    return group_id


  # Add group in the model
  def add_group_in_model(self, model, agroup):
    iter = model.append()
    model.set (iter,
               COLUMN_GROUPID,          agroup[COLUMN_GROUPID],
               COLUMN_NAME,             agroup[COLUMN_NAME],
               COLUMN_DESCRIPTION,      agroup[COLUMN_DESCRIPTION],
               COLUMN_GROUP_EDITABLE,   True
               )

    
  #
  def on_add_group_clicked(self, button, model):
    group_id = self.get_next_group_id()

    new_group = [group_id, "?", "?", 0]
    self.add_group_in_model(model, new_group)


  def on_remove_group_clicked(self, button, treeview):

    selection = treeview.get_selection()
    model, iter = selection.get_selected()

    if iter:
      path = model.get_path(iter)[0]
      group_id = model.get_value(iter, COLUMN_GROUPID)
      model.remove(iter)
      # Remove it from the base
      print "Deleting group_id=" + str(group_id)
      self.cur.execute('delete from groups where group_id=?', (group_id,))
      self.con.commit()


  def on_cell_group_edited(self, cell, path_string, new_text, model):

    iter = model.get_iter_from_string(path_string)
    path = model.get_path(iter)[0]
    column = cell.get_data("column")

    group_id = model.get_value(iter, COLUMN_GROUPID)
    
    if column == COLUMN_NAME:
      model.set(iter, column, new_text)

    elif column == COLUMN_DESCRIPTION:
      model.set(iter, column, new_text)

    group_data = (group_id,
                  self.current_class_id,
                  model.get_value(iter, COLUMN_NAME),
                  model.get_value(iter, COLUMN_DESCRIPTION))
    # Save the changes in the base
    self.cur.execute('insert or replace into groups (group_id, class_id, name, description) values (?, ?, ?, ?)',
                     group_data)
    self.con.commit()


  # Return the selected text in the given combobox
  def get_active_text(self, combobox):
      model = combobox.get_model()
      active = combobox.get_active()
      if active < 0:
        return None
      return model[active][0]

  def on_edit_group_clicked(self, button, treeview):

    selection = treeview.get_selection()
    model, iter = selection.get_selected()

    if iter:
      path = model.get_path(iter)[0]
      group_id   = model.get_value(iter, COLUMN_GROUPID)
      group_name = model.get_value(iter, COLUMN_NAME)
      group_edit.GroupEdit(self.con, self.cur,
                           self.current_class_id, self.get_active_text(self.combo_class),
                           group_id, group_name,
                           self.group_user)

    else:
      # Tell the user to select a group first
      dialog = gtk.MessageDialog(None,
                                 gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                                 gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                                 _("You must first select a group in the list"))
      dialog.run()
      dialog.destroy()


  def group_changed_cb(self, selection, group_user):
    print "group_changed_cb"
    model, iter = selection.get_selected()

    if iter:
      path = model.get_path(iter)[0]
      self.current_group_id = model.get_value(iter, COLUMN_GROUPID)

      group_user.reload(self.current_group_id)

  def class_changed_cb(self, combobox):
    self.current_class_id = self.class_list[combobox.get_active()]
    self.reload_group()
