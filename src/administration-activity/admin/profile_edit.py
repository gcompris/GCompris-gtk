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
from gcompris import gcompris_gettext as _

import profile_widget


class ProfileEdit(gtk.Window):

    def __init__(self, db_connect, db_cursor,
                 profile_id, profile_name, profile_description,
                 profile_list):

        # Create the toplevel window
        gtk.Window.__init__(self)

        self.profile_list = profile_list;

        self.set_title(_("Editing a Profile"))
        self.set_border_width(8)
        self.set_default_size(320, 350)

        if(profile_name):
            frame = gtk.Frame(_("Editing profile: ") + profile_name)
            new_profile = False
        else:
            frame = gtk.Frame(_("Editing a new profile"))
            new_profile = True
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
        self.profile_widget = profile_widget.ProfileWidget(db_connect,
                db_cursor, profile_id, new_profile)
        vbox.pack_start(self.profile_widget, True, True, 0)

        # Confirmation Buttons
        # --------------------
        vbox.pack_start(gtk.HSeparator(), False, False, 0)

        bbox = gtk.HBox(homogeneous=False, spacing=8)

        button = gtk.Button(stock='gtk-ok')
        bbox.pack_end(button, expand=False, fill=False, padding=0)
        button.connect("clicked", self.ok)

        button = gtk.Button(stock='gtk-close')
        bbox.pack_end(button, expand=False, fill=False, padding=0)
        button.connect("clicked", self.close)

        vbox.pack_start(bbox, False, False, 0)

        # Ready GO
        self.show_all()

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

        self.profile_widget.ok(self.entry_profile.get_text().strip(),
                self.entry_description.get_text())

        # Close the dialog window now
        self.profile_list.reload_profile()

        self.destroy()

