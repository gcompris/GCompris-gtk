#  gcompris - Land_Safe.py
#
# Copyright (C) 2003, 2008 Bruno Coudoin and Matilda Bernard
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
# Land_Safe activity.
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import goocanvas
import pango
import gobject
import random

from gcompris import gcompris_gettext as _

class Gcompris_Land_Safe:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard
    self.gcomprisBoard.level = 1
    self.gcomprisBoard.maxlevel = 2

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    gcompris.bar_set_level(self.gcomprisBoard)

    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())

    # Set a background image
    level = str(self.gcomprisBoard.level)
    image = 'Land_Safe/background'+level+'.png'
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            image)

    # Load spaceship
    self.space_ship = Spaceship(self,
                                self.rootitem,
                                self.gcomprisBoard.level)

  def end(self):
    # Remove the root item removes all the others inside it
    self.rootitem.remove()

  def ok(self):
    pass

  def repeat(self):
    pass

  # mandatory but unused yet
  def config_stop(self):
    pass

  # Configuration function.
  def config_start(self, profile):
    print("intro_gravity config_start.")

  def key_press(self, keyval, commit_str, preedit_str):
    self.space_ship.handle_key(keyval)

  def pause(self, pause):
    self.board_paused = pause
    if pause == 0:
      self.end()
      self.start()

  def set_level(self, level):
    self.gcomprisBoard.level = level
    gcompris.bar_set_level(self.gcomprisBoard)
    self.end()
    self.start()

  def crash(self):
    gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.TUX)

  def win(self):
    self.next_level()
    gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.TUX)

  def next_level(self):
    if self.gcomprisBoard.level < self.gcomprisBoard.maxlevel:
      self.gcomprisBoard.level +=1
    else:
      self.gcomprisBoard.level = 1

class Spaceship:
  """Class for the spaceship"""


  def __init__(self, game_instance, rootitem, level):
    self.game = game_instance
    self.rootitem = rootitem
    self.level = level
    self.subrootitem = goocanvas.Group(parent = rootitem)

    x = gcompris.BOARD_WIDTH/2
    y = 10

    # Load flames
    self.flame(x + 15, y)
    self.flame_1.props.visibility = goocanvas.ITEM_INVISIBLE
    self.flame_2.props.visibility = goocanvas.ITEM_INVISIBLE
    self.flame_3.props.visibility = goocanvas.ITEM_INVISIBLE

    pixbuf = gcompris.utils.load_pixmap("Land_Safe/rocket.png")
    self.spaceship_image = goocanvas.Image(
      parent = rootitem,
      pixbuf = pixbuf,
      x = x,
      y = y
      )

    self.y = 0.005
    self.x = 0
    self.key_vertical = 4

    # Load fuel, altitude and landing area
    self.info = Display(self, rootitem)

    # incase of landing return false
    gobject.timeout_add(30, self.spaceship_movement)

  def handle_key(self, key):
    if key == gtk.keysyms.Left:
      self.x -= 0.05

    elif key == gtk.keysyms.Right:
      self.x += 0.05

    elif key == gtk.keysyms.Up:
      if self.key_vertical > 1:
        self.key_vertical -= 1

    elif key == gtk.keysyms.Down:
      if self.key_vertical < 4:
        self.key_vertical += 1

  def spaceship_movement(self):
    if self.info.increase_vel():
      self.key_vertical = 4

    # handle increase in velocity and flame
    if self.key_vertical == 1:
      self.flame_3.props.visibility = goocanvas.ITEM_VISIBLE
      self.y -= 0.005

    elif self.key_vertical == 2:
      self.flame_3.props.visibility = goocanvas.ITEM_INVISIBLE
      self.flame_2.props.visibility = goocanvas.ITEM_VISIBLE
      self.y -= 0.002

    elif self.key_vertical == 3:
      self.flame_2.props.visibility = goocanvas.ITEM_INVISIBLE
      self.flame_3.props.visibility = goocanvas.ITEM_INVISIBLE
      self.flame_1.props.visibility = goocanvas.ITEM_VISIBLE
      self.y +=0.002 * self.level

    elif self.key_vertical == 4:
      self.flame_1.props.visibility = goocanvas.ITEM_INVISIBLE
      self.flame_2.props.visibility = goocanvas.ITEM_INVISIBLE
      self.flame_3.props.visibility = goocanvas.ITEM_INVISIBLE
      self.y += 0.005 * self.level

    # move spaceship and flame
    position = self.spaceship_image.get_bounds().y1
    if  position < 365:
      self.spaceship_image.translate(self.x, self.y)
      self.subrootitem.translate(self.x, self.y)
      self.info.altitude(position)
      self.info.set_velocity()
      return True

    else:
      self.info.altitude(364.000)
      self.subrootitem.remove()
      self.info.stop_fuel()
      self.check_landing()

  def check_landing(self):
    x = self.spaceship_image.get_bounds().x1
    land_x = self.info.get_area()
    if land_x < x < land_x + 100 and self.y < 0.7:
      self.game.win()
    else:
      self.crash_image()

  def crash_image(self):
    x = self.spaceship_image.get_bounds().x1
    self.spaceship_image.remove()
    pixbuf = gcompris.utils.load_pixmap("Land_Safe/crash.png")
    explosion = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = pixbuf,
      x = x - 20,
      y = 340
      )

    self.game.crash()

  def flame(self,x,y):
    pixbuf = gcompris.utils.load_pixmap("Land_Safe/flame1.png")
    self.flame_1 = goocanvas.Image(
      parent = self.subrootitem,
      pixbuf = pixbuf,
      x = x,
      y = y + 68
      )

    pixbuf = gcompris.utils.load_pixmap("Land_Safe/flame2.png")
    self.flame_2 = goocanvas.Image(
      parent = self.subrootitem,
      pixbuf = pixbuf,
      x = x,
      y = y + 65
      )

    pixbuf = gcompris.utils.load_pixmap("Land_Safe/flame3.png")
    self.flame_3 = goocanvas.Image(
      parent = self.subrootitem,
      pixbuf = pixbuf,
      x = x,
      y = y + 65
      )

