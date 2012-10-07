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

class Gcompris_intro_gravity:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):

    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard
    self.gcomprisBoard.level = 1
    self.gcomprisBoard.maxlevel = 4

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    self.board_paused = False

    # Set the buttons we want in the bar
    gcompris.bar_set(0)
    self.game()

  def game(self):
    self.game_completed = False

    self.rootitem = goocanvas.Group(parent = self.gcomprisBoard.canvas.get_root_item())

    # Set a background image
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                           "intro_gravity/background.svg")

    # Load planet on the left (saturn) and it's slider
    planet_left = Fixed_planet(self.rootitem,
                                    70, 200, "saturn.png")
    Slider(self.rootitem, self, 20, 200, planet_left)

    # Planet on right (neptune) and it's slider
    planet_right = Fixed_planet(self.rootitem,
                                     680, 200, "neptune.png")
    Slider(self.rootitem, self, 780, 200, planet_right)

    # Load the tux_ship
    self.ship_instance = Spaceship(self, self.rootitem,
                              gcompris.BOARD_WIDTH/2.0, 200,
                              self.gcomprisBoard.level,
                              planet_left,
                              planet_right)
    # Set the buttons we want in the bar
    gcompris.bar_set(0)
    gcompris.bar_location(2,-1,0.5)

    # Message button
    self.message = Message(self.rootitem, 400, 320, 20, self.tuto_event)

    self.tuto_step = 0
    self.tutorial(self.tuto_step)

  def tuto_event(self, widget = None, target = None, event = None):
    self.tuto_step += 1
    self.tutorial(self.tuto_step)

  def tutorial(self, step):
    """Display a tutorial message for the given step"""
    if step == 0:
      self.board_paused = True
      self.message.show(
        _("Gravity is universal and Newton's law of universal gravitation extends gravity beyond earth."
          " This force of gravitational attraction is directly dependent upon the masses of both objects"
          " and inversely proportional to the square of the distance that separates their centers.")
        )
    elif step == 1:
      self.board_paused = True
      self.message.show(
        _("Since the gravitational force is directly proportional to the"
          " mass of both interacting objects, more massive objects will"
          " attract each other with a greater gravitational force."
          " So as the mass of either object increases, the force of"
          " gravitational attraction between them also increases but"
          " this force is inversely proportional to the square of the"
          " separation distance between the two interacting objects,"
          " more separation distance will result in weaker gravitational"
          " forces.")
        )
    elif step == 2:
      self.board_paused = True
      self.message.show(
        _("You goal is to let Tux's spaceship move by changing the mass"
          " of its surrounding planets. Don't get too close to the planets"
          " or you will crash on them."
          " The arrow indicates the direction of the force on your ship.")
        )
    elif step == 3:
      self.board_paused = False
      self.message.hide()
    elif step == 4:
      self.board_paused = True
      self.message.show( _("Take care you an in danger, avoid the asteroids.") )
    elif step == 5:
      self.board_paused = False
      self.message.hide()
      Asteroids(self.ship_instance, self.rootitem)


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

class Spaceship(Gcompris_intro_gravity):
  """Class representing the spaceship"""


  def __init__(self, game, rootitem, x, y, level,
               planet_left, planet_right):
    self.game = game
    self.rootitem = rootitem
    self.level = level
    # This counts how much space travel the children did
    # Let us determine a success case
    self.trip_distance = 0


    # load spaceship
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


    # load arrows for force applied on spacehip
    point = goocanvas.Points([(x - 50, y),(x - 90, y)])
    self.force_line = goocanvas.Polyline(
      parent = self.rootitem,
      points = point,
      stroke_color_rgba = 0xFFFFFFFFL,
      end_arrow = True,
      line_width = 0.5)

    # Set to true to stop the calculation
    self.done = False
    self.move = 0

    gobject.timeout_add(30, self.calculate)

  def calculate(self):
    if self.done:
      return False

    if self.game.board_paused:
      return True

    (x, y) = self.rootitem.get_canvas().\
        convert_from_item_space( self.tux_spaceship,
                                 self.tux_spaceship.props.x,
                                 self.tux_spaceship.props.y)
    dist_planet_left = abs ( x - self.planet_left.x )
    dist_planet_right = abs ( self.planet_right.x - x )
    self.move += ( ( self.planet_right.scale / dist_planet_right**2 ) -
                   ( self.planet_left.scale / dist_planet_left**2 ) ) * 200.0 * self.level
    self.tux_spaceship.translate(self.move, 0)

    force_l = (self.planet_left.scale / dist_planet_left**2) * 10**5
    force_r = (self.planet_right.scale / dist_planet_right**2) * 10**5

    # Manage force direction and intensity
    if force_r > force_l:
      if force_r < 3:
        right_force_intensity = force_r *2
      else:
        right_force_intensity = 6
      p = goocanvas.Points([(450, 200),(490, 200)])
      self.force_line.set_properties(points = p, line_width = right_force_intensity)

    else:
      if force_l < 3:
        left_force_intensity = force_l *2
      else:
        left_force_intensity = 6
      p = goocanvas.Points([(350, 200),(310, 200)])
      self.force_line.set_properties(points = p,line_width = left_force_intensity)

    self.force_line.translate(self.move, 0)

    # Manage the crash case
    if  x - self.planet_left.x < 60:
      self.crash()
    elif self.planet_right.x - x < 60:
      self.crash()
    # Manage the success case
    self.trip_distance += abs(self.move)

    if self.trip_distance > 100:
      # Let's move to the next step in the tutorial
      self.game.tuto_event()

    return True

  def crash(self):
    self.done = True
    self.game.crash()

