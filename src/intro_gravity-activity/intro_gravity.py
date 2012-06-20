#  gcompris - intro_gravity.py
#
# Copyright (C) 2012 Matilda Bernard and Bruno Coudoin
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
# intro_gravity activity.
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.anim
import goocanvas
import pango
import gcompris.bonus
import gobject
from gcompris import gcompris_gettext as _

class Gcompris_intro_gravity:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):

    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard
    self.gcomprisBoard.level = 1
    self.gcomprisBoard.sublevel = 1
    self.gcomprisBoard.number_of_sublevel = 1
    self.gcomprisBoard.maxlevel = 4

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    print "intro_gravity start"

    # Set the buttons we want in the bar
    gcompris.bar_set(0)
    self.game()

  def game(self):
    self.game_completed = False

    self.rootitem = goocanvas.Group(parent = self.gcomprisBoard.canvas.get_root_item())

    goocanvas.Text(
      parent = self.rootitem,
      x = gcompris.BOARD_WIDTH/2.0,
      y = 50.0,
      text = _("Mass is directly proportional to gravitational force"),
      font = gcompris.skin.get_font("gcompris/subtitle"),
      fill_color = "white",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )

    # Set a background image
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                           "intro_gravity/background.svg")

    # Load planet on the left (saturn) and it's slider
    planet_left = fixed_planet(self.rootitem,
                                    80, 200, "saturn.png")
    slider(self.rootitem, 20, 200, planet_left)

    # Planet on right (neptune) and it's slider
    planet_right = fixed_planet(self.rootitem,
                                     630, 200, "neptune.png")
    slider(self.rootitem, 780, 200, planet_right)

    # Load the tux_ship
    ship_instance = spaceship(self, self.rootitem,
                              gcompris.BOARD_WIDTH/2.0, 200,
                              self.gcomprisBoard.level,
                              planet_left,
                              planet_right)

  def end(self):
    # Remove the root item removes all the others inside it
    self.rootitem.remove()

  def ok(self):
    pass

  def repeat(self):
    pass

  #mandatory but unused yet
  def config_stop(self):
    pass

  # Configuration function.
  def config_start(self, profile):
    print("intro_gravity config_start.")

  def key_press(self, keyval, commit_str, preedit_str):
    pass

  def pause(self, pause):
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    # the game is won
    self.board_paused = pause
    if pause == 0 and self.game_completed:
      self.end()
      self.game()

  def set_level(self, level):
    self.gcomprisBoard.level = level;
    self.game()

  def next_level(self):
    if self.gcomprisBoard.level <= self.gcomprisBoard.maxlevel:
      self.gcomprisBoard.level += 1

  def crash(self):
    self.game_completed = True
    gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.TUX)

  def win(self):
    self.game_completed = True
    self.next_level()
    gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.TUX)

class spaceship(Gcompris_intro_gravity):
  """Class representing the spaceship"""

  # load spaceship
  def __init__(self, game, rootitem, x, y, level,
               planet_left, planet_right):
    self.game = game
    self.rootitem = rootitem
    self.level = level

    # This counts how much space travel the children did
    # Let us determine a success case
    self.trip_distance = 0

    self.tux_spaceship = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = gcompris.utils.load_pixmap("intro_gravity/tux_spaceship.png"),
      x = x,
      y = y)
    # Center it
    bounds = self.tux_spaceship.get_bounds()
    self.tux_spaceship.translate( (bounds.x2 - bounds.x1) / 2.0 * -1,
                                  (bounds.y2 - bounds.y1) / 2.0 * -1)

    self.planet_right = planet_right
    self.planet_left = planet_left

    # Set to true to stop the calculation
    self.done = False
    self.move = 0

    gobject.timeout_add(30, self.calculate)

  def calculate(self):
    if self.done:
      return False

    (x, y) = self.rootitem.get_canvas().\
        convert_from_item_space( self.tux_spaceship,
                                 self.tux_spaceship.props.x,
                                 self.tux_spaceship.props.y)
    dist_planet_left = abs ( x - self.planet_left.x )
    dist_planet_right = abs ( self.planet_right.x - x )
    self.move += ( ( self.planet_right.scale / dist_planet_right**2 ) -
                   ( self.planet_left.scale / dist_planet_left**2 ) ) * 200.0
    self.tux_spaceship.translate(self.move, 0)

    # Manage the crash case
    if  x - self.planet_left.x < 10:
      self.crash()
    elif self.planet_right.x - x < 10:
      self.crash()
    # Manage the success case
    self.trip_distance += abs(self.move)
    if self.trip_distance > 500 * self.level:
      self.done = True
      self.game.win()

    return True

  def crash(self):
    self.done = True
    print "Crash !!!"
    self.game.crash()

