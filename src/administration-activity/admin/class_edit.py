#  gcompris - class_edit.py
#
# Copyright (C) 2005 Bruno Coudoin and Yves Combe
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, see <http://www.gnu.org/licenses/>.
#


import gtk
import gobject
from gcompris import gcompris_gettext as _

# Database
from pysqlite2 import dbapi2 as sqlite

import user_list

import constants

# User List Management
(
  COLUMN_USERID,
  COLUMN_FIRSTNAME,
  COLUMN_LASTNAME,
  COLUMN_USER_EDITABLE
) = range(4)


class ClassEdit(gtk.Window):

    def __init__(self, db_connect, db_cursor,
                 class_id, class_name, teacher_name,
                 list_class):
        # Create the toplevel window
        gtk.Window.__init__(self)

        self.cur = db_cursor
        self.con = db_connect

        self.class_id = class_id
        self.class_name = class_name
        self.teacher_name = teacher_name

        # A pointer to the user_list class
        # Will be called to refresh the list when edit is done
        self.list_class = list_class

        self.set_title(_("Editing a Class"))
        self.set_border_width(8)
        self.set_default_size(320, 350)

        if(self.class_name):
            frame = gtk.Frame(_("Editing class: ") + self.class_name)
            self.new_class = False
        else:
            frame = gtk.Frame(_("Editing a new class"))
            self.new_class = True

        # Connect the "destroy" event to close
        # FIXME: This makes the close code beeing called twice
        #        because the close destroy also call close again.
        frame.connect("destroy", self.close)

        self.add(frame)

        # Main VBOX
        vbox = gtk.VBox(False, 8)
        vbox.set_border_width(8)
        frame.add(vbox)

        # Label and Entry for the class name
        table = gtk.Table(2, 2, homogeneous=False)
        table.set_border_width(0)
        table.set_row_spacings(0)
        table.set_col_spacings(20)
        vbox.pack_start(table, True, True, 0)

        label = gtk.Label(_('Class:'))
        label.set_alignment(0, 0)
        table.attach(label, 0, 1, 0, 1, xoptions=gtk.SHRINK,
                     yoptions=gtk.EXPAND)
        self.entry_class = gtk.Entry()
        self.entry_class.set_max_length(20)
        self.entry_class.insert_text(self.class_name, position=0)
        table.attach(self.entry_class, 1, 2, 0, 1,
                     xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)

        # FIXME: How to remove the default selection

        # Label and Entry for the teacher name
        label = gtk.Label(_('Teacher:'))
        label.set_alignment(0, 0)
        table.attach(label, 0, 1, 1, 2, xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)
        self.entry_teacher = gtk.Entry()
        self.entry_teacher.set_max_length(30)
        self.entry_teacher.insert_text(self.teacher_name, position=0)
        table.attach(self.entry_teacher, 1, 2, 1, 2, xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)

        # Top message gives instructions
        vbox.pack_start(gtk.HSeparator(), False, False, 0)
        label = gtk.Label(_('Assign all the users belonging to this class'))
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
        treeview.get_selection().set_mode(gtk.SELECTION_MULTIPLE)

        sw.add(treeview)

        # add columns to the tree view
        self.__add_columns(treeview)

        hbox.pack_start(sw, True, True, 0)


        # Middle Button
        # -------------
        vbox2 = gtk.VBox(False, 8)
        vbox2.set_border_width(8)
        hbox.pack_start(vbox2, True, True, 0)

        button_add = gtk.Button(stock='gtk-add')
        button_add.connect("clicked", self.add_user, treeview)
        vbox2.pack_start(button_add, False, False, 0)

        button_delete = gtk.Button(stock='gtk-remove')
        vbox2.pack_start(button_delete, False, False, 0)

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
        treeview2.get_selection().set_mode(gtk.SELECTION_MULTIPLE)

        sw2.add(treeview2)

        # add columns to the tree view
        self.__add_columns(treeview2)

        hbox.pack_start(sw2, True, True, 0)

        # Confirmation Buttons
        # --------------------
        vbox.pack_start(gtk.HSeparator(), False, False, 0)

        bbox = gtk.HBox(homogeneous=False, spacing=8)

        button = gtk.Button(stock='gtk-help')
        bbox.pack_start(button, expand=False, fill=False, padding=0)

        button = gtk.Button(stock='gtk-ok')
        bbox.pack_end(button, expand=False, fill=False, padding=0)
        button.connect("clicked", self.ok)

        button = gtk.Button(stock='gtk-close')
        bbox.pack_end(button, expand=False, fill=False, padding=0)
        button.connect("clicked", self.close)

        vbox.pack_start(bbox, False, False, 0)

        # Missing callbacks
        button_delete.connect("clicked", self.remove_user, treeview2)

        # Ready GO
        self.show_all()


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
    # If gwith = True, create a list only with the given class_id.
    #           False, create a list only without the given class_id
    def __create_model(self, gwith, class_id):

        # Grab the user data
        if(gwith):
            self.cur.execute('SELECT user_id,firstname,lastname FROM users where class_id=? ORDER BY login', (class_id,))
        else:
            self.cur.execute('SELECT user_id,firstname,lastname FROM users WHERE class_id!=? ORDER BY login', (class_id,))
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
        column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
        column.set_fixed_width(constants.COLUMN_WIDTH_FIRSTNAME)
        treeview.append_column(column)

        # column for last name
        renderer = gtk.CellRendererText()
        renderer.set_data("column", COLUMN_LASTNAME)
        column = gtk.TreeViewColumn(_('Last Name'), renderer,
                                    text=COLUMN_LASTNAME,
                                    editable=COLUMN_USER_EDITABLE)
        column.set_sort_column_id(COLUMN_LASTNAME)
        column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
        column.set_fixed_width(constants.COLUMN_WIDTH_LASTNAME)
        treeview.append_column(column)


    # Add a user from the left list to the right list
    #
    def add_user(self, button, treeview):
        model = treeview.get_model()
        treestore, paths = treeview.get_selection().get_selected_rows()
        paths.reverse()

        if(len(paths)>0 and self.new_class):
            self.create_class()

        for path in paths:
            iter = treestore.get_iter(path)
            path = model.get_path(iter)[0]
            user_id        = model.get_value(iter, COLUMN_USERID)
            user_firstname = model.get_value(iter, COLUMN_FIRSTNAME)
            user_lastname  = model.get_value(iter, COLUMN_LASTNAME)
            model.remove(iter)

            # Add in the the right view
            self.add_user_in_model(self.model_right, (user_id, user_firstname, user_lastname))

            # Save the change in the base
            self.cur.execute('UPDATE users SET class_id=? WHERE user_id=?',
                             (self.class_id, user_id))
            self.con.commit()

    # Remove a user from the right list to the left list
    #
    def remove_user(self, button, treeview):
        model = treeview.get_model()
        treestore, paths = treeview.get_selection().get_selected_rows()
        paths.reverse()

        for path in paths:
            iter = treestore.get_iter(path)
            path = model.get_path(iter)[0]
            user_id        = model.get_value(iter, COLUMN_USERID)
            user_firstname = model.get_value(iter, COLUMN_FIRSTNAME)
            user_lastname  = model.get_value(iter, COLUMN_LASTNAME)
            model.remove(iter)

            # Add in the the left view
            self.add_user_in_model(self.model_left, (user_id, user_firstname, user_lastname))

            # Save the change in the base (1 Is the 'Unselected user' class)
            self.cur.execute('UPDATE users SET class_id=? where user_id=?', (1, user_id))
            self.con.commit()

    # Done, can quit this dialog (without saving)
    #
    def close(self, button):

        self.list_class.reload(self.class_id,
                               self.class_name,
                               self.teacher_name)
        self.destroy()

    # Done, can quit this dialog with saving
    #
    def ok(self, button):

        # Tell the user he must provide enough information
        if(self.entry_class.get_text() == ""):
            dialog = gtk.MessageDialog(None,
                                       gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                                       gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                                       _("You need to provide at least a name for your class"))
            dialog.run()
            dialog.destroy()
            return

        #
        # Now everything is correct, create the class
        #

        class_data = (self.class_id,
                      self.entry_class.get_text(),
                      self.entry_teacher.get_text()
                      )

        if(self.new_class):
            self.create_class()

        # Save the changes in the base
        self.cur.execute('UPDATE class set name=?,teacher=? where class_id=?',
                         (self.entry_class.get_text(),
                          self.entry_teacher.get_text(),
                          self.class_id));
        self.con.commit()

        # Close the dialog window now
        # (The close code will refresh the class_list)
        self.class_name   = self.entry_class.get_text()
        self.teacher_name = self.entry_teacher.get_text()

        self.destroy()


    #
    # Create a class
    # Make the necessary checks and create the class in the base
    #
    def create_class(self):

        # Check the login do not exist already
        self.cur.execute('SELECT name FROM class WHERE name=?',
                         (self.entry_class.get_text(),))
        if(self.cur.fetchone()):
            dialog = gtk.MessageDialog(None,
                                       gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                                       gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                                       _("There is already a class with this name"))
            dialog.run()
            dialog.destroy()
            return

        # Create its Whole group
        group_id = constants.get_next_group_id(self.con, self.cur)
        self.cur.execute('INSERT INTO groups (group_id, name, class_id, description) ' +
                         'VALUES ( ?, "All", ?, "All users")',
                         (group_id, self.class_id));

        class_data = (self.class_id,
                      self.entry_class.get_text(),
                      self.entry_teacher.get_text(),
                      group_id
                      )

        self.cur.execute('INSERT OR REPLACE INTO class (class_id, name, teacher, wholegroup_id) ' +
                         'values (?, ?, ?, ?)', class_data)

        # No more need to create this class, it's done
        self.new_class = False
