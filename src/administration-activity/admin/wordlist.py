#  gcompris - board_list.py
#
# Copyright (C) 2005, 2008 Yves Combe
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

import goocanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.admin
import gtk
import gtk.gdk
import gobject
from gcompris import gcompris_gettext as _

import glob

class Words_list:
  """GCompris Words  List Tool"""


  # area is the drawing area for the list
  def __init__(self, db_connect, db_cursor, profile):

    self.cur = db_cursor
    self.con = db_connect
    self.active_profile = profile

    files = glob.glob('wordlist_*.xml')

    self.main_vbox = gcompris.configuration_window ( \
      _('<b>{config}</b> configuration\n for profile <b>{profile}</b>').format( \
                        config='Wordlist',
                        profile=profile.name if profile else _("Default")),
      self.wordlist_callback
      )

    self.prop = gcompris.get_properties()

    self.wordlist_dir = self.prop.shared_dir +'/wordlist'

    gcompris.textview('Words list',
                      'wordlist',
                      'Enter the words, comma, space, return separated. Then click on th check button.',
                      '',
                      self.wordlist_validate)

  def wordlist_callback(self, table):
    for k, v in table.iteritems():
      print k, v


  def wordlist_validate( self, key, text, label):
    label.set_markup(text)
    return True



