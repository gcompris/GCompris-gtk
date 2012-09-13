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
    self.gcomprisBoard.level = 1
    self.gcomprisBoard.maxlevel = 4
    self.win = 0
    self.lost = False
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
    c_points = goocanvas.Points( [(242, 210), (130, 175)] )
    m_points = goocanvas.Points( [(390, 352), (390, 470)] )
    y_points = goocanvas.Points( [(548, 208), (665, 175)] )

    colors = Colors(self, self.rootitem, self.gcomprisBoard.level)

    # Pass the points of the buttons and slider for the color tubes
    cyan_tube = Color_tubes(self.rootitem, colors, 'cyan_tube.png',
                            1, 80, 120, c_points, 242, 210, 130, 175, self.gcomprisBoard.level)
    magenta_tube = Color_tubes(self.rootitem, colors, 'magenta_tube.png',
                               2, 350, 290, m_points, 390, 352, 390, 470, self.gcomprisBoard.level)
    yellow_tube = Color_tubes(self.rootitem, colors, 'yellow_tube.png',
                              3, 460, 120, y_points, 548, 208, 665, 175, self.gcomprisBoard.level)

  def game_over(self, result):
    self.game_complete = True
    if result == 1:
      self.win +=1
      self.lost = False
      self.next_level()
      gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.TUX)
    else:
      self.lost = True
      gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.TUX)

  def next_level(self):
    if self.gcomprisBoard.level < self.gcomprisBoard.maxlevel and self.win == 5:
      self.gcomprisBoard.level += 1
      self.win = 0

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
      if self.lost == False:
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
      self.color_instance.cyan += int(change * -1)
      self.color_instance.resultant_color(1, change/abs(change))
    elif self.primary_color == 2:
      self.color_instance.magenta += int(change * -1)
      self.color_instance.resultant_color(2, change/abs(change))
    if self.primary_color == 3:
      self.color_instance.yellow += int(change * -1)
      self.color_instance.resultant_color(3, change/abs(change))

class Colors:
  """ Class containing all the colors"""


  def __init__(self, game, rootitem, level):
    self.game = game
    self.rootitem = rootitem

    self.color_rgb = [255, 255, 255]
    self.cyan = self.magenta = self.yellow = 255
    self.color_cmy = [255 - self.color_rgb[0], 255 - self.color_rgb[1],
                 255 - self.color_rgb[2]]

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
    self.increment = int(255/ (level * 2 + 1))
    rand_r = random.randrange(0, 2 * level + 1)
    rand_g = random.randrange(0, 2 * level + 1)
    rand_b = random.randrange(0, 2 * level + 1)

    self.r_random = rand_r * self.increment
    self.g_random = rand_g * self.increment
    self.b_random = rand_b * self.increment
    self.required_color_cmy = [255 - self.r_random, 255 - self.g_random,
                 255 - self.b_random]
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
    if self.game.lost == True:
      self.message_rootitem.remove()
    self.message_rootitem = goocanvas.Group(parent = self.rootitem)

    c_diff = self.color_cmy[0] - self.required_color_cmy[0]
    m_diff = self.color_cmy[1] - self.required_color_cmy[1]
    y_diff = self.color_cmy[2] - self.required_color_cmy[2]
    if self.color_rgb[0] - self.increment <= self.r_random <= self.color_rgb[0] + self.increment and \
       self.color_rgb[1] - self.increment <= self.g_random <= self.color_rgb[1] + self.increment and \
       self.color_rgb[2] - self.increment <= self.b_random <= self.color_rgb[2] + self.increment:
      self.game.game_over(1)
    else:
      self.show_message(c_diff, m_diff, y_diff)
      self.game.game_over(2)

  def show_message(self, cyan, magenta, yellow):
    if cyan > 0 and abs(cyan) > self.increment:
      self.message('Too much cyan', 300)
    elif cyan < 0 and abs(cyan) > self.increment:
      self.message('Not enough cyan', 300)

    if magenta > 0 and abs(magenta) > self.increment:
      self.message('Too much magenta', 375)
    elif magenta < 0 and abs(magenta) > self.increment:
      self.message('Not enough magenta', 375)

    if yellow > 0 and abs(yellow) > self.increment:
      self.message('Too much yellow', 450)
    elif yellow < 0 and abs(yellow) > self.increment:
      self.message('Not enough yellow', 450)

  def message(self, msg, y):
    text = goocanvas.Text(
      parent = self.message_rootitem,
      x = 150,
      y = y,
      fill_color_rgba = 0x550000FFL,
      anchor = gtk.ANCHOR_CENTER,
      alignment = pango.ALIGN_CENTER,
      text = _(msg))
    bounds = text.get_bounds()
    gap = 20

    back = goocanvas.Rect(
      parent = self.message_rootitem,
      radius_x = 6,
      radius_y = 6,
      x = bounds.x1 - gap,
      y = bounds.y1 - gap,
      width = bounds.x2 - bounds.x1 + gap * 2,
      height = bounds.y2 - bounds.y1 + gap * 2,
      stroke_color_rgba = 0xFFFFFFFFL,
      fill_color_rgba = 0xCCCCCC44L)

  def resultant_color(self, change, button):
    cyan_cmy  = (255 - self.cyan, 0, 0)
    magenta_cmy  = (0, 255 - self.magenta, 0)
    yellow_cmy  = (0, 0, 255 - self.yellow)

    self.color_cmy = [255 - self.color_rgb[0], 255 - self.color_rgb[1],
                 255 - self.color_rgb[2]]

    if change == 1:
      if button < 0:
        self.color_cmy[0] = min(self.color_cmy[0], cyan_cmy[0])
      else:
        self.color_cmy[0] = max(self.color_cmy[0], cyan_cmy[0])

    elif change == 2:
      if button < 0:
        self.color_cmy[1] = min(self.color_cmy[1], magenta_cmy[1])
      else:
        self.color_cmy[1] = max(self.color_cmy[1], magenta_cmy[1])

    elif change == 3:
      if button < 0:
        self.color_cmy[2] = min(self.color_cmy[2], yellow_cmy[2])
      else:
        self.color_cmy[2] = max(self.color_cmy[2], yellow_cmy[2])

    self.color_rgb[0] = 255 - self.color_cmy[0]
    self.color_rgb[1] = 255 - self.color_cmy[1]
    self.color_rgb[2] = 255 - self.color_cmy[2]

    color_code = self.hex_code(self.color_rgb[0], self.color_rgb[1],
                               self.color_rgb[2])
    self.color_image.set_property('fill_color_rgba', long(color_code, 16))

  def hex_code(self, r, g, b):
    hexchars = "0123456789ABCDEF"
    return hexchars[r / 16] + hexchars[r % 16] + hexchars[g / 16] + \
          hexchars[g % 16] + hexchars[b / 16] + hexchars[b % 16] + 'FFL'

