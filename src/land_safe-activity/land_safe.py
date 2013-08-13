#  gcompris - land_safe.py
#
# Copyright (C) 2012 Bruno Coudoin and Matilda Bernard
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
# land_safe activity.
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import goocanvas
import gobject
import random
import pango

from gcompris import gcompris_gettext as _

class Gcompris_land_safe:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard
    self.gcomprisBoard.level = 1
    self.gcomprisBoard.maxlevel = 4

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

    self.game_complete = False

  def start(self):
    self.board_paused = False
    self.game_start = False
    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())
    land_rootitem = goocanvas.Group(parent = self.rootitem)

    # Set a background image
    level = str(self.gcomprisBoard.level)
    image = 'land_safe/background'+level+'.jpg'
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            image)

    #Set the land
    image = 'land_safe/land'+str(level)+'.png'
    pixbuf = gcompris.utils.load_pixmap(image)
    land = goocanvas.Image(
      parent = land_rootitem,
      pixbuf = pixbuf,
      x = -550,
      y = gcompris.BOARD_HEIGHT - 125
      )
    land.lower(None)

    # Text for Gravity
    gravity = str(0.58 * self.gcomprisBoard.level)
    text = _('Gravity: %s') % (gravity)
    gravity_text = goocanvas.Text(
      parent = self.rootitem,
      x = 760,
      y = 50,
      fill_color = "white",
      font = gcompris.skin.get_font("gcompris/board/small"),
      anchor = gtk.ANCHOR_E,
      alignment = pango.ALIGN_CENTER,
      text = _(text))
    bounds = gravity_text.get_bounds()
    gap = 20

    gravity_back = goocanvas.Rect(
      parent = self.rootitem,
      radius_x = 6,
      radius_y = 6,
      x = bounds.x1 - gap,
      y = bounds.y1 - gap,
      width = bounds.x2 - bounds.x1 + gap * 2,
      height = bounds.y2 - bounds.y1 + gap * 2,
      stroke_color_rgba = 0xFFFFFFFFL,
      fill_color_rgba = 0xCCCCCC44L)

    # Load spaceship
    self.space_ship = Spaceship(self,
                                self.rootitem,
                                land_rootitem,
                                self.gcomprisBoard.level)

    gcompris.bar_set_level(self.gcomprisBoard)

    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.bar_location(2,-1,0.5)

    self.ready_button(self.rootitem)

  def end(self):
    # Remove the root item removes all the others inside it
    self.rootitem.remove()
    self.space_ship.set_done()

  def ok(self):
    gcompris.utils.dialog_close()

  def repeat(self):
    pass

  # mandatory but unused yet
  def config_stop(self):
    pass

  # Configuration function.
  def config_start(self, profile):
    print("intro_gravity config_start.")

  def key_press(self, keyval, commit_str, preedit_str):
    if self.game_start == True or keyval == gtk.keysyms.Return:
      self.space_ship.handle_key(keyval)

  def pause(self, pause):
    self.board_paused = pause
    if pause == False and self.game_complete:
      self.game_complete = False
      self.end()
      self.start()

  def set_level(self, level):
    self.gcomprisBoard.level = level
    gcompris.bar_set_level(self.gcomprisBoard)
    self.end()
    self.start()

  def crash(self):
    self.game_complete = True
    gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.TUX)

  def win(self):
    self.next_level()
    self.game_complete = True
    gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.TUX)

  def next_level(self):
    if self.gcomprisBoard.level < self.gcomprisBoard.maxlevel:
      self.gcomprisBoard.level += 1

  def ready_button(self, rootitem):
    if self.gcomprisBoard.level == 1:
     intro = _("Use the up and down keys to control the thrust"
               "\nUse the right and left keys to control direction."
               "\nYou must drive Tux's ship towards the landing platform."
               "\nThe landing platform turns green when the velocity is safe to land")
     intro += "\n\n"
     intro += _("Click on me or press the Return key when you are ready.")
    else:
      intro = _('I am ready!')

    # Ready button
    self.ready_text = goocanvas.Text(
      parent = rootitem,
      x = 384,
      y = 203,
      fill_color = "white",
      font = gcompris.skin.get_font("gcompris/board/small"),
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER,
      text = intro )
    self.ready_text.connect('button_press_event', self.ready_event, False)
    bounds = self.ready_text.get_bounds()
    gap = 20

    self.ready_back = goocanvas.Rect(
      parent = rootitem,
      radius_x = 6,
      radius_y = 6,
      x = bounds.x1 - gap,
      y = bounds.y1 - gap,
      width = bounds.x2 - bounds.x1 + gap * 2,
      height = bounds.y2 - bounds.y1 + gap * 2,
      stroke_color_rgba = 0xFFFFFFFFL,
      fill_color_rgba = 0xCCCCCC44L)
    gcompris.utils.item_focus_init(self.ready_back, None)
    gcompris.utils.item_focus_init(self.ready_text, self.ready_back)
    self.ready_back.connect('button_press_event', self.ready_event)

  def ready_event(self, widget, target, event):
    self.ready_back.props.visibility = goocanvas.ITEM_INVISIBLE
    self.ready_text.props.visibility = goocanvas.ITEM_INVISIBLE
    self.game_start = True
    self.space_ship.initiate()

