#  gcompris - lightsoff.py
#
# Copyright (C) 2010 Bruno and Clement Coudoin
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
# lightsoff Board module
import gtk
import gtk.gdk
import gcompris
import gcompris.utils
import gcompris.skin
import gcompris.bonus
import goocanvas

from gcompris import gcompris_gettext as _

class Gcompris_lightsoff:
  """Empty gcompris python class"""


  def __init__(self, gcomprisBoard):
    # Save the gcomprisBoard, it defines everything we need
    # to know from the core
    self.gcomprisBoard = gcomprisBoard

    self.rootitem = None

    # These are used to let us restart only after the bonus is displayed.
    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    self.board_paused  = False;
    self.gamewon       = False;

    self.data = [
      [[0,0,0,0,0],
       [0,0,1,0,0],
       [0,1,1,1,0],
       [0,0,1,0,0],
       [0,0,0,0,0]],
      [[1,1,0,0,0],
       [1,0,0,0,0],
       [0,0,0,0,0],
       [0,0,0,0,0],
       [0,0,0,0,0]],
      [[0,0,0,0,0],
       [0,0,0,0,0],
       [0,0,0,0,0],
       [0,0,1,0,0],
       [0,1,1,1,0]],
      [[1,1,0,1,1],
       [1,0,0,0,1],
       [0,0,0,0,0],
       [0,0,0,1,0],
       [0,0,1,1,1]],
      [[0,1,1,1,0],
       [1,0,1,0,1],
       [1,1,0,1,1],
       [1,0,1,0,1],
       [0,1,1,1,0]],
      [[1,1,0,1,1],
       [1,0,1,0,1],
       [0,1,1,1,0],
       [1,0,1,0,1],
       [1,1,0,1,1]],
      [[0,1,0,1,0],
       [1,1,0,1,1],
       [0,1,0,1,0],
       [1,0,1,0,1],
       [1,0,1,0,1]],
      [[1,0,0,0,1],
       [1,1,0,1,1],
       [0,0,1,0,0],
       [1,0,1,0,0],
       [1,0,1,1,0]],
      [[1,1,0,1,1],
       [0,0,0,0,0],
       [1,1,0,1,1],
       [0,0,0,0,1],
       [1,1,0,0,0]],
      # 5
      [[1,1,1,1,1],
       [1,1,1,1,1],
       [1,1,1,1,1],
       [1,1,1,1,1],
       [1,1,1,1,1]],
      # 6
      [[0,0,0,1,1],
       [0,0,0,1,1],
       [0,0,0,0,0],
       [1,1,0,0,0],
       [1,1,0,0,0]],
      # 7
      [[0,0,0,0,0],
       [0,1,1,1,0],
       [1,1,1,1,1],
       [0,1,1,1,0],
       [0,0,0,0,0]],
      # 8
      [[0,0,0,0,0],
       [0,1,1,1,0],
       [0,1,1,1,0],
       [0,1,1,1,0],
       [0,0,0,0,0]],
      # 9
      [[1,1,0,1,1],
       [1,1,0,1,1],
       [0,0,0,0,0],
       [1,1,0,1,1],
       [1,1,0,1,1]],
      # 10
      [[1,1,1,1,1],
       [0,1,1,1,0],
       [0,0,1,0,0],
       [0,1,1,1,0],
       [1,1,1,1,1]],
      # 11
      [[1,1,1,1,1],
       [1,0,0,0,1],
       [1,0,0,0,1],
       [1,0,0,0,1],
       [1,1,1,1,1]],
      # 12
      [[0,0,1,1,1],
       [0,0,0,1,1],
       [1,0,0,0,1],
       [1,1,0,0,0],
       [1,1,1,0,0]],
      # 13
      [[1,0,0,0,1],
       [0,1,0,1,0],
       [0,0,1,0,0],
       [0,1,0,1,0],
       [1,0,0,0,1]],
      # 14
      [[1,1,1,1,1],
       [1,0,1,0,1],
       [1,1,1,1,1],
       [1,0,1,0,1],
       [1,1,1,1,1]],
      # 15
      [[1,0,0,0,0],
       [1,1,1,1,0],
       [1,1,1,1,0],
       [1,1,1,1,0],
       [1,1,1,1,1]],
      # 16
      [[1,1,1,1,1],
       [1,1,1,1,1],
       [1,1,0,1,1],
       [1,1,1,1,1],
       [1,1,1,1,1]],
      # 17
      [[1,0,1,0,1],
       [0,1,0,1,0],
       [0,0,1,0,0],
       [0,1,0,1,0],
       [1,0,1,0,1]],
      # 18
      [[1,1,1,0,1],
       [1,1,1,0,1],
       [0,0,0,0,0],
       [1,0,1,1,1],
       [1,0,1,1,1]],
      # 19
      [[1,1,0,1,1],
       [1,1,0,1,1],
       [1,1,0,1,1],
       [1,1,0,1,1],
       [1,1,0,1,1]],
      # 20
      [[1,1,1,1,1],
       [1,0,0,0,1],
       [1,1,0,1,1],
       [1,1,0,1,1],
       [1,1,1,1,1]],
      # 21
      [[1,1,1,1,1],
       [1,1,1,1,1],
       [0,0,0,1,1],
       [0,0,0,1,1],
       [0,0,0,1,1]],
      # 22
      [[1,1,1,0,1],
       [1,1,1,0,0],
       [1,1,1,0,0],
       [1,1,1,0,0],
       [1,1,1,0,1]],
      # 23
      [[1,1,1,1,1],
       [0,0,1,0,0],
       [0,0,1,0,0],
       [0,0,1,0,0],
       [1,1,1,1,1]],
      # 24
      [[0,0,0,0,0],
       [0,1,1,0,0],
       [0,1,1,1,0],
       [0,0,1,1,0],
       [0,0,0,0,0]],
      # 25
      [[0,0,0,1,1],
       [0,0,1,1,1],
       [0,0,1,0,0],
       [1,1,1,0,0],
       [1,1,0,0,0]],
      # 26
      [[0,0,0,1,1],
       [1,1,0,1,1],
       [1,1,0,0,0],
       [1,1,0,0,0],
       [1,1,0,1,1]],
      # 27
      [[1,0,0,0,1],
       [0,1,1,1,0],
       [0,1,1,1,0],
       [0,1,1,1,0],
       [1,0,0,0,1]],
      # 28
      [[1,0,1,0,1],
       [0,1,0,1,0],
       [1,0,1,0,1],
       [0,1,0,1,0],
       [1,0,1,0,1]],
      # 29
      [[0,0,1,0,0],
       [1,0,0,1,0],
       [0,1,1,1,1],
       [1,0,0,0,0],
       [1,1,0,1,0]],
      # 30
      [[0,0,0,0,1],
       [0,0,0,1,1],
       [0,0,1,0,1],
       [0,1,0,0,1],
       [1,1,1,1,1]],
      # 31
      [[1,1,0,1,1],
       [0,1,0,1,0],
       [1,1,1,1,1],
       [1,1,0,1,1],
       [1,0,0,0,1]],
      # 32
      [[0,1,1,0,0],
       [0,1,1,0,1],
       [0,1,0,0,1],
       [1,1,0,0,0],
       [1,1,1,1,0]],
      # 33
      [[0,0,0,0,1],
       [1,1,1,0,0],
       [1,0,1,1,1],
       [1,1,1,1,0],
       [1,0,0,1,0]],
      # 34
      [[1,0,1,1,1],
       [0,0,1,0,1],
       [0,0,0,0,0],
       [1,1,1,1,0],
       [1,1,0,1,0]],
      # 35
      [[1,1,0,1,1],
       [0,1,0,1,1],
       [0,0,0,1,0],
       [1,1,0,0,0],
       [1,1,1,1,0]],
      # 36
      [[1,1,1,1,1],
       [0,0,0,1,0],
       [0,1,0,1,1],
       [1,1,1,0,1],
       [1,0,1,0,0]],
      # 37
      [[0,0,0,1,1],
       [1,0,1,1,0],
       [0,0,1,0,0],
       [0,1,1,1,1],
       [1,0,0,1,0]],
      # 38
      [[0,0,0,0,1],
       [0,0,1,1,1],
       [1,1,0,0,1],
       [1,1,1,0,0],
       [0,0,1,0,0]],
      # 39
      [[0,0,1,1,1],
       [1,0,1,1,1],
       [1,1,1,0,0],
       [0,0,1,0,0],
       [1,1,0,1,1]],
      # 40
      [[0,1,1,1,1],
       [0,0,1,1,1],
       [0,0,1,1,1],
       [1,1,1,1,0],
       [0,0,0,1,0]],
      # 41
      [[1,1,1,1,1],
       [1,0,0,0,0],
       [0,1,0,0,1],
       [0,0,0,1,1],
       [1,1,1,1,1]],
      # 42
      [[1,1,1,1,1],
       [1,0,0,0,0],
       [0,0,1,0,0],
       [0,1,1,1,0],
       [0,1,0,0,1]],
      # 43
      [[0,0,0,0,0],
       [0,0,0,1,0],
       [1,1,0,1,1],
       [0,1,1,0,0],
       [1,1,1,1,1]],
      # 44
      [[0,0,0,1,1],
       [0,1,1,0,0],
       [0,1,0,0,0],
       [1,1,1,1,0],
       [1,1,1,1,0]],
      # 45
      [[0,0,0,1,0],
       [1,1,1,1,1],
       [0,0,0,0,0],
       [0,0,1,0,0],
       [1,1,1,1,0]],
      # 46
      [[0,1,0,1,0],
       [0,0,0,1,0],
       [0,1,0,1,0],
       [0,0,1,0,0],
       [0,1,1,1,0]],
      # 47
      [[1,0,0,1,0],
       [0,0,0,0,1],
       [0,1,0,0,0],
       [0,0,0,0,0],
       [1,0,1,0,0]],
      # 48
      [[1,1,0,0,1],
       [0,1,0,0,1],
       [0,1,1,1,1],
       [0,1,0,1,0],
       [1,1,1,1,1]],
      # 49
      [[1,1,1,1,1],
       [0,0,1,0,0],
       [0,1,1,0,0],
       [0,0,1,1,0],
       [1,1,1,0,1]],
      # 50
      [[1,0,1,0,1],
       [1,0,1,0,0],
       [0,0,0,1,1],
       [0,1,0,1,0],
       [1,0,0,1,0]],
      # 51
      [[0,1,0,1,0],
       [1,0,0,1,0],
       [0,1,1,1,1],
       [1,0,1,0,0],
       [0,1,1,0,0]],
      # 52
      [[1,1,1,1,1],
       [1,1,0,0,0],
       [0,0,0,1,1],
       [0,1,1,1,0],
       [0,0,1,0,0]],
      ]


  def start(self):
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=len(self.data)
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=1

    # Create our rootitem. We put each canvas item in it so at the end we
    # only have to kill it. The canvas deletes all the items it contains
    # automaticaly.
    self.backroot = goocanvas.Group(parent = \
                                      self.gcomprisBoard.canvas.get_root_item())

    # A color changing background
    self.background = goocanvas.Rect(
      parent = self.backroot,
      x = 0,
      y = 0,
      width = gcompris.BOARD_WIDTH,
      height = 200,
      fill_color_rgba = 0xFFFFFFFFL
      )

    svghandle = gcompris.utils.load_svg("lightsoff/back.svgz")
    goocanvas.Svg(
      parent = self.backroot,
      svg_handle = svghandle,
      svg_id = "#BACKGROUND",
      pointer_events = goocanvas.EVENTS_NONE
      )

    # The Sun
    self.sunitem = goocanvas.Svg(
      parent = self.backroot,
      svg_handle = svghandle,
      svg_id = "#SUN",
      )
    self.sunitem_offset = 0

    # TUX
    self.tuxitem = goocanvas.Svg(
      parent = self.backroot,
      svg_handle = svghandle,
      svg_id = "#TUX",
      )
    self.tuxitem.connect("button_press_event", self.solve_event)
    gcompris.utils.item_focus_init(self.tuxitem, None)

    # The Buble
    self.bubbleitem = goocanvas.Svg(
      parent = self.backroot,
      svg_handle = svghandle,
      svg_id = "#BUBBLE1",
      )

    self.textitem = goocanvas.Text(
      parent = self.backroot,
      x = gcompris.BOARD_WIDTH/2 + 80,
      y = gcompris.BOARD_HEIGHT - 80,
      width = 400,
      font = gcompris.skin.get_font("gcompris/content"),
      text = _("Switch off all the lights, I have to go to sleep.\n"
               "If you need help, click on me."),
      fill_color = "black",
      anchor = gtk.ANCHOR_CENTER
      )

    # The Tipi
    self.tipiitem = goocanvas.Svg(
      parent = self.backroot,
      svg_handle = svghandle,
      svg_id = "#TIPI",
      )

    # The image foreground
    goocanvas.Svg(
      parent = self.backroot,
      svg_handle = svghandle,
      svg_id = "#FOREGROUND",
      pointer_events = goocanvas.EVENTS_NONE
      )

    # Set the buttons we want in the bar
    gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_REPEAT)
    gcompris.bar_location(gcompris.BOARD_WIDTH/2 - 90, -1, 0.6)
    gcompris.bar_set_level(self.gcomprisBoard)

    self.display_game()

  def end(self):
    self.backroot.remove()
    self.backroot = None
    self.rootitem = None


  def ok(self):
    print("lightsoff ok.")


  def repeat(self):
    self.display_game()

  def config(self):
    print("lightsoff config.")


  def key_press(self, keyval, commit_str, preedit_str):
   pass

  def pause(self, pause):
    self.board_paused = pause

    # When the bonus is displayed, it call us first
    # with pause(1) and then with pause(0)
    # the game is won
    if(self.gamewon == True and pause == False):
      self.gamewon = False
      if(self.increment_level()):
        self.display_game()



  def set_level(self, level):
    self.gcomprisBoard.level = level
    gcompris.bar_set_level(self.gcomprisBoard)
    self.display_game()


  # Code that increments the sublevel and level
  # And bail out if no more levels are available
  # return 1 if continue, 0 if bail out
  def increment_level(self):
    self.gcomprisBoard.sublevel += 1

    if(self.gcomprisBoard.sublevel>self.gcomprisBoard.number_of_sublevel):
      # Try the next level
      self.gcomprisBoard.sublevel=1
      self.gcomprisBoard.level += 1
      gcompris.bar_set_level(self.gcomprisBoard)

      if(self.gcomprisBoard.level>self.gcomprisBoard.maxlevel):
        self.gcomprisBoard.level = self.gcomprisBoard.maxlevel

    return 1

  def create_empty_list(self):
    items = []
    for x in range(5):
      items.append(range(5))
    for y in range(len(items)):
      for x in range(len(items[0])):
        items[y][x] = 0
    return items

  # Display the board game
  def display_game(self):
      # The grid we display
      # It contains all the graphic items
      self.items = self.create_empty_list()

      # The grid of hints items
      self.hints = self.create_empty_list()

      # Do we display the hints
      self.hints_mode = False

      if self.rootitem:
        self.rootitem.remove()

      self.tipiitem.props.visibility = goocanvas.ITEM_INVISIBLE
      self.textitem.props.visibility = goocanvas.ITEM_VISIBLE
      self.tuxitem.props.visibility = goocanvas.ITEM_VISIBLE
      self.bubbleitem.props.visibility = goocanvas.ITEM_VISIBLE

      # Create our rootitem. We put each canvas item in it so at the end we
      # only have to kill it. The canvas deletes all the items it contains
      # automaticaly.
      self.rootitem =  \
          goocanvas.Group(parent = self.gcomprisBoard.canvas.get_root_item())

      svghandle = gcompris.utils.load_svg("lightsoff/onoff.svgz")
      iwidth = svghandle.props.width
      iheight = svghandle.props.height

      gap = 10
      x_start = (gcompris.BOARD_WIDTH - len(self.items) * (iwidth + gap) ) / 2
      y_start = (gcompris.BOARD_HEIGHT - len(self.items[0]) \
                   * (iheight + gap) ) / 2 - 40

      goocanvas.Rect(
        parent = self.rootitem,
        x = x_start - gap,
        y = y_start - gap,
        width = len(self.items) * (iwidth + gap) + gap,
        height = len(self.items[0]) * (iheight + gap) + gap,
        fill_color_rgba = 0x445533AAL,
        stroke_color_rgba = 0xC0C0C0AAL,
        radius_x = 10,
        radius_y = 10,
        line_width = 2
        )

      data = self.data[self.gcomprisBoard.level - 1]
      for y in range(len(self.items)):
        for x in range(len(self.items[0])):
          item = goocanvas.Rect(
            parent = self.rootitem,
            x = x_start + (iwidth + gap) * x - gap/2,
            y = y_start + (iheight + gap) * y - gap/2,
            width = iwidth + gap,
            height = iheight + gap,
            stroke_color_rgba = 0xC0C0C0FAL,
            fill_color_rgba = 0x5050509AL,
            line_width = 2,
            radius_x = 10,
            radius_y = 10,
            )
          self.hints[y][x] = item
          item.props.visibility = goocanvas.ITEM_INVISIBLE

          state = data[y][x]
          svg_id = "#off"
          if state:
            svg_id = "#on"
          item = goocanvas.Svg(
            parent = self.rootitem,
            svg_handle = svghandle,
            svg_id = svg_id,
            )
          item.set_data("state", state)
          item.translate(x_start + (iwidth + gap) * x,
                         y_start + (iheight + gap) * y)
          item.connect("button_press_event", self.button_press_event, [y,x])
          self.items[y][x] = item

      self.solve_it()

  def is_on(self, item):
    return item.get_data("state")

  def switch(self, item):
    if not item:
      return
    mystate =  self.is_on(item)
    if mystate == False:
      item.set_properties(svg_id = "#on")
    else:
      item.set_properties(svg_id = "#off")
    item.set_data("state", not mystate)

  def get_item_up(self, y, x):
    if y == 0:
      return None
    return self.items[y-1][x]

  def get_item_down(self, y, x):
    if y == len(self.items[0])-1:
      return None
    return self.items[y+1][x]

  def get_item_left(self, y, x):
    if x == 0:
      return None
    return self.items[y][x-1]

  def get_item_right(self, y, x):
    if x == len(self.items)-1:
      return None
    return self.items[y][x+1]

  # Returns True when complete
  def is_done(self):
    for y in range(len(self.items)):
      for x in range(len(self.items[0])):
        if self.is_on(self.items[y][x]):
          return False

    return True

  def button_press_event(self, widget, target, event, spot):
    self.switch(target)
    self.switch(self.get_item_up(spot[0], spot[1]))
    self.switch(self.get_item_left(spot[0], spot[1]))
    self.switch(self.get_item_right(spot[0], spot[1]))
    self.switch(self.get_item_down(spot[0], spot[1]))

    self.solve_it()

    if self.is_done():
      self.tipiitem.props.visibility = goocanvas.ITEM_VISIBLE
      self.textitem.props.visibility = goocanvas.ITEM_INVISIBLE
      self.tuxitem.props.visibility = goocanvas.ITEM_INVISIBLE
      self.bubbleitem.props.visibility = goocanvas.ITEM_INVISIBLE
      self.gamewon = True
      gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.FLOWER)

  def solution_switch(self, items, clicks, y, x):
    items[y][x] = not items[y][x]
    clicks[y][x] = not clicks[y][x]
    if y >= 1:
      items[y-1][x] = not items[y-1][x]
    if y <= 3:
      items[y+1][x] = not items[y+1][x]
    if x >= 1:
      items[y][x-1] = not items[y][x-1]
    if x <= 3:
      items[y][x+1] = not items[y][x+1]


  def chase_light(self, items, clicks):
    for y in range(1, len(items)):
      for x in range(len(items[0])):
        if items[y-1][x]:
          self.solution_switch(items, clicks, y, x)

  def is_solution_pattern(self, s, a, b, c, d, e):
    if s[4][0] == a and \
          s[4][1] == b and \
          s[4][2] == c and \
          s[4][3] == d and \
          s[4][4] == e:
      return True
    return False


  # Return False if the is no solution
  def solution_wrap(self, solution, clicks):
    if self.is_solution_pattern(solution, 1, 0, 0 , 0, 1):
      self.solution_switch(solution, clicks, 0, 0)
      self.solution_switch(solution, clicks, 0, 1)
    elif self.is_solution_pattern(solution, 0, 1, 0, 1, 0):
      self.solution_switch(solution, clicks, 0, 0)
      self.solution_switch(solution, clicks, 0, 3)
    elif self.is_solution_pattern(solution, 1, 1, 1, 0, 0):
      self.solution_switch(solution, clicks, 0, 1)
    elif self.is_solution_pattern(solution, 0, 0, 1, 1 , 1):
      self.solution_switch(solution, clicks, 0, 3)
    elif self.is_solution_pattern(solution, 1, 0, 1, 1, 0):
      self.solution_switch(solution, clicks, 0, 4)
    elif self.is_solution_pattern(solution, 0, 1, 1, 0, 1):
      self.solution_switch(solution, clicks, 0, 0)
    elif self.is_solution_pattern(solution, 1, 1, 0, 1, 1):
      self.solution_switch(solution, clicks, 0, 2)
    else:
      return False
    return True

  def items2list(self, items):
    list = []
    for y in range(len(items[0])):
      line = []
      for x in range(len(items)):
        if self.is_on(items[y][x]):
          line.append(1)
        else:
          line.append(0)
      list.append(line)
    return list

  # We check only the last line
  def solution_found(self, solution):
    for x in range(len(solution[0])):
      if solution[4][x]:
        return False
    return True

  def solution_length(self, clicks):
    click = 0
    for y in range(0, len(clicks)):
      for x in range(len(clicks[0])):
        if clicks[y][x]:
          click += 1
    return click

  def solve_one(self, solution, clicks):
    found = False
    for index in range(0, 5):

      self.chase_light( solution, clicks )

      if self.solution_found(solution):
        found = True
        break

      if not self.solution_wrap(solution, clicks):
        break

    if found:
      return clicks
    else:
      return None

  # Solving algorithm is the one described here:
  # http://www.haar.clara.co.uk/Lights/solving.html To begin, you turn
  # out all the lights on the top row, by pressing the buttons on the
  # second row that are directly underneath any lit buttons on the top
  # row. The top row will then have all it's lights off.  Repeat this
  # step for the second, third and fourth row. (i.e. chase the lights
  # all the way down to the bottom row). This may have solved the
  # puzzle already ( click here for an example of this ), but is more
  # likely that there will now be some lights left on in the bottom
  # row. If so, there are only 7 posible configurations. Depending on
  # which configuration you are left with, you need to press some
  # buttons in the top row. You can determine which buttons you need
  # to press from the following table.
  # Light on bottom row     Press on this on top row
  # 10001                   11000
  # 01010                   10010
  # 11100                   01000
  # 00111                   00010
  # 10110                   00001
  # 01101                   10000
  # 11011                   00100

  def solve_it(self):
    clicks = None

    # Our solving algorithm does not find the shortest solution. We
    # don't really care but we'd like to keep the proposed solution
    # stable (not propose a complete new solution when one light
    # changes).  To achieve this (closely), we test here all the
    # combination of the first line, trying to find the shortest
    # solution.
    for x in range(64):
      solution = self.items2list(self.items)
      clicks2 = self.create_empty_list()
      if x & 1:
        self.solution_switch(solution, clicks2, 0, 0)
      if x & 2:
        self.solution_switch(solution, clicks2, 0, 1)
      if x & 4:
        self.solution_switch(solution, clicks2, 0, 2)
      if x & 8:
        self.solution_switch(solution, clicks2, 0, 3)
      if x & 16:
        self.solution_switch(solution, clicks2, 0, 4)

      clicks2 = self.solve_one(solution, clicks2)
      if clicks == None and clicks2:
        clicks = clicks2
      elif clicks2 and \
            self.solution_length(clicks2) < self.solution_length(clicks):
        clicks = clicks2

    if self.hints_mode:
      self.show_hints(clicks)
    self.update_background(clicks)



  def solve_event(self, widget, target, event):
    clicks = self.create_empty_list()
    self.hints_mode = not self.hints_mode
    if not self.hints_mode:
      self.show_hints(clicks)
    else:
      self.solve_it()


  def update_background(self, clicks):
    length = self.solution_length(clicks)
    c = int(length * 0xFF / 18.0)
    color = 0X33 << 24 | 0x11 << 16 | c << 8 | 0xFFL
    self.background.set_properties(fill_color_rgba = color)

    self.sunitem.translate(0, self.sunitem_offset)
    self.sunitem_offset = length * 10
    self.sunitem.translate(0, self.sunitem_offset * -1)


  def show_hints(self, clicks):
    for y in range(len(clicks)):
      for x in range(len(clicks[0])):
        if clicks[y][x]:
          self.hints[y][x].props.visibility = goocanvas.ITEM_VISIBLE
        else:
          self.hints[y][x].props.visibility = goocanvas.ITEM_INVISIBLE

  def print_sol(self, clicks):
    for y in range(len(clicks)):
      s = ""
      for x in range(len(clicks[0])):
        if clicks[y][x]:
          s += "1"
        else:
          s += "0"
      print s
    print ""
