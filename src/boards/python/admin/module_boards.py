#  gcompris - module_boards.py
# 
# Copyright (C) 2005 Yves Combe
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

import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.admin
import gtk
import gtk.gdk
from gettext import gettext as _
import sys;

# Database
from pysqlite2 import dbapi2 as sqlite

import module
import board_list

class Boards(module.Module):
  """Administrating GCompris Boards"""


  def __init__(self, canvas):
    print("Gcompris_administration __init__ boards panel.")
    module.Module.__init__(self, canvas, "boards", _("Boards"))

  def start(self, area):
    print "starting boards panel"

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.

    self.rootitem = self.canvas.add(
        gnome.canvas.CanvasGroup,
        x=0.0,
        y=0.0
        )

    module.Module.start(self)

    # Connect to our database
    self.con = sqlite.connect(gcompris.get_database())
    self.cur = self.con.cursor()

    frame = gtk.Frame(_("Boards"))
    frame.show()

    self.rootitem.add(
      gnome.canvas.CanvasWidget,
      widget=frame,
      x=area[0]+self.module_panel_ofset,
      y=area[1]+self.module_panel_ofset,
      width=area[2]-area[0]-2*self.module_panel_ofset,
      height=area[3]-area[1]-2*self.module_panel_ofset,
      anchor=gtk.ANCHOR_NW,
      size_pixels=False)

    board_list.Board_list(self.con, self.cur,
                          frame)
    
  def stop(self):
    print "stopping boards panel"
    module.Module.stop(self)

    # Remove the root item removes all the others inside it
    self.rootitem.destroy()

    # Close the database
    self.cur.close()
    self.con.close()
