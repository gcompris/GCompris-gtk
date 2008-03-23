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
from gcompris import gcompris_gettext as _

import profile_group_list

import constants

# Database
from pysqlite2 import dbapi2 as sqlite

# Group Management
(
  COLUMN_GROUPID,
  COLUMN_CLASSNAME,
  COLUMN_GROUPNAME,
  COLUMN_DESCRIPTION,
) = range(4)


class ProfileEdit(gtk.Window):

    def __init__(self, db_connect, db_cursor,
                 profile_id, profile_name, profile_description,
                 profile_list):

        # Create the toplevel window
        gtk.Window.__init__(self)

        self.cur = db_cursor
        self.con = db_connect

        self.profile_id = profile_id

        # A pointer to the profile_list_list class
        # Will be called to refresh the list when edit is done
        self.profile_list = profile_list

        self.set_title(_("Editing a Profile"))
        self.set_border_width(8)
        self.set_default_size(320, 350)

        if(profile_name):
            frame = gtk.Frame(_("Editing profile: ") + profile_name)
            self.new_profile = False
        else:
            frame = gtk.Frame(_("Editing a new profile"))
            self.new_profile = True
            profile_name =""
            profile_description = ""


        self.add(frame)

        vbox = gtk.VBox(False, 8)
        vbox.set_border_width(8)
        frame.add(vbox)

        # Label and Entry for the group and description
        table = gtk.Table(2, 2, homogeneous=False)
        table.set_border_width(0)
        table.set_row_spacings(0)
        table.set_col_spacings(20)
        vbox.pack_start(table, True, True, 0)

        label = gtk.Label(_('Profile:'))
        label.set_alignment(0, 0)
        table.attach(label, 0, 1, 0, 1, xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)
        self.entry_profile = gtk.Entry()
        self.entry_profile.set_max_length(20)
        self.entry_profile.insert_text(profile_name, position=0)
        table.attach(self.entry_profile, 1, 2, 0, 1,
                     xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)

        # FIXME: How to remove the selection

        # Label and Entry for the first name
        label = gtk.Label(_('Description:'))
        label.set_alignment(0, 0)
        table.attach(label, 0, 1, 1, 2, xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)
        self.entry_description = gtk.Entry()
        self.entry_description.set_max_length(30)
        self.entry_description.insert_text(profile_description, position=0)
        table.attach(self.entry_description, 1, 2, 1, 2,
                     xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)


        # Top message gives instructions
        label = gtk.Label(_('Assign all the groups belonging to this profile'))
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
        self.model_left = self.__create_model(False, profile_id)

        # create tree view
        treeview = gtk.TreeView(self.model_left)
        treeview.set_rules_hint(True)
        treeview.set_search_column(COLUMN_GROUPNAME)
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
        button_delete.connect("clicked", self.remove_group, treeview2)

        # Ready GO
        self.show_all()


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
        column = gtk.TreeViewColumn(_('Class'), renderer,
                                    text=COLUMN_CLASSNAME)
        column.set_sort_column_id(COLUMN_CLASSNAME)
        column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
        column.set_fixed_width(constants.COLUMN_WIDTH_CLASSNAME)
        treeview.append_column(column)

        # columns for group name
        renderer = gtk.CellRendererText()
        renderer.set_data("column", COLUMN_GROUPNAME)
        column = gtk.TreeViewColumn(_('Group'), renderer,
                                    text=COLUMN_GROUPNAME)
        column.set_sort_column_id(COLUMN_GROUPNAME)
        column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
        column.set_fixed_width(constants.COLUMN_WIDTH_GROUPNAME)
        treeview.append_column(column)

        # column for description
        renderer = gtk.CellRendererText()
        renderer.set_data("column", COLUMN_DESCRIPTION)
        column = gtk.TreeViewColumn(_('Description'), renderer,
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



    # Done, can quit this dialog
    #
    def close(self, button):
        self.profile_list.reload_profile()
        self.destroy()

    # Done, can quit this dialog with saving
    #
    def ok(self, button):

        # Tell the user he must provide enough information
        if(self.entry_profile.get_text().strip() == ""):
            dialog = gtk.MessageDialog(None,
                                       gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                                       gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                                       _("You need to provide at least a name for your profile"))
            dialog.run()
            dialog.destroy()
            return

        #
        # Now everything is correct, create the profile
        #

        profile_data = (self.profile_id,
                        self.entry_profile.get_text().strip(),
                        self.entry_description.get_text()
                      )

        if(self.new_profile):
            # Check the login do not exist already
            self.cur.execute('SELECT name FROM profiles WHERE name=?',
                             (self.entry_profile.get_text(),))
            if(self.cur.fetchone()):
                dialog = gtk.MessageDialog(None,
                                           gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                                           gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                                           _("There is already a profile with this name"))
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
                             (self.entry_profile.get_text(),
                              self.entry_description.get_text(),
                              self.profile_id));
        self.con.commit()

        # Close the dialog window now
        self.profile_list.reload_profile()

        self.destroy()