class Spaceship:
  """Class for the spaceship"""


  def __init__(self, game_instance, rootitem, land_rootitem, level):
    self.rootitem = rootitem
    self.game = game_instance
    self.level = level
    self.flame_rootitem = goocanvas.Group(parent = rootitem)
    self.land_rootitem = land_rootitem
    self.land_rootitem.lower(self.flame_rootitem)
    x = gcompris.BOARD_WIDTH/2
    y = 10

    # Load flames and hide them
    self.flame(x + 15, y)
    self.flame_1.props.visibility = goocanvas.ITEM_INVISIBLE
    self.flame_2.props.visibility = goocanvas.ITEM_INVISIBLE
    self.flame_3.props.visibility = goocanvas.ITEM_INVISIBLE
    self.flame_left.props.visibility = goocanvas.ITEM_INVISIBLE
    self.flame_right.props.visibility = goocanvas.ITEM_INVISIBLE

    # Load landing area
    self.land_x = random.randrange(100, 400)
    self.landing_red = goocanvas.Image(
      parent = self.land_rootitem,
      pixbuf = gcompris.utils.load_pixmap("land_safe/landing_area_red.png"),
      width = 145 - (self.level * 12),
      x = self.land_x,
      y = 365)
    self.landing_red.props.visibility = goocanvas.ITEM_INVISIBLE

    self.landing_green = goocanvas.Image(
      parent = self.land_rootitem,
      pixbuf = gcompris.utils.load_pixmap("land_safe/landing_area_green.png"),
      width = 145 - (self.level * 12),
      x = self.land_x,
      y = 365)

    # Load spaceship
    pixbuf = gcompris.utils.load_pixmap("land_safe/rocket.png")
    self.spaceship_image = goocanvas.Image(
      parent = rootitem,
      pixbuf = pixbuf,
      x = x,
      y = y
      )

    # Declaring variables
    self.y = 0.005
    self.x = 0
    self.key_vertical = 4
    self.zoom_out = False
    self.scale = 1

    # Load fuel, altitude and landing area
    self.info = Display(self, rootitem)

    # Set to true to stop the timeout
    self.done = False

  def set_done(self):
    self.done = True
    self.info.done = True

  def initiate(self):
    # incase of landing return false
    gobject.timeout_add(30, self.spaceship_movement)

  def handle_key(self, key):
    if key == gtk.keysyms.Return and self.game.game_start == False:
      self.game.ready_event(1,2,3)

    if self.game.board_paused:
      return

    if key == gtk.keysyms.Left:
      self.x -= 0.05
      self.flame_right.props.visibility = goocanvas.ITEM_VISIBLE
      gobject.timeout_add(300,self.remove_flame,2)

    elif key == gtk.keysyms.Right:
      self.x += 0.05
      self.flame_left.props.visibility = goocanvas.ITEM_VISIBLE
      gobject.timeout_add(300,self.remove_flame,1)

    elif key == gtk.keysyms.Up:
      if self.key_vertical > 1:
        self.key_vertical -= 1

    elif key == gtk.keysyms.Down:
      if self.key_vertical < 4:
        self.key_vertical += 1

  def remove_flame(self,side):
    if self.done:
      return False

    if side == 1:
      self.flame_left.props.visibility = goocanvas.ITEM_INVISIBLE
    else:
      self.flame_right.props.visibility = goocanvas.ITEM_INVISIBLE

  def spaceship_movement(self):
    if self.done:
      return False

    if self.game.board_paused:
      return True

    if self.info.increase_vel():
      self.key_vertical = 4
      self.x = 0
      self.flame_rootitem.props.visibility = goocanvas.ITEM_INVISIBLE

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
    bounds = self.spaceship_image.get_bounds()
    if  bounds.y1 < 365:
      self.horizontal_limit(bounds.x1)
      self.screen_zoom(bounds.x1, bounds.y1)
      self.spaceship_image.translate(self.x * self.scale, self.y * self.scale)
      self.flame_rootitem.translate(self.x * self.scale, self.y * self.scale)
      self.info.altitude(bounds.y1,self.zoom_out)
      self.info.set_velocity()
      return True

    else:
      self.flame_rootitem.remove()
      self.info.stop_fuel()
      self.check_landing()
      return False

  def horizontal_limit(self, x):
    if self.zoom_out == False:
      x_translate = 750
    else:
      x_translate = 1500
    if x < 3:
      self.spaceship_image.translate(x_translate,0)
      self.flame_rootitem.translate(x_translate,0)
    elif x > 755:
      self.spaceship_image.translate(-x_translate,0)
      self.flame_rootitem.translate(-x_translate,0)

  def screen_zoom(self, x, y):
    if y < 10 and self.zoom_out == False:
      y_int = int(y/0.5 * 29)
      self.land_rootitem.scale(0.5, 0.5)
      self.land_rootitem.translate(300, 510)
      self.spaceship_image.scale(0.5,0.5)
      self.spaceship_image.translate(x, y_int)
      self.flame_rootitem.scale(0.5,0.5)
      self.flame_rootitem.translate(x, y_int)
      self.scale = 0.5
      self.zoom_out = True

    elif y > 298 and self.zoom_out == True:
      self.land_rootitem.set_transform(None)
      gcompris.utils.item_absolute_move(self.spaceship_image, int(x), 11)
      gcompris.utils.item_absolute_move(self.flame_rootitem, int(x - 12), 11 + 56)
      self.scale = 1
      self.zoom_out = False

  def check_landing(self):
    bounds = self.spaceship_image.get_bounds()
    x = (bounds.x1 + bounds.x2)/2
    width = 135 - (self.level * 5)
    if self.land_x < x < self.land_x + width and self.y < 0.8:
      self.game.win()
    else:
      self.crash_image()

  def crash_image(self):
    x = self.spaceship_image.get_bounds().x1
    self.spaceship_image.remove()
    pixbuf = gcompris.utils.load_pixmap("land_safe/crash.png")
    explosion = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = pixbuf,
      x = x - 20,
      y = 340
      )

    self.game.crash()

  def flame(self,x,y):
    pixbuf = gcompris.utils.load_pixmap("land_safe/flame1.png")
    self.flame_1 = goocanvas.Image(
      parent = self.flame_rootitem,
      pixbuf = pixbuf,
      x = x,
      y = y + 68
      )

    pixbuf = gcompris.utils.load_pixmap("land_safe/flame2.png")
    self.flame_2 = goocanvas.Image(
      parent = self.flame_rootitem,
      pixbuf = pixbuf,
      x = x,
      y = y + 65
      )

    pixbuf = gcompris.utils.load_pixmap("land_safe/flame3.png")
    self.flame_3 = goocanvas.Image(
      parent = self.flame_rootitem,
      pixbuf = pixbuf,
      x = x,
      y = y + 65
      )

    pixbuf = gcompris.utils.load_pixmap("land_safe/flame_left.png")
    self.flame_left = goocanvas.Image(
      parent = self.flame_rootitem,
      pixbuf = pixbuf,
      x = x - 27,
      y = y + 55
      )

    pixbuf = gcompris.utils.load_pixmap("land_safe/flame_right.png")
    self.flame_right = goocanvas.Image(
      parent = self.flame_rootitem,
      pixbuf = pixbuf,
      x = x + 25,
      y = y + 55
      )

