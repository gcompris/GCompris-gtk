#  gcompris - board_list.py
# 
# Copyright (C) 2005 Yves Combe
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
import gcompris.admin
import gtk
import gtk.gdk
import gobject
from gettext import gettext as _

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
  def __init__(self, db_connect, db_cursor, frame):

      self.frame = frame
      self.cur = db_cursor
      self.con = db_connect
      
      # ---------------
      # Boards Management
      # ---------------

      # Grab the user data
      self.cur.execute('select * from boards')
      self.board_data = self.cur.fetchall()


      # Main box is vertical
      top_box = gtk.VBox(False, 8)
      top_box.show()
      self.frame.add(top_box)
      
      # Create the profiles Combo
      self.profiles_list = gcompris.admin.get_profiles_list()
      print self.profiles_list

      box1 = gtk.HBox(False, 0)
      box1.show()

      box2 = gtk.HBox(False, 0)
      box2.show()

      box3 = gtk.VBox(False, 0)
      box3.show()
      
      top_box.pack_start(box1, False, False, 0)
      top_box.pack_start(box2, True, True, 0)

      box2.pack_end(box3, False, False, 0)

      label = gtk.Label(_('Select a profile:'))
      label.show()
      box1.pack_start(label,False, False, 0)

      combobox = gtk.combo_box_new_text()
      combobox.show()
      box1.pack_start(combobox, False, False, 0)
      for profile in self.profiles_list:
        combobox.append_text(profile.name)
      combobox.connect('changed', self.changed_cb)
      combobox.set_active(0)
      self.active_profile = self.profiles_list[0]
      print 'Active profile is now', self.active_profile.name
      
      # Create the table
      sw = gtk.ScrolledWindow()
      sw.set_shadow_type(gtk.SHADOW_ETCHED_IN)
#      sw.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
      sw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)

      # create tree model
      model = self.__create_model()

      # create tree view
      treeview = gtk.TreeView(model)
      treeview.set_rules_hint(True)
      treeview.set_search_column(COLUMN_BOARDNAME)

      sw.add(treeview)

      # Some constants for the layout
      #but_height = hgap * 2
      #but_width  = hgap * 6

      box2.pack_start(sw, True, True, 0)

      sw.show()
      treeview.show()

      # add columns to the tree view
      self.__add_columns(treeview)

      # Add buttons

      button_configure = gtk.Button(stock=gtk.STOCK_PROPERTIES)
#       button_add.connect("clicked", self.on_add_item_clicked, model)
      button_configure.show()
      box3.pack_start(button_configure, True, False, 0)
      
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


  def changed_cb(self, combobox):
    index = combobox.get_active()
    self.active_profile = self.profiles_list[index]

    print 'Active profile is now', self.active_profile.name




  # -------------------
  # Board Management
  # -------------------

  # Add boards in the model
  def get_board_by_name(self, name, list):
    for board in list:
      if (board.section + '/' + board.name) == name:
        return board
    return None
   
  def get_board_from_menu(self, menu, list):
    if menu.name =='':
      section = ''
    else:
      section = menu.section + '/' + menu.name
    return_list = []
    for board in list:
      if board.section == section:
        return_list.append([section, board])
        
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


    row_dict = {}
    self.board_dict = {}
    height = 24
    for board_cell in menu_list:
      self.board_dict['%s/%s' % (board_cell[1].section,board_cell[1].name)] = board_cell[1]
      if  board_cell[0] == None:
        row_dict[''] = model.append(None, [self.pixbuf_at_height('gcompris/misc/tuxplane.png', height), _('Main menu') + '\n' + _('/'), True, '%s/%s' % (board_cell[1].section,board_cell[1].name)])
      else:
        row_dict['%s/%s' % (board_cell[1].section,board_cell[1].name)] = model.append(row_dict[board_cell[1].section], [self.pixbuf_at_height(board_cell[1].icon_name, height), _(board_cell[1].title) + '\n' + '%s/%s' % (board_cell[1].section,board_cell[1].name), True, '%s/%s' % (board_cell[1].section,board_cell[1].name)])

  def pixbuf_at_height(self,file, height):
    pixbuf = gcompris.utils.load_pixmap(file)
    width = pixbuf.get_width()* height / pixbuf.get_height()
    del pixbuf
    pixbuf = gtk.gdk.pixbuf_new_from_file_at_size(gcompris.DATA_DIR + '/' + file, width, height)
    return pixbuf

  def __create_model(self):
    model = gtk.TreeStore(
      gtk.gdk.Pixbuf,
      gobject.TYPE_STRING,
      gobject.TYPE_BOOLEAN,
      gobject.TYPE_STRING,
      )

    boards_list = gcompris.admin.get_boards_list()
    
    self.add_boards_in_model(model, boards_list)

    return model


  def __add_columns(self, treeview):
    
    model = treeview.get_model()

    # Render for Board name with icon.
    cell_board_icon = gtk.CellRendererPixbuf()
#    cell_board_name = gtk.CellRendererText()
    cell_board_title = gtk.CellRendererText()
    cell_active_board = gtk.CellRendererToggle()
    cell_active_board.set_property('activatable', True)
    cell_active_board.connect( 'toggled', self.board_acive_cb, model )
    
    # columns for Board name
    column0 = gtk.TreeViewColumn(_('active'))
    column1 = gtk.TreeViewColumn(_('Board title'))
    column1.pack_start(cell_board_icon, False)
    column1.pack_start(cell_board_title, True)
#    column2 = gtk.TreeViewColumn(_('Name'))
    column0.pack_start(cell_active_board, False)
    treeview.append_column(column0)
    treeview.append_column(column1)
#    treeview.append_column(column2)
#    column2.pack_start(cell_board_name, True)
    column1.add_attribute(cell_board_icon, 'pixbuf', 0)
    column1.add_attribute(cell_board_title, 'text', 1)
    column0.add_attribute(cell_active_board, 'active', 2)


  def board_acive_cb(self, cell, path, model):
    model[path][2] = not model[path][2]
    print "Toggle '%s' to: %s" % (self.board_dict[model[path][3]].title, model[path][2],)
    return





