#  gcompris - color_mix.py
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
# color_mix activity.
import gtk
import gtk.gdk
import random
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import goocanvas
import pango

from gcompris import gcompris_gettext as _

class Gcompris_color_mix:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    self.game_complete = False

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    # Set the buttons we want in the bar
    gcompris.bar_set(0)
    gcompris.bar_location(2,-1,0.5)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())

    # Set a background image
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            'color_mix/background.jpg')

    text = goocanvas.Text(
      parent = self.rootitem,
      x = 250,
      y = 80,
      fill_color = "black",
      font = gcompris.skin.get_font("gcompris/subtitle"),
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER,
      text = _("Match the \n color "))

    # Set the points for the sliders
    c_points = goocanvas.Points( [(245, 212), (130, 175)] )
    m_points = goocanvas.Points( [(390, 355), (390, 465)] )
    y_points = goocanvas.Points( [(550, 212), (665, 175)] )

    colors = Colors(self, self.rootitem)
    cyan_tube = Color_tubes(self.rootitem, colors, 'cyan_tube.png',
                            1, 80, 120, c_points, 245, 212, 130, 175)
    magenta_tube = Color_tubes(self.rootitem, colors, 'magenta_tube.png',
                               2, 350, 290, m_points, 390, 355, 390, 465)
    yellow_tube = Color_tubes(self.rootitem, colors, 'yellow_tube.png',
                              3, 460, 120, y_points, 550, 212, 665, 175)

  def game_over(self, result):
    self.game_complete = True
    if result == 1:
      gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.TUX)
    else:
      gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.TUX)

  def end(self):
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
    pass

  def key_press(self, keyval, commit_str, preedit_str):
    pass

  def pause(self, pause):
    self.board_paused = pause
    if pause == False and self.game_complete:
      self.game_complete = False
      self.end()
      self.start()

  def set_level(self, level):
    pass

class Color_tubes:
  """ Class containing the three primary color tubes"""


  def __init__(self, rootitem, color_instance, image, primary_color, x, y, points, incr_x, incr_y, decr_x, decr_y):
    self.rootitem = rootitem
    self.primary_color = primary_color
    self.color_instance = color_instance

    # Load the tube image
    image = 'color_mix/' + image
    pixbuf = gcompris.utils.load_pixmap(image)
    tube = goocanvas.Image(
      parent = self.rootitem,
      pixbuf = pixbuf,
      x = x,
      y = y
      )

    button_width = 20
    line = goocanvas.Polyline(
      parent = self.rootitem,
      points = points,
      stroke_color = "white",
      width = 2.0)

    slider_x = (incr_x + decr_x) / 2
    slider_y = (incr_y + decr_y) / 2
    self.bar = goocanvas.Polyline(
      parent = self.rootitem,
      points = goocanvas.Points([(decr_x + 10, decr_y - 5),
                                 (decr_x + 10, decr_y + 5)]),
      stroke_color = "white",
      line_width = 10.0)

    self.scale_value = 1
    self.color_button(decr_x, decr_y, button_width, '-', -1)
    self.color_button(incr_x, incr_y, button_width, '+', 1)

  def color_button(self, x, y, size, text, move):
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

  def move_bar(self, widget, target, event, move):
    self.scale_value += move
    # Take care not to bypass bounds
    if self.scale_value > 255:
      self.scale_value = 1.0
      return
    elif self.scale_value < 0:
      self.scale_value = 0.0
      return

    if self.primary_color == 1:
      self.bar.translate(move * 0.1, move * 0.05)
    self.set_color(move)

  def set_color(self, change):
    if self.primary_color == 1:
      self.color_instance.cyan += int(change * -1)
      self.color_instance.resultant_color(1)
    elif self.primary_color == 2:
      self.color_instance.magenta += int(change * -1)
      self.color_instance.resultant_color(2)
    if self.primary_color == 3:
      self.color_instance.yellow += int(change * -1)
      self.color_instance.resultant_color(3)

class Colors:
  """ Class containing all the colors"""


  def __init__(self, game, rootitem):
    self.game = game

    self.color_image = goocanvas.Ellipse(
      parent = rootitem,
      center_x = 395,
      center_y = 230,
      radius_y = 60,
      radius_x = 75,
      stroke_color_rgba = 0xFFFFFFFFL,
      fill_color_rgba = 0xFFFFFFFFL,
      line_width = 0.5)

    # Random color to be matched
    self.r_random = random.randrange(0,256)
    self.g_random = random.randrange(0,256)
    self.b_random = random.randrange(0,256)
    code = self.hex_code(self.r_random, self.g_random, self.b_random)
    random_color = goocanvas.Rect(
      parent = rootitem,
      radius_x = 6,
      radius_y = 6,
      x = 350,
      y = 50,
      width = 100,
      height = 70,
      stroke_color_rgba = long(code, 16),
      fill_color_rgba = long(code, 16))


    # OK Button
    ok = goocanvas.Svg(parent = rootitem,
                       svg_handle = gcompris.skin.svg_get(),
                       svg_id = "#OK"
                       )
    ok.translate(0, -130)

    ok.connect("button_press_event", self.ok_event)
    gcompris.utils.item_focus_init(ok, None)

    # initialise variables
    self.color_rgb = [255,255,255]
    self.cyan = self.magenta = self.yellow = 255

  def ok_event(self, widget, target, event):
    if self.color_rgb[0] - 30 < self.r_random < self.color_rgb[0] + 30 and \
       self.color_rgb[1] - 30 < self.g_random < self.color_rgb[1] + 30 and \
       self.color_rgb[2] - 30 < self.b_random < self.color_rgb[2] + 30:
      self.game.game_over(1)
    else:
      self.game.game_over(2)

  def resultant_color(self, change):
    if -1 < self.cyan < 256 and -1 < self.magenta < 256 and -1 < self.yellow < 256:
      cyan_cmy  = (255 - self.cyan, 0, 0)
      magenta_cmy  = (0, 255 - self.magenta, 0)
      yellow_cmy  = (0, 0, 255 - self.yellow)

      color_cmy = [255 - self.color_rgb[0], 255 - self.color_rgb[1],
                   255 - self.color_rgb[2]]

      if change == 1:
        color_cmy[0] = (color_cmy[0] + cyan_cmy[0]) / 2
      elif change == 2:
        color_cmy[1] = (color_cmy[1] + magenta_cmy[1]) / 2
      elif change == 3:
        color_cmy[2] = (color_cmy[2] + yellow_cmy[2]) / 2

      self.color_rgb[0] = 255 - color_cmy[0]
      self.color_rgb[1] = 255 - color_cmy[1]
      self.color_rgb[2] = 255 - color_cmy[2]

      color_code = self.hex_code(self.color_rgb[0], self.color_rgb[1],
                                 self.color_rgb[2])
      self.color_image.set_property('fill_color_rgba', long(color_code, 16))

  def hex_code(self, r, g, b):
    hexchars = "0123456789ABCDEF"
    return hexchars[r / 16] + hexchars[r % 16] + hexchars[g / 16] + \
          hexchars[g % 16] + hexchars[b / 16] + hexchars[b % 16] + 'FFL'