class Display:
  """class for displays"""


  def __init__(self, ship_instance, rootitem):
    # text for altitude
    self.rootitem = rootitem
    text = goocanvas.Text(
      parent = rootitem,
      x = 20,
      y = 20,
      fill_color = "white",
      text = _("Altitude : "))

    # initiate text for altitude
    self.alt_text = goocanvas.Text(
      parent = rootitem,
      x = 100,
      y = 20,
      fill_color = "white",
      text = _('start'))

    # text for fuel display
    fuel_text = goocanvas.Text(
      parent = rootitem,
      x = 20,
      y = 40,
      fill_color = "white",
      text = _('Fuel'))

    # fuel tank
    rectangle = goocanvas.Rect(
      parent = rootitem,
      radius_x = 10,
      radius_y = 10,
      x=65.0,
      y=40.0,
      width=100,
      height=20,
      stroke_color="blue")

    # initial fuel in tank
    self.fuel_amt = 100
    self.fuel_tank = goocanvas.Rect(
      parent = rootitem,
      radius_x = 10,
      radius_y = 10,
      x=65.0,
      y=40.0,
      width=self.fuel_amt,
      height=20,
      fill_color="blue")

    # Load landing area
    self.land_x = random.randrange(10, 700)
    landing_area = goocanvas.Rect(
      parent = rootitem,
      x=self.land_x,
      y=440,
      width=100,
      height=6,
      fill_color="green",
      stroke_color="green")

    # text for velocity
    velocity_text = goocanvas.Text(
      parent = rootitem,
      x = 20,
      y = 60,
      fill_color = "white",
      text = _('Velocity : '))

    # display velocity
    self.velocity = goocanvas.Text(
      parent = rootitem,
      x = 100,
      y = 60,
      fill_color = 'white',
      text = _('4'))

    self.ship_instance = ship_instance
    self.key = 0
    self.stop_consumtion = False
    self.set_fuel_time()

  def altitude(self, altitude): #display current altitude
    self.alt = str(364-altitude)
    self.alt_text.set_property('text', self.alt)

    self.key = self.ship_instance.key_vertical
    self.vel = self.ship_instance.y

  def set_fuel_time(self):
    if self.stop_consumtion == True:
      return False

    if self.key == 3:
      gobject.timeout_add(1000, self.fuel)

    elif self.key == 2:
      gobject.timeout_add(500, self.fuel)

    elif self.key == 1:
      gobject.timeout_add(250, self.fuel)

    else:
      gobject.timeout_add(30, self.set_fuel_time)

  def fuel(self):
    self.fuel_amt -= 1
    self.fuel_tank.set_property('width', self.fuel_amt)

    if self.fuel_amt != 1:
      self.set_fuel_time()

  def increase_vel(self):
    if self.fuel_amt == 1:
      return True

  def stop_fuel(self):
    self.stop_consumtion = True

  def get_area(self):
    return self.land_x

  def set_velocity(self):
    self.velocity.set_property('text',self.vel * 10)

