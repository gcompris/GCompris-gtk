#  gcompris - profile_edit.py
#
# Copyright (C) 2005, 2008 Bruno Coudoin and Yves Combe
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
import gcompris
import gettext
from gcompris import gcompris_gettext as _
from gettext import dgettext as D_
import profile_group_list

import constants

# Group Management
(
  COLUMN_GROUPID,
  COLUMN_CLASSNAME,
  COLUMN_GROUPNAME,
  COLUMN_DESCRIPTION,
) = range(4)


class ProfileWidget(gtk.HBox):

    def __init__(self, db_connect, db_cursor, profile_id, new_profile):

        # Create the toplevel window
        gtk.HBox.__init__(self)

        self.cur = db_cursor
        self.con = db_connect
        self.profile_id = profile_id
        self.new_profile = new_profile


        # Left list
        # ---------

        # Create the table
        sw = gtk.ScrolledWindow()
        sw.set_shadow_type(gtk.SHADOW_ETCHED_IN)
        sw.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)

        # create tree model
        self.model_left = self.__create_model(False, profile_id)

        # create tree view
        treeview = gtk.TreeView(self.model_left)
        treeview.set_rules_hint(True)
        treeview.set_search_column(COLUMN_GROUPNAME)
        treeview.get_selection().set_mode(gtk.SELECTION_MULTIPLE)

        sw.add(treeview)

        # add columns to the tree view
        self.__add_columns(treeview)

        self.pack_start(sw, True, True, 0)


        # Middle Button
        # -------------
        vbox2 = gtk.VBox(False, 8)
        vbox2.set_border_width(8)
        self.pack_start(vbox2, True, True, 0)

        button = gtk.Button(stock='gtk-add')
        button.connect("clicked", self.add_group, treeview)
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
        self.model_right = self.__create_model(True, profile_id)

        # create tree view
        treeview2 = gtk.TreeView(self.model_right)
        treeview2.set_rules_hint(True)
        treeview2.set_search_column(COLUMN_GROUPNAME)
        treeview2.get_selection().set_mode(gtk.SELECTION_MULTIPLE)

        sw2.add(treeview2)

        # add columns to the tree view
        self.__add_columns(treeview2)

        self.pack_start(sw2, True, True, 0)

        # Missing callbacks
        button_delete.connect("clicked", self.remove_group, treeview2)


    # -------------------
    # User Management
    # -------------------

    # Add user in the model
    def add_group_in_model(self, model, group):
        iter = model.append()
        model.set (iter,
                   COLUMN_GROUPID,      group[COLUMN_GROUPID],
                   COLUMN_CLASSNAME,    group[COLUMN_CLASSNAME],
                   COLUMN_GROUPNAME,    group[COLUMN_GROUPNAME],
                   COLUMN_DESCRIPTION,  group[COLUMN_DESCRIPTION]
                   )

    # profile_id: only groups in this profile are inserted
    # If gwith = True,  create a list only with groups in the given profile_id
    #           False, create a list only with groups NOT this profile_id
    def __create_model(self, gwith, profile_id):

        model = gtk.ListStore(
            gobject.TYPE_INT,
            gobject.TYPE_STRING,
            gobject.TYPE_STRING,
            gobject.TYPE_STRING)

        # Grab the all the groups
        self.cur.execute('SELECT group_id,name,description FROM groups ORDER BY name')
        group_data = self.cur.fetchall()

        for group in group_data:

            group_id = group[0]
            # Check our group is already in the profile
            self.cur.execute('SELECT * FROM list_groups_in_profiles ' +
                             'WHERE profile_id=? AND group_id=?',
                             (profile_id, group_id))
            group_is_already = self.cur.fetchall()

            # Extract the class name of this group
            class_name = constants.get_class_name_for_group_id(self.con,
                                                               self.cur,
                                                               group_id)

            # Insert the class name in the group
            group = (group[0], class_name, group[1], group[2])

            if(gwith and group_is_already):
                self.add_group_in_model(model, group)
            elif(not gwith and not group_is_already):
                self.add_group_in_model(model, group)

        return model

    def __add_columns(self, treeview):

        model = treeview.get_model()

        # columns for class name
        renderer = gtk.CellRendererText()
        renderer.set_data("column", COLUMN_CLASSNAME)
        column = gtk.TreeViewColumn(D_(gcompris.GETTEXT_ADMIN,'Class'), renderer,
                                    text=COLUMN_CLASSNAME)
        column.set_sort_column_id(COLUMN_CLASSNAME)
        column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
        column.set_fixed_width(constants.COLUMN_WIDTH_CLASSNAME)
        treeview.append_column(column)

        # columns for group name
        renderer = gtk.CellRendererText()
        renderer.set_data("column", COLUMN_GROUPNAME)
        column = gtk.TreeViewColumn(D_(gcompris.GETTEXT_ADMIN,'Group'), renderer,
                                    text=COLUMN_GROUPNAME)
        column.set_sort_column_id(COLUMN_GROUPNAME)
        column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
        column.set_fixed_width(constants.COLUMN_WIDTH_GROUPNAME)
        treeview.append_column(column)

        # column for description
        renderer = gtk.CellRendererText()
        renderer.set_data("column", COLUMN_DESCRIPTION)
        column = gtk.TreeViewColumn(D_(gcompris.GETTEXT_ADMIN,'Description'), renderer,
                                    text=COLUMN_DESCRIPTION)
        column.set_sort_column_id(COLUMN_DESCRIPTION)
        column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
        column.set_fixed_width(constants.COLUMN_WIDTH_GROUPDESCRIPTION_EDIT)
        treeview.append_column(column)


    # Add a group from the left list to the right list
    #
    def add_group(self, button, treeview):

        model = treeview.get_model()

        treestore, paths = treeview.get_selection().get_selected_rows()

        paths.reverse()

        for path in paths:

            iter = treestore.get_iter(path)

            path = model.get_path(iter)[0]
            group_id           = model.get_value(iter, COLUMN_GROUPID)
            class_name         = model.get_value(iter, COLUMN_CLASSNAME)
            group_name         = model.get_value(iter, COLUMN_GROUPNAME)
            group_description  = model.get_value(iter, COLUMN_DESCRIPTION)
            model.remove(iter)

            # Add in the the right view
            self.add_group_in_model(self.model_right,
                                    (group_id, class_name, group_name, group_description))

            # Save the change in the base
            self.cur.execute('INSERT OR REPLACE INTO list_groups_in_profiles ' +
                             '(profile_id, group_id) VALUES (?, ?)',
                             (self.profile_id, group_id))
            self.con.commit()


    # Add a group from the left list to the right list
    #
    def remove_group(self, button, treeview):

        model = treeview.get_model()

        treestore, paths = treeview.get_selection().get_selected_rows()

        paths.reverse()

        for path in paths:

            iter = treestore.get_iter(path)

            path = model.get_path(iter)[0]
            group_id           = model.get_value(iter, COLUMN_GROUPID)
            class_name         = model.get_value(iter, COLUMN_CLASSNAME)
            group_name         = model.get_value(iter, COLUMN_GROUPNAME)
            group_description  = model.get_value(iter, COLUMN_DESCRIPTION)
            model.remove(iter)

            # Add in the the left view
            self.add_group_in_model(self.model_left,
                                    (group_id, class_name, group_name, group_description))

            # Save the change in the base
            self.cur.execute('DELETE FROM list_groups_in_profiles ' +
                             'WHERE profile_id=? AND group_id=?',
                             (self.profile_id, group_id))
            self.con.commit()

    # Done, can quit this dialog with saving
    #
    def ok(self, entry_profile, entry_description):
        #
        # Now everything is correct, create the profile
        #

        profile_data = (self.profile_id,
                        entry_profile,
                        entry_description
                      )

        if(self.new_profile):
            # Check the login do not exist already
            self.cur.execute('SELECT name FROM profiles WHERE name=?',
                             (entry_profile,))
            if(self.cur.fetchone()):
                dialog = gtk.MessageDialog(None,
                                           gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                                           gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                                           D_(gcompris.GETTEXT_ADMIN,"There is already a profile with this name"))
                dialog.run()
                dialog.destroy()
                return

            # Create the new profile
            profile_id = constants.get_next_profile_id(self.con, self.cur)
            self.cur.execute('INSERT INTO profiles (profile_id, name, description) ' +
                             'VALUES ( ?, ?, ?)',
                             (profile_data));
        else:
            # Save the profile changes
            self.cur.execute('UPDATE profiles SET name=?, description=? where profile_id=?',
                             (entry_profile,
                              entry_description,
                              self.profile_id));

        self.con.commit()
