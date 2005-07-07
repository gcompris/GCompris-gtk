#  gcompris - user_list.py
# 
# Time-stamp: <2001/08/20 00:54:45 bruno>
# 
# Copyright (C) 2005 Bruno Coudoin
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

print "gtk.STOCK_OPEN", type(gtk.STOCK_OPEN), gtk.STOCK_OPEN

# Database
#from pysqlite2 import dbapi2 as sqlite

# Board Management
(
   COLUMN_BOARDICON,
   COLUMN_BOARDNAME
) = range(2)

class Board_list:
  """GCompris Boards List Table"""


  # area is the drawing area for the list
  def __init__(self, canvas, db_connect, db_cursor, area, hgap, vgap):

      self.rootitem = canvas
      self.cur = db_cursor
      self.con = db_connect
      
      # ---------------
      # Boards Management
      # ---------------

      # Grab the user data
      self.cur.execute('select * from boards')
      self.board_data = self.cur.fetchall()

      # Create the table
      sw = gtk.ScrolledWindow()
      sw.set_shadow_type(gtk.SHADOW_ETCHED_IN)
      sw.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)

      # create tree model
      model = self.__create_model()

      # create tree view
      treeview = gtk.TreeView(model)
      treeview.set_rules_hint(True)
      treeview.set_search_column(COLUMN_BOARDNAME)

      sw.add(treeview)

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
        size_pixels=gtk.FALSE)
      sw.show()
      treeview.show()

      # add columns to the tree view
      self.__add_columns(treeview)

      # Add buttons

#       button_add = gtk.Button(_("Add user"))
#       button_add.connect("clicked", self.on_add_item_clicked, model)
#       self.rootitem.add(
#         gnome.canvas.CanvasWidget,
#         widget=button_add,
#         x=area[2]-but_width,
#         y=area[1],
#         width=100,
#         height=but_height,
#         anchor=gtk.ANCHOR_NW,
#         size_pixels=gtk.FALSE)
#       button_add.show()
      
#       button_rem = gtk.Button(_("Remove user"))
#       button_rem.connect("clicked", self.on_remove_item_clicked, treeview)
#       self.rootitem.add(
#         gnome.canvas.CanvasWidget,
#         widget=button_rem,
#         x=area[2]-but_width,
#         y=area[1] + but_height + vgap,
#         width=100,
#         height=but_height,
#         anchor=gtk.ANCHOR_NW,
#         size_pixels=gtk.FALSE)
#       button_rem.show()

#       button_imp = gtk.Button(_("Import file"))
#       button_imp.connect("clicked", self.on_import_cvs_clicked, treeview)
#       self.rootitem.add(
#         gnome.canvas.CanvasWidget,
#         widget=button_imp,
#         x=area[2]-but_width,
#         y=area[1] + (but_height + vgap) * 2,
#         width=100,
#         height=but_height,
#         anchor=gtk.ANCHOR_NW,
#         size_pixels=gtk.FALSE)
#       button_imp.show()


  # -------------------
  # Board Management
  # -------------------

  # Add boards in the model
  def get_board_by_name(self, name, list):
    for board in list:
      if (board.section + '/' + board.name) == name:
        return board
    print "get_board_by_name: ", name, " return ", None 
    return None
   
  def get_board_from_menu(self, menu, list):
    if menu.name =='':
      section = ''
    else:
      section = menu.section + '/' + menu.name
    print "get_board_from_menu : ", section
    return_list = []
    for board in list:
      if board.section == section:
        return_list.append([section, board])
        
    print "get_board_from_menu", return_list
    return return_list
  
  def add_boards_in_model(self, model, boards_list):
    root_menu = '/'
    root = self.get_board_by_name(root_menu, boards_list)
    boards_list.remove(root)
    menu_list = [[ None,  root ]]

    list = menu_list[:]
    while len(list) > 0:
      menu = list.pop(0)[1]
      list_board = self.get_board_from_menu(menu, boards_list)
      menu_list = menu_list + list_board
    
      for board_cell in list_board:
        if board_cell[1].type == 'menu':
          list.append(board_cell)

      print "add_boards_in_model", len(list)
      print "add_boards_in_model", len(menu_list)

    row_dict = {}
    height = 24
    for board_cell in menu_list:
      print board_cell[0], '%s/%s' % (board_cell[1].section,board_cell[1].name)
      if  board_cell[0] == None:
        row_dict[''] = model.append(None, [self.pixbuf_at_height('gcompris/misc/tuxplane.png', height), 'Main Menu'])
      else:
        row_dict['%s/%s' % (board_cell[1].section,board_cell[1].name)] = model.append(row_dict[board_cell[1].section], [self.pixbuf_at_height(board_cell[1].icon_name, height), '%s/%s' % (board_cell[1].section,board_cell[1].name)])

  def pixbuf_at_height(self,file, height):
    pixbuf = gcompris.utils.load_pixmap(file)
    width = pixbuf.get_width()* height / pixbuf.get_height()
    del pixbuf
    pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(gcompris.DATA_DIR + '/' + file, width, height)
    return pixbuf

  def __create_model(self):
    model = gtk.TreeStore(
      gtk.gdk.Pixbuf,
      gobject.TYPE_STRING
      )

    boards_list = gcompris.get_boards_list()
    self.add_boards_in_model(model, boards_list)

    return model


  def __add_columns(self, treeview):
    
    model = treeview.get_model()

    # Render for Board name with icon.
    cell_board_icon = gtk.CellRendererPixbuf()
    cell_board_name = gtk.CellRendererText()

    # columns for Board name
    column = gtk.TreeViewColumn(_('Board name'))
    column.pack_start(cell_board_icon, False)
    column.pack_start(cell_board_name, True)
    treeview.append_column(column)
    column.add_attribute(cell_board_icon, 'pixbuf', 0)
    column.add_attribute(cell_board_name, 'text', 1)








