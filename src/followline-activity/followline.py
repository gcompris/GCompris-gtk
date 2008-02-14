#  gcompris - administration.py
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
# Follow Line Board module
import gobject
import goocanvas
import cairo
import gcompris
import gcompris.skin
import gcompris.bonus
import gcompris.utils
import gcompris.sound
import gtk
import gtk.gdk
import random
import math
from gcompris import gcompris_gettext as _

# ----------------------------------------
#

class Gcompris_followline:
  """follow the line"""


  def __init__(self, gcomprisBoard):
    self.gcomprisBoard = gcomprisBoard

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = 0
    self.gamewon       = 0

    self.loosing_count = 0

    self.color_empty   = 0xcee871FFL
    self.color_full    = 0x1a24cbffL
    self.color_target  = 0xFF0000FFL
    self.color_border  = 0x101010FFL

    self.timeout       = 0


  def start(self):
    self.saved_policy = gcompris.sound.policy_get()
    gcompris.sound.policy_set(gcompris.sound.PLAY_AND_INTERRUPT)

    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=9
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1
    gcompris.bar_set(gcompris.BAR_LEVEL)
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            "followline/followline.png")
    self.background_item_connect_id = \
        self.gcomprisBoard.canvas.get_root_item().connect("motion_notify_event",
                                                          self.loosing_item_event)

    gcompris.bar_set_level(self.gcomprisBoard)

    self.init_board()


  def end(self):

    # Remove the root item removes all the others inside it
    self.cleanup()

    # Disconnect from the background item
    self.gcomprisBoard.canvas.get_root_item().disconnect(
      self.background_item_connect_id)

    gcompris.sound.policy_set(self.saved_policy)

    if self.timeout:
      gobject.source_remove(self.timeout)
    self.timeout = 0

  def ok(self):
    pass


  def repeat(self):
    pass


  def config(self):
    pass


  def key_press(self, keyval, commit_str, preedit_str):
    return False


  # Called by gcompris core
  def pause(self, pause):

    self.board_paused = pause

    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    # the game is won
    if(pause == 0):
      self.next_level()
      self.gamewon = 0

    return


  # Called by gcompris when the user click on the level icon
  def set_level(self, level):
    self.gcomprisBoard.level=level
    self.gcomprisBoard.sublevel=1
    self.next_level()

  # End of Initialisation
  # ---------------------

  def cleanup(self):
    # Remove the root item removes all the others inside it
    self.state = "Done"

    self.rootitem.remove()
    self.lines_group.remove()
    self.water_spot_group.remove()

  def next_level(self):

    self.cleanup()

    # Set the level in the control bar
    gcompris.bar_set_level(self.gcomprisBoard);

    self.init_board()



  def init_board(self):

    self.state = "Ready"

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains automaticaly.
    self.rootitem = goocanvas.Group(
      parent = self.gcomprisBoard.canvas.get_root_item())

    # Another group where we put each canvas line item in it
    self.lines_group = goocanvas.Group(
      parent = self.gcomprisBoard.canvas.get_root_item())

    start_x  = 40
    start_y  = gcompris.BOARD_HEIGHT/2
    stop_x   = gcompris.BOARD_WIDTH - 100

    # Line path (create several little line)
    min_boundary = 40
    y            = start_y
    line_width   = 45 - self.gcomprisBoard.level*2
    step         = (stop_x-start_x)/(30)

    frequency = 1 + int(self.gcomprisBoard.level/4)

    xpi = math.pi/2*frequency
    y   = start_y + math.cos(xpi)*(self.gcomprisBoard.level*10)
    for x in range(start_x, stop_x, step):

      xpi += (math.pi/2*frequency)/step
      y2 = start_y + math.cos(xpi)*(self.gcomprisBoard.level*10)

      # Check we stay within boundaries
      if(y2 >= gcompris.BOARD_HEIGHT - min_boundary):
        y2 = gcompris.BOARD_HEIGHT - min_boundary
      elif(y2 <= min_boundary):
        y2 = min_boundary

      item = goocanvas.Polyline(
        parent = self.lines_group,
        points = goocanvas.Points([(x, y), (x + step, y2)]),
        stroke_color_rgba = self.color_empty,
        line_width = line_width,
        line_cap = cairo.LINE_CAP_ROUND
        )
      item.connect("enter_notify_event", self.line_item_event)

      # Draw the black tube
      if x > start_x and x < stop_x-step:
          goocanvas.Polyline(
          parent = self.rootitem,
          points = goocanvas.Points([( x, y),
                                     (x + step, y2)]),
          fill_color_rgba = self.color_border,
          line_width = line_width + 20,
          line_cap = cairo.LINE_CAP_ROUND
          )
      y = y2

    self.highlight_next_line()

    # Another group where we put each canvas line item in it
    self.water_spot_group = goocanvas.Group(
      parent = self.gcomprisBoard.canvas.get_root_item())

    # A water spot will be displayed when the user win
    self.water_spot = goocanvas.Image(
      parent = self.water_spot_group,
      pixbuf = gcompris.utils.load_pixmap("followline/water_spot.png"),
      x = 580,
      y = 260,
      )
    self.water_spot.props.visibility = goocanvas.ITEM_INVISIBLE


  # Code that increments the sublevel and level
  # And bail out if no more levels are available
  # return 1 if continue, 0 if bail out
  def increment_level(self):
    self.gcomprisBoard.sublevel += 1

    if(self.gcomprisBoard.sublevel > self.gcomprisBoard.number_of_sublevel):
      # Try the next level
      self.gcomprisBoard.sublevel=1
      self.gcomprisBoard.level += 1
      if(self.gcomprisBoard.level > self.gcomprisBoard.maxlevel):
        self.gcomprisBoard.level = self.gcomprisBoard.maxlevel

    return 1

  #
  # Highlight the next spot on the line
  #
  def highlight_next_line(self):
    for i in range(0, self.lines_group.get_n_children()):
      item = self.lines_group.get_child(i)
      if(item.get_data("gotit") != True):
        item.props.stroke_color_rgba = self.color_target
        item.set_data("iamnext", True);
        return

  #
  # Highlight the previous spot on the line
  # Called on loose situation
  #
  def highlight_previous_line(self):
    previous_item = []

    for i in range(0, self.lines_group.get_n_children()):
      item = self.lines_group.get_child(i)

      if(item.get_data("iamnext") == True):

        if(previous_item):
          # Remove the target info for this item
          item.set_data("iamnext", False)
          item.props.stroke_color_rgba = self.color_empty

          # Set the target info on the previous item
          previous_item.props.stroke_color_rgba = self.color_target
          item.set_data("gotit", False)
          previous_item.set_data("iamnext", True);

        else:
          self.state = "Ready"

        return

      previous_item = item

  def lauch_bonus(self):
    gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.FLOWER)

  def is_done(self):
    done = True
    for i in range(0, self.lines_group.get_n_children()):
      item = self.lines_group.get_child(i)
      if(item.get_data("gotit") != True):
        done = False

    if(done):
      gcompris.sound.play_ogg("sounds/bubble.wav")
      # This is a win
      if (self.increment_level() == 1):
        self.state = "Done"
        self.gamewon = 1
        self.water_spot.raise_(None)
        self.water_spot.props.visibility = goocanvas.ITEM_VISIBLE
        self.timeout = gobject.timeout_add(1500, self.lauch_bonus)

    return done

  def loosing_item_event(self, widget, target, event=None):
    if target.__class__ != goocanvas.Image:
      return False

    if(self.state == "Started"):
      self.loosing_count += 1
      if(self.loosing_count % 10 == 0):
        gcompris.sound.play_ogg("sounds/smudge.wav")
        self.highlight_previous_line()
    return False


  def line_item_event(self, widget, target, event=None):
    if not self.board_paused and widget.get_data("iamnext") == True:
      # The first line touch means the game is started
      gcompris.sound.play_ogg("sounds/drip.wav")
      self.state = "Started"
      widget.props.stroke_color_rgba = self.color_full

      widget.set_data("gotit", True);
      widget.set_data("iamnext", False);
      self.highlight_next_line()
      self.is_done()

    return False

