#  gcompris - log_list.py
#
# Copyright (C) 2007 Bruno Coudoin
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
import gcompris.bonus
import gtk
import gtk.gdk
import gobject
from gettext import gettext as _

# Database
from pysqlite2 import dbapi2 as sqlite

import constants

# Log Management
(
  COLUMN_DATE,
  COLUMN_USER,
  COLUMN_BOARD,
  COLUMN_LEVEL,
  COLUMN_SUBLEVEL,
  COLUMN_DURATION,
  COLUMN_STATUS,
) = range(7)

class Log_list:
  """GCompris Log List Table"""


  # area is the drawing area for the list
  def __init__(self, frame, db_connect, db_cursor):

      self.cur = db_cursor
      self.con = db_connect

      # The class_id to work on
      self.current_class_id = 0
      self.class_list = []

      # ---------------
      # Log Management
      # ---------------

      # create tree model
      self.log_model = gtk.ListStore(
          gobject.TYPE_STRING,
          gobject.TYPE_STRING,
          gobject.TYPE_STRING,
          gobject.TYPE_INT,
          gobject.TYPE_INT,
          gobject.TYPE_INT,
          gobject.TYPE_STRING)

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

      self.cur.execute('SELECT * FROM class WHERE class_id>1 ORDER BY name')
      class_list = self.cur.fetchall()

      self.combo_class = gtk.combo_box_new_text()
      self.combo_class.show()
      for aclass in class_list:
        self.combo_class.append_text(aclass[1])
        # Save in a list the combo index => the class_id
        self.class_list.append(aclass[0])

      self.combo_class.set_active(self.current_class_id)
      label_box.pack_end(self.combo_class, True, True, 0)

      # Second line logs and button
      log_hbox = gtk.HBox(False, 8)
      log_hbox.show()
      top_box.add(log_hbox)

      loglist_box = gtk.VBox(False, 8)
      loglist_box.show()
      log_hbox.add(loglist_box)

      vbox_button = gtk.VBox(False, 8)
      vbox_button.show()
      log_hbox.add(vbox_button)


      # Create the table
      sw = gtk.ScrolledWindow()
      sw.show()
      sw.set_shadow_type(gtk.SHADOW_ETCHED_IN)
      sw.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)

      # create tree view
      treeview_log = gtk.TreeView(self.log_model)
      treeview_log.show()
      treeview_log.set_rules_hint(True)
      treeview_log.set_search_column(COLUMN_DATE)

      sw.add(treeview_log)

      loglist_box.pack_start(sw, True, True, 0)


      # add columns to the tree view
      self.__add_columns_log(treeview_log)

      # Add buttons
      self.button_remove = gtk.Button(stock='gtk-remove')
      self.button_remove.connect("clicked", self.on_remove_log_clicked, treeview_log)
      vbox_button.pack_start(self.button_remove, False, False, 0)
      self.button_remove.show()
      self.button_remove.set_sensitive(True)

      # Add buttons
      self.button_refresh = gtk.Button(stock='gtk-refresh')
      self.button_refresh.connect("clicked", self.on_refresh_log_clicked)
      vbox_button.pack_start(self.button_refresh, False, False, 0)
      self.button_refresh.show()
      self.button_refresh.set_sensitive(True)

      # Load lists
      self.class_changed_cb(self.combo_class)
      self.reload_log()

  # -------------------
  # Log Management
  # -------------------

  # Update the log list area
  def reload_log(self):

    # Remove all entries in the list
    self.log_model.clear()

    # Grab the log data
    self.cur.execute('SELECT date, user_id, board_id, level, sublevel, duration, status FROM logs ORDER BY date')
    self.log_data = self.cur.fetchall()

    for alog in self.log_data:
      self.add_log_in_model(self.log_model, alog)


  def __add_columns_log(self, treeview):

    model = treeview.get_model()

    # columns for date
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_DATE)
    column = gtk.TreeViewColumn(_('Date'), renderer,
                                text=COLUMN_DATE)
    column.set_sort_column_id(COLUMN_DATE)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_DATE)
    treeview.append_column(column)

    # columns for USER
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_USER)
    column = gtk.TreeViewColumn(_('User'), renderer,
                                text=COLUMN_USER)
    column.set_sort_column_id(COLUMN_USER)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_LOGIN)
    treeview.append_column(column)

    # columns for BOARD
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_BOARD)
    column = gtk.TreeViewColumn(_('Board'), renderer,
                                text=COLUMN_BOARD)
    column.set_sort_column_id(COLUMN_BOARD)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_LOGIN)
    treeview.append_column(column)

    # columns for LEVEL
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_LEVEL)
    column = gtk.TreeViewColumn(_('Level'), renderer,
                                text=COLUMN_LEVEL)
    column.set_sort_column_id(COLUMN_LEVEL)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_NUMBER)
    treeview.append_column(column)

    # columns for SUBLEVEL
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_SUBLEVEL)
    column = gtk.TreeViewColumn(_('Sublevel'), renderer,
                                text=COLUMN_SUBLEVEL)
    column.set_sort_column_id(COLUMN_SUBLEVEL)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_NUMBER)
    treeview.append_column(column)

    # columns for duration
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_DURATION)
    column = gtk.TreeViewColumn(_('Duration'), renderer,
                                text=COLUMN_DURATION)
    column.set_sort_column_id(COLUMN_DURATION)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_NUMBER)
    treeview.append_column(column)

    # columns for STATUS
    renderer = gtk.CellRendererText()
    renderer.set_data("column", COLUMN_STATUS)
    column = gtk.TreeViewColumn(_('Status'), renderer,
                                text=COLUMN_STATUS)
    column.set_sort_column_id(COLUMN_STATUS)
    column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
    column.set_fixed_width(constants.COLUMN_WIDTH_LOGIN)
    treeview.append_column(column)

  # Add log in the model
  def add_log_in_model(self, model, alog):
    iter = model.append()

    status = "Lost"
    if  alog[COLUMN_STATUS] == gcompris.bonus.WIN:
        status = "Win"
    elif  alog[COLUMN_STATUS] == gcompris.bonus.DRAW:
        status = "Draw"
    elif  alog[COLUMN_STATUS] == gcompris.bonus.COMPLETED:
        status = "Compl."

    login = _("Default")
    self.cur.execute('SELECT login FROM users WHERE user_id=?', (alog[COLUMN_USER],))
    result = self.cur.fetchall()
    if(result):
        login = result[0][0]

    board = ""
    self.cur.execute('SELECT name FROM boards WHERE board_id=?', (alog[COLUMN_BOARD],))
    result = self.cur.fetchall()
    if(result):
        board = result[0][0]

    model.set (iter,
               COLUMN_DATE,     alog[COLUMN_DATE],
               COLUMN_USER,     login,
               COLUMN_BOARD,    board,
               COLUMN_LEVEL,    alog[COLUMN_LEVEL],
               COLUMN_SUBLEVEL, alog[COLUMN_SUBLEVEL],
               COLUMN_DURATION, alog[COLUMN_DURATION],
               COLUMN_STATUS,   status,
               )


  #
  def on_remove_log_clicked(self, button, treeview):

      # Remove it from the base
      self.cur.execute('delete from logs')
      self.con.commit()

      self.log_model.clear()

  def on_refresh_log_clicked(self, button):

      self.reload_log()

  def class_changed_cb(self, combobox):
    active = combobox.get_active()
    if active >= 0:
        self.current_class_id = self.class_list[active]
        self.reload_log()

