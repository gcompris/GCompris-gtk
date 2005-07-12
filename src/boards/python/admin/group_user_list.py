#  gcompris - group_user_list.py
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

#import group_edit

# User Management
(
  COLUMN_USERID,
  COLUMN_LOGIN,
  COLUMN_FIRSTNAME,
  COLUMN_LASTNAME,
  COLUMN_BIRTHDATE,
  COLUMN_USER_EDITABLE
) = range(6)


class Group_user_list:
  """GCompris Group User List Table"""


  # area is the drawing area for the list
  #
  # return the list item
  #
  def __init__(self, container, db_connect, db_cursor, group_id):

      self.cur = db_cursor
      self.con = db_connect

      # The group_id to work on
      self.group_id = group_id
      
      # ---------------
      # User Group Management
      # ---------------

      # create tree model
      self.model = self.__create_model()
      
      self.reload(self.group_id)
      
      # Create the table
      sw = gtk.ScrolledWindow()
      sw.show()
      sw.set_shadow_type(gtk.SHADOW_ETCHED_IN)
      sw.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)

      # create tree view
      treeview_group = gtk.TreeView(self.model)
      treeview_group.show()
      treeview_group.set_rules_hint(True)
      treeview_group.set_search_column(COLUMN_FIRSTNAME)

      sw.add(treeview_group)

      # add columns to the tree view
      self.__add_columns(treeview_group)

      container.pack_start(sw)


  # -------------------
  # User Management
  # -------------------

  # Retrieve data from the database for the given group_id
  def reload(self, group_id):
      print "Reloading group_user_list for group_id=" + str(group_id)
      self.group_id = group_id
      
      # Remove all entries in the list
      self.model.clear()

      self.cur.execute('select user_id from list_users_in_groups where group_id=?', (self.group_id,))
      list_user_id = self.cur.fetchall()

      # Now retrieve users detail
      print list_user_id
      for user_id in list_user_id:
        self.cur.execute('select user_id,login,firstname,lastname,birthdate from users where user_id=?',
                         user_id)
        user = self.cur.fetchall()[0]
        self.add_user_in_model(self.model, user)

    

  # Add user in the model
  def add_user_in_model(self, model, user):
    iter = model.append()
    model.set (iter,
               COLUMN_USERID,    user[COLUMN_USERID],
               COLUMN_LOGIN,     user[COLUMN_LOGIN],
               COLUMN_FIRSTNAME, user[COLUMN_FIRSTNAME],
               COLUMN_LASTNAME,  user[COLUMN_LASTNAME],
               COLUMN_BIRTHDATE, user[COLUMN_BIRTHDATE],
               COLUMN_USER_EDITABLE,  True
               )

    

  def __create_model(self):
    model = gtk.ListStore(
      gobject.TYPE_INT,
      gobject.TYPE_STRING,
      gobject.TYPE_STRING,
      gobject.TYPE_STRING,
      gobject.TYPE_STRING,
      gobject.TYPE_BOOLEAN)

    return model


  def __add_columns(self, treeview):
    
    model = treeview.get_model()

    # columns for login
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_LOGIN)
    column = gtk.TreeViewColumn(_('Login'), renderer,
                                text=COLUMN_LOGIN,
                                editable=COLUMN_USER_EDITABLE)
    column.set_sort_column_id(COLUMN_LOGIN)
    treeview.append_column(column)

    # columns for first name
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_FIRSTNAME)
    column = gtk.TreeViewColumn(_('First Name'), renderer,
                                text=COLUMN_FIRSTNAME,
                                editable=COLUMN_USER_EDITABLE)
    column.set_sort_column_id(COLUMN_FIRSTNAME)
    treeview.append_column(column)

    # column for last name
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_LASTNAME)
    column = gtk.TreeViewColumn(_('Last Name'), renderer,
                                text=COLUMN_LASTNAME,
                                editable=COLUMN_USER_EDITABLE)
    column.set_sort_column_id(COLUMN_LASTNAME)
    treeview.append_column(column)

    # column for birth date
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_BIRTHDATE)
    column = gtk.TreeViewColumn(_('Birth Date'), renderer,
                                text=COLUMN_BIRTHDATE,
                                editable=COLUMN_USER_EDITABLE)
    column.set_sort_column_id(COLUMN_BIRTHDATE)
    treeview.append_column(column)