class fixed_planet:
  """ Fixed planets """

  def __init__(self, rootitem, x, y, planet_image):
    self.rootitem = rootitem
    self.scale = 0
    self.x = x
    self.y = y
    self.planet = goocanvas.Image(
      parent = rootitem,
      pixbuf = gcompris.utils.load_pixmap("intro_gravity/"
                                          + planet_image),
      x = 0,
      y = 0)

  def set_scale(self, scale):
    self.scale = scale
    self.planet.set_transform(None)
    # Center it
    bounds = self.planet.get_bounds()
    self.planet.scale(self.scale + 0.5, self.scale + 0.5)
    (x, y) = self.rootitem.get_canvas().\
        convert_to_item_space(\
      self.planet,
      self.x + ((bounds.x2 - bounds.x1) / 2.0 * -1) * self.scale + 0.5,
      self.y + ((bounds.y2 - bounds.y1) / 2.0 * -1) * self.scale + 0.5)
    self.planet.translate( x, y )

class slider:
  """ class for scale slider"""

  def __init__(self, rootitem, x, y, planet_instance):
    self.planet_instance = planet_instance
    self.height = 60
    self.button_width = 20
    self.x = x
    self.y = y
    self.rootitem = goocanvas.Group(parent = rootitem)
    line = goocanvas.Polyline(
      parent = self.rootitem,
      points = goocanvas.Points( [(x, y + self.button_width / 2.0),
                                  (x, y + self.button_width / 2.0 + self.height)] ),
      stroke_color = "grey",
      width = 2.0)

    # This is the relative position of the scale from 0 to 1
    # 0 is the bottom
    self.scale_value = 0.5
    slider_y = y + self.height / 2.0
    self.sliding_bar( goocanvas.Points([(x - 5, slider_y),
                                        (x + 5, slider_y)] ) )
    self.button(self.x, self.y, self.button_width, '+', 0.1)
    self.button(self.x, self.y + self.height, self.button_width, '-', -0.1)

    self.planet_instance.set_scale( self.scale_value )

  def button(self, x, y, size, text, move):
    button = goocanvas.Rect(
      parent = self.rootitem,
      x = x - size / 2.0,
      y = y - size / 2.0,
      width = size,
      height =  size,
      line_width = 1.0,
      stroke_color_rgba= 0xCECECEFFL,
      fill_color_rgba = 0x333333FFL,
      radius_x = 15.0,
      radius_y = 5.0,
      )
    gcompris.utils.item_focus_init(button, None)

    text = goocanvas.Text(
      parent = self.rootitem,
      x = x,
      y = y,
      text = text,
      font = gcompris.skin.get_font("gcompris/subtitle"),
      fill_color = "white",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER
      )
    gcompris.utils.item_focus_init(text, button)

    button.connect("button_press_event", self.move_bar, move)
    text.connect("button_press_event", self.move_bar, move)

  def sliding_bar(self, points):
    self.bar = goocanvas.Polyline(
      parent = self.rootitem,
      points = points,
      stroke_color = "grey",
      line_width = 5.0)

  def move_bar(self, widget, target, event, move):
    self.scale_value += move
    # Take care not to bypass bounds
    if self.scale_value > 1.0:
      self.scale_value = 1.0
      return
    elif self.scale_value < 0.0:
      self.scale_value = 0.0
      return

    self.bar.translate(0, move * -1 * self.height / 4.0);

    # Change the planet mass
    self.planet_instance.set_scale(self.scale_value)