class Asteroids:
  """Class for the asteroids"""

  def __init__(self, ship_instance, rootitem):
    self.ship_instance = ship_instance
    self.rootitem = rootitem
    self.load_asteroid()

  def load_asteroid(self):
    self.count = 1
    self.asteroid_rootitem = goocanvas.Group(parent = self.rootitem)

    # Make sure the asteroids are loaded between the planet and spaceship
    bounds = self.ship_instance.tux_spaceship.get_bounds()
    left_asteroid_x = random.uniform(150, bounds.x1 - 50)
    right_asteroid_x = random.uniform(450, bounds.x2 + 20)
    left_asteroid_y = 550
    right_asteroid_y = -20

    # Pick a random asteroid and load image
    asteroid_number = [0, 1, 2, 3, 4]
    asteroid = random.choice(asteroid_number)
    image = "intro_gravity/asteroid" + str(asteroid) + ".jpg"
    self.asteroid1 = goocanvas.Image(
      parent = self.asteroid_rootitem,
      pixbuf = gcompris.utils.load_pixmap(image),
      x = left_asteroid_x,
      y = left_asteroid_y)

    # Make sure same asteroid is not picked
    asteroid_number.remove(asteroid)
    asteroid = random.choice(asteroid_number)
    image = "intro_gravity/asteroid" + str(asteroid) + ".jpg"
    self.asteroid2 = goocanvas.Image(
      parent = self.asteroid_rootitem,
      pixbuf = gcompris.utils.load_pixmap(image),
      x = right_asteroid_x,
      y = right_asteroid_y)

    self.asteroid_rootitem.lower(self.ship_instance.tux_spaceship)
    gobject.timeout_add(30, self.check_asteroid)

  def get_real_bounds_center(self, item):
    b = item.get_bounds()
    return (b.x1 + (b.x2 - b.x1) / 2, b.y1 + (b.y2 - b.y1) / 2)

  def check_asteroid(self):
    if self.ship_instance.game.board_paused:
      return True

    # Move asteroids
    self.asteroid1.translate(0, -0.05)
    self.asteroid2.translate(0, 0.09)

    # Check whether ship and asteroid have collided
    (bound1_x, bound1_y) = self.get_real_bounds_center(self.asteroid1)
    (bound2_x, bound2_y) = self.get_real_bounds_center(self.asteroid2)
    (bound_ship_x, bound_ship_y) = self.get_real_bounds_center(self.ship_instance.tux_spaceship)

    if abs(bound1_x - bound_ship_x) < 40 and abs(bound1_y - bound_ship_y) < 40:
      self.crash_image(bound_ship_x, bound_ship_y)
      return False
    elif abs(bound2_x - bound_ship_x) < 40 and abs(bound2_y - bound_ship_y) < 40:
      self.crash_image(bound_ship_x, bound_ship_y)
      return False
    else:
      if bound2_y > 300:
        self.ship_instance.done = True
        self.ship_instance.game.win()
        return False

    return True

  def crash_image(self, x, y):
    image = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = gcompris.utils.load_pixmap('/intro_gravity/crash.png'),
      x = x - 50,
      y = y - 50)

    self.ship_instance.crash()

class Fixed_planet:
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

class Slider:
  """ class for scale slider"""


  def __init__(self, rootitem, game, x, y, planet_instance):
    self.game = game
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
    if self.game.board_paused:
      return

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

class Message:
  """Create a message on screen"""

  def __init__(self, rootitem, x, y, gap, tuto_event):
    self.rootitem = goocanvas.Group(parent = rootitem)
    self.gap = gap
    self.text = goocanvas.Text(
      parent = self.rootitem,
      x = x,
      y = y,
      width = 500,
      fill_color = "white",
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER,
      text = "")

    self.back = goocanvas.Rect(
      parent = self.rootitem,
      radius_x = 6,
      radius_y = 6,
      stroke_color_rgba = 0xFFFFFFFFL,
      fill_color_rgba = 0x666666AAL)
    self.back.lower(None)
    self.hide()
    self.back.connect("button_press_event", tuto_event)
    self.text.connect("button_press_event", tuto_event)

  def _refreshBack(self):
    bounds = self.text.get_bounds()
    self.back.set_properties(
      x = bounds.x1 - self.gap,
      y = bounds.y1 - self.gap,
      width = bounds.x2 - bounds.x1 + self.gap * 2,
      height = bounds.y2 - bounds.y1 + self.gap * 2)
    gcompris.utils.item_focus_init(self.back, None)
    gcompris.utils.item_focus_init(self.text, self.back)

  def hide(self):
    self.rootitem.props.visibility = goocanvas.ITEM_INVISIBLE

  def show(self, msg):
    self.text.props.text = msg
    self._refreshBack()
    self.rootitem.props.visibility = goocanvas.ITEM_VISIBLE