class Display:
  """class for displays"""


  def __init__(self, ship_instance, rootitem):
    # Set to true to stop the timeout
    self.done = False

    # text for altitude
    self.rootitem = rootitem
    text = goocanvas.Text(
      parent = rootitem,
      x = 20,
      y = 20,
      font = gcompris.skin.get_font("gcompris/board/small"),
      fill_color = "white",
      text = _("Height: "))

    # initiate text for altitude
    self.alt_text = goocanvas.Text(
      parent = rootitem,
      x = 100,
      y = 20,
      font = gcompris.skin.get_font("gcompris/board/small"),
      fill_color = "white",
      text = '')

    # text for fuel display
    fuel_text = goocanvas.Text(
      parent = rootitem,
      x = 20,
      y = 40,
      font = gcompris.skin.get_font("gcompris/board/small"),
      fill_color = "white",
      text = _('Fuel'))

    # fuel tank
    bounds = fuel_text.get_bounds()
    rectangle = goocanvas.Rect(
      parent = rootitem,
      radius_x = 5,
      radius_y = 5,
      x = bounds.x2 + 5,
      y = 40.0,
      width = 100,
      height = 20,
      stroke_color = "grey")

    # initial fuel in tank
    self.fuel_amt = 96
    self.fuel_tank = goocanvas.Rect(
      parent = rootitem,
      radius_x = 5,
      radius_y = 5,
      x = bounds.x2 + 6,
      y = 41.3,
      width = self.fuel_amt,
      height = 16.5,
      fill_color = "blue",
      stroke_color = "blue")

    # text for velocity
    velocity_text = goocanvas.Text(
      parent = rootitem,
      x = 20,
      y = 60,
      font = gcompris.skin.get_font("gcompris/board/small"),
      fill_color = "white",
      text = _('Velocity: '))

    # display velocity
    self.velocity = goocanvas.Text(
      parent = rootitem,
      x = 100,
      y = 60,
      font = gcompris.skin.get_font("gcompris/board/small"),
      fill_color = 'white',
      text = '')

    self.ship_instance = ship_instance
    self.key = 0
    self.stop_consumtion = False
    self.set_fuel_time()


  def altitude(self, altitude, zoom): #display current altitude
    if zoom == False:
      self.alt = str(int(364 - altitude))
    else:
      self.alt = str(int(700 - altitude))
    self.alt_text.set_property('text', self.alt)

    self.key = self.ship_instance.key_vertical
    self.vel = self.ship_instance.y

  def set_fuel_time(self):
    if self.done:
      return False

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
    if self.done:
      return False

    if self.ship_instance.level == 1:
      self.fuel_amt -= 1 * 0.5
    else:
      self.fuel_amt -= 1
    self.fuel_tank.set_property('width', self.fuel_amt)

    if self.fuel_amt != 1:
      self.set_fuel_time()

  def increase_vel(self):
    if self.fuel_amt == 1:
      self.fuel_tank.remove()
      return True

  def stop_fuel(self):
    self.stop_consumtion = True

  def set_velocity(self):
    self.velocity.set_property('text',int(self.vel * 10))
    if int(self.vel * 10) < 8:
      self.ship_instance.landing_red.props.visibility = goocanvas.ITEM_INVISIBLE
      self.ship_instance.landing_green.props.visibility = goocanvas.ITEM_VISIBLE
    else:
      self.ship_instance.landing_green.props.visibility = goocanvas.ITEM_INVISIBLE
      self.ship_instance.landing_red.props.visibility = goocanvas.ITEM_VISIBLE

