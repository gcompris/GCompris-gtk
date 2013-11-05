#  gcompris - exercise.py
#
# Copyright (C) 2003, 2008 Bruno Coudoin and Kesha Shah
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
# exercise activity.
import gtk
import gtk.gdk
import random
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.anim
import gcompris.bonus
import goocanvas
import pango
import gobject
from gcompris import gcompris_gettext as _

class Gcompris_exercise:
  """Empty gcompris Python class"""


  def __init__(self, gcomprisBoard):
    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True
 
  # Catching an event and responding to it via printing on terminal the item which was clicked and the position.
  def on_button_press(self, item, target, event, id):
    print "%s received 'button-press' signal at %f, %f (root: %f, %f)" % \
           (id, event.x, event.y, event.x_root, event.y_root)
    return True  
  

  def start(self):
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=2
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=4

    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.bar_location(0, -1, 0.8)
    self.rootitem = goocanvas.Group(parent = self.gcomprisBoard.canvas.get_root_item())

    # Set a background image
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
								"exercise/background.png")

 
    self.Title = goocanvas.Text(
      parent = self.rootitem,
      x=400.0,
      y=50.0,
	  width=1000,
      text=(" SEASON CYCLE "),
      fill_color="purple",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )
   
    self.Spring = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = gcompris.utils.load_pixmap("exercise/spring.jpg"),
      x = 100,
      y = 75,
	  width = 250,
      height = 170
      )
    gcompris.utils.item_focus_init(self.Spring, None)
    self.Spring.connect("button_press_event", self.on_button_press,"Spring")

    self.Summer = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = gcompris.utils.load_pixmap("exercise/summer.jpg"),
      x = 450,
      y = 75,
	  width = 250,
      height = 170
      )
    gcompris.utils.item_focus_init(self.Summer, None)
    self.Summer.connect("button_press_event", self.on_button_press,"Summer")

    self.Autumn = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = gcompris.utils.load_pixmap("exercise/autumn.jpg"),
      x = 100,
      y = 300,
	  width = 250,
      height = 170
      )
    gcompris.utils.item_focus_init(self.Autumn, None)
    self.Autumn.connect("button_press_event", self.on_button_press,"Autumn")

    self.Winter = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = gcompris.utils.load_pixmap("exercise/winter.jpg"),
      x = 450,
      y = 300,
	  width = 250,
      height = 170
      )
    gcompris.utils.item_focus_init(self.Winter, None)
    self.Winter.connect("button_press_event", self.on_button_press,"Winter")

    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_REPEAT)
    gcompris.bar_location(gcompris.BOARD_WIDTH/2 - 90, -1, 0.6)
    gcompris.bar_set_level(self.gcomprisBoard)

    self.display_game()

  def end(self):
    print "exercise end"
    # Remove the root item removes all the others inside it
    self.rootitem.remove()


  def ok(self):
    print("exercise ok.")


  def repeat(self):
    print("exercise repeat.")


  #mandatory but unused yet
  def config_stop(self):
    pass

  # Configuration function.
  def config_start(self, profile):
    print("exercise config_start.")

  def key_press(self, keyval, commit_str, preedit_str):
    pass

  def pause(self, pause):
    print("exercise pause. %i" % pause)


  def set_level(self, level):
    self.gcomprisBoard.level = level
    gcompris.bar_set_level(self.gcomprisBoard)
    self.display_game()
    print("exercise set level. %i" % level)

