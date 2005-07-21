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

import profile_group_list
import profile_edit

import constants

# Profile Management
(
  COLUMN_PROFILEID,
  COLUMN_NAME,
  COLUMN_DESCRIPTION,
  COLUMN_PROFILE_EDITABLE
) = range(4)

class Profile_list:
  """GCompris Profile List Table"""


  # area is the drawing area for the list
  def __init__(self, canvas, db_connect, db_cursor, area, hgap, vgap):

      self.rootitem = canvas
      self.cur = db_cursor
      self.con = db_connect

      # The profile_id to work on
      self.current_profile_id = 0

      # Get the default profile
      self.cur.execute('select * from informations')
      self.default_profile_id = self.cur.fetchall()[0][2]
      self.default_profile_id_iter = None
      self.default_profile_id_description = None
      
      # ---------------
      # Profile Management
      # ---------------

      frame = gtk.Frame(_("Profile"))
      frame.show()

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
      
      button = gtk.Button(stock='gtk-edit')
      button.connect("clicked", self.on_edit_profile_clicked, treeview_profile)
      vbox_button.pack_start(button, False, False, 0)
      button.show()

      button = gtk.Button(stock='gtk-remove')
      button.connect("clicked", self.on_remove_profile_clicked, treeview_profile)
      vbox_button.pack_start(button, False, False, 0)
      button.show()

      button = gtk.Button(_("Default"))
      button.connect("clicked", self.on_default_profile_clicked, treeview_profile)
      vbox_button.pack_start(button, False, False, 0)
      button.show()

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
      self.reload_profile()

  # -------------------
  # Profile Management
  # -------------------

  # Update the profile list area
  def reload_profile(self):

    # Remove all entries in the list
    self.profile_model.clear()
    
    # Grab the profile data
    self.cur.execute('select profile_id, name, description from profiles')
    self.profile_data = self.cur.fetchall()

    for aprofile in self.profile_data:
      self.add_profile_in_model(self.profile_model, aprofile)


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
    renderer.connect("edited", self.on_cell_profile_edited, model)
    renderer.set_data("column", COLUMN_NAME)
    column = gtk.TreeViewColumn(_('Profile'), renderer,
                                text=COLUMN_NAME,
                                editable=COLUMN_PROFILE_EDITABLE)
    column.set_sort_column_id(COLUMN_NAME)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_PROFILENAME)
    treeview.append_column(column)

    # columns for description
    renderer = gtk.CellRendererText()
    renderer.connect("edited", self.on_cell_profile_edited, model)
    renderer.set_data("column", COLUMN_DESCRIPTION)
    column = gtk.TreeViewColumn(_('Description'), renderer,
                                text=COLUMN_DESCRIPTION,
                                editable=COLUMN_PROFILE_EDITABLE)
    column.set_sort_column_id(COLUMN_DESCRIPTION)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_PROFILEDESCRIPTION)
    treeview.append_column(column)


  # Return the next profile id
  def get_next_profile_id(self):
    self.cur.execute('select max(profile_id) from profiles')
    profile_id = self.cur.fetchone()[0]
    if(profile_id == None):
      profile_id=0
    else:
      profile_id += 1
      
    return profile_id


  # Add profile in the model
  def add_profile_in_model(self, model, aprofile):
    iter = model.append()
    model.set (iter,
               COLUMN_PROFILEID,          aprofile[COLUMN_PROFILEID],
               COLUMN_NAME,               aprofile[COLUMN_NAME],
               COLUMN_DESCRIPTION,        aprofile[COLUMN_DESCRIPTION],
               COLUMN_PROFILE_EDITABLE,   True
               )
    
    if self.default_profile_id ==  aprofile[COLUMN_PROFILEID]:
      self.set_default_in_description(iter)
    
  #
  def on_add_profile_clicked(self, button, model):
    profile_id = self.get_next_profile_id()

    new_profile = [profile_id, "?", "?", 0]
    self.add_profile_in_model(model, new_profile)


  def on_remove_profile_clicked(self, button, treeview):

    selection = treeview.get_selection()
    model, iter = selection.get_selected()

    if iter:
      path = model.get_path(iter)[0]
      profile_id = model.get_value(iter, COLUMN_PROFILEID)

      if(profile_id == 1):
          # Tell the user it' not possible to remove default profile
          dialog = gtk.MessageDialog(None,
                                     gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                                     gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                                     _("You cannot remove the default profile"))
          dialog.run()
          dialog.destroy()
          return

      model.remove(iter)
      # Remove it from the base
      print "Deleting profile_id=" + str(profile_id)
      self.cur.execute('delete from profiles where profile_id=?', (profile_id,))
      self.con.commit()


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
      profile_id   = model.get_value(iter, COLUMN_PROFILEID)
      profile_name = model.get_value(iter, COLUMN_NAME)
      profile_edit.ProfileEdit(self.con, self.cur,
                               profile_id, profile_name,
                               self.profile_group)

    else:
      # Tell the user to select a profile first
      dialog = gtk.MessageDialog(None,
                                 gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                                 gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                                 _("You must first select a profile in the list"))
      dialog.run()
      dialog.destroy()



  # Set the default profile in the 'informations' table
  def on_default_profile_clicked(self, button, treeview):
    selection = treeview.get_selection()
    model, iter = selection.get_selected()

    if iter:
      path = model.get_path(iter)[0]
      profile_id   = model.get_value(iter, COLUMN_PROFILEID)

      # Get the old data (informations is a one line table)
      self.cur.execute('select * from informations')
      info_data = self.cur.fetchall()[0]

      # Save the changes in the base
      self.cur.execute('insert or replace into informations (gcompris_version, init_date, profile_id) values (?, ?, ?)',
                       (info_data[0], info_data[1], profile_id))
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
      path = model.get_path(iter)[0]
      self.current_profile_id = model.get_value(iter, COLUMN_PROFILEID)

      profile_group.reload(self.current_profile_id)

