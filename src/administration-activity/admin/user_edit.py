#  gcompris - user_edit.py
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

import user_list

import constants

class UserEdit(gtk.Window):

    def __init__(self, db_connect, db_cursor,
                 user_id, login, firstname, lastname, birthdate, class_id,
                 list_user):
        # Create the toplevel window
        gtk.Window.__init__(self)

        self.cur = db_cursor
        self.con = db_connect

        self.user_id  = user_id
        self.class_id = class_id

        # A pointer to the user_list class
        # Will be called to refresh the list when edit is done
        self.list_user = list_user

        self.set_title(_("Editing a User"))
        self.set_border_width(8)
        self.set_default_size(320, 250)

        if(firstname and lastname):
            frame = gtk.Frame(_("Editing a User ") +
                              lastname + " " +
                              firstname)
            self.new_user = False
            self.old_login = login
        else:
            frame = gtk.Frame(_("Editing a new user"))
            self.new_user = True

        self.add(frame)

        # Main VBOX
        vbox = gtk.VBox(False, 8)
        vbox.set_border_width(8)
        frame.add(vbox)

        # Label and Entry for the user name
        table = gtk.Table(2, 5, homogeneous=False)
        table.set_border_width(0)
        table.set_row_spacings(0)
        table.set_col_spacings(20)
        vbox.pack_start(table, True, True, 0)

        label = gtk.Label(_('Login:'))
        label.set_alignment(0, 0)
        table.attach(label, 0, 1, 0, 1, xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)
        self.entry_login = gtk.Entry()
        self.entry_login.set_max_length(20)
        self.entry_login.insert_text(login, position=0)
        table.attach(self.entry_login, 1, 2, 0, 1, xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)

        # FIXME: How to remove the selection

        # Label and Entry for the first name
        label = gtk.Label(_('First name:'))
        label.set_alignment(0, 0)
        table.attach(label, 0, 1, 1, 2, xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)
        self.entry_firstname = gtk.Entry()
        self.entry_firstname.set_max_length(30)
        self.entry_firstname.insert_text(firstname, position=0)
        table.attach(self.entry_firstname, 1, 2, 1, 2, xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)

        # Label and Entry for the last name
        label = gtk.Label(_('Last name:'))
        label.set_alignment(0, 0)
        table.attach(label, 0, 1, 2, 3, xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)
        self.entry_lastname = gtk.Entry()
        self.entry_lastname.set_max_length(30)
        self.entry_lastname.insert_text(lastname, position=0)
        table.attach(self.entry_lastname, 1, 2, 2, 3, xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)

        # Label and Entry for the birth date
        label = gtk.Label(_('Birth date:'))
        label.set_alignment(0, 0)
        table.attach(label, 0, 1, 3, 4, xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)
        self.entry_birthdate = gtk.Entry()
        self.entry_birthdate.set_max_length(30)
        self.entry_birthdate.insert_text(birthdate, position=0)
        table.attach(self.entry_birthdate, 1, 2, 3, 4, xoptions=gtk.SHRINK, yoptions=gtk.EXPAND)

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

        # Ready GO
        self.show_all()


    # -------------------
    # User Management
    # -------------------


    # Done, can quit this dialog (without saving)
    #
    def close(self, button):
        self.destroy()

    # Done, can quit this dialog with saving
    #
    def ok(self, button):

        # Tell the user he must provide enough information
        if(self.entry_login.get_text() == "" or
           self.entry_firstname.get_text() == "" or
           self.entry_lastname.get_text() == ""):
            dialog = gtk.MessageDialog(None,
                                       gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                                       gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                                       _("You need to provide at least a login, first name and last name for your users"))
            dialog.run()
            dialog.destroy()
            return

        if(self.new_user):
            # Check the login do not exist already
            self.cur.execute('SELECT login FROM users')
        else:
            self.cur.execute('SELECT login FROM users WHERE login != \'%s\'' % self.old_login)
        logins = [x[0].upper() for x in self.cur.fetchall()]
        login = self.entry_login.get_text().decode('utf-8').upper()

        if login in logins:
            dialog = gtk.MessageDialog(None,
                                       gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                                       gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                                       _("There is already a user with this login"))
            dialog.run()
            dialog.destroy()
            return

        #
        # Now everything is correct, create the user
        #
        user_data = (self.user_id,
                     self.entry_login.get_text(),
                     self.entry_firstname.get_text(),
                     self.entry_lastname.get_text(),
                     self.entry_birthdate.get_text(),
                     self.class_id
                     )
        # Save the changes in the base
        self.cur.execute('insert or replace into users (user_id, login, firstname, lastname, birthdate, class_id) values (?, ?, ?, ?, ?, ?)', user_data)
        self.con.commit()

        # Close the dialog window now
        self.list_user.reload(self.class_id)

        self.destroy()

