#  gcompris - module_activities.py
#
# Copyright (C) 2005, 2008 Yves Combe
# Copyright (C) 2012, Aleksey Lim
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
import gettext
from gcompris import gcompris_gettext as _
from gettext import dgettext as D_
import sys;

# Database
try:
  from sqlite3 import dbapi2 as sqlite # python 2.5
except:
  try:
    from pysqlite2 import dbapi2 as sqlite
  except:
    print 'This program requires pysqlite2\n',\
        'http://initd.org/tracker/pysqlite/'
    sys.exit(1)

import module
import board_list

class Activities(module.Module):
  """Administrating GCompris Boards"""

  already_loaded = False

  def __init__(self, canvas):
    module.Module.__init__(self, canvas, "activities", D_(gcompris.GETTEXT_ADMIN,"Activities"))

  # Return the position it must have in the administration menu
  # The smaller number is the highest.
  def position(self):
    return 3

  def start(self, area):
    # Connect to our database
    self.con = sqlite.connect(gcompris.get_database())
    self.cur = self.con.cursor()

    if Activities.already_loaded:
      self.rootitem.props.visibility = goocanvas.ITEM_VISIBLE
      self.boardList.show(self.con, self.cur)
      return

    Activities.already_loaded = True

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.

    self.rootitem = goocanvas.Group(
      parent = self.canvas,
      )

    module.Module.start(self)

    self.frame = gtk.Frame(D_(gcompris.GETTEXT_ADMIN,"Activities"))
    self.frame.show()

    goocanvas.Widget(
      parent = self.rootitem,
      widget=self.frame,
      x=area[0]+self.module_panel_ofset,
      y=area[1]+self.module_panel_ofset,
      width=area[2]-area[0]-2*self.module_panel_ofset,
      height=area[3]-area[1]-2*self.module_panel_ofset,
      anchor=gtk.ANCHOR_NW)

    self.boardList = board_list.Board_list(self.con, self.cur, self.frame,
            gcompris.sugar_get_profile_id(), hide_profiles=True)
    self.boardList.init()

  def stop(self):
    module.Module.stop(self)

    # This module is slow to start, we just hide it
    self.rootitem.props.visibility = goocanvas.ITEM_INVISIBLE
    self.boardList.hide()

    # Close the database
    self.cur.close()
    self.con.close()
