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
from color_mix import Colors, Color_tubes

class Gcompris_color_mix_light:
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
    r_points = goocanvas.Points( [(242, 212), (130, 177)] )
    b_points = goocanvas.Points( [(548, 213), (665, 177)] )
    m_points = goocanvas.Points( [(390, 372), (390, 490)] )

    colors = Colors(self, self.rootitem, self.gcomprisBoard.level, 0)

    # Pass the points of the buttons and slider for the color tubes
    red_tube = Color_tubes(self.rootitem, colors, 'torch_red.png',
                           1, 90, 115, r_points, 232, 210, 120, 175,
                           self.gcomprisBoard.level, 1)
    green_tube = Color_tubes(self.rootitem, colors, 'torch_green.png',
                             2, 265, 265, m_points, 390, 372, 390, 490,
                             self.gcomprisBoard.level, 1)
    blue_tube = Color_tubes(self.rootitem, colors, 'torch_blue.png',
                            3, 462, 115, b_points, 554, 210, 672, 175,
                            self.gcomprisBoard.level, 1)

  def game_over(self, result):
    self.game_complete = True
    if result == 1:
      self.win += 1
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

