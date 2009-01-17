#  gcompris - BarGame
#
# Copyright (C) 2004, 2008  Yves Combe
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
import goocanvas
import gcompris
import gcompris.utils
import gcompris.bonus
import gcompris.skin
import pygtk
import gtk
import gtk.gdk
import random
import math

backgrounds = [ 'bargame/school_bg1.jpg',
                'bargame/school_bg2.jpg',
                'bargame/school_bg3.jpg',
                'bargame/school_bg4.jpg'
                ]

profs = [ 'bargame/tux.png',
          'bargame/tux_teen.png',
          'bargame/tux_graduate.png',
          'bargame/tux_albert.png'
          ]

class Gcompris_bargame:
  """The Bar Game"""

  def __init__(self, gcomprisBoard):
    random.seed()
    self.gcomprisBoard = gcomprisBoard
    self.rootitem = None

    # To display the bonus, we need to pause the board.
    self.board_paused  = 0
    self.number_balls = [[1,4],[2,6],[3,6]]
    self.board_size = [15,19,29]
    self.gcomprisBoard.level=1
    self.gcomprisBoard.maxlevel=4
    self.gcomprisBoard.sublevel=1
    self.gcomprisBoard.number_of_sublevel=3

    # global scale depending on the level
    self.scales = [0.208, 0.164, 0.107]

    self.holes = []
    self.balls = []
    self.list_win = []
    self.profbutton = []
    self.answer = []

    self.pixmap_blue_ball  = gcompris.utils.load_pixmap("bargame/blue_ball.png")
    self.pixmap_green_ball = gcompris.utils.load_pixmap("bargame/green_ball.png")
    self.pixmap_case       = gcompris.utils.load_pixmap("bargame/case.png")
    self.pixmap_case_last  = gcompris.utils.load_pixmap("bargame/case_last.png")
    self.pixmap_ombre      = gcompris.utils.load_pixmap("bargame/ombre.png")
    self.pixmap_mask       = gcompris.utils.load_pixmap("bargame/mask.png")
    self.pixmap_mask_last  = gcompris.utils.load_pixmap("bargame/mask_last.png")

  def start(self):
    # load pixmaps for the ui.
    #
    gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_REPEAT)
    gcompris.bar_set_level(self.gcomprisBoard)
    gcompris.bar_location(10, -1, 0.7)

    # Create persistent over levels root item canvas for the board
    self.rootitem_p = goocanvas.Group(
      parent = self.gcomprisBoard.canvas.get_root_item()
      )

    self.profbutton = self.prof_button(self,
                                       self.rootitem_p,
                                       profs[self.gcomprisBoard.level - 1])

    self.answer = self.answer_button(self.rootitem_p,
                                     self.scales[self.gcomprisBoard.sublevel-1],
                                     self.pixmap_green_ball,
                                     self.number_balls[self.gcomprisBoard.sublevel-1])

    #
    self.newGame()

  def end(self):
    self.cleanup()

    del self.profbutton
    self.profbutton = []

    del self.answer
    self.answer = []

    if self.rootitem_p:
      self.rootitem_p.remove()


  def set_level(self,level):
    self.gcomprisBoard.level = level
    self.gcomprisBoard.sublevel = 1
    gcompris.bar_set_level(self.gcomprisBoard)

    self.newGame()
    pass

  def ok(self):
    self.answer.has_focus()
    self.play(self.answer.value,True)

  def key_press(self, keyval, commit_str, preedit_str):
    return False

  def repeat(self):
    self.newGame()

  def pause(self, pause):
    self.board_paused = pause

    # When the bonus is displayed, it call us first with pause(1) and then with pause(0)
    # the game is won
    if ((pause == 0) and self.finished_sublevel):
      self.set_sublevel(self.gcomprisBoard.sublevel+1)
      return 0


  #-------------------------------------------------
  #-------------------------------------------------
  #-------------------------------------------------

  def set_sublevel(self, sublevel):
    #sublevel change only in game_won
    if sublevel > self.gcomprisBoard.number_of_sublevel:
      if self.game_won:
        if self.gcomprisBoard.level == self.gcomprisBoard.maxlevel:
          self.set_level(self.gcomprisBoard.level)
        else:
          self.set_level(self.gcomprisBoard.level+1)
      else:
        self.gcomprisBoard.sublevel = 1
        self.newGame()
    else:
      self.gcomprisBoard.sublevel = sublevel
      self.newGame()


  def calculate_win_places(self):
    winners = []

    min = self.number_balls[self.gcomprisBoard.sublevel-1][0]
    max = self.number_balls[self.gcomprisBoard.sublevel-1][1]
    period = (min + max)

    winners_list = [(self.board_size[self.gcomprisBoard.sublevel-1] -1 -x)% period for x in range(min)]
    for i in range(self.board_size[self.gcomprisBoard.sublevel-1]):
      if ((i+1) % period) in  winners_list:
        winners.append(i)

    level_win = (self.gcomprisBoard.level -1) * min

    if level_win == 0:
      winners = []
    else:
      winners = winners[-level_win:]

    return winners

  # Remove uneeded objects
  def cleanup(self):
    if self.rootitem:
      self.rootitem.remove()

    # Holes reinit
    for h in self.holes:
      del h

    del self.holes
    self.holes = []

    # Balls reinit
    for h in self.balls:
      del h

    del self.balls
    self.balls = []


    # Other variable cleanup
    del self.list_win
    self.list_win = []

  def newGame(self):
    self.cleanup()

    width_ref = 256

    scale = self.scales[self.gcomprisBoard.sublevel-1]

    self.last_played = -1
    self.finished_sublevel = False
    self.game_won = False
    self.list_win = self.calculate_win_places()


    # Create root item canvas for the board
    self.rootitem = goocanvas.Group(parent = self.gcomprisBoard.canvas.get_root_item())

    # background
    gcompris.set_background(self.gcomprisBoard.canvas.get_root_item(),
                            backgrounds[self.gcomprisBoard.level - 1])

    self.profbutton.set_prof(profs[self.gcomprisBoard.level - 1])

    for i in range(self.board_size[self.gcomprisBoard.sublevel-1]):
      self.holes.append(self.hole(self,
                                  self.rootitem,
                                  i * width_ref * scale,
                                  gcompris.BOARD_HEIGHT - 140, i,
                                  self.board_size[self.gcomprisBoard.sublevel-1],
                                  scale))

    for i in range(self.number_balls[self.gcomprisBoard.sublevel-1][1]):
      self.balls.append(self.ball(self.rootitem,
                                  i * width_ref * scale + 150,
                                  gcompris.BOARD_HEIGHT - 180,
                                  scale,
                                  self.pixmap_blue_ball))
      self.balls.append(self.ball(self.rootitem,
                                  i * width_ref * scale +150,
                                  gcompris.BOARD_HEIGHT-90,
                                  scale,
                                  self.pixmap_green_ball))

    self.answer.set_number_of_balls(self.number_balls[self.gcomprisBoard.sublevel-1])

    # The OK Button
    item = goocanvas.Svg(parent = self.rootitem,
                         svg_handle = gcompris.skin.svg_get(),
                         svg_id = "#OK"
                         )
    item.translate(item.get_bounds().x1 * -1
                   + gcompris.BOARD_WIDTH - (item.get_bounds().x2 - item.get_bounds().x1) - 10,
                   item.get_bounds().y1 * -1
                   + gcompris.BOARD_HEIGHT - 230)

    item.connect("button_press_event", self.ok_event)
    gcompris.utils.item_focus_init(item, None)


  def play(self, value, human):
    for i in range(1,value+1):
      self.last_played += 1
      if human:
        self.holes[self.last_played].isGreen()
      else:
        self.holes[self.last_played].isBlue()

      if self.last_played == self.board_size[self.gcomprisBoard.sublevel-1] - 1 :
        self.gamelost(human)
        return

    if human:
      self.machine_play()


  def machine_play(self):

    def accessible(x):
      if ((x + self.last_played) in self.list_win):
        return True
      return False

    playable = filter(accessible,
                      range(self.number_balls[self.gcomprisBoard.sublevel-1][0],
                            self.number_balls[self.gcomprisBoard.sublevel-1][1]+1))

    if playable != []:
      self.play(random.choice(playable),False)
    else:
      self.play(random.choice(range(self.number_balls[self.gcomprisBoard.sublevel-1][0],
                                    self.number_balls[self.gcomprisBoard.sublevel-1][1]+1)),
                False)

    self.answer.new_value(self.number_balls[self.gcomprisBoard.sublevel-1][0])

  def gamelost(self,human):
    self.finished_sublevel = True
    if human:
      gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.GNU)
    else:
      self.game_won = True
      gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.GNU)


  class hole:
    def __init__(self, board, root, x, y, index,
                 board_size,
                 scale):
      self.board = board

      self.itemgroup = goocanvas.Group(parent = root)
      self.itemgroup.translate(x, y)
      self.scale = scale

      if (index == (board_size-1)):
        pixbuf_case = self.board.pixmap_case_last
        pixbuf_mask = self.board.pixmap_mask_last
      else:
        pixbuf_case = self.board.pixmap_case
        pixbuf_mask = self.board.pixmap_mask


      item = goocanvas.Image(
        parent = self.itemgroup,
        pixbuf = pixbuf_case,
        x=0,
        y=0)
      bounds = item.get_bounds()
      item.scale(scale, scale)

      self.ombre = goocanvas.Image(
        parent = self.itemgroup,
        pixbuf = self.board.pixmap_ombre,
        x=0,
        y=0)
      self.ombre.props.visibility = goocanvas.ITEM_INVISIBLE
      bounds = self.ombre.get_bounds()
      self.ombre.scale(scale, scale)

      self.ball = goocanvas.Image(
        parent = self.itemgroup,
        x=0,
        y=0)
      self.ball.props.visibility = goocanvas.ITEM_INVISIBLE

      item = goocanvas.Image(
        parent = self.itemgroup,
        pixbuf = pixbuf_mask,
        x=0,
        y=0)
      bounds = item.get_bounds()
      item.scale(scale, scale)

      if ((index+1)%5 == 0):
        goocanvas.Text(
          parent = self.itemgroup,
          x = (bounds.x2-bounds.x1)/2,
          y = -10,
          fill_color_rgba = 0x000000ffL,
          font=gcompris.skin.get_font("gcompris/board/small bold"),
          anchor = gtk.ANCHOR_CENTER,
          text = index + 1)


    def isBlue(self):
      self.ball.props.pixbuf = self.board.pixmap_blue_ball
      bounds = self.ball.get_bounds()
      self.ball.scale(self.scale, self.scale)
      self.ball.props.visibility = goocanvas.ITEM_VISIBLE
      self.ombre.props.visibility = goocanvas.ITEM_VISIBLE

    def isGreen(self):
      self.ball.props.pixbuf = self.board.pixmap_green_ball
      bounds = self.ball.get_bounds()
      self.ball.scale(self.scale, self.scale)
      self.ball.props.visibility = goocanvas.ITEM_VISIBLE
      self.ombre.props.visibility = goocanvas.ITEM_VISIBLE


  class ball:
    def __init__(self, root, x, y, scale, image):

      self.ball = goocanvas.Image(
        parent = root,
        pixbuf = image)

      bounds = self.ball.get_bounds()
      self.ball.scale(scale, scale)
      (x, y) = root.get_canvas().convert_to_item_space(self.ball, x, y)
      self.ball.translate(x, y)

  class answer_button:
    def __init__(self, root, scale, image, number_balls):
      self.focus = False

      self.number_balls = number_balls

      self.itemgroup = goocanvas.Group(parent = root)

      self.background = goocanvas.Image(
        parent = self.itemgroup,
        pixbuf = gcompris.utils.load_pixmap("bargame/enumerate_answer.png"),
        )
      answer_bounds = self.background.get_bounds()
      gcompris.utils.item_focus_init(self.background, None)

      self.itemgroup.translate(gcompris.BOARD_WIDTH - 180,
                               gcompris.BOARD_HEIGHT - answer_bounds.y2 \
                                 - answer_bounds.y1 - 10)

      self.background_focused = goocanvas.Image(
        parent = self.itemgroup,
        pixbuf = gcompris.utils.load_pixmap("bargame/enumerate_answer_focus.png"),
        )
      gcompris.utils.item_focus_init(self.background_focused, self.background)
      self.background_focused.props.visibility = goocanvas.ITEM_INVISIBLE

      self.icone = goocanvas.Image(
        parent = self.itemgroup,
        pixbuf = image)
      bounds = self.icone.get_bounds()
      self.icone.scale(scale, scale)
      (x, y) = root.get_canvas().convert_from_item_space(self.itemgroup, 10, 20)
      (x, y) = root.get_canvas().convert_to_item_space(self.icone, x, y)
      self.icone.translate(x, y)
      gcompris.utils.item_focus_init(self.icone, self.background)

      self.value = number_balls[0]

      self.text = goocanvas.Text(
        parent = self.itemgroup,
        x = answer_bounds.x2-answer_bounds.x1 - 50,
        y = 40,
        fill_color_rgba=0xff0000ffL,
        font=gcompris.skin.get_font("gcompris/board/huge bold"),
        anchor=gtk.ANCHOR_CENTER,
        text = self.value
        )
      gcompris.utils.item_focus_init(self.text, self.background)

      self.background.connect("button_press_event",self.answer_event)
      self.background_focused.connect("button_press_event",self.answer_event)
      self.icone.connect("button_press_event",self.answer_event)
      self.text.connect("button_press_event",self.answer_event)


    def new_value(self, value):
      self.value = value
      self.text.set_property('text',value)

    def has_focus(self):
      self.background_focused.props.visibility = goocanvas.ITEM_VISIBLE
      self.focus = True

    def set_number_of_balls(self, number_balls):
      self.number_balls = number_balls
      self.new_value(1)

    def answer_event(self, item, target, event):
      if ((event.type != gtk.gdk.BUTTON_PRESS) or
          (event.button != 1)):
        return False

      self.has_focus()

      value = ( (self.value
                 + 1
                 - self.number_balls[0])
                % (self.number_balls[1]
                   + 1
                   - self.number_balls[0])
                + self.number_balls[0]
                )
      self.new_value(value)

      return True

  class prof_button:
    def __init__(self, board, root, prof_image):
      self.board = board

      self.prof_item = goocanvas.Image(
        parent = root,
        y = 210
        )


    def set_prof(self, prof_image):
      self.prof_item.props.pixbuf = gcompris.utils.load_pixmap(prof_image)
      bounds = self.prof_item.get_bounds()
      self.prof_item.props.x = \
          (gcompris.BOARD_WIDTH - bounds.x2-bounds.x1)/2 - 90
      self.prof_item.connect("button_press_event",self.event_play)
      # This item is clickeable and it must be seen
      gcompris.utils.item_focus_init(self.prof_item, None)

    def event_play(self, item, target, event):
      if ((event.type != gtk.gdk.BUTTON_PRESS) or
          (event.button != 1)):
        return False

      # if answer button has been clicked, that play the human turn
      if self.board.last_played == -1 :
        self.board.machine_play()
        return True

      return False

  def ok_event(self, widget, target, event=None):
    self.ok()
