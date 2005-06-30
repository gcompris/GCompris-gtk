#  gcompris - class_edit.py
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


import gtk
import gobject
from gettext import gettext as _

# Database
from pysqlite2 import dbapi2 as sqlite

# User Management
(
  COLUMN_USERID,
  COLUMN_FIRSTNAME,
  COLUMN_LASTNAME,
  COLUMN_USER_EDITABLE
) = range(4)


class ClassEdit(gtk.Window):
    counter = 1
    def __init__(self, db_connect, db_cursor, class_id, class_name):
        # Create the toplevel window
        gtk.Window.__init__(self)

        self.cur = db_cursor
        self.con = db_connect

        self.set_title(_("Class Edition"))
        self.set_border_width(8)
        self.set_default_size(320, 200)

        frame = gtk.Frame(_("Editing class: ") + class_name)
        self.add(frame)
        
        vbox = gtk.VBox(False, 8)
        vbox.set_border_width(8)
        frame.add(vbox)

        # Top message gives instructions
        label = gtk.Label('Assign all the users bellonging to this class\n')
        vbox.pack_start(label, False, False, 0)
        vbox.pack_start(gtk.HSeparator(), False, False, 0)

        # Lower area
        hbox = gtk.HBox(False, 8)
        vbox.pack_start(hbox, True, True, 0)

        # Left list
        # ---------

        # Create the table
        sw = gtk.ScrolledWindow()
        sw.set_shadow_type(gtk.SHADOW_ETCHED_IN)
        sw.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)

        # create tree model
        model = self.__create_model()

        # create tree view
        treeview = gtk.TreeView(model)
        treeview.set_rules_hint(True)
        treeview.set_search_column(COLUMN_FIRSTNAME)

        sw.add(treeview)
        
        # add columns to the tree view
        self.__add_columns(treeview)

        hbox.pack_start(sw, True, True, 0)


        # Middle Button
        vbox2 = gtk.VBox(False, 8)
        vbox2.set_border_width(8)
        hbox.pack_start(vbox2, True, True, 0)

        button_add = gtk.Button(_("Add user >"))
        vbox2.pack_start(button_add, False, False, 0)

        # Right List
        
        # Ready GO
        self.show_all()

    def on_interactive_dialog_clicked(self, button):

        dialog = gtk.Dialog("Interactive Dialog", self, 0,
                (gtk.STOCK_OK, gtk.RESPONSE_OK,
                "_Non-stock button", gtk.RESPONSE_CANCEL))

        hbox = gtk.HBox(False, 8)
        hbox.set_border_width(8)
        dialog.vbox.pack_start(hbox, False, False, 0)

        stock = gtk.image_new_from_stock(
                gtk.STOCK_DIALOG_QUESTION,
                gtk.ICON_SIZE_DIALOG)
        hbox.pack_start(stock, False, False, 0)

        table = gtk.Table(2, 2)
        table.set_row_spacings(4)
        table.set_col_spacings(4)
        hbox.pack_start(table, True, True, 0)

        label = gtk.Label("Entry _1")
        label.set_use_underline(True)
        table.attach(label, 0, 1, 0, 1)
        local_entry1 = gtk.Entry()
        local_entry1.set_text(self.entry1.get_text())
        table.attach(local_entry1, 1, 2, 0, 1)
        label.set_mnemonic_widget(local_entry1)

        label = gtk.Label("Entry _2")
        label.set_use_underline(True)
        table.attach(label, 0, 1, 1, 2)
        local_entry2 = gtk.Entry()
        local_entry2.set_text(self.entry2.get_text())
        table.attach(local_entry2, 1, 2, 1, 2)
        label.set_mnemonic_widget(local_entry2)

        dialog.show_all()

        response = dialog.run()

        if response == gtk.RESPONSE_OK:
            self.entry1.set_text(local_entry1.get_text())
            self.entry2.set_text(local_entry2.get_text())

        dialog.destroy()

    # -------------------
    # User Management
    # -------------------

    def __create_model(self):

        # Grab the user data
        self.cur.execute('select user_id,firstname,name from users')
        user_data = self.cur.fetchall()

        model = gtk.ListStore(
            gobject.TYPE_INT,
            gobject.TYPE_STRING,
            gobject.TYPE_STRING,
            gobject.TYPE_BOOLEAN)

        for item in user_data:
            iter = model.append()
            model.set(iter,
                      COLUMN_USERID,    item[COLUMN_USERID],
                      COLUMN_FIRSTNAME, item[COLUMN_FIRSTNAME],
                      COLUMN_LASTNAME,  item[COLUMN_LASTNAME],
                      COLUMN_USER_EDITABLE,  False)
        return model

    def __add_columns(self, treeview):

        model = treeview.get_model()

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



