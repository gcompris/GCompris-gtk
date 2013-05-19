#  gcompris - module_profiles.py
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

import goocanvas
import gcompris
import gcompris.utils
import gcompris.skin
import gtk
import gtk.gdk
import gettext
from gcompris import gcompris_gettext as _
from gettext import dgettext as D_
import module
import profile_list

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

class Profiles(module.Module):
  """Administrating GCompris Profiles"""


  def __init__(self, canvas):
    module.Module.__init__(self, canvas, "profiles", D_(gcompris.GETTEXT_ADMIN,"Profiles"))

  # Return the position it must have in the administration menu
  # The smaller number is the highest.
  def position(self):
    return 2

  def start(self, area):
    # Connect to our database
    self.con = sqlite.connect(gcompris.get_database())
    self.cur = self.con.cursor()

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.

    self.rootitem = goocanvas.Group(
      parent = self.canvas,
      )

    module.Module.start(self)

    frame = gtk.Frame(D_(gcompris.GETTEXT_ADMIN,"Profiles"))
    frame.show()

    goocanvas.Widget(
      parent = self.rootitem,
      widget=frame,
      x=area[0]+self.module_panel_ofset,
      y=area[1]+self.module_panel_ofset,
      width=area[2]-area[0]-2*self.module_panel_ofset,
      height=area[3]-area[1]-2*self.module_panel_ofset,
      anchor=gtk.ANCHOR_NW)

    profile_list.Profile_list(frame, self.con, self.cur)

  def stop(self):
    module.Module.stop(self)

    # Remove the root item removes all the others inside it
    self.rootitem.remove()

    # Close the database
    self.cur.close()
    self.con.close()
