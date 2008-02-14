#  gcompris - group_edit.py
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
from gcompris import gcompris_gettext as _

import group_user_list

import constants

# Database
from pysqlite2 import dbapi2 as sqlite

# User List Management
(
  COLUMN_USERID,
  COLUMN_FIRSTNAME,
  COLUMN_LASTNAME,
  COLUMN_USER_EDITABLE
) = range(4)


class GroupEdit(gtk.Window):

    def __init__(self, db_connect, db_cursor,
                 class_id, class_name,
                 group_id, group_name, group_description,
                 group_user):
        # Create the toplevel window
        gtk.Window.__init__(self)

        self.cur = db_cursor
        self.con = db_connect

        self.group_id = group_id
        self.class_id = class_id

        # A pointer to the group_user_list class
        # Will be called to refresh the list when edit is done
        self.group_user = group_user

        self.set_title(_("Editing a Group"))
        self.set_border_width(8)
        self.set_default_size(320, 350)

        self.group_name = group_name
        if(self.group_name):
            frame = gtk.Frame(_("Editing group: ") + self.group_name +
                              _(" for class: ") + class_name)
            self.new_group = False
        else:
            frame = gtk.Frame(_("Editing a new group"))
            self.new_group = True
            self.group_name =""
            group_description = ""


        self.add(frame)

        # Main VBOX
        vbox = gtk.VBox(False, 8)
        vbox.set_border_width(8)
        frame.add(vbox)

        # Label and Entry for the group and description
        table = gtk.Table(2, 2, homogeneous=False)
        table.set_border_width(0)
        table.set_row_spacings(0)
        table.set_col_spacings(20)
        vbox.pack_start(table, True, True, 0)

        label = gtk.Label(_('Group:'))
        label.set_alignment(0, 0)
        table.attach(label, 0, 1, 0, 1, xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)
        self.entry_group = gtk.Entry()
        self.entry_group.set_max_length(20)
        self.entry_group.insert_text(self.group_name, position=0)
        table.attach(self.entry_group, 1, 2, 0, 1,
                     xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)

        # FIXME: How to remove the selection

        # Label and Entry for the first name
        label = gtk.Label(_('Description:'))
        label.set_alignment(0, 0)
        table.attach(label, 0, 1, 1, 2, xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)
        self.entry_description = gtk.Entry()
        self.entry_description.set_max_length(30)
        self.entry_description.insert_text(group_description, position=0)
        table.attach(self.entry_description, 1, 2, 1, 2,
                     xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)


        # Top message gives instructions
        label = gtk.Label(_('Assign all the users belonging to this group'))
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
        self.model_left = self.__create_model(False, class_id, group_id)

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

        button = gtk.Button(stock='gtk-add')
        button.connect("clicked", self.add_user, treeview)
        vbox2.pack_start(button, False, False, 0)
        button.show()

        button_delete = gtk.Button(stock='gtk-remove')
        vbox2.pack_start(button_delete, False, False, 0)
        button_delete.show()


        # Right List
        # ----------

        # Create the table
        sw2 = gtk.ScrolledWindow()
        sw2.set_shadow_type(gtk.SHADOW_ETCHED_IN)
        sw2.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)

        # create tree model
        self.model_right = self.__create_model(True, class_id, group_id)

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
    # GROUP Management
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

    # class_id: only users in this class are inserted
    # group_id: only users in this group are inserted
    # If gwith = True,  create a list only with user in the given class_id and group_id.
    #           False, create a list only with user in the given class_id but NOT this group_id
    def __create_model(self, gwith, class_id, group_id):

        model = gtk.ListStore(
            gobject.TYPE_INT,
            gobject.TYPE_STRING,
            gobject.TYPE_STRING,
            gobject.TYPE_BOOLEAN)

        # Grab the all the users from this class
        self.cur.execute('SELECT user_id,firstname,lastname FROM users WHERE class_id=? ORDER BY login', (class_id,))
        user_data = self.cur.fetchall()

        for user in user_data:

            # Check our user is already in the group
            self.cur.execute('SELECT * FROM list_users_in_groups WHERE group_id=? AND user_id=?',
                             (group_id, user[0]))
            user_is_already = self.cur.fetchall()

            if(gwith and user_is_already):
                self.add_user_in_model(model, user)
            elif(not gwith and not user_is_already):
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
            self.cur.execute('INSERT OR REPLACE INTO list_users_in_groups (group_id, user_id) VALUES (?, ?)',
                             (self.group_id, user_id))
            self.con.commit()


    # Add a user from the left list to the right list
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

            # Save the change in the base
            self.cur.execute('delete from list_users_in_groups where group_id=? and user_id=?',
                             (self.group_id, user_id))
            self.con.commit()



    # Done, can quit this dialog
    #
    def close(self, button):
        self.group_user.reload_group()
        self.destroy()

    # Done, can quit this dialog with saving
    #
    def ok(self, button):

        # Tell the user he must provide enough information
        if(self.entry_group.get_text() == ""):
            dialog = gtk.MessageDialog(None,
                                       gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                                       gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                                       _("You need to provide at least a name for your group"))
            dialog.run()
            dialog.destroy()
            return

        # If the group name as changed, check it does not exists already
        if(self.entry_group.get_text() != self.group_name):
            # Check the group does not exist already
            self.cur.execute('SELECT name FROM groups WHERE name=?',
                             (self.entry_group.get_text(),))
            if(self.cur.fetchone()):
                dialog = gtk.MessageDialog(None,
                                           gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                                           gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                                           _("There is already a group with this name"))
                dialog.run()
                dialog.destroy()
                return

        #
        # Now everything is correct, create the group
        #

        group_data = (self.group_id,
                      self.entry_group.get_text(),
                      self.class_id,
                      self.entry_description.get_text()
                      )

        if(self.new_group):
            # Create the new group
            group_id = constants.get_next_group_id(self.con, self.cur)
            self.cur.execute('INSERT INTO groups (group_id, name, class_id, description) ' +
                             'VALUES ( ?, ?, ?, ?)',
                             (group_data));
        else:
            # Save the group changes
            self.cur.execute('UPDATE groups SET name=?, description=? where group_id=?',
                             (self.entry_group.get_text(),
                              self.entry_description.get_text(),
                              self.group_id));
        self.con.commit()

        # Close the dialog window now
        self.group_user.reload_group()

        self.destroy()

