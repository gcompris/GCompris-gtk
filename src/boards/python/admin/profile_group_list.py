#  gcompris - profile_group_list.py
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

import constants

# Database
from pysqlite2 import dbapi2 as sqlite

#import group_edit

# Group Management
(
  COLUMN_GROUPID,
  COLUMN_NAME,
  COLUMN_DESCRIPTION,
  COLUMN_GROUP_EDITABLE
) = range(4)


class Profile_group_list:
  """GCompris Profile Group List Table"""


  # The created list will be packed in the given container
  #
  def __init__(self, container, db_connect, db_cursor, profile_id):

      self.cur = db_cursor
      self.con = db_connect

      # The profile_id to work on
      self.profile_id = profile_id
      
      # ---------------
      # User Group Management
      # ---------------

      # create tree model
      self.model = self.__create_model()
      
      self.reload(self.profile_id)
      
      # Create the table
      sw = gtk.ScrolledWindow()
      sw.show()
      sw.set_shadow_type(gtk.SHADOW_ETCHED_IN)
      sw.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)

      # create tree view
      treeview_group = gtk.TreeView(self.model)
      treeview_group.show()
      treeview_group.set_rules_hint(True)
      treeview_group.set_search_column(COLUMN_NAME)

      sw.add(treeview_group)

      # add columns to the tree view
      self.__add_columns(treeview_group)

      container.pack_start(sw)


  # -------------------
  # Group Management
  # -------------------

  # clear all data in the list
  def clear(self):
      self.model.clear()

  # Retrieve data from the database for the given group_id
  def reload(self, profile_id):
      print "Reloading profile_group_list for profile_id=" + str(profile_id)
      self.profile_id = profile_id
      
      # Remove all entries in the list
      self.model.clear()

      self.cur.execute('select group_id from list_groups_in_profiles where profile_id=?', (self.profile_id,))
      list_group_id = self.cur.fetchall()

      # Now retrieve group detail
      print list_group_id
      for group_id in list_group_id:
        self.cur.execute('select group_id,name,description from groups where group_id=?',
                         group_id)
        user = self.cur.fetchall()[0]
        self.add_group_in_model(self.model, group)

    

  # Add group in the model
  def add_group_in_model(self, model, group):
    iter = model.append()
    model.set (iter,
               COLUMN_GROUPID,     group[COLUMN_GROUPID],
               COLUMN_NAME,        group[COLUMN_NAME],
               COLUMN_DESCRIPTION, group[COLUMN_DESCRIPTION],
               COLUMN_USER_EDITABLE,  True
               )

    

  def __create_model(self):
    model = gtk.ListStore(
      gobject.TYPE_INT,
      gobject.TYPE_STRING,
      gobject.TYPE_STRING,
      gobject.TYPE_BOOLEAN)

    return model


  def __add_columns(self, treeview):
    
    model = treeview.get_model()

    # Total column lengh must be 400
    
    # columns for name
    renderer = gtk.CellRendererText()
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
    renderer.set_data("column", COLUMN_DESCRIPTION)
    column = gtk.TreeViewColumn(_('Description'), renderer,
                                text=COLUMN_DESCRIPTION,
                                editable=COLUMN_GROUP_EDITABLE)
    column.set_sort_column_id(COLUMN_DESCRIPTION)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_GROUPDESCRIPTION)
    treeview.append_column(column)


