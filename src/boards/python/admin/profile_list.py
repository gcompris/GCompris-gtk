#  gcompris - profile_list.py
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

import gnomecanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gtk
import gtk.gdk
import gobject
from gettext import gettext as _

# Database
from pysqlite2 import dbapi2 as sqlite

import profile_group_list
import profile_edit

import constants

# Profile Management
(
  COLUMN_PROFILEID,
  COLUMN_NAME,
  COLUMN_DESCRIPTION
) = range(3)

class Profile_list:
  """GCompris Profile List Table"""


  # area is the drawing area for the list
  def __init__(self, frame, db_connect, db_cursor):

      self.cur = db_cursor
      self.con = db_connect

      # The profile_id to work on
      self.current_profile_id = 0

      # ---------------
      # Profile Management
      # ---------------

      # create tree model
      self.profile_model = self.__create_model_profile()

      # Main box is vertical
      top_box = gtk.VBox(False, 8)
      top_box.show()
      frame.add(top_box)

      # Second line profiles and button
      profile_hbox = gtk.HBox(False, 8)
      profile_hbox.show()
      top_box.add(profile_hbox)

      profilelist_box = gtk.VBox(False, 8)
      profilelist_box.show()
      profile_hbox.add(profilelist_box)

      vbox_button = gtk.VBox(False, 8)
      vbox_button.show()
      profile_hbox.add(vbox_button)


      # Create the table
      sw = gtk.ScrolledWindow()
      sw.show()
      sw.set_shadow_type(gtk.SHADOW_ETCHED_IN)
      sw.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)

      # create tree view
      treeview_profile = gtk.TreeView(self.profile_model)
      treeview_profile.show()
      treeview_profile.set_rules_hint(True)
      treeview_profile.set_search_column(COLUMN_NAME)

      sw.add(treeview_profile)

      profilelist_box.pack_start(sw, True, True, 0)


      # add columns to the tree view
      self.__add_columns_profile(treeview_profile)

      # Add buttons
      button = gtk.Button(stock='gtk-add')
      button.connect("clicked", self.on_add_profile_clicked, self.profile_model)
      vbox_button.pack_start(button, False, False, 0)
      button.show()

      self.button_edit = gtk.Button(stock='gtk-edit')
      self.button_edit.connect("clicked", self.on_edit_profile_clicked, treeview_profile)
      vbox_button.pack_start(self.button_edit, False, False, 0)
      self.button_edit.show()
      self.button_edit.set_sensitive(False)

      self.button_remove = gtk.Button(stock='gtk-remove')
      self.button_remove.connect("clicked", self.on_remove_profile_clicked, treeview_profile)
      vbox_button.pack_start(self.button_remove, False, False, 0)
      self.button_remove.show()
      self.button_remove.set_sensitive(False)

      self.button_default = gtk.Button(_("Default"))
      self.button_default.connect("clicked", self.on_default_profile_clicked, treeview_profile)
      vbox_button.pack_start(self.button_default, False, False, 0)
      self.button_default.show()
      self.button_default.set_sensitive(False)

      # Group list for the profile
      group_hbox = gtk.HBox(False, 8)
      group_hbox.show()
      top_box.add(group_hbox)

      self.profile_group = profile_group_list.Profile_group_list(group_hbox,
                                                        self.con, self.cur,
                                                        self.current_profile_id)

      # Missing callbacks
      selection = treeview_profile.get_selection()
      selection.connect('changed', self.profile_changed_cb, self.profile_group)

      # Load lists
      self.reload_profile()

  # -------------------
  # Profile Management
  # -------------------

  # Update the profile list area
  def reload_profile(self):

    # Remove all entries in the list
    self.profile_model.clear()

    # Get the default profile
    self.cur.execute('select * from informations')
    self.default_profile_id = self.cur.fetchall()[0][2]
    self.default_profile_id_iter = None
    self.default_profile_id_description = None

    # Grab the profile data
    self.cur.execute('SELECT profile_id, name, description FROM profiles ORDER BY name')
    self.profile_data = self.cur.fetchall()

    for aprofile in self.profile_data:
      self.add_profile_in_model(self.profile_model, aprofile)

    self.profile_group.reload(self.current_profile_id)



  # Create the model for the profile list
  def __create_model_profile(self):
    model = gtk.ListStore(
      gobject.TYPE_INT,
      gobject.TYPE_STRING,
      gobject.TYPE_STRING,
      gobject.TYPE_BOOLEAN)

    return model


  def __add_columns_profile(self, treeview):

    model = treeview.get_model()

    # columns for name
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_NAME)
    column = gtk.TreeViewColumn(_('Profile'), renderer,
                                text=COLUMN_NAME)
    column.set_sort_column_id(COLUMN_NAME)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_PROFILENAME)
    treeview.append_column(column)

    # columns for description
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_DESCRIPTION)
    column = gtk.TreeViewColumn(_('Description'), renderer,
                                text=COLUMN_DESCRIPTION)
    column.set_sort_column_id(COLUMN_DESCRIPTION)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_PROFILEDESCRIPTION)
    treeview.append_column(column)


  # Add profile in the model
  def add_profile_in_model(self, model, aprofile):
    iter = model.append()
    model.set (iter,
               COLUMN_PROFILEID,          aprofile[COLUMN_PROFILEID],
               COLUMN_NAME,               aprofile[COLUMN_NAME],
               COLUMN_DESCRIPTION,        aprofile[COLUMN_DESCRIPTION]
               )

    if self.default_profile_id ==  aprofile[COLUMN_PROFILEID]:
      self.set_default_in_description(iter)

  #
  def on_add_profile_clicked(self, button, model):

    profile_id = constants.get_next_profile_id(self.con, self.cur)

    profile_edit.ProfileEdit(self.con, self.cur,
                             profile_id, None, None,
                             self)


  def on_remove_profile_clicked(self, button, treeview):

    selection = treeview.get_selection()
    model, iter = selection.get_selected()

    if iter:
      path = model.get_path(iter)[0]
      profile_id = model.get_value(iter, COLUMN_PROFILEID)

      model.remove(iter)
      # Remove it from the base
      self.cur.execute('DELETE FROM profiles WHERE profile_id=?', (profile_id,))

      # Remove it from list_groups_in_profiles
      self.cur.execute('DELETE FROM list_groups_in_profiles ' +
                       'WHERE profile_id=?',
                       (profile_id,))

      self.con.commit()

      self.profile_group.reload(-1)


  def on_cell_profile_edited(self, cell, path_string, new_text, model):

    iter = model.get_iter_from_string(path_string)
    path = model.get_path(iter)[0]
    column = cell.get_data("column")

    profile_id = model.get_value(iter, COLUMN_PROFILEID)

    if column == COLUMN_NAME:
      model.set(iter, column, new_text)

    elif column == COLUMN_DESCRIPTION:
      model.set(iter, column, new_text)

    profile_data = (profile_id,
                    model.get_value(iter, COLUMN_NAME),
                    model.get_value(iter, COLUMN_DESCRIPTION))
    # Save the changes in the base
    self.cur.execute('insert or replace into profiles (profile_id, name, description) values (?, ?, ?)',
                     profile_data)
    self.con.commit()


  def on_edit_profile_clicked(self, button, treeview):

    selection = treeview.get_selection()
    model, iter = selection.get_selected()

    if iter:
      path = model.get_path(iter)[0]
      profile_id          = model.get_value(iter, COLUMN_PROFILEID)
      profile_name        = model.get_value(iter, COLUMN_NAME)
      profile_description = model.get_value(iter, COLUMN_DESCRIPTION)
      profile_edit.ProfileEdit(self.con, self.cur,
                               profile_id, profile_name, profile_description,
                               self)



  # Set the default profile in the 'informations' table
  def on_default_profile_clicked(self, button, treeview):
    selection = treeview.get_selection()
    model, iter = selection.get_selected()

    if iter:
      path = model.get_path(iter)[0]
      profile_id   = model.get_value(iter, COLUMN_PROFILEID)

      self.default_profile_id = profile_id
      self.button_default.set_sensitive(False)

      # Save the changes in the base
      self.cur.execute('UPDATE informations SET profile_id=?',
                       (profile_id,))
      self.con.commit()

      self.set_default_in_description(iter)

  # Write an additional [Default] in the profile description
  def set_default_in_description(self, iter):
      # set default in description
      description  = self.profile_model.get_value(iter, COLUMN_DESCRIPTION)

      self.profile_model.set (iter,
                              COLUMN_DESCRIPTION,        description + " " + _("[Default]"),
                              )

      # There was a previous default TAG, erase it
      if self.default_profile_id_iter:
        self.profile_model.set (self.default_profile_id_iter,
                                COLUMN_DESCRIPTION, self.default_profile_id_description,
                                )

      # Save the default profile iter to erase it's "default" tag if needed
      self.default_profile_id_iter        = iter
      self.default_profile_id_description = description


  #
  # Called on profile change
  #
  def profile_changed_cb(self, selection, profile_group):
    model, iter = selection.get_selected()

    if iter:
      self.current_profile_id = model.get_value(iter, COLUMN_PROFILEID)

      profile_group.reload(self.current_profile_id)

      self.button_edit.set_sensitive(True)
      self.button_remove.set_sensitive(True)

      # Set the default button on if needed
      if(self.default_profile_id == self.current_profile_id):
        self.button_default.set_sensitive(False)
      else:
        self.button_default.set_sensitive(True)

      if(self.current_profile_id == 1):
         self.button_remove.set_sensitive(False)
      else:
         self.button_remove.set_sensitive(True)

