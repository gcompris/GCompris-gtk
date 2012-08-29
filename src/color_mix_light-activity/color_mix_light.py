#  gcompris - color_mix_light.py
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
# color_mix_light activity.
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import goocanvas
import pango
import random

from gcompris import gcompris_gettext as _

class Gcompris_color_mix_light:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard
    self.game_complete = False
    self.gcomprisBoard.level = 1
    self.gcomprisBoard.maxlevel = 4

    # Needed to get key_press
    gcomprisBoard.disable_im_context = True

  def start(self):
    gcompris.bar_set_level(self.gcomprisBoard)

    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.bar_location(2, -1, 0.5)

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.rootitem = goocanvas.Group(parent =
                                    self.gcomprisBoard.canvas.get_root_item())

    # Set a background image
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            'color_mix_light/background.jpg')

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
    r_points = goocanvas.Points( [(242, 212), (130, 177)] )
    b_points = goocanvas.Points( [(548, 213), (665, 177)] )
    m_points = goocanvas.Points( [(390, 372), (390, 490)] )

    colors = Colors(self, self.rootitem, self.gcomprisBoard.level)

    # Pass the points of the buttons and slider for the color tubes
    red_tube = Color_tubes(self.rootitem, colors, 'torch_red.png',
                           1, 90, 115, r_points, 232, 210, 120, 175, self.gcomprisBoard.level)
    green_tube = Color_tubes(self.rootitem, colors, 'torch_green.png',
                             2, 265, 265, m_points, 390, 372, 390, 490, self.gcomprisBoard.level)
    blue_tube = Color_tubes(self.rootitem, colors, 'torch_blue.png',
                            3, 462, 115, b_points, 554, 210, 672, 175, self.gcomprisBoard.level)

  def game_over(self, result):
    self.game_complete = True
    if result == 1:
      self.next_level()
      gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.TUX)
    else:
      gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.TUX)

  def next_level(self):
    if self.gcomprisBoard.level < self.gcomprisBoard.maxlevel:
      self.gcomprisBoard.level += 1

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
    self.gcomprisBoard.level = level
    gcompris.bar_set_level(self.gcomprisBoard)
    self.end()
    self.start()

class Color_tubes:
  """ Class containing the three primary color tubes"""


  def __init__(self, rootitem, color_instance, image, primary_color,
               x, y, points, incr_x, incr_y, decr_x, decr_y, level):
    self.rootitem = rootitem
    self.primary_color = primary_color
    self.color_instance = color_instance

    # Load the tube image
    image = 'color_mix_light/' + image
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

    self.difference_x = (incr_x - decr_x) / 7
    self.difference_y = (incr_y - decr_y) / 8
    slider_x = decr_x + self.difference_x
    slider_y = decr_y + self.difference_y
    self.bar = goocanvas.Polyline(
      parent = self.rootitem,
      points = goocanvas.Points([(slider_x, slider_y - 4),
                                 (slider_x, slider_y + 6)]),
      stroke_color = "white",
      line_width = 10.0)

    self.scale_value = 1
    move = int(255/ (level * 2 + 1))
    self.color_button(decr_x, decr_y, button_width, '-', -move)
    self.color_button(incr_x, incr_y, button_width, '+', move)

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
    if self.scale_value > 256:
      self.scale_value = 255
      return
    elif self.scale_value < 0:
      self.scale_value = 1
      return

    self.bar.translate(move * self.difference_x * 0.019,
                       move * self.difference_y * 0.023)
    self.set_color(move)

  def set_color(self, change):
    if self.primary_color == 1:
      self.color_instance.color1 += int(change)
      self.color_instance.resultant_color(1)
    elif self.primary_color == 2:
      self.color_instance.color2 += int(change)
      self.color_instance.resultant_color(2)
    if self.primary_color == 3:
      self.color_instance.color3 += int(change)
      self.color_instance.resultant_color(3)

class Colors:
  """ Class containing all the colors"""


  def __init__(self, game, rootitem,  level):
    self.game = game
    self.rootitem = rootitem

    self.color_rgb = [0, 0, 0]
    self.color1 = self.color2 = self.color3 = 0

    self.color_image = goocanvas.Ellipse(
      parent = rootitem,
      center_x = 395,
      center_y = 230,
      radius_y = 60,
      radius_x = 75,
      stroke_color_rgba = 0x000000FFL,
      fill_color_rgba = 0x000000FFL,
      line_width = 0.5)

    # Random color to be matched
    self.increment = int(255/ (level * 2 + 1))
    rand_r = random.randrange(0, 2 * level + 1)
    rand_g = random.randrange(0, 2 * level + 1)
    rand_b = random.randrange(0, 2 * level + 1)

    self.r_random = rand_r * self.increment
    self.g_random = rand_g * self.increment
    self.b_random = rand_b * self.increment
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

  def ok_event(self, widget, target, event):
    c_diff = self.color_rgb[0] - self.r_random
    m_diff = self.color_rgb[1] - self.g_random
    y_diff = self.color_rgb[2] - self.b_random
    if self.color_rgb[0] - self.increment <= self.r_random <= self.color_rgb[0] + self.increment and \
       self.color_rgb[1] - self.increment <= self.g_random <= self.color_rgb[1] + self.increment and \
       self.color_rgb[2] - self.increment <= self.b_random <= self.color_rgb[2] + self.increment:
      self.game.game_over(1)
    else:
      self.show_message(c_diff, m_diff, y_diff)
      self.game.game_over(2)

  def show_message(self, red, green, blue):
    if red > 0 and abs(red) > self.increment:
      self.message('Too much red', 300)
    elif red < 0 and abs(red) > self.increment:
      self.message('Not enough red', 300)

    if green > 0 and abs(green) > self.increment:
      self.message('Too much green', 375)
    elif green < 0 and abs(green) > self.increment:
      self.message('Not enough green', 375)

    if blue > 0 and abs(blue) > self.increment:
      self.message('Too much blue', 450)
    elif blue < 0 and abs(blue) > self.increment:
      self.message('Not enough blue', 450)

  def message(self, msg, y):
    text = goocanvas.Text(
      parent = self.rootitem,
      x = 150,
      y = y,
      fill_color_rgba = 0x550000FFL,
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER,
      text = _(msg))
    bounds = text.get_bounds()
    gap = 20

    back = goocanvas.Rect(
      parent = self.rootitem,
      radius_x = 6,
      radius_y = 6,
      x = bounds.x1 - gap,
      y = bounds.y1 - gap,
      width = bounds.x2 - bounds.x1 + gap * 2,
      height = bounds.y2 - bounds.y1 + gap * 2,
      stroke_color_rgba = 0xFFFFFFFFL,
      fill_color_rgba = 0xCCCCCC44L)

  def resultant_color(self, change):
    self.color_rgb[0] = self.color1
    self.color_rgb[1] = self.color2
    self.color_rgb[2] = self.color3

    color_code = self.hex_code(self.color_rgb[0], self.color_rgb[1],
                               self.color_rgb[2])
    self.color_image.set_property('fill_color_rgba', long(color_code, 16))

  def hex_code(self, r, g, b):
    hexchars = "0123456789ABCDEF"
    return hexchars[r / 16] + hexchars[r % 16] + hexchars[g / 16] + \
          hexchars[g % 16] + hexchars[b / 16] + hexchars[b % 16] + 'FFL'

