#  gcompris - class_edit.py
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


import gtk
import gobject
from gettext import gettext as _

# Database
from pysqlite2 import dbapi2 as sqlite

# User List Management
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

        self.class_id = class_id
        
        self.set_title(_("Class Edition"))
        self.set_border_width(8)
        self.set_default_size(320, 350)

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
        self.model_left = self.__create_model(False, class_id)

        # create tree view
        treeview = gtk.TreeView(self.model_left)
        treeview.set_rules_hint(True)
        treeview.set_search_column(COLUMN_FIRSTNAME)

        sw.add(treeview)
        
        # add columns to the tree view
        self.__add_columns(treeview)

        hbox.pack_start(sw, True, True, 0)


        # Middle Button
        # -------------
        vbox2 = gtk.VBox(False, 8)
        vbox2.set_border_width(8)
        hbox.pack_start(vbox2, True, True, 0)

        button_add = gtk.Button(_("Add user >"))
        vbox2.pack_start(button_add, False, False, 0)
        button_add.connect("clicked", self.add_user, treeview)

        # Right List
        # ----------

        # Create the table
        sw2 = gtk.ScrolledWindow()
        sw2.set_shadow_type(gtk.SHADOW_ETCHED_IN)
        sw2.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)

        # create tree model
        self.model_right = self.__create_model(True, class_id)

        # create tree view
        treeview2 = gtk.TreeView(self.model_right)
        treeview2.set_rules_hint(True)
        treeview2.set_search_column(COLUMN_FIRSTNAME)

        sw2.add(treeview2)
        
        # add columns to the tree view
        self.__add_columns(treeview2)

        hbox.pack_start(sw2, True, True, 0)

        # Confirmation Buttons
        # --------------------
        vbox.pack_start(gtk.HSeparator(), False, False, 0)

        bbox = gtk.HButtonBox()
        bbox.set_border_width(5)
        bbox.set_layout(gtk.BUTTONBOX_EDGE)
        bbox.set_spacing(40)
        
        button = gtk.Button(stock='gtk-help')
        bbox.add(button)

        button = gtk.Button(stock='gtk-close')
        bbox.add(button)
        button.connect("clicked", self.close)

        vbox.pack_start(bbox, False, False, 0)
        
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

    # Add user in the model
    def add_user_in_model(self, model, user):
        iter = model.append()
        model.set (iter,
                   COLUMN_USERID,    user[COLUMN_USERID],
                   COLUMN_FIRSTNAME, user[COLUMN_FIRSTNAME],
                   COLUMN_LASTNAME,  user[COLUMN_LASTNAME],
                   COLUMN_USER_EDITABLE,  False
                   )

    # If class_id is provided, only users in this class are inserted
    # If with = True, create a list only with the given class_id.
    #           False, create a list only without the given class_id
    def __create_model(self, with, class_id):

        # Grab the user data
        if(with):
            self.cur.execute('select user_id,firstname,lastname from users where class_id=?', (class_id,))
        else:
            self.cur.execute('select user_id,firstname,lastname from users where class_id!=?', (class_id,))
        user_data = self.cur.fetchall()

        model = gtk.ListStore(
            gobject.TYPE_INT,
            gobject.TYPE_STRING,
            gobject.TYPE_STRING,
            gobject.TYPE_BOOLEAN)

        for user in user_data:
            self.add_user_in_model(model, user)

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


    # Add a user from the left list to the right list
    #
    def add_user(self, button, treeview):
        selection = treeview.get_selection()
        model, iter = selection.get_selected()

        if iter:
            path = model.get_path(iter)[0]
            user_id        = model.get_value(iter, COLUMN_USERID)
            user_firstname = model.get_value(iter, COLUMN_FIRSTNAME)
            user_lastname  = model.get_value(iter, COLUMN_LASTNAME)
            model.remove(iter)

            # Add in the the right view
            self.add_user_in_model(self.model_right, (user_id, user_firstname, user_lastname))
            
            # Save the change in the base
            self.cur.execute('update users set class_id=? where user_id=?', (self.class_id, user_id))
            self.con.commit()


    # Done, can quit this dialog
    #
    def close(self, button):
        self.destroy()
        
