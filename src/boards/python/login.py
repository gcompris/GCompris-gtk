#  gcompris - login
# 
# Time-stamp: <2001/08/20 00:54:45 bruno>
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
import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.skin
import gtk
import gtk.gdk
from gettext import gettext as _

# Background screens
backgrounds = [
  "images/scenery2_background.png",
  "images/scenery3_background.png",
  "images/scenery4_background.png",
  "images/scenery5_background.png",
  "images/scenery6_background.png"
]

class Gcompris_login:
  """Login screen for gcompris"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

    print("Gcompris_pythontest __init__.")


  def start(self):
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=1
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    gcompris.bar_set(0)
    gcompris.set_background(self.gcomprisBoard.canvas.root(),
                            gcompris.utils.load_pixmap(backgrounds[self.gcomprisBoard.level-1]))
    gcompris.bar_set_level(self.gcomprisBoard)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0.0,
      y=0.0
      )
    
    print("Gcompris_pythontest start.")


  def end(self):

    # Remove the root item removes all the others inside it
    self.rootitem.destroy()


  def ok(self):
    print("Gcompris_pythontest ok.")


  def repeat(self):
    print("Gcompris_pythontest repeat.")


  def config(self):
    print("Gcompris_pythontest config.")


  def key_press(self, keyval):
    print("Gcompris_pythontest key press. %i" % keyval)

    # Return  True  if you did process a key
    # Return  False if you did not processed a key
    #         (gtk need to send it to next widget)
    return False

  def pause(self, pause):
    print("Gcompris_pythontest pause. %i" % pause)


  def set_level(self, level):
    print("Gcompris_pythontest set level. %i" % level)

# ---- End of Initialisation

