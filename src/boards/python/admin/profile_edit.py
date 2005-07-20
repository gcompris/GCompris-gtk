#  gcompris - profile_edit.py
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

import profile_group_list

import constants

# Database
from pysqlite2 import dbapi2 as sqlite

# Group Management
(
  COLUMN_GROUPID,
  COLUMN_NAME,
  COLUMN_DESCRIPTION,
  COLUMN_GROUP_EDITABLE
) = range(4)


class ProfileEdit(gtk.Window):

    def __init__(self, db_connect, db_cursor,
                 profile_id, profile_name,
                 profile_group):

        # Create the toplevel window
        gtk.Window.__init__(self)

        self.cur = db_cursor
        self.con = db_connect

        self.profile_id = profile_id

        # A pointer to the profile_group_list class
        # Will be called to refresh the list when edit is done
        self.profile_group = profile_group
        
        self.set_title(_("Group Edition"))
        self.set_border_width(8)
        self.set_default_size(320, 350)

        frame = gtk.Frame(_("Editing profile: ") + profile_name)
        self.add(frame)
        
        vbox = gtk.VBox(False, 8)
        vbox.set_border_width(8)
        frame.add(vbox)

        # Top message gives instructions
        label = gtk.Label(_('Assign all the groups bellonging to this profile'))
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
        treeview.set_search_column(COLUMN_NAME)
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

        button_delete = gtk.Button(stock='gtk-delete')
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
        treeview2.set_search_column(COLUMN_NAME)
        treeview2.get_selection().set_mode(gtk.SELECTION_MULTIPLE)

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
                   COLUMN_NAME,         group[COLUMN_NAME],
                   COLUMN_DESCRIPTION,  group[COLUMN_DESCRIPTION],
                   COLUMN_GROUP_EDITABLE,  False
                   )

    # profile_id: only groups in this profile are inserted
    # If with = True,  create a list only with groups in the given profile_id
    #           False, create a list only with groups NOT this profile_id
    def __create_model(self, with, profile_id):

        model = gtk.ListStore(
            gobject.TYPE_INT,
            gobject.TYPE_STRING,
            gobject.TYPE_STRING,
            gobject.TYPE_BOOLEAN)

        # Grab the all the groups
        self.cur.execute('select group_id,name,description from groups')
        group_data = self.cur.fetchall()

        for group in group_data:

            # Check our group is already in the profile
            self.cur.execute('select * from list_groups_in_profiles where profile_id=? and group_id=?',
                             (profile_id, group[0]))
            group_is_already = self.cur.fetchall()
            
            if(with and group_is_already):
                self.add_group_in_model(model, group)
            elif(not with and not group_is_already):
                self.add_group_in_model(model, group)

        return model

    def __add_columns(self, treeview):

        model = treeview.get_model()
        
        # columns for name
        renderer = gtk.CellRendererText()
        renderer.set_data("column", COLUMN_NAME)
        column = gtk.TreeViewColumn(_('Group'), renderer,
                                    text=COLUMN_NAME,
                                    editable=COLUMN_GROUP_EDITABLE)
        column.set_sort_column_id(COLUMN_NAME)
        column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
        column.set_fixed_width(constants.COLUMN_WIDTH_GROUPNAME)
        treeview.append_column(column)

        # column for description
        renderer = gtk.CellRendererText()
        renderer.set_data("column", COLUMN_DESCRIPTION)
        column = gtk.TreeViewColumn(_('Description'), renderer,
                                    text=COLUMN_DESCRIPTION,
                                    editable=COLUMN_GROUP_EDITABLE)
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
            group_name         = model.get_value(iter, COLUMN_NAME)
            group_description  = model.get_value(iter, COLUMN_DESCRIPTION)
            model.remove(iter)

            # Add in the the right view
            self.add_group_in_model(self.model_right, (group_id, group_name, group_description))
            
            # Save the change in the base
            self.cur.execute('insert or replace into list_groups_in_profiles (profile_id, group_id) values (?, ?)',
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
            group_name         = model.get_value(iter, COLUMN_NAME)
            group_description  = model.get_value(iter, COLUMN_DESCRIPTION)
            model.remove(iter)

            # Add in the the left view
            self.add_group_in_model(self.model_left, (group_id, group_name, group_description))
            
            # Save the change in the base
            self.cur.execute('delete from list_groups_in_profiles where profile_id=? and group_id=?',
                             (self.profile_id, group_id))
            self.con.commit()



    # Done, can quit this dialog
    #
    def close(self, button):
        self.profile_group.reload(self.profile_id)
        self.destroy()
        
