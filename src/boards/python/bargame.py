#  gcompris - BarGame
# 
# 
# 
# Copyright (C) 2004  Yves Combe
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
import gnome
import gnome.canvas
import gcompris
import gcompris.utils
import gcompris.bonus
import gcompris.skin
import pygtk
import gtk
import gtk.gdk
import random
import math

backgrounds = [ 'images/school_bg1.jpg',
                'images/school_bg2.jpg',
                'images/school_bg3.jpg',
                'images/school_bg4.jpg'
                ]

profs = [ 'images/tux.png',
          'images/tux_teen.png',
          'images/tux_graduate.png',
          'images/tux_albert.png'
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
    self.rootitem = None
    
  def start(self):
    # load pixmaps for the ui.
    self.pixmap_blue_ball = gcompris.utils.load_pixmap("bargame/blue_ball.png")
    self.pixmap_green_ball = gcompris.utils.load_pixmap("bargame/green_ball.png")
    self.pixmap_case = gcompris.utils.load_pixmap("bargame/case.png")
    self.pixmap_case_last = gcompris.utils.load_pixmap("bargame/case_last.png")
    self.pixmap_ombre = gcompris.utils.load_pixmap("bargame/ombre.png")
    self.pixmap_mask = gcompris.utils.load_pixmap("bargame/mask.png")
    self.pixmap_mask_last = gcompris.utils.load_pixmap("bargame/mask_last.png")

    self.pixmap_answer = gcompris.utils.load_pixmap("images/enumerate_answer.png")
    self.pixmap_answer_focus = gcompris.utils.load_pixmap("images/enumerate_answer_focus.png")
    self.pixmap_background = gcompris.utils.load_pixmap(backgrounds[self.gcomprisBoard.level - 1])
    
    self.ANSWER_X = gcompris.BOARD_WIDTH - 200
    self.ANSWER_Y = gcompris.BOARD_HEIGHT - self.pixmap_answer.get_height() - 5
    self.ANSWER_WIDTH =  self.pixmap_answer.get_width()
    self.ANSWER_HEIGHT = self.pixmap_answer.get_height()

    self.pixmap_prof = gcompris.utils.load_pixmap(profs[self.gcomprisBoard.level - 1])

    #
    pixmap = gcompris.utils.load_pixmap(gcompris.skin.image_to_skin("button_reload.png"))
    if(pixmap):
      gcompris.bar_set_repeat_icon(pixmap)
      gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_OK|gcompris.BAR_REPEAT_ICON)
    else:
      gcompris.bar_set(gcompris.BAR_LEVEL|gcompris.BAR_OK|gcompris.BAR_REPEAT)
    gcompris.bar_set_level(self.gcomprisBoard)

    #
    self.newGame()

  def end(self):
    self.rootitem.destroy()
    pass

  def set_level(self,level):
    print 'set_level', level
    self.gcomprisBoard.level = level
    self.gcomprisBoard.sublevel = 1
    gcompris.bar_set_level(self.gcomprisBoard)
    self.pixmap_background = gcompris.utils.load_pixmap(backgrounds[self.gcomprisBoard.level - 1])
    self.pixmap_prof       = gcompris.utils.load_pixmap(profs[self.gcomprisBoard.level - 1])
    self.newGame()
    pass
        
  def ok(self):
    self.answer.has_focus()
    self.play(self.answer.value,True)
    pass

  def key_press(self, keyval, commit_str, preedit_str):
    #print("got key %i" % keyval)
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
    print 'set_sublevel', sublevel

    #sublevel change only in game_won
    if sublevel > self.gcomprisBoard.number_of_sublevel:
      if self.game_won:
        if self.gcomprisBoard.level == self.gcomprisBoard.maxlevel:
          gcompris.bonus.board_finished(gcompris.bonus.FINISHED_RANDOM)
          return 0
        else:
          self.set_level(self.gcomprisBoard.level+1)
      else:
        self.gcomprisBoard.sublevel = 1
        self.newGame()
    else:
      self.gcomprisBoard.sublevel = sublevel 
      self.newGame()
    

  def scale_pixbuf(self,pixbuf, scale):
    return pixbuf.scale_simple(pixbuf.get_width()*scale,
                               pixbuf.get_height()*scale,
                               gtk.gdk.INTERP_HYPER)

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

    print 'winners', winners

    return winners

  def newGame(self):
    if self.rootitem:
      self.rootitem.destroy()

    self.scale = gcompris.BOARD_WIDTH/(float(self.board_size[self.gcomprisBoard.sublevel-1])*self.pixmap_case.get_width())
    print "scale :", self.scale

    self.px_case = self.scale_pixbuf(self.pixmap_case,self.scale)
    self.px_mask = self.scale_pixbuf(self.pixmap_mask,self.scale)
    self.px_case_last = self.scale_pixbuf(self.pixmap_case_last,self.scale)
    self.px_mask_last = self.scale_pixbuf(self.pixmap_mask_last,self.scale)
    self.px_ombre = self.scale_pixbuf(self.pixmap_ombre,self.scale)
    self.px_green_ball = self.scale_pixbuf(self.pixmap_green_ball,self.scale)
    self.px_blue_ball = self.scale_pixbuf(self.pixmap_blue_ball,self.scale)

    self.holes = []
    self.last_played = -1
    self.finished_sublevel = False
    self.game_won = False
    self.list_win = self.calculate_win_places()


    # Create root item canvas for the board
    self.rootitem = self.gcomprisBoard.canvas.root().add(
      gnome.canvas.CanvasGroup,
      x=0,
      y=0
      )

    # background
    self.background = self.rootitem.add(
      gnome.canvas.CanvasPixbuf,
      pixbuf = self.pixmap_background,
      x=0,
      y=0
      )

    self.prof = self.prof_button(self,
                                 self.rootitem,
                                 (gcompris.BOARD_WIDTH - self.pixmap_prof.get_width())/2 - 90 ,
                                 230
                                 )

    
    for i in range(self.board_size[self.gcomprisBoard.sublevel-1]):
      self.holes.append(self.hole(self,
                                  self.rootitem,
                                  i*self.px_case.get_width(),
                                  gcompris.BOARD_HEIGHT - 120,i))

    for i in range(self.number_balls[self.gcomprisBoard.sublevel-1][1]):
      self.ball(self.rootitem,
                i*self.px_case.get_width()+ 150,
                gcompris.BOARD_HEIGHT - 160,
                self.px_blue_ball)
      self.ball(self.rootitem,
                i*self.px_case.get_width()+150,
                gcompris.BOARD_HEIGHT-70,
                self.px_green_ball)

    self.answer = self.answer_button(self,
                                     self.rootitem,
                                     self.ANSWER_X,
                                     self.ANSWER_Y,
                                     self.px_green_ball)

  def play(self, value, human):
    print 'play:', value
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
    print 'machine_play'

    def accessible(x):
      if ((x + self.last_played) in self.list_win):
        return True
      return False

    playable = filter(accessible, range(self.number_balls[self.gcomprisBoard.sublevel-1][0], self.number_balls[self.gcomprisBoard.sublevel-1][1]+1))

    print 'playable', playable

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
      print 'Lost !'
      gcompris.bonus.display(gcompris.bonus.LOOSE, gcompris.bonus.GNU)
    else:
      print 'Won !'
      self.game_won = True
      gcompris.bonus.display(gcompris.bonus.WIN, gcompris.bonus.GNU)

                     
  class hole:
    def __init__(self, board, root, x, y, index):
      self.board = board
      self.itemgroup = root.add(
        gnome.canvas.CanvasGroup,
        x=x,
        y=y
        )

      if (index == (self.board.board_size[self.board.gcomprisBoard.sublevel-1]-1)):
        pixbuf_case = self.board.px_case_last
        pixbuf_mask = self.board.px_mask_last
      else:
        pixbuf_case = self.board.px_case
        pixbuf_mask = self.board.px_mask


      self.base = self.itemgroup.add(
        gnome.canvas.CanvasPixbuf,
        pixbuf = pixbuf_case,
        x=0,
        y=0
        )

      self.ombre = self.itemgroup.add(
        gnome.canvas.CanvasPixbuf,
        pixbuf = self.board.px_ombre,
        x=0,
        y=0
        )
      self.ombre.hide()

      self.blue = self.itemgroup.add(
        gnome.canvas.CanvasPixbuf,
        pixbuf = self.board.px_blue_ball,
        x=0,
        y=0
        )
      self.blue.hide()

      self.green = self.itemgroup.add(
        gnome.canvas.CanvasPixbuf,
        pixbuf = self.board.px_green_ball,
        x=0,
        y=0
        )
      self.green.hide()
      
      self.mask = self.itemgroup.add(
        gnome.canvas.CanvasPixbuf,
        pixbuf = pixbuf_mask,
        x=0,
        y=0,
        )

      if ((index+1)%5 == 0):
        self.text= self.itemgroup.add(
          gnome.canvas.CanvasText,
          x=self.board.px_case.get_width()/2,
          y=-10,
          fill_color_rgba=0x000000ffL,
          font=gcompris.skin.get_font("gcompris/board/small bold"),
          anchor=gtk.ANCHOR_CENTER,
          text = index + 1
          )


    def isBlue(self):
      self.blue.show()
      self.ombre.show()

    def isGreen(self):
      self.green.show()
      self.ombre.show()
      

  class ball:
    def __init__(self, root, x, y, pixbuf):
      
      self.ball = root.add(
        gnome.canvas.CanvasPixbuf,
        pixbuf = pixbuf,
        x=x,
        y=y
        )

  class answer_button:
    def __init__(self, board, root, x, y, pixbuf):
      self.board = board
      self.focus = False
      
      self.itemgroup = root.add(
        gnome.canvas.CanvasGroup,
        x=x,
        y=y
        )
      
      self.background = self.itemgroup.add(
        gnome.canvas.CanvasPixbuf,
        pixbuf = self.board.pixmap_answer,
        x=0,
        y=0
        )

      self.background_focused = self.itemgroup.add(
        gnome.canvas.CanvasPixbuf,
        pixbuf = self.board.pixmap_answer_focus,
        x=0,
        y=0
        )
      self.background_focused.hide() 
    
      self.icone = self.itemgroup.add(
        gnome.canvas.CanvasPixbuf,
        pixbuf = pixbuf,
        x=10,
        y=20
        )

      self.value = self.board.number_balls[self.board.gcomprisBoard.sublevel-1][0]
      
      self.text = self.itemgroup.add(
        gnome.canvas.CanvasText,
        x=self.board.ANSWER_WIDTH - 50,
        y=40,
        fill_color_rgba=0xff0000ffL,
        font=gcompris.skin.get_font("gcompris/board/huge bold"),
        anchor=gtk.ANCHOR_CENTER,
        text = self.value
        )

      self.background.connect("event",self.answer_event)
      self.background_focused.connect("event",self.answer_event) 
      self.icone.connect("event",self.answer_event)
      self.text.connect("event",self.answer_event)
      

    def new_value(self, value):
      self.value = value
      self.text.set_property('text',value)

    def has_focus(self):
      self.background_focused.show()
      self.focus = True


    def answer_event(self, item, event):
      if ((event.type != gtk.gdk.BUTTON_PRESS) or
          (event.button != 1)):
        return False

      self.has_focus()

      value = ( (self.value
                 + 1
                 - self.board.number_balls[self.board.gcomprisBoard.sublevel-1][0])
                % (self.board.number_balls[self.board.gcomprisBoard.sublevel-1][1]
                   + 1
                   - self.board.number_balls[self.board.gcomprisBoard.sublevel-1][0])
                + self.board.number_balls[self.board.gcomprisBoard.sublevel-1][0]
                )
      self.new_value(value)

      return True

  class prof_button:
    def __init__(self, board, root, x, y):
      self.board = board
      
      self.prof_image = root.add(
        gnome.canvas.CanvasPixbuf,
        pixbuf = self.board.pixmap_prof,
        x=x,
        y=y
        )
      
      self.prof_image.connect("event",self.event_play)
      # This item is clickeable and it must be seen
      self.prof_image.connect("event", gcompris.utils.item_event_focus)
          
    def event_play(self, item, event):
      if ((event.type != gtk.gdk.BUTTON_PRESS) or
          (event.button != 1)):
        return False

      # if answer button has been clicked, that play the human turn
      if self.board.last_played == -1 :
        self.board.machine_play()
        return True
      
      return False
      
