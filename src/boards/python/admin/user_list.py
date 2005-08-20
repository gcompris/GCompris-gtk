#  gcompris - user_list.py
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
import gcompris
import gcompris.utils
import gcompris.skin
import gtk
import gtk.gdk
import gobject
from gettext import gettext as _

# Database
from pysqlite2 import dbapi2 as sqlite

import constants
import user_edit

# User Management
(
  COLUMN_USERID,
  COLUMN_LOGIN,
  COLUMN_FIRSTNAME,
  COLUMN_LASTNAME,
  COLUMN_BIRTHDATE,
) = range(5)

class User_list:
  """GCompris Users List Table"""


  # The created list will be packed in the given container
  #
  # Display the users for a given class_id in a table
  # The class_id to display must be passed to the reload function
  def __init__(self, container, db_connect, db_cursor):

      self.cur = db_cursor
      self.con = db_connect

      # ---------------
      # User Management
      # ---------------

      # The class to work on
      self.class_id = 1

      self.user_data = []

      # create tree model
      self.model = self.__create_model()

      # First line
      group_hbox = gtk.HBox(False, 8)
      group_hbox.show()
      container.add(group_hbox)

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
      treeview = gtk.TreeView(self.model)
      treeview.show()
      treeview.set_rules_hint(True)
      treeview.set_search_column(COLUMN_FIRSTNAME)
      treeview.get_selection().set_mode(gtk.SELECTION_MULTIPLE)

      sw.add(treeview)

      left_box.pack_start(sw, True, True, 0)

      # add columns to the tree view
      self.__add_columns(treeview)

      # Add buttons
      button = gtk.Button(stock='gtk-add')
      button.connect("clicked", self.on_add_item_clicked, self.model)
      right_box.pack_start(button, False, False, 0)
      button.show()

      self.button_edit = gtk.Button(stock='gtk-edit')
      self.button_edit.connect("clicked", self.on_edit_clicked, treeview)
      right_box.pack_start(self.button_edit, False, False, 0)
      self.button_edit.show()
      # Not editable until one class is selected
      self.button_edit.set_sensitive(False)

      button = gtk.Button(stock='gtk-open')
      button.connect("clicked", self.on_import_cvs_clicked, treeview)
      right_box.pack_start(button, False, False, 0)
      button.show()

      self.button_remove = gtk.Button(stock='gtk-remove')
      self.button_remove.connect("clicked", self.on_remove_item_clicked, treeview)
      right_box.pack_start(self.button_remove, False, False, 0)
      self.button_remove.show()
      
      # Missing callbacks
      selection = treeview.get_selection()
      selection.connect('changed', self.user_changed_cb, treeview)
      
  # -------------------
  # User Management
  # -------------------

  # Retrieve data from the database for the given class_id
  def reload(self, class_id):
    self.class_id = class_id
      
    # Remove all entries in the list
    self.model.clear()

    self.button_edit.set_sensitive(False)
    self.button_remove.set_sensitive(False)

    # Grab the user data
    self.cur.execute('select user_id,login,firstname,lastname,birthdate from users where class_id=?',
                     (class_id,))
    self.user_data = self.cur.fetchall()

    for user in self.user_data:
      self.add_user_in_model(self.model, user)

  # Add user in the model
  def add_user_in_model(self, model, user):
    iter = model.append()
    model.set (iter,
               COLUMN_USERID,    user[COLUMN_USERID],
               COLUMN_LOGIN,     user[COLUMN_LOGIN],
               COLUMN_FIRSTNAME, user[COLUMN_FIRSTNAME],
               COLUMN_LASTNAME,  user[COLUMN_LASTNAME],
               COLUMN_BIRTHDATE, user[COLUMN_BIRTHDATE]
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

    # Total column length must be 400
    
    model = treeview.get_model()

    # columns for login
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_LOGIN)
    column = gtk.TreeViewColumn(_('Login'), renderer,
                                text=COLUMN_LOGIN)
    column.set_sort_column_id(COLUMN_LOGIN)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_LOGIN)
    treeview.append_column(column)

    # columns for first name
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_FIRSTNAME)
    column = gtk.TreeViewColumn(_('First Name'), renderer,
                                text=COLUMN_FIRSTNAME)
    column.set_sort_column_id(COLUMN_FIRSTNAME)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED) 
    column.set_fixed_width(constants.COLUMN_WIDTH_FIRSTNAME)
    treeview.append_column(column)

    # column for last name
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_LASTNAME)
    column = gtk.TreeViewColumn(_('Last Name'), renderer,
                                text=COLUMN_LASTNAME)
    column.set_sort_column_id(COLUMN_LASTNAME)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_LASTNAME)
    treeview.append_column(column)

    # column for birth date
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_BIRTHDATE)
    column = gtk.TreeViewColumn(_('Birth Date'), renderer,
                                text=COLUMN_BIRTHDATE)
    column.set_sort_column_id(COLUMN_BIRTHDATE)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_BIRTHDATE)
    treeview.append_column(column)


  # Return the next user id
  def get_next_user_id(self):
    self.cur.execute('select max(user_id) from users')
    user_id = self.cur.fetchone()[0]
    if(user_id == None):
      user_id=0
    else:
      user_id += 1
      
    return user_id


  # Edition of a user
  # It create a dialog box
  # If multiselection is on, it will create a dialog per selection
  def on_edit_clicked(self, button, treeview):
    model = treeview.get_model()
    treestore, paths = treeview.get_selection().get_selected_rows()
    paths.reverse()
        
    for path in paths:
      iter = treestore.get_iter(path)
      path = model.get_path(iter)[0]

      user_id       = model.get_value(iter, COLUMN_USERID)
      login         = model.get_value(iter, COLUMN_LOGIN)
      firstname     = model.get_value(iter, COLUMN_FIRSTNAME)
      lastname      = model.get_value(iter, COLUMN_LASTNAME)
      birthdate     = model.get_value(iter, COLUMN_BIRTHDATE)
      user_edit.UserEdit(self.con, self.cur,
                         user_id, login, firstname, lastname, birthdate, self.class_id,
                         self)


  # Create a new user
  def on_add_item_clicked(self, button, model):
    user_id = self.get_next_user_id()

    user_edit.UserEdit(self.con, self.cur,
                       user_id, "", "", "", "", self.class_id,
                       self)



  def on_remove_item_clicked(self, button, treeview):
    model = treeview.get_model()
    treestore, paths = treeview.get_selection().get_selected_rows()
    paths.reverse()
        
    for path in paths:
      iter = treestore.get_iter(path)
      path = model.get_path(iter)[0]

      user_id = model.get_value(iter, COLUMN_USERID)
      model.remove(iter)
      # Remove it from the base
      print "Deleting user_id=" + str(user_id)
      self.cur.execute('delete from users where user_id=?', (user_id,))
      self.con.commit()


  def on_import_cvs_clicked(self, button, treeview):

    # Tell the user to select a class first
    dialog = gtk.MessageDialog(None,
                               gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                               gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                               _("To import a user list from file, first select a class.\nFILE FORMAT: Your file must be formated like this:\nlogin;First name;Last name;Birth date\nThe separator is autodetected and can be one of ',', ';' or ':'"))
    dialog.run()
    dialog.destroy()
    
    model = treeview.get_model()
    
    dialog = gtk.FileChooserDialog("Open..",
                                   None,
                                   gtk.FILE_CHOOSER_ACTION_OPEN,
                                   (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                                    gtk.STOCK_OPEN, gtk.RESPONSE_OK))
    dialog.set_default_response(gtk.RESPONSE_OK)
    
    filter = gtk.FileFilter()
    filter.set_name("All files")
    filter.add_pattern("*")
    dialog.add_filter(filter)

    response = dialog.run()
    if response == gtk.RESPONSE_OK:
      filename = dialog.get_filename()
    elif response == gtk.RESPONSE_CANCEL:
      dialog.destroy()
      return
    
    dialog.destroy()

    # Parse the file and include each line in the user table
    file = open(filename, 'r')

    # Determine the separator (the most used in the set ,;:)
    # Warning, the input file must use the same separator on each line and
    # between each fields
    line = file.readline()
    file.seek(0)
    sep=''
    count=0
    for asep in [',', ';', ':']:
      c = line.count(asep)
      if(c>count):
        count=c
        sep=asep

    for line in file.readlines():
      print line
      line = line.rstrip("\n\r")
      user_id = self.get_next_user_id()
      login, firstname, lastname, birthdate = line.split(sep)
      # Save the changes in the base
      new_user = [user_id, login, firstname, lastname, birthdate, self.class_id]
      self.add_user_in_model(model, new_user)
      self.cur.execute('insert or replace into users (user_id, login, firstname, lastname, birthdate, class_id) values (?, ?, ?, ?, ?, ?)', new_user)
      self.con.commit()

    file.close()


  # The user is changed ...
  def user_changed_cb(self, selection, treeview):
    self.button_edit.set_sensitive(True)
    self.button_remove.set_sensitive(True)

